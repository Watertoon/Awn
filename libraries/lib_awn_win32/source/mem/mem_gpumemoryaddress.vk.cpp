#include <awn.hpp>

namespace awn::mem {

    VkBuffer GpuMemoryAddress::CreateBuffer(VkBufferUsageFlags usage_flags, size_t size) {

        /* Create VkBuffer */
        VkBuffer buffer = 0;
        const VkBufferCreateInfo vk_buffer_info = {
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .size  = size,
            .usage = static_cast<u32>(usage_flags) | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT
        };
        const u32 result0 = ::pfn_vkCreateBuffer(gfx::Context::GetInstance()->GetVkDevice(), std::addressof(vk_buffer_info), gfx::Context::GetInstance()->GetVkAllocationCallbacks(), std::addressof(buffer));
        VP_ASSERT(result0 == VK_SUCCESS);

        /* Bind gpu memory allocation to buffer from offset */
        const u32 result1 = ::pfn_vkBindBufferMemory(gfx::Context::GetInstance()->GetVkDevice(), buffer, gpu_root_heap_context->vk_device_memory, gpu_root_heap_context->GetVkDeviceMemoryOffset(address));
        VP_ASSERT(result1 == VK_SUCCESS);
        
        return buffer;
    }

    VkImage GpuMemoryAddress::CreateImage(VkImageUsageFlags image_usage_flags, gfx::TextureInfo *texture_info) {

        /* Create VkImage */
        VkImage image = 0;
        const VkImageCreateInfo image_info = {
            .sType                 = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
            .flags                 = vp::res::ResGfxTextureInfoToVkImageCreateFlags(texture_info),
            .imageType             = vp::res::GfxImageStorageDimensionToVkImageType(static_cast<gfx::ImageStorageDimension>(texture_info->storage_dimension)),
            .format                = vp::res::GfxImageFormatToVkFormat(static_cast<gfx::ImageFormat>(texture_info->image_format)),
            .extent = {
                .width             = texture_info->width,
                .height            = texture_info->height,
                .depth             = texture_info->depth
            },
            .mipLevels             = texture_info->mip_levels,
            .arrayLayers           = texture_info->array_layers,
            .samples               = static_cast<VkSampleCountFlagBits>(texture_info->sample_count),
            .tiling                = static_cast<VkImageTiling>(texture_info->tile_mode),
            .usage                 = image_usage_flags,
            .sharingMode           = VK_SHARING_MODE_CONCURRENT,
            .queueFamilyIndexCount = gfx::Context::GetInstance()->GetQueueFamilyCount(),
            .pQueueFamilyIndices   = gfx::Context::GetInstance()->GetQueueFamilyIndiceArray(),
            .initialLayout         = VK_IMAGE_LAYOUT_UNDEFINED,
        };
        const u32 result0 = ::pfn_vkCreateImage(gfx::Context::GetInstance()->GetVkDevice(), std::addressof(image_info), gfx::Context::GetInstance()->GetVkAllocationCallbacks(), std::addressof(image));
        VP_ASSERT(result0 == VK_SUCCESS);

        /* Bind gpu memory to image from offset */
        const u32 result1 = ::pfn_vkBindImageMemory(gfx::Context::GetInstance()->GetVkDevice(), image, gpu_root_heap_context->vk_device_memory, gpu_root_heap_context->GetVkDeviceMemoryOffset(address));
        VP_ASSERT(result1 == VK_SUCCESS);

        return image;
    }

    void GpuMemoryAddress::FlushCpuCache(size_t size) {
        gpu_root_heap_context->FlushCpuCache(address, size);
    }

    void GpuMemoryAddress::InvalidateCpuCache(size_t size) {
        gpu_root_heap_context->InvalidateCpuCache(address, size);
    }
}
