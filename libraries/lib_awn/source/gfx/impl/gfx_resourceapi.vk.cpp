/*
 *  Copyright (C) W. Michael Knudson
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as 
 *  published by the Free Software Foundation.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along with this program; 
 *  if not, see <https://www.gnu.org/licenses/>.
 */
#include <awn.hpp>

namespace awn::gfx::impl {

	VkBuffer CreateVkBuffer(const BufferInfo *buffer_info) {
        return CreateVkBuffer(vp::res::GfxGpuAccessFlagsToVkBufferUsageFlags(static_cast<vp::res::GfxGpuAccessFlags>(buffer_info->gpu_access_flags)), buffer_info->size);
    }

    VkBuffer CreateVkBuffer(VkBufferUsageFlags usage_flags, size_t size) {

        /* Create VkBuffer */
        VkBuffer buffer = VK_NULL_HANDLE;
        const VkBufferCreateInfo vk_buffer_info = {
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .size  = size,
            .usage = static_cast<u32>(usage_flags) | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT
        };
        const u32 result0 = ::pfn_vkCreateBuffer(Context::GetInstance()->GetVkDevice(), std::addressof(vk_buffer_info), Context::GetInstance()->GetVkAllocationCallbacks(), std::addressof(buffer));
        if (result0 != VK_SUCCESS) { return VK_NULL_HANDLE; }
        
        return buffer;
    }

	VkBuffer CreateVkBuffer(void *gpu_address, const BufferInfo *buffer_info) {
        return CreateVkBuffer(gpu_address, vp::res::GfxGpuAccessFlagsToVkBufferUsageFlags(static_cast<vp::res::GfxGpuAccessFlags>(buffer_info->gpu_access_flags)), buffer_info->size);
    }

    VkBuffer CreateVkBuffer(void *gpu_address, VkBufferUsageFlags usage_flags, size_t size) {

        /* Ensure address */
        if (gpu_address == nullptr) { return VK_NULL_HANDLE; }

        /* Create VkBuffer */
        VkBuffer buffer = VK_NULL_HANDLE;
        const VkBufferCreateInfo vk_buffer_info = {
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .size  = size,
            .usage = static_cast<u32>(usage_flags) | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT
        };
        const u32 result0 = ::pfn_vkCreateBuffer(Context::GetInstance()->GetVkDevice(), std::addressof(vk_buffer_info), Context::GetInstance()->GetVkAllocationCallbacks(), std::addressof(buffer));
        if (result0 != VK_SUCCESS) { return VK_NULL_HANDLE; }

        /* Bind gpu memory allocation to buffer from offset */
        mem::GpuRootHeapContext *gpu_root_heap_context = mem::FindGpuRootHeapContextFromAddress(gpu_address);
        const u32 result1 = ::pfn_vkBindBufferMemory(Context::GetInstance()->GetVkDevice(), buffer, gpu_root_heap_context->gpu_memory_handle.vk_device_memory, gpu_root_heap_context->GetMemoryOffset(gpu_address));
        if (result1 != VK_SUCCESS) { DestroyVkBuffer(buffer); return VK_NULL_HANDLE; }
        
        return buffer;
    }

	VkImage  CreateVkImage(const TextureInfo *texture_info) {
        return CreateVkImage(vp::res::GfxGpuAccessFlagsToVkImageUsageFlags(static_cast<vp::res::GfxGpuAccessFlags>(texture_info->gpu_access_flags)), texture_info, nullptr);
    }

	VkImage  CreateVkImage(const TextureInfo *texture_info, const void *pnext) {
        return CreateVkImage(vp::res::GfxGpuAccessFlagsToVkImageUsageFlags(static_cast<vp::res::GfxGpuAccessFlags>(texture_info->gpu_access_flags)), texture_info, pnext);
    }

    VkImage CreateVkImage(VkImageUsageFlags image_usage_flags, const gfx::TextureInfo *texture_info) {
        return CreateVkImage(image_usage_flags, texture_info, nullptr);
    }

    VkImage CreateVkImage(VkImageUsageFlags image_usage_flags, const gfx::TextureInfo *texture_info, const void *pnext) {

        /* Ensure texture info */
        if (texture_info == nullptr) { return VK_NULL_HANDLE; }

        /* Create VkImage */
        VkImage image = VK_NULL_HANDLE;
        const VkImageCreateInfo image_info = {
            .sType                 = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
            .pNext                 = pnext,
            .flags                 = vp::res::ResGfxTextureInfoToVkImageCreateFlags(texture_info),
            .imageType             = vp::res::GfxImageStorageDimensionToVkImageType(static_cast<gfx::ImageStorageDimension>(texture_info->image_storage_dimension)),
            .format                = vp::res::GfxImageFormatToVkFormat(static_cast<gfx::ImageFormat>(texture_info->image_format)),
            .extent = {
                .width             = texture_info->width,
                .height            = texture_info->height,
                .depth             = texture_info->depth
            },
            .mipLevels             = texture_info->mip_level_count,
            .arrayLayers           = texture_info->array_layer_count,
            .samples               = static_cast<VkSampleCountFlagBits>(texture_info->sample_count),
            .tiling                = vp::res::GfxTileModeToVkImageTiling(static_cast<gfx::TileMode>(texture_info->tile_mode)),
            .usage                 = image_usage_flags,
            .sharingMode           = (0 < Context::GetInstance()->GetQueueFamilyCount()) ? VK_SHARING_MODE_CONCURRENT : VK_SHARING_MODE_EXCLUSIVE,
            .queueFamilyIndexCount = Context::GetInstance()->GetQueueFamilyCount(),
            .pQueueFamilyIndices   = Context::GetInstance()->GetQueueFamilyIndiceArray(),
            .initialLayout         = VK_IMAGE_LAYOUT_UNDEFINED,
        };
        const u32 result0 = ::pfn_vkCreateImage(Context::GetInstance()->GetVkDevice(), std::addressof(image_info), Context::GetInstance()->GetVkAllocationCallbacks(), std::addressof(image));
        if (result0 != VK_SUCCESS) { return VK_NULL_HANDLE; }

        return image;
    }

