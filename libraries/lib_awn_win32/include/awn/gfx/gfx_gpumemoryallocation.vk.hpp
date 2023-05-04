#pragma once

namespace awn::gfx {

    class GpuHeapMemory;
    class GpuHeap;
    class GpuHeapManager;

    class GpuMemoryAllocation {
        public:
            friend GpuHeapMemory;
            friend GpuHeap;
            friend GpuHeapManager;
        protected:
            size_t                       m_offset;
            u32                          m_size;
            u16                          m_memory_property_flags;
            u16                          m_is_dedicated_allocation;
            GpuHeapMemory               *m_parent_gpu_heap_memory;
            vp::util::IntrusiveListNode  m_gpu_heap_memory_list_node;
        public:
            constexpr GpuMemoryAllocation() {/*...*/}

            Result TryAllocateGpuMemory(mem::Heap *meta_data_heap, size_t size, s32 alignment, MemoryPropertyFlags memory_property_flags);
            void   FreeGpuMemory();

            VkBuffer CreateBuffer(VkBufferUsageFlagBits usage_flags, u32 size = 0, size_t offset = 0);
            VkImage  CreateImage(TextureInfo *texture_info, VkImageUsageFlags vk_image_usage_flags, size_t offset = 0);

            constexpr ALWAYS_INLINE GpuMemoryAddress GetGpuMemoryAddress(size_t offset) {
                const GpuMemoryAddress address(this, offset);
                return address;
            }

            void FlushCache(size_t size, size_t offset = 0);
    };
}
