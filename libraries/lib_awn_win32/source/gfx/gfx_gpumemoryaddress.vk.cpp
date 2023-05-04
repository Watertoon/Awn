#include <awn.hpp>

namespace awn::gfx {

    VkBuffer GpuMemoryAddress::CreateBuffer(VkBufferUsageFlagBits usage_flags, size_t size) {
        return m_parent_gpu_allocation->CreateBuffer(usage_flags, size, m_offset);
    }

    VkBuffer GpuMemoryAddress::CreateBuffer(BufferInfo *buffer_info) {
        return m_parent_gpu_allocation->CreateBuffer(static_cast<VkBufferUsageFlagBits>(vp::res::GfxGpuAccessFlagsToVkBufferUsageFlags(static_cast<GpuAccessFlags>(buffer_info->gpu_access_flags))), buffer_info->size, m_offset);
    }

    VkImage GpuMemoryAddress::CreateImage(TextureInfo *texture_info) {
        return m_parent_gpu_allocation->CreateImage(texture_info, vp::res::GfxGpuAccessFlagsToVkImageUsageFlags(static_cast<GpuAccessFlags>(texture_info->gpu_access_flags)), m_offset);
    }

    VkImage GpuMemoryAddress::CreateImage(TextureInfo *texture_info, VkImageUsageFlags manual_usage_flags) {
        return m_parent_gpu_allocation->CreateImage(texture_info, manual_usage_flags, m_offset);
    }

    void GpuMemoryAddress::FlushCache() {
        m_parent_gpu_allocation->FlushCache(m_offset);
    }
}
