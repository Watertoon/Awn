#pragma once

namespace awn::gfx {

    class GpuMemoryAllocation;

    class GpuMemoryAddress {
        private:
            GpuMemoryAllocation *m_parent_gpu_allocation;
            size_t               m_offset;
        public:
            constexpr ALWAYS_INLINE GpuMemoryAddress() : m_parent_gpu_allocation(nullptr), m_offset(0) {/*...*/}
            constexpr ALWAYS_INLINE GpuMemoryAddress(GpuMemoryAllocation *allocation, size_t offset) : m_parent_gpu_allocation(allocation), m_offset(offset) {/*...*/}

            constexpr ALWAYS_INLINE ~GpuMemoryAddress() {/*...*/}

            constexpr ALWAYS_INLINE GpuMemoryAddress &operator=(const GpuMemoryAddress& rhs) {
                m_parent_gpu_allocation = rhs.m_parent_gpu_allocation;
                m_offset                = rhs.m_offset;
                return *this;
            }

            VkBuffer CreateBuffer(VkBufferUsageFlagBits usage_flags, size_t size);
            VkBuffer CreateBuffer(BufferInfo *buffer_info);

            VkImage CreateImage(TextureInfo *texture_info);
            VkImage CreateImage(TextureInfo *texture_info, VkImageUsageFlags manual_usage_flags);

            void FlushCache();
    };
}
