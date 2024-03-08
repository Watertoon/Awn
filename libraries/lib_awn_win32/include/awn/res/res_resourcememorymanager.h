#pragma once

namespace awn::res {

    enum class ResourceHeapType : u32 {
        FrameHeap = 0,
        ExpHeap   = 1,
    };
    enum class ManagerHeapType : u32 {
        VirtualAddressHeap = 0,
        ExpHeap            = 1,
    };

    struct ResourceMemoryManagerInfo {
        const char       *name;
        size_t            heap_size;
        s32               heap_alignment;
        ManagerHeapType   manager_heap_type;
    };

    class ResourceUnit;

    class ResourceMemoryManager {
        public:
            friend class ResourceUnit;
        public:
            using ResourceUnitList          = vp::util::IntrusiveListTraits<ResourceUnit, &ResourceUnit::m_memory_manager_node>::List;
            using ResourceUnitFreeCacheList = vp::util::IntrusiveListTraits<ResourceUnit, &ResourceUnit::m_memory_manager_free_cache_node>::List;
        private:
            mem::Heap                 *m_resource_heap;
            ResourceUnitList           m_resource_unit_list;
            ResourceUnitFreeCacheList  m_resource_unit_free_cache_list;
            size_t                     m_max_allocatable_size;
            sys::ServiceMutex          m_memory_mgr_mutex;
            size_t                     m_global_memory_usage;
            size_t                     m_active_memory_usage;
        private:
            mem::Heap *CreateHeapImpl(const char *heap_name, size_t size, ResourceHeapType heap_type);

            void FreeHeap(mem::Heap *heap, mem::Heap *gpu_heap, ResourceUnit *resource_unit);

            bool FreeFromCache(size_t target_size);
        public:
            constexpr  ResourceMemoryManager() : m_resource_heap(), m_resource_unit_list(), m_resource_unit_free_cache_list(), m_max_allocatable_size(), m_memory_mgr_mutex() {/*...*/}
            constexpr ~ResourceMemoryManager() {/*...*/}

            void Initialize(mem::Heap *heap, ResourceMemoryManagerInfo *manager_info);
            void Finalize();

            void AddResourceUnitToFreeCache(ResourceUnit *res_unit);
            void ClearCacheForAllocate(u32 count);

            mem::Heap *CreateResourceHeap(ResourceUnit *resource_unit, const char *heap_name, size_t size, ResourceHeapType heap_type);

            void TrackMemoryUsageGlobal(mem::Heap *heap);
            void TrackMemoryUsageActive(mem::Heap *heap);
            void ReleaseMemoryUsageGlobal(mem::Heap *heap);
            void ReleaseMemoryUsageActive(mem::Heap *heap);
    };
}
