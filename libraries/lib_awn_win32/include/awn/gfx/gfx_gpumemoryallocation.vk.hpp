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
            void                        *m_mapped_memory;
            size_t                       m_offset;
            size_t                       m_size;
            u16                          m_memory_property_flags;
            u16                          m_is_dedicated_allocation;
            GpuHeapMemory               *m_parent_gpu_heap_memory;
            vp::util::IntrusiveListNode  m_gpu_heap_memory_list_node;
        public:
            constexpr GpuMemoryAllocation() {/*...*/}

            Result TryAllocateGpuMemory(mem::Heap *meta_data_heap, size_t size, s32 alignment, MemoryPropertyFlags memory_property_flags);
            void   FreeGpuMemory();

            VkBuffer CreateBuffer(VkBufferUsageFlagBits usage_flags, size_t size = 0, size_t offset = 0);
            VkImage  CreateImage(TextureInfo *texture_info, VkImageUsageFlags vk_image_usage_flags, size_t offset = 0);

            constexpr ALWAYS_INLINE GpuMemoryAddress GetGpuMemoryAddress(size_t offset) {
                const GpuMemoryAddress address(this, offset);
                return address;
            }

            constexpr ALWAYS_INLINE void *Map() const { return m_mapped_memory; }

            void FlushCpuCache();
            void FlushCpuCache(size_t size, size_t offset = 0);

            void InvalidateCpuCache();
            void InvalidateCpuCache(size_t size, size_t offset = 0);
    };

    void *GpuMemoryAddress::Map() const { return reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(m_parent_gpu_allocation->Map()) + m_offset); };
}
