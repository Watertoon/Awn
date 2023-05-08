#include <awn.hpp>

namespace awn::gfx {

    Result GpuMemoryAllocation::TryAllocateGpuMemory(mem::Heap *meta_data_heap, size_t size, s32 alignment, MemoryPropertyFlags memory_property_flags) {

        /* Allocate from heap manager */
        return GpuHeapManager::GetInstance()->TryAllocateGpuMemory(this, meta_data_heap, size, alignment, memory_property_flags);
    }

    void GpuMemoryAllocation::FreeGpuMemory() {

        /* Free from gpu heap memory */
        GpuHeapManager::GetInstance()->FreeGpuMemoryAllocation(this);
    }

    VkBuffer GpuMemoryAllocation::CreateBuffer(VkBufferUsageFlagBits usage_flags, size_t size, size_t offset) {

        /* Create VkBuffer */
        VkBuffer buffer = 0;
        const VkBufferCreateInfo buffer_info = {
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .size = size,
            .usage = static_cast<u32>(usage_flags) | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT
        };
        const u32 result0 = ::pfn_vkCreateBuffer(Context::GetInstance()->GetVkDevice(), std::addressof(buffer_info), Context::GetInstance()->GetVkAllocationCallbacks(), std::addressof(buffer));
        VP_ASSERT(result0 == VK_SUCCESS);

        /* Bind gpu memory allocation to buffer from offset */
        const u32 result1 = ::pfn_vkBindBufferMemory(Context::GetInstance()->GetVkDevice(), buffer, m_parent_gpu_heap_memory->GetVkDeviceMemory(), m_offset + offset);
        VP_ASSERT(result1 == VK_SUCCESS);

        return buffer;
    }

    VkImage GpuMemoryAllocation::CreateImage(TextureInfo *texture_info, VkImageUsageFlags vk_image_usage_flags, size_t offset) {

        /* Create VkImage */
        VkImage image = 0;
        const VkImageCreateInfo image_info {
            .sType                 = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
            .flags                 = vp::res::ResGfxTextureInfoToVkImageCreateFlags(texture_info),
            .imageType             = vp::res::GfxImageStorageDimensionToVkImageType(static_cast<ImageStorageDimension>(texture_info->storage_dimension)),
            .format                = vp::res::GfxImageFormatToVkFormat(static_cast<ImageFormat>(texture_info->image_format)),
            .extent = {
                .width             = texture_info->width,
                .height            = texture_info->height,
                .depth             = texture_info->depth
            },
            .mipLevels             = texture_info->mip_levels,
            .arrayLayers           = texture_info->array_layers,
            .samples               = static_cast<VkSampleCountFlagBits>(texture_info->sample_count),
            .tiling                = static_cast<VkImageTiling>(texture_info->tile_mode),
            .usage                 = vk_image_usage_flags,
            .sharingMode           = VK_SHARING_MODE_CONCURRENT,
            .queueFamilyIndexCount = Context::GetInstance()->GetQueueFamilyCount(),
            .pQueueFamilyIndices   = Context::GetInstance()->GetQueueFamilyIndiceArray(),
            .initialLayout         = VK_IMAGE_LAYOUT_UNDEFINED
        };
        const u32 result0 = ::pfn_vkCreateImage(Context::GetInstance()->GetVkDevice(), std::addressof(image_info), Context::GetInstance()->GetVkAllocationCallbacks(), std::addressof(image));
        VP_ASSERT(result0 == VK_SUCCESS);

        /* Bind gpu memory to image from offset */
        const u32 result1 = ::pfn_vkBindImageMemory(Context::GetInstance()->GetVkDevice(), image, m_parent_gpu_heap_memory->GetVkDeviceMemory(), m_offset + offset);
        VP_ASSERT(result1 == VK_SUCCESS);

        return image;
    }

    void GpuMemoryAllocation::FlushCpuCache() {
        m_parent_gpu_heap_memory->FlushCpuCache(m_size, m_offset);
    }
    void GpuMemoryAllocation::FlushCpuCache(size_t size, size_t offset) {
        VP_ASSERT((offset + size) < m_size);
        m_parent_gpu_heap_memory->FlushCpuCache(size, m_offset + offset);
    }

    void GpuMemoryAllocation::InvalidateCpuCache() {
        m_parent_gpu_heap_memory->InvalidateCpuCache(m_size, m_offset);
    }
    void GpuMemoryAllocation::InvalidateCpuCache(size_t size, size_t offset) {
        VP_ASSERT((offset + size) < m_size);
        m_parent_gpu_heap_memory->InvalidateCpuCache(size, m_offset + offset);
    }
}
