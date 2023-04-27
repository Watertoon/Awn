#pragma once

namespace awn::gfx {

    VkBuffer GpuMemoryAddress::CreateBuffer(VkBufferUsageFlagBits usage_flags, size_t size) {
        return m_parent_gpu_allocation->CreateBuffer(usage_flags, size, m_offset);
    }

    VkBuffer GpuMemoryAddress::CreateBuffer(BufferInfo *buffer_info) {
        return m_parent_gpu_allocation->CreateBuffer(res::GfxGpuAccessFlagsToVkBufferUsageFlags(static_cast<>(buffer_info->gpu_access_flags)), buffer_info->size, m_offset);
    }

    VkImage GpuMemoryAddress::CreateImage(TextureInfo texture_info) {
        return m_parent_gpu_allocation->CreateImage(texture_info, res::GfxGpuAccessFlagsToVkImageUsageFlags(static_cast<res::GfxGpuAccessFlags>(texture_info->gpu_access_flags)), m_offset);
    }

    VkImage GpuMemoryAddress::CreateImage(TextureInfo texture_info, VkImageUsageFlags manual_usage_flags) {
        return m_parent_gpu_allocation->CreateImage(texture_info, manual_usage_flags, m_offset);
    }

    void GpuMemoryAddress::FlushCache() {
        m_parent_gpu_allocation->FlushCache(m_offset);
    }
}
