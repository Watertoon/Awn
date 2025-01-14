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
#pragma once

namespace awn::mem {

    constexpr inline size_t cMaxGpuRootHeapCount = 2;

#ifdef VP_TARGET_GRAPHICS_API_vk
    struct GpuMemoryHandle {
        VkDeviceMemory vk_device_memory = VK_NULL_HANDLE;
        void *memory;

        constexpr void *GetAddress() const {
            return this->memory;
        }
        constexpr bool IsValid() const {
            return this->vk_device_memory != VK_NULL_HANDLE;
        }
    };

    static constexpr size_t cGpuMemoryHandleMetaDataSize = 0;
    static constexpr size_t cGpuMemoryHandleMetaDataAlignment = 0;
#elif VP_TARGET_GRAPHICS_API_nvn
    struct GpuMemoryHandle {
        NVNmemoryPool *nvn_memory_pool = nullptr;

        constexpr bool IsValid() const {
            return this->nvn_memory_pool != nullptr;
        }
    };

    static constexpr size_t cGpuMemoryHandleMetaDataSize = sizeof(NVNmemoryPool);
    static constexpr size_t cGpuMemoryHandleMetaDataAlignment = alignof(NVNmemoryPool);
#endif
    static constexpr GpuMemoryHandle cInvalidGpuMemoryHandle = {};

    struct GpuRootHeapInfo {
        GpuMemoryHandle handle;
        size_t size;
    };
        
    struct GpuHeapManagerInfo {
        u32 cpu_uncached_root_heap_count;
        u32 cpu_cached_root_heap_count;
        u32 gpu_uncached_root_heap_count;
        GpuRootHeapInfo cpu_uncached_info_array[cMaxGpuRootHeapCount];
        GpuRootHeapInfo cpu_cached_info_array[cMaxGpuRootHeapCount];
        GpuRootHeapInfo gpu_uncached_info_array[cMaxGpuRootHeapCount];

        static constexpr inline size_t cDefaultHostUncachedSize    = vp::util::c4MB;
        static constexpr inline size_t cDefaultHostCachedSize      = vp::util::c8MB;
        static constexpr inline size_t cDefaultGpuHostUncachedSize = vp::util::c16MB;
    };

    struct GpuRootHeapContext {
        gfx::MemoryPropertyFlags memory_property_flags;
        void                   *base_address;
        size_t                  memory_size;
        GpuMemoryHandle         gpu_memory_handle;
        GpuExpHeap             *root_heap;

        ALWAYS_INLINE size_t GetMemoryOffset(void *address) {
            return reinterpret_cast<uintptr_t>(address) - reinterpret_cast<uintptr_t>(base_address);
        }

        void FlushCpuCache(void *address, size_t size);
        void InvalidateCpuCache(void *address, size_t size);
    };

    class GpuHeapManager {
        public:
            friend Heap *FindHeapFromAddress(void *address);
            friend Heap *FindHeapByName(const char *name);
        protected:
            u32 m_cpu_uncached_heap_count;
            u32 m_cpu_cached_heap_count;
            u32 m_gpu_uncached_heap_count;
            GpuRootHeapContext m_cpu_uncached_heap_context_array[cMaxGpuRootHeapCount];
            GpuRootHeapContext m_cpu_cached_heap_context_array[cMaxGpuRootHeapCount];
            GpuRootHeapContext m_gpu_uncached_heap_context_array[cMaxGpuRootHeapCount];
        public:
            AWN_SINGLETON_TRAITS(GpuHeapManager);
        public:
            constexpr  GpuHeapManager() : m_cpu_uncached_heap_count(), m_cpu_cached_heap_count(), m_gpu_uncached_heap_count(), m_cpu_uncached_heap_context_array(), m_cpu_cached_heap_context_array(), m_gpu_uncached_heap_context_array() {/*...*/}
            constexpr ~GpuHeapManager() {/*...*/}

            bool Initialize(mem::Heap *heap, const GpuHeapManagerInfo *gpu_heap_mgr_info);
            void Finalize();

            GpuRootHeapContext *FindGpuRootHeapContextFromAddress(void *address) {

                for (u32 i = 0; i < m_cpu_uncached_heap_count; ++i) {
                    if (m_cpu_uncached_heap_context_array[i].root_heap->IsAddressInHeap(address)) { return std::addressof(m_cpu_uncached_heap_context_array[i]); }
                }
                for (u32 i = 0; i < m_cpu_cached_heap_count; ++i) {
                    if (m_cpu_cached_heap_context_array[i].root_heap->IsAddressInHeap(address)) { return std::addressof(m_cpu_cached_heap_context_array[i]); }
                }
                for (u32 i = 0; i < m_gpu_uncached_heap_count; ++i) {
                    if (m_gpu_uncached_heap_context_array[i].root_heap->IsAddressInHeap(address)) { return std::addressof(m_gpu_uncached_heap_context_array[i]); }
                }

                return nullptr;
            }

            constexpr ALWAYS_INLINE GpuRootHeapContext *GetGpuRootHeapContextCpuUncached(u32 root_heap_index) { return std::addressof(m_cpu_uncached_heap_context_array[root_heap_index]); }
            constexpr ALWAYS_INLINE GpuRootHeapContext *GetGpuRootHeapContextCpuCached(u32 root_heap_index)   { return std::addressof(m_cpu_cached_heap_context_array[root_heap_index]); }
            constexpr ALWAYS_INLINE GpuRootHeapContext *GetGpuRootHeapContextGpuUncached(u32 root_heap_index) { return std::addressof(m_gpu_uncached_heap_context_array[root_heap_index]); }

            constexpr ALWAYS_INLINE GpuExpHeap *GetGpuRootHeapCpuUncached(u32 root_heap_index) { return m_cpu_uncached_heap_context_array[root_heap_index].root_heap; }
            constexpr ALWAYS_INLINE GpuExpHeap *GetGpuRootHeapCpuCached(u32 root_heap_index)   { return m_cpu_cached_heap_context_array[root_heap_index].root_heap; }
            constexpr ALWAYS_INLINE GpuExpHeap *GetGpuRootHeapGpuUncached(u32 root_heap_index) { return m_gpu_uncached_heap_context_array[root_heap_index].root_heap; }
    };

    ALWAYS_INLINE GpuRootHeapContext *FindGpuRootHeapContextFromAddress(void *address) {
        GpuHeapManager *heap_manager = GpuHeapManager::GetInstance();
        if (heap_manager == nullptr) { return nullptr; }
        return heap_manager->FindGpuRootHeapContextFromAddress(address);
    }
}
