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
#include <ares.h>

namespace awn::res {

	mem::Heap *ResourceMemoryManager::CreateHeapImpl(const char *heap_name, size_t size, ResourceHeapType heap_type) {

        mem::Heap *heap = nullptr;
        if (heap_type == ResourceHeapType::FrameHeap)    { heap = mem::FrameHeap::TryCreate(heap_name, m_resource_heap, size, alignof(size_t), false); }
        else if (heap_type == ResourceHeapType::ExpHeap) { heap = mem::ExpHeap::TryCreate(heap_name, m_resource_heap, size, alignof(size_t), false); }

        return heap;
    }

    void ResourceMemoryManager::FreeHeap(mem::Heap *heap, [[maybe_unused]] mem::Heap *gpu_heap, ResourceUnit *resource_unit) {

        /* Lock mgr */
        std::scoped_lock l(m_memory_mgr_mutex);

        /* Unlink Resource Unit */
        if (resource_unit != nullptr) {
            resource_unit->m_memory_manager_node.Unlink();
            resource_unit->m_memory_manager_free_cache_node.Unlink();
        }

        /* Destroy heaps and update allocatable memory size */
        vp::imem::IHeap *parent = heap->GetParentHeap();
        std::destroy_at(heap);
        m_max_allocatable_size = parent->GetMaximumAllocatableSize(alignof(size_t));

        return;
    }

    bool ResourceMemoryManager::FreeFromCache(size_t target_size) {

        std::scoped_lock l(m_memory_mgr_mutex);
        
        while (m_resource_unit_free_cache_list.IsEmpty() == true) {
            ResourceUnit *resource_unit = std::addressof(m_resource_unit_free_cache_list.PopBack());
            if (resource_unit->m_is_freeable_for_memory_manager == false) { continue; }
            
            resource_unit->UnregisterFromResourceUnitManager();
            AsyncResourceManager::GetInstance()->RemoveResourceUnitFromFinalizeList(resource_unit);
            
            m_max_allocatable_size = 0;
            AsyncResourceManager::GetInstance()->FinalizeResourceUnitSync(resource_unit);
            if (m_max_allocatable_size < target_size) { return true; }
        }

        return false;
    }

    void ResourceMemoryManager::ClearCacheForAllocate(u32 count) {

        std::scoped_lock l(m_memory_mgr_mutex);
        
        while (0 < count && m_resource_unit_free_cache_list.IsEmpty() == true) {
            --count;
            ResourceUnit *resource_unit = std::addressof(m_resource_unit_free_cache_list.PopBack());
            if (resource_unit->m_is_freeable_for_memory_manager == false) { continue; }
            
            resource_unit->UnregisterFromResourceUnitManager();
            AsyncResourceManager::GetInstance()->RemoveResourceUnitFromFinalizeList(resource_unit);
            
            m_max_allocatable_size = 0;
            AsyncResourceManager::GetInstance()->FinalizeResourceUnitSync(resource_unit);
        }

        return;
    }

    void ResourceMemoryManager::Initialize(mem::Heap *heap, ResourceMemoryManagerInfo *manager_info) {

        /* Create virtual address heap */
        if (manager_info->manager_heap_type == ManagerHeapType::VirtualAddressHeap) {                    
            m_resource_heap = mem::VirtualAddressHeap::Create(manager_info->name);
        } else if (manager_info->manager_heap_type == ManagerHeapType::ExpHeap) {
            m_resource_heap = mem::ExpHeap::TryCreate(manager_info->name, heap, manager_info->heap_size, manager_info->heap_alignment, true);
        }

        m_memory_mgr_mutex.Initialize();

        return;
    }

    void ResourceMemoryManager::Finalize() {

        /* Destroy resource heap */
        if (m_resource_heap != nullptr) {
            if (mem::VirtualAddressHeap::CheckRuntimeTypeInfoStatic(m_resource_heap->GetRuntimeTypeInfo()) == true) {
                m_resource_heap->Finalize();
            } else {
                delete m_resource_heap;
            }
            m_resource_heap = nullptr;
        }

        m_memory_mgr_mutex.Finalize();

        return;
    }

    void ResourceMemoryManager::AddResourceUnitToFreeCache(ResourceUnit *res_unit) {
        if (res_unit->m_memory_manager_free_cache_node.IsLinked() == true) { return; }
        std::scoped_lock l(m_memory_mgr_mutex);
        m_resource_unit_free_cache_list.PushBack(*res_unit);
    }

    mem::Heap *ResourceMemoryManager::CreateResourceHeap(ResourceUnit *resource_unit, const char *heap_name, size_t size, ResourceHeapType heap_type) {

        /* Lock manager */
        std::scoped_lock l(m_memory_mgr_mutex);

        /* Try create heap */
        mem::Heap *new_heap = this->CreateHeapImpl(heap_name, size, heap_type);

        /* Free resource units sync for memory if necessary */
        while (new_heap == nullptr) {
            this->FreeFromCache(size);
            new_heap = this->CreateHeapImpl(heap_name, size, heap_type);
        }

        /* Add resource unit to list */
        m_resource_unit_list.PushBack(*resource_unit);

        return new_heap;
    }

    void ResourceMemoryManager::TrackMemoryUsageGlobal(mem::Heap *heap) {
        const size_t size = (mem::VirtualAddressHeap::CheckRuntimeTypeInfoStatic(m_resource_heap->GetRuntimeTypeInfo()) == true) ? reinterpret_cast<mem::VirtualAddressHeap*>(m_resource_heap)->GetSizeOfAllocation(heap) : heap->GetTotalSize();
        vp::util::InterlockedAdd(std::addressof(m_global_memory_usage), size);
    }
    void ResourceMemoryManager::TrackMemoryUsageActive(mem::Heap *heap) {
        const size_t size = (mem::VirtualAddressHeap::CheckRuntimeTypeInfoStatic(m_resource_heap->GetRuntimeTypeInfo()) == true) ? reinterpret_cast<mem::VirtualAddressHeap*>(m_resource_heap)->GetSizeOfAllocation(heap) : heap->GetTotalSize();
        vp::util::InterlockedAdd(std::addressof(m_global_memory_usage), size);
    }
    void ResourceMemoryManager::ReleaseMemoryUsageGlobal(mem::Heap *heap) {
        const size_t size = (mem::VirtualAddressHeap::CheckRuntimeTypeInfoStatic(m_resource_heap->GetRuntimeTypeInfo()) == true) ? reinterpret_cast<mem::VirtualAddressHeap*>(m_resource_heap)->GetSizeOfAllocation(heap) : heap->GetTotalSize();
        vp::util::InterlockedSubtract(std::addressof(m_global_memory_usage), size);
    }
    void ResourceMemoryManager::ReleaseMemoryUsageActive(mem::Heap *heap) {
        const size_t size = (mem::VirtualAddressHeap::CheckRuntimeTypeInfoStatic(m_resource_heap->GetRuntimeTypeInfo()) == true) ? reinterpret_cast<mem::VirtualAddressHeap*>(m_resource_heap)->GetSizeOfAllocation(heap) : heap->GetTotalSize();
        vp::util::InterlockedSubtract(std::addressof(m_active_memory_usage), size);
    }
}
