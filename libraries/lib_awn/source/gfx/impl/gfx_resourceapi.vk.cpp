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

	Result CreateVkBuffer(VkBuffer *out_buffer, const BufferInfo *buffer_info) {
        return CreateVkBuffer(out_buffer, vp::res::GfxGpuAccessFlagsToVkBufferUsageFlags(static_cast<vp::res::GfxGpuAccessFlags>(buffer_info->gpu_access_flags)), buffer_info->size);
    }

    Result CreateVkBuffer(VkBuffer *out_buffer, VkBufferUsageFlags usage_flags, size_t size) {

        /* Create VkBuffer */
        VkBuffer buffer = VK_NULL_HANDLE;
        const VkBufferCreateInfo vk_buffer_info = {
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .size  = size,
            .usage = static_cast<u32>(usage_flags) | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT
        };
        const VkResult vk_result = ::pfn_vkCreateBuffer(Context::GetInstance()->GetVkDevice(), std::addressof(vk_buffer_info), Context::GetInstance()->GetVkAllocationCallbacks(), std::addressof(buffer));
        const Result result = ConvertVkResultToResult(vk_result);
        RESULT_TRY(result);
        *out_buffer = buffer;

        RESULT_RETURN_SUCCESS;
    }

	Result CreateVkBuffer(VkBuffer *out_buffer, void *gpu_address, const BufferInfo *buffer_info) {
        return CreateVkBuffer(out_buffer, gpu_address, vp::res::GfxGpuAccessFlagsToVkBufferUsageFlags(static_cast<vp::res::GfxGpuAccessFlags>(buffer_info->gpu_access_flags)), buffer_info->size);
    }

    Result CreateVkBuffer(VkBuffer *out_buffer, void *gpu_address, VkBufferUsageFlags usage_flags, size_t size) {

        /* Ensure address */
        size_t offset;
        MemoryPool *memory_pool = gfx::FindMemoryPool(std::addressof(offset), gpu_address);

        /* Create VkBuffer */
        return CreateVkBuffer(out_buffer, memory_pool, offset, usage_flags, size);
    }

	Result CreateVkBuffer(VkBuffer *out_buffer, MemoryPool *memory_pool, size_t memory_offset, const BufferInfo *buffer_info) {
        return CreateVkBuffer(out_buffer, memory_pool, memory_offset, vp::res::GfxGpuAccessFlagsToVkBufferUsageFlags(static_cast<vp::res::GfxGpuAccessFlags>(buffer_info->gpu_access_flags)), buffer_info->size);
    }

    Result CreateVkBuffer(VkBuffer *out_buffer, MemoryPool *memory_pool, size_t memory_offset, VkBufferUsageFlags usage_flags, size_t size) {

        /* Ensure memory handle */
        RESULT_RETURN_IF(!memory_pool->IsValid(), ResultInvalidMemoryPool);

        /* Create VkBuffer */
        RESULT_TRY(CreateVkBuffer(out_buffer, usage_flags, size));

        /* Bind gpu memory allocation to buffer from offset */
        const VkResult vk_result = ::pfn_vkBindBufferMemory(Context::GetInstance()->GetVkDevice(), *out_buffer, memory_pool->GetVkDeviceMemory(), memory_offset);
        const Result result = ConvertVkResultToResult(vk_result);
        if (RESULT_FAILED(result)) {
            DestroyVkBuffer(*out_buffer);
            *out_buffer = VK_NULL_HANDLE;
            RESULT_RETURN(result);
        }

        RESULT_RETURN_SUCCESS;
    }

	Result CreateVkImage(VkImage *out_image, const TextureInfo *texture_info) {
        return CreateVkImage(out_image, texture_info, nullptr);
    }

	Result CreateVkImage(VkImage *out_image, const TextureInfo *texture_info, const void *pnext) {
        const VkImageUsageFlags flags = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_HOST_TRANSFER_BIT | vp::res::GfxGpuAccessFlagsToVkImageUsageFlags(static_cast<vp::res::GfxGpuAccessFlags>(texture_info->gpu_access_flags));
        return CreateVkImage(out_image, flags, texture_info, pnext);
    }

    Result CreateVkImage(VkImage *out_image, VkImageUsageFlags image_usage_flags, const gfx::TextureInfo *texture_info) {
        return CreateVkImage(out_image, image_usage_flags, texture_info, nullptr);
    }

    Result CreateVkImage(VkImage *out_image, VkImageUsageFlags image_usage_flags, const gfx::TextureInfo *texture_info, const void *pnext) {
        return CreateVkImage(out_image, image_usage_flags, texture_info, pnext, true);
    }

    Result CreateVkImage(VkImage *out_image, VkImageUsageFlags image_usage_flags, const gfx::TextureInfo *texture_info, const void *pnext, bool is_preinit) {

        /* Ensure texture info */
        RESULT_RETURN_IF(texture_info == nullptr, ResultNullArgument);

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
            .initialLayout         = (is_preinit) ? VK_IMAGE_LAYOUT_PREINITIALIZED : VK_IMAGE_LAYOUT_UNDEFINED,
        };
        const VkResult vk_result = ::pfn_vkCreateImage(Context::GetInstance()->GetVkDevice(), std::addressof(image_info), Context::GetInstance()->GetVkAllocationCallbacks(), std::addressof(image));
        const Result result = ConvertVkResultToResult(vk_result);
        RESULT_TRY(result);
        *out_image = image;

        RESULT_RETURN_SUCCESS;
    }

	Result CreateVkImage(VkImage *out_image, void *gpu_address, const TextureInfo *texture_info) {
        return CreateVkImage(out_image, gpu_address, texture_info, nullptr);
    }

	Result CreateVkImage(VkImage *out_image, void *gpu_address, const TextureInfo *texture_info, const void *pnext) {
        const VkImageUsageFlags flags = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_HOST_TRANSFER_BIT | vp::res::GfxGpuAccessFlagsToVkImageUsageFlags(static_cast<vp::res::GfxGpuAccessFlags>(texture_info->gpu_access_flags));
        return CreateVkImage(out_image, gpu_address, flags, texture_info, pnext);
    }

    Result CreateVkImage(VkImage *out_image, void *gpu_address, VkImageUsageFlags image_usage_flags, const gfx::TextureInfo *texture_info) {
        return CreateVkImage(out_image, gpu_address, image_usage_flags, texture_info, nullptr);
    }

    Result CreateVkImage(VkImage *out_image, void *gpu_address, VkImageUsageFlags image_usage_flags, const gfx::TextureInfo *texture_info, const void *pnext) {

        /* Get handle by address */
        size_t offset;
        MemoryPool *memory_pool = gfx::FindMemoryPool(std::addressof(offset), gpu_address);

        /* Create VkImage */
        return CreateVkImage(out_image, memory_pool, offset, image_usage_flags, texture_info, pnext);
    }

	Result CreateVkImage(VkImage *out_image, MemoryPool *memory_pool, size_t offset, const TextureInfo *texture_info) {
        return CreateVkImage(out_image, memory_pool, offset, texture_info, nullptr);
    }

	Result CreateVkImage(VkImage *out_image, MemoryPool *memory_pool, size_t offset, const TextureInfo *texture_info, const void *pnext) {
        const VkImageUsageFlags flags = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_HOST_TRANSFER_BIT | vp::res::GfxGpuAccessFlagsToVkImageUsageFlags(static_cast<vp::res::GfxGpuAccessFlags>(texture_info->gpu_access_flags));
        return CreateVkImage(out_image, memory_pool, offset, flags, texture_info, pnext);
    }

    Result CreateVkImage(VkImage *out_image, MemoryPool *memory_pool, size_t offset, VkImageUsageFlags image_usage_flags, const gfx::TextureInfo *texture_info) {
        return CreateVkImage(out_image, memory_pool, offset, image_usage_flags, texture_info, nullptr);
    }

	Result CreateVkImage(VkImage *out_image, MemoryPool *memory_pool, size_t offset, VkImageUsageFlags image_usage_flags, const gfx::TextureInfo *texture_info, const void *pnext)  {

        /* Ensure memory handle */
        RESULT_RETURN_IF(!memory_pool->IsValid(), ResultInvalidMemoryPool);

        /* Create VkImage */
        RESULT_TRY(CreateVkImage(out_image, image_usage_flags, texture_info, pnext));

        /* Bind gpu memory to image from offset */
        const VkResult vk_result = ::pfn_vkBindImageMemory(Context::GetInstance()->GetVkDevice(), *out_image, memory_pool->GetVkDeviceMemory(), offset);
        const Result result = ConvertVkResultToResult(vk_result);
        if (RESULT_FAILED(result)) {
            DestroyVkImage(*out_image);
            *out_image = VK_NULL_HANDLE;
            RESULT_RETURN(result);
        }

        RESULT_RETURN_SUCCESS;
    }

    Result CreateVkSampler(VkSampler *out_sampler, const SamplerInfo *sampler_info) {

        /* Ensure sampler info */
        RESULT_RETURN_IF(sampler_info == nullptr, ResultNullArgument);

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
        const VkResult vk_result = ::pfn_vkCreateSampler(Context::GetInstance()->GetVkDevice(), std::addressof(sampler_create_info), Context::GetInstance()->GetVkAllocationCallbacks(), std::addressof(vk_sampler));
        const Result result = ConvertVkResultToResult(vk_result);
        RESULT_TRY(result);
        *out_sampler = vk_sampler;
        RESULT_RETURN_SUCCESS;
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

        return { memory_requirements_2.memoryRequirements.size, static_cast<s32>(memory_requirements_2.memoryRequirements.alignment), };
    }

    Result CreateVkImageView(VkImageView *out_image_view, const TextureViewInfo *texture_view_info) {

        /* Ensure texture view info */
        RESULT_RETURN_IF(texture_view_info == nullptr, ResultNullArgument);

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
        const VkResult vk_result = ::pfn_vkCreateImageView(Context::GetInstance()->GetVkDevice(), std::addressof(image_view_info), Context::GetInstance()->GetVkAllocationCallbacks(), std::addressof(vk_image_view));
        const Result result = ConvertVkResultToResult(vk_result);
        RESULT_TRY(result);
        *out_image_view = vk_image_view;
        RESULT_RETURN_SUCCESS;
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

    GpuAddress GetGpuAddress(VkBuffer vk_buffer) {
        const VkBufferDeviceAddressInfo address_info = {
            .sType  = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO_KHR,
            .buffer = vk_buffer,
        };
        return { ::pfn_vkGetBufferDeviceAddress(Context::GetInstance()->GetVkDevice(), std::addressof(address_info)), };
    }
}