	VkImage  CreateVkImage(void *gpu_address, const TextureInfo *texture_info) {
        return CreateVkImage(gpu_address, vp::res::GfxGpuAccessFlagsToVkImageUsageFlags(static_cast<vp::res::GfxGpuAccessFlags>(texture_info->gpu_access_flags)), texture_info, nullptr);
    }

	VkImage  CreateVkImage(void *gpu_address, const TextureInfo *texture_info, const void *pnext) {
        return CreateVkImage(gpu_address, vp::res::GfxGpuAccessFlagsToVkImageUsageFlags(static_cast<vp::res::GfxGpuAccessFlags>(texture_info->gpu_access_flags)), texture_info, pnext);
    }

    VkImage CreateVkImage(void *gpu_address, VkImageUsageFlags image_usage_flags, const gfx::TextureInfo *texture_info) {
        return CreateVkImage(gpu_address, image_usage_flags, texture_info, nullptr);
    }

    VkImage CreateVkImage(void *gpu_address, VkImageUsageFlags image_usage_flags, const gfx::TextureInfo *texture_info, const void *pnext) {

        /* Ensure address */
        if (gpu_address == nullptr) { return VK_NULL_HANDLE; }

        /* Create VkImage */
        VkImage image = CreateVkImage(image_usage_flags, texture_info, pnext);
        if (image == VK_NULL_HANDLE) { return VK_NULL_HANDLE; }

        /* Bind gpu memory to image from offset */
        mem::GpuRootHeapContext *gpu_root_heap_context = mem::FindGpuRootHeapContextFromAddress(gpu_address);
        const u32 result1 = ::pfn_vkBindImageMemory(Context::GetInstance()->GetVkDevice(), image, gpu_root_heap_context->gpu_memory_handle.vk_device_memory, gpu_root_heap_context->GetMemoryOffset(gpu_address));
        if (result1 != VK_SUCCESS) { DestroyVkImage(image); return VK_NULL_HANDLE; }

        return image;
    }

    VkSampler CreateVkSampler(const SamplerInfo *sampler_info) {

        /* Ensure sampler info */
        if (sampler_info == nullptr) { return VK_NULL_HANDLE; }

        /* Create VkSampler */
        VkSampler vk_sampler = VK_NULL_HANDLE;
        const VkSamplerReductionModeCreateInfo reduction_mode_info = {
            .sType         = VK_STRUCTURE_TYPE_SAMPLER_REDUCTION_MODE_CREATE_INFO,
            .reductionMode = vp::res::GfxReductionFilterToVkSamplerReductionMode(static_cast<ReductionFilter>(sampler_info->reduction_filter))
        };
        const VkSamplerCreateInfo sampler_create_info = {
            .sType            = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
            .pNext            = std::addressof(reduction_mode_info),
            .magFilter        = vp::res::GfxMagFilterToVkFilter(static_cast<MagFilter>(sampler_info->mag_filter)),
            .minFilter        = vp::res::GfxMinFilterToVkFilter(static_cast<MinFilter>(sampler_info->min_filter)),
            .mipmapMode       = vp::res::GfxMipMapFilterToVkSamplerMipMapMode(static_cast<MipMapFilter>(sampler_info->mip_map_filter)),
            .addressModeU     = vp::res::GfxWrapModeToVkSamplerAddressMode(static_cast<WrapMode>(sampler_info->wrap_mode_u)),
            .addressModeV     = vp::res::GfxWrapModeToVkSamplerAddressMode(static_cast<WrapMode>(sampler_info->wrap_mode_v)),
            .addressModeW     = vp::res::GfxWrapModeToVkSamplerAddressMode(static_cast<WrapMode>(sampler_info->wrap_mode_w)),
            .mipLodBias       = sampler_info->lod_bias,
            .anisotropyEnable = sampler_info->is_enable_anisotropy,
            .maxAnisotropy    = static_cast<float>(sampler_info->max_anisotropy),
            .compareEnable    = true,
            .compareOp        = vp::res::GfxCompareOperationToVkCompareOp(static_cast<CompareOperation>(sampler_info->compare_op)),
            .minLod           = sampler_info->lod_clamp_min,
            .maxLod           = sampler_info->lod_clamp_max,
            .borderColor      = vp::res::GfxBorderColorToVkBorderColor(static_cast<BorderColor>(sampler_info->border_color)),
        };
        const u32 result = ::pfn_vkCreateSampler(Context::GetInstance()->GetVkDevice(), std::addressof(sampler_create_info), Context::GetInstance()->GetVkAllocationCallbacks(), std::addressof(vk_sampler));
        if (result != VK_SUCCESS) { return VK_NULL_HANDLE; }

        return vk_sampler;
    }

