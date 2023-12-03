/*
 *  Copyright (C) W. Michael Knudson
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as 
 *  published by the Free Software Foundation.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along with this program; 
 *  if not, see <https://www.gnu.org/licenses/>.
 */
#include <awn.hpp>

namespace awn::mem {

    namespace {
        constinit vp::util::TypeStorage<HeapManager>   sHeapManagerStorage       = {};
        constinit sys::ServiceCriticalSection          sHeapManagerCS            = {};
        constinit bool                                 sIsHeapManagerInitialized = false;
        constexpr const char *                         cRootHeapNameArray[]      = {
            "RootHeap0",
            "RootHeap1",
            "RootHeap2",
        };
        static_assert(sizeof(cRootHeapNameArray) / sizeof(char*) == HeapManager::cMaxRootHeaps);
    }

    bool InitializeHeapManager(HeapManagerInfo *heap_manager_info) {

        /* Integrity checks */
        if (heap_manager_info == nullptr 
         || heap_manager_info->root_heap_count <= 0
         || heap_manager_info->root_heap_count >  5
         || heap_manager_info->root_heap_info_array == nullptr
         || sIsHeapManagerInitialized == true) { return false; }
    
        /* Construct the heap manager */
        vp::util::ConstructAt(sHeapManagerStorage);
        HeapManager *heap_mgr = vp::util::GetPointer(sHeapManagerStorage);

        /* Initialize RootHeaps */
        for (u32 i = 0; i < heap_manager_info->root_heap_count; ++i) {
            heap_mgr->root_heap_array[i] = ExpHeap::TryCreate(cRootHeapNameArray[i], heap_manager_info->root_heap_info_array[i].arena, heap_manager_info->root_heap_info_array[i].arena_size, false);
        }

        /* Query our allocation granularity */
        //size_t allocation_granularity = 0x1000;
    
        /* Create our program arena */
        //heap_mgr->memory = ::VirtualAlloc(nullptr, vp::util::AlignUp(size, allocation_granularity), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
        //VP_ASSERT(heap_mgr->memory != nullptr);
    
        //heap_mgr->memory_size = vp::util::AlignUp(size, allocation_granularity);
    
        /* Create our root heap spanning the arena */
        //sRootHeap = ExpHeap::TryCreate(heap_mgr->memory, heap_mgr->memory_size, "HeapManager::sRootHeap", false);

        /* Set state */
        heap_mgr->out_of_memory_resize_alignment    = heap_manager_info->out_of_memory_resize_alignment;
        heap_mgr->out_of_memory_delegate.m_function = heap_manager_info->out_of_memory_callback;
        sIsHeapManagerInitialized                   = true;

        return true;
    }
    
    void FinalizeHeapManager() {

        /* Destroy root heap */
        HeapManager *heap_mgr = vp::util::GetPointer(sHeapManagerStorage);
        for (u32 i = 0; i < HeapManager::cMaxRootHeaps; ++i) {
            if (heap_mgr->root_heap_array[i] != nullptr) {
                heap_mgr->root_heap_array[i]->Finalize();
                heap_mgr->root_heap_array[i] = nullptr;
            }
        }

        /* Destruct heap manager */
        vp::util::DestructAt(sHeapManagerStorage);
    }

    bool OutOfMemoryImpl(OutOfMemoryInfo *out_of_memory_info) {
        return vp::util::GetReference(sHeapManagerStorage).out_of_memory_delegate.Invoke(out_of_memory_info);
    }

    size_t GetOutOfMemoryResizeAlignment() {
        return vp::util::GetReference(sHeapManagerStorage).out_of_memory_resize_alignment;
    }

