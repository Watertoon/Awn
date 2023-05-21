#pragma once

namespace awn::gfx {

    class GpuHeap;
    class GpuHeapManager;

    class GpuHeapMemory {
        public:
            friend class GpuHeap;
            friend class GpuHeapManager;
        public:
            static constexpr size_t cMaxBlockCount = 0x800;
        private:
            using GpuMemoryAllocationList = vp::util::IntrusiveListTraits<GpuMemoryAllocation, &GpuMemoryAllocation::m_gpu_heap_memory_list_node>::List;
        protected:
            GpuHeap                     *m_parent_gpu_heap;
            VkDeviceMemory               m_vk_device_memory;
            void                        *m_mapped_memory;
            size_t                       m_memory_size;
            u32                          m_memory_property_flags;
            mem::SeparateHeap           *m_gpu_separate_heap;
            GpuMemoryAllocationList      m_gpu_memory_allocation_list;
            vp::util::IntrusiveListNode  m_gpu_heap_list_node;
        public:
            constexpr ALWAYS_INLINE GpuHeapMemory() {/*...*/}

            static GpuHeapMemory *Create(GpuHeap *parent_heap, mem::Heap *heap, size_t size, s32 alignment, MemoryPropertyFlags memory_properties);
            
            Result TryAllocateGpuMemory(GpuMemoryAllocation *out_allocation, size_t size, s32 alignment, MemoryPropertyFlags memory_property_flags);
            bool   FreeGpuMemoryAllocation(GpuMemoryAllocation *allocation);
            
            void FlushCpuCache(size_t size = VK_WHOLE_SIZE, size_t offset = 0);
            void InvalidateCpuCache(size_t size = VK_WHOLE_SIZE, size_t offset = 0);

            constexpr ALWAYS_INLINE VkDeviceMemory  GetVkDeviceMemory() const { return m_vk_device_memory; }
            constexpr ALWAYS_INLINE void           *Map()               const { return m_mapped_memory; }
    };
}