    GpuMemoryRequirements GetVkImageMemoryRequirements(VkImage vk_image) {

        const VkImageMemoryRequirementsInfo2 memory_requirement_info = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_REQUIREMENTS_INFO_2,
            .image = vk_image,
        };
        VkMemoryRequirements2 memory_requirements_2 = {
            .sType = VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2,
        };
        ::pfn_vkGetImageMemoryRequirements2(Context::GetInstance()->GetVkDevice(), std::addressof(memory_requirement_info), std::addressof(memory_requirements_2));

        return { memory_requirements_2.memoryRequirements.size, memory_requirements_2.memoryRequirements.alignment };
    }

    VkImageView CreateVkImageView(const TextureViewInfo *texture_view_info) {

        /* Ensure texture view info */
        if (texture_view_info == nullptr) { return VK_NULL_HANDLE; }

        /* Create VkImageView */
        VkImageView vk_image_view = VK_NULL_HANDLE;
        const VkImageViewCreateInfo image_view_info = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .image = texture_view_info->texture->GetVkImage(),
            .viewType = vp::res::GfxImageDimensionToVkImageViewType(static_cast<ImageDimension>(texture_view_info->image_dimension)),
            .format   = vp::res::GfxImageFormatToVkFormat(static_cast<ImageFormat>(texture_view_info->image_format)),
            .components = {
                .r = vp::res::GfxTextureSwizzleToVkComponentSwizzle(static_cast<TextureSwizzle>(texture_view_info->swizzle_x)),
                .g = vp::res::GfxTextureSwizzleToVkComponentSwizzle(static_cast<TextureSwizzle>(texture_view_info->swizzle_y)),
                .b = vp::res::GfxTextureSwizzleToVkComponentSwizzle(static_cast<TextureSwizzle>(texture_view_info->swizzle_z)),
                .a = vp::res::GfxTextureSwizzleToVkComponentSwizzle(static_cast<TextureSwizzle>(texture_view_info->swizzle_w)),
            },
            .subresourceRange = {
                .aspectMask     = vp::res::GfxTextureViewInfoToVkImageAspectFlags(texture_view_info),
                .baseMipLevel   = texture_view_info->base_mip_level,
                .levelCount     = texture_view_info->mip_level_count,
                .baseArrayLayer = texture_view_info->base_array_layer,
                .layerCount     = texture_view_info->array_layer_count,
            }
        };
        const u32 result = ::pfn_vkCreateImageView(Context::GetInstance()->GetVkDevice(), std::addressof(image_view_info), Context::GetInstance()->GetVkAllocationCallbacks(), std::addressof(vk_image_view));
        if (result != VK_SUCCESS) { return VK_NULL_HANDLE; }

        return vk_image_view;
    }

    void DestroyVkBuffer(VkBuffer vk_buffer) {
        if (vk_buffer == VK_NULL_HANDLE) { return; }
        Context *context = Context::GetInstance();
        ::pfn_vkDestroyBuffer(context->GetVkDevice(), vk_buffer, context->GetVkAllocationCallbacks());
    }

    void DestroyVkImage(VkImage vk_image) {
        if (vk_image == VK_NULL_HANDLE) { return; }
        Context *context = Context::GetInstance();
        ::pfn_vkDestroyImage(context->GetVkDevice(), vk_image, context->GetVkAllocationCallbacks());
    }

    void DestroyVkImageView(VkImageView vk_image_view) {
        if (vk_image_view == VK_NULL_HANDLE) { return; }
        Context *context = Context::GetInstance();
        ::pfn_vkDestroyImageView(context->GetVkDevice(), vk_image_view, context->GetVkAllocationCallbacks());
    }

    void DestroyVkSampler(VkSampler vk_sampler) {
        if (vk_sampler == VK_NULL_HANDLE) { return; }
        Context *context = Context::GetInstance();
        ::pfn_vkDestroySampler(context->GetVkDevice(), vk_sampler, context->GetVkAllocationCallbacks());
    }

    GpuBufferAddress GetGpuBufferAddress(VkBuffer vk_buffer) {
        const VkBufferDeviceAddressInfo address_info = {
            .sType  = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO_KHR,
            .buffer = vk_buffer,
        };
        return { ::pfn_vkGetBufferDeviceAddress(Context::GetInstance()->GetVkDevice(), std::addressof(address_info)), vk_buffer, };
    }
}
