#pragma once

namespace awn::gfx {

    class GpuHeapManager;

    class GpuHeap {
        public:
            friend class GpuHeapManager;
        private:
            using GpuHeapMemoryList = vp::util::IntrusiveListTraits<GpuHeapMemory, &GpuHeapMemory::m_gpu_heap_list_node>::List;
        protected:
            vp::util::IntrusiveListNode  m_gpu_heap_manager_list_node;
            GpuHeapMemoryList            m_gpu_heap_memory_list;
            sys::Mutex                   m_heap_memory_list_mutex;
            mem::Heap                   *m_parent_heap;
        public:
            constexpr ALWAYS_INLINE GpuHeap() : m_gpu_heap_manager_list_node(), m_gpu_heap_memory_list(), m_heap_memory_list_mutex(), m_parent_heap(nullptr) {/*...*/}
            constexpr ALWAYS_INLINE ~GpuHeap() {/*...*/}

            Result TryAllocateGpuMemory(GpuMemoryAllocation *out_allocation, mem::Heap *heap, size_t size, s32 alignment, MemoryPropertyFlags memory_property_flags, size_t minimum_size, size_t maximum_size);
            bool   FreeGpuMemoryAllocation(GpuMemoryAllocation *allocation);
    };
}