    Heap *FindHeapFromAddress(void *address) {

        /* Try to acquire thread */
        sys::ThreadManager *thread_manager = sys::ThreadManager::GetInstance();

        sys::ThreadBase *thread = nullptr;
        if (thread_manager != nullptr) { thread = thread_manager->GetCurrentThread(); }

        /* Try to lookup address in the thread heaps */
        Heap *thread_heap      = nullptr;
        Heap *last_lookup_heap = nullptr;
        if (thread != nullptr) {
            thread_heap      = thread->GetThreadHeap();
            last_lookup_heap = thread->GetLookupHeap();

            /* Check lookup heap to see if we have no children and contain the address*/
            if (last_lookup_heap != nullptr && last_lookup_heap->HasChildren() == false && last_lookup_heap->IsAddressInHeap(address) == true) {
                return last_lookup_heap;
            }

            /* Check thread's current heap to see if we have no children and contain the address */
            if (thread_heap != nullptr && thread_heap->HasChildren() == false && thread_heap->IsAddressInHeap(address) == true) {
                thread->SetLookupHeap(thread_heap);
                return thread_heap;
            }
        }

        std::scoped_lock l(sHeapManagerCS);

        /* Lookup all children in thread's lookup heap */
        if (last_lookup_heap != nullptr && last_lookup_heap->HasChildren() == true) {
            vp::imem::IHeap *contained_heap = last_lookup_heap->FindHeapFromAddress(address);
            if (contained_heap != nullptr && Heap::CheckRuntimeTypeInfo(contained_heap) == true) {
                mem::Heap *out_heap = reinterpret_cast<Heap*>(contained_heap);
                thread->SetLookupHeap(out_heap);
                return out_heap;
            }
        }

        /* Lookup all children in thread's current heap */
        if (thread_heap != nullptr && thread_heap->HasChildren() == true) {
            vp::imem::IHeap *contained_heap = thread_heap->FindHeapFromAddress(address);
            if (contained_heap != nullptr && Heap::CheckRuntimeTypeInfo(contained_heap) == true) {
                mem::Heap *out_heap = reinterpret_cast<Heap*>(contained_heap);
                thread->SetLookupHeap(out_heap);
                return out_heap;
            }
        }

        /* If thread heaps fail fallback to the root heaps */
        HeapManager *heap_mgr = vp::util::GetPointer(sHeapManagerStorage);
        for (u32 i = 0; i < HeapManager::cMaxRootHeaps; ++i) {
            if (heap_mgr->root_heap_array[i] == nullptr) { continue; }

            /* Lookup all children in root heap */
            vp::imem::IHeap *contained_heap = heap_mgr->root_heap_array[i]->FindHeapFromAddress(address);
            if (contained_heap != nullptr && Heap::CheckRuntimeTypeInfo(contained_heap) == true) {
                mem::Heap *out_heap = reinterpret_cast<Heap*>(contained_heap);
                thread->SetLookupHeap(out_heap);
                return out_heap;
            }
        }

        /* If root heaps fail fallback to the gpu root heaps */
        GpuHeapManager *gpu_heap_mgr = GpuHeapManager::GetInstance();
        for (u32 i = 0; i < cMaxGpuRootHeapCount; ++i) {
            if (gpu_heap_mgr->m_host_uncached_heap_context_array[i].root_heap != nullptr) {

                /* Lookup all children in root heap */
                vp::imem::IHeap *contained_heap = gpu_heap_mgr->m_host_uncached_heap_context_array[i].root_heap->FindHeapFromAddress(address);
                if (contained_heap != nullptr && Heap::CheckRuntimeTypeInfo(contained_heap) == true) {
                    mem::Heap *out_heap = reinterpret_cast<Heap*>(contained_heap);
                    thread->SetLookupHeap(out_heap);
                    return out_heap;
                }
            }
            if (gpu_heap_mgr->m_host_cached_heap_context_array[i].root_heap != nullptr) {
                
                /* Lookup all children in root heap */
                vp::imem::IHeap *contained_heap = gpu_heap_mgr->m_host_cached_heap_context_array[i].root_heap->FindHeapFromAddress(address);
                if (contained_heap != nullptr && Heap::CheckRuntimeTypeInfo(contained_heap) == true) {
                    mem::Heap *out_heap = reinterpret_cast<Heap*>(contained_heap);
                    thread->SetLookupHeap(out_heap);
                    return out_heap;
                }
            }
            if (gpu_heap_mgr->m_gpu_host_uncached_heap_context_array[i].root_heap != nullptr) {
                
                /* Lookup all children in root heap */
                vp::imem::IHeap *contained_heap = gpu_heap_mgr->m_gpu_host_uncached_heap_context_array[i].root_heap->FindHeapFromAddress(address);
                if (contained_heap != nullptr && Heap::CheckRuntimeTypeInfo(contained_heap) == true) {
                    mem::Heap *out_heap = reinterpret_cast<Heap*>(contained_heap);
                    thread->SetLookupHeap(out_heap);
                    return out_heap;
                }
            }
        }

        return nullptr;
    }

    Heap *FindHeapByNameImpl(Heap *parent_heap, const char *heap_name) {

        /* Sift children recursively */
        for (vp::imem::IHeap &heap : parent_heap->m_child_list) {

            /* Return heap on success */
            int result = ::strcmp(heap.GetName(), heap_name);
            if (result == 0 && Heap::CheckRuntimeTypeInfo(std::addressof(heap)) == true) { return reinterpret_cast<mem::Heap*>(std::addressof(heap)); }

            /* Recurse through childs children on failure */
            if (Heap::CheckRuntimeTypeInfo(std::addressof(heap)) == true && reinterpret_cast<Heap&>(heap).m_child_list.IsEmpty() == false) {
                Heap *candidate = FindHeapByNameImpl(reinterpret_cast<mem::Heap*>(std::addressof(heap)), heap_name);
                if (candidate != nullptr) { return candidate; }
            }
        }

        return nullptr;
    }

