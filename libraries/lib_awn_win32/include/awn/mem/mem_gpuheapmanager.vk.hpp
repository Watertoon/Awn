#pragma once

namespace awn::gfx {
    using MemoryPropertyFlags = vp::res::GfxMemoryPoolFlags;
}

namespace awn::mem {

    constexpr inline size_t cMaxGpuRootHeapCount = 2;

    struct GpuHeapManagerInfo {
        u32    host_uncached_root_heap_count;
        u32    host_cached_root_heap_count;
        u32    gpu_host_uncached_root_heap_count;
        size_t host_uncached_size_array[cMaxGpuRootHeapCount];
        size_t host_cached_size_array[cMaxGpuRootHeapCount];
        size_t gpu_host_uncached_size_array[cMaxGpuRootHeapCount];

        static constexpr inline size_t cDefaultHostUncachedSize    = vp::util::c4MB;
        static constexpr inline size_t cDefaultHostCachedSize      = vp::util::c8MB;
        static constexpr inline size_t cDefaultGpuHostUncachedSize = vp::util::c16MB;

        constexpr void SetDefaults() {
            host_uncached_root_heap_count     = 1;
            host_cached_root_heap_count       = 1;
            gpu_host_uncached_root_heap_count = 1;
            host_uncached_size_array[0]       = cDefaultHostUncachedSize;
            host_cached_size_array[0]         = cDefaultHostCachedSize;
            gpu_host_uncached_size_array[0]   = cDefaultGpuHostUncachedSize;  
        }
    };

    struct GpuRootHeapContext {
        gfx::MemoryPropertyFlags  memory_property_flags;
        void                     *base_address;
        VkDeviceMemory            vk_device_memory;
        GpuExpHeap               *root_heap;

        ALWAYS_INLINE size_t GetVkDeviceMemoryOffset(void *address) {
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
            GpuRootHeapContext m_host_uncached_heap_context_array[cMaxGpuRootHeapCount];
            GpuRootHeapContext m_host_cached_heap_context_array[cMaxGpuRootHeapCount];
            GpuRootHeapContext m_gpu_host_uncached_heap_context_array[cMaxGpuRootHeapCount];
            GpuHeapManagerInfo m_manager_info;
        public:
            AWN_SINGLETON_TRAITS(GpuHeapManager);
        private:
            bool AllocateContext(GpuRootHeapContext *heap_context, const char *name, size_t size, gfx::MemoryPropertyFlags memory_properties);
            void FreeContext(GpuRootHeapContext *heap_context);
        public:
            constexpr  GpuHeapManager() : m_host_uncached_heap_context_array(), m_host_cached_heap_context_array(), m_gpu_host_uncached_heap_context_array(), m_manager_info() {/*...*/}
            constexpr ~GpuHeapManager() {/*...*/}

            bool Initialize(const GpuHeapManagerInfo *gpu_heap_mgr_info);
            void Finalize();

            constexpr ALWAYS_INLINE GpuRootHeapContext *GetGpuRootHeapContextHostUncached(u32 root_heap_index)    { return std::addressof(m_host_uncached_heap_context_array[root_heap_index]); }
            constexpr ALWAYS_INLINE GpuRootHeapContext *GetGpuRootHeapContextHostCached(u32 root_heap_index)      { return std::addressof(m_host_cached_heap_context_array[root_heap_index]); }
            constexpr ALWAYS_INLINE GpuRootHeapContext *GetGpuRootHeapContextGpuHostUncached(u32 root_heap_index) { return std::addressof(m_gpu_host_uncached_heap_context_array[root_heap_index]); }

            constexpr ALWAYS_INLINE GpuExpHeap *GetGpuRootHeapHostUncached(u32 root_heap_index)    { return m_host_uncached_heap_context_array[root_heap_index].root_heap; }
            constexpr ALWAYS_INLINE GpuExpHeap *GetGpuRootHeapHostCached(u32 root_heap_index)      { return m_host_cached_heap_context_array[root_heap_index].root_heap; }
            constexpr ALWAYS_INLINE GpuExpHeap *GetGpuRootHeapGpuHostUncached(u32 root_heap_index) { return m_gpu_host_uncached_heap_context_array[root_heap_index].root_heap; }
    };
}
