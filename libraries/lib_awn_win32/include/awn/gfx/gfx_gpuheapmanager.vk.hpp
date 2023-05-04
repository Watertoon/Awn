#pragma once

namespace awn::gfx {

    class GpuHeapManager {
        public:
            static constexpr size_t cMaxGpuHeaps = 0x200;
        public:
            using GpuHeapAllocator = vp::util::FixedObjectAllocator<GpuHeap, cMaxGpuHeaps>;
            using GpuHeapList      = vp::util::IntrusiveListTraits<GpuHeap, &GpuHeap::m_gpu_heap_manager_list_node>::List;
        private:
            size_t           m_minimum_block_size;
            sys::Mutex       m_gpu_heap_list_mutex;
            GpuHeapList      m_gpu_heap_list;
            GpuHeapAllocator m_gpu_heap_allocator;
        private:
            AWN_SINGLETON_TRAITS(GpuHeapManager);
        public:
            constexpr GpuHeapManager() : m_minimum_block_size(), m_gpu_heap_list_mutex(), m_gpu_heap_allocator() {/*...*/}

            constexpr ALWAYS_INLINE void Initialize(size_t minimum_block_size) {

                /* Set state */
                m_minimum_block_size = minimum_block_size;
            }

            Result TryAllocateGpuMemory(GpuMemoryAllocation *out_allocation, mem::Heap *heap, size_t size, s32 alignment, MemoryPropertyFlags memory_property_flags);
            void   FreeGpuMemoryAllocation(GpuMemoryAllocation *allocation);
    };
}