    Heap *FindHeapByName(const char *heap_name) {

        /* Search every root heap by name */
        HeapManager *heap_mgr = vp::util::GetPointer(sHeapManagerStorage);
        for (u32 i = 0; i < HeapManager::cMaxRootHeaps; ++i) {
            if (heap_mgr->root_heap_array[i] == nullptr) { return nullptr; }

            /* Lookup all children in root heap */
            mem::Heap *heap_by_name = FindHeapByNameImpl(heap_mgr->root_heap_array[i], heap_name);
            if (heap_by_name != nullptr) {
                return heap_by_name;
            } else if (::strcmp(heap_name, heap_mgr->root_heap_array[i]->GetName()) == 0) {
                return heap_mgr->root_heap_array[i];
            }
        }

        /* Search every gpu root heap by name */
        GpuHeapManager *gpu_heap_mgr = GpuHeapManager::GetInstance();
        for (u32 i = 0; i < cMaxGpuRootHeapCount; ++i) {
            if (gpu_heap_mgr->m_host_uncached_heap_context_array[i].root_heap != nullptr) {

                /* Lookup all children in root heap */
                mem::Heap *heap_by_name = FindHeapByNameImpl(gpu_heap_mgr->m_host_uncached_heap_context_array[i].root_heap, heap_name);
                if (heap_by_name != nullptr) {
                    return heap_by_name;
                } else if (::strcmp(heap_name, gpu_heap_mgr->m_host_uncached_heap_context_array[i].root_heap->GetName()) == 0) {
                    return gpu_heap_mgr->m_host_uncached_heap_context_array[i].root_heap;
                }
            }
            if (gpu_heap_mgr->m_host_cached_heap_context_array[i].root_heap != nullptr) {
                
                /* Lookup all children in root heap */
                mem::Heap *heap_by_name = FindHeapByNameImpl(gpu_heap_mgr->m_host_cached_heap_context_array[i].root_heap, heap_name);
                if (heap_by_name != nullptr) {
                    return heap_by_name;
                } else if (::strcmp(heap_name, gpu_heap_mgr->m_host_cached_heap_context_array[i].root_heap->GetName()) == 0) {
                    return gpu_heap_mgr->m_host_cached_heap_context_array[i].root_heap;
                }
            }
            if (gpu_heap_mgr->m_gpu_host_uncached_heap_context_array[i].root_heap != nullptr) {
                
                /* Lookup all children in root heap */
                mem::Heap *heap_by_name = FindHeapByNameImpl(gpu_heap_mgr->m_gpu_host_uncached_heap_context_array[i].root_heap, heap_name);
                if (heap_by_name != nullptr) {
                    return heap_by_name;
                } else if (::strcmp(heap_name, gpu_heap_mgr->m_gpu_host_uncached_heap_context_array[i].root_heap->GetName()) == 0) {
                    return gpu_heap_mgr->m_gpu_host_uncached_heap_context_array[i].root_heap;
                }
            }
        }

        return nullptr;
    }

    bool IsHeapManagerInitialized() { return sIsHeapManagerInitialized; }

    ALWAYS_INLINE HeapManager *GetHeapManager() { return vp::util::GetPointer(sHeapManagerStorage); }

    mem::Heap *GetRootHeap(u32 index) { return vp::util::GetPointer(sHeapManagerStorage)->root_heap_array[index]; }

    size_t GetRootHeapTotalSize(u32 index) {
        return GetRootHeap(index)->GetTotalSize() + sizeof(mem::ExpHeap);
    }

    sys::ServiceCriticalSection *GetHeapManagerLock() { return std::addressof(sHeapManagerCS); }

    Heap *GetCurrentThreadHeap() {
        sys::ThreadBase *thread = sys::ThreadManager::GetInstance()->GetCurrentThread();
        if (thread != nullptr) {
            return thread->GetThreadHeap();
        }
        return vp::util::GetPointer(sHeapManagerStorage)->root_heap_array[0];
    }

    void SetCurrentThreadHeap(Heap *heap) {

        /* Try to set current thread heap */
        sys::ThreadBase *thread = sys::ThreadManager::GetInstance()->GetCurrentThread();
        if (thread != nullptr) {
            thread->SetThreadCurrentHeap(heap);
        }

        return;
    }

    bool IsAddressFromAnyHeap(void *address) {

        /* Search every heap by name */
        HeapManager *heap_mgr = vp::util::GetPointer(sHeapManagerStorage);
        for (u32 i = 0; i < HeapManager::cMaxRootHeaps; ++i) {
            if (heap_mgr->root_heap_array[i] == nullptr) { return false; }

            /* Lookup all children in root heap */
            bool is_address_from_heap = heap_mgr->root_heap_array[i]->IsAddressInHeap(address);
            if (is_address_from_heap == true) {
                return is_address_from_heap;
            }
        }

        return false;
    }
}
