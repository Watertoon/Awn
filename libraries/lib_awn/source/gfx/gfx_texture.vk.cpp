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

namespace awn::gfx {

    Result Texture::Initialize(mem::GpuMemoryHandle memory_handle, size_t memory_offset, const TextureInfo *texture_info) {

        /* Create image by gpu memory address */
        RESULT_TRY(gfx::impl::CreateVkImage(std::addressof(m_vk_image), memory_handle, memory_offset, texture_info));

        /* Transition to general */
        const VkHostImageLayoutTransitionInfo trans_info = {
            .sType = VK_STRUCTURE_TYPE_HOST_IMAGE_LAYOUT_TRANSITION_INFO,
            .image = m_vk_image,
            .oldLayout = VK_IMAGE_LAYOUT_PREINITIALIZED,
            .newLayout = VK_IMAGE_LAYOUT_GENERAL,
            .subresourceRange = {
                .aspectMask = this->GetVkAspectMask(),
                .baseMipLevel = 0,
                .levelCount = VK_REMAINING_MIP_LEVELS,
                .baseArrayLayer = 0,
                .layerCount = VK_REMAINING_ARRAY_LAYERS,
            },
        };
        const VkResult vk_result = ::pfn_vkTransitionImageLayout(impl::Context::GetInstance()->GetVkDevice(), 1, std::addressof(trans_info));
        if (vk_result != VK_SUCCESS) {
            this->Finalize();
            RESULT_RETURN(impl::ConvertVkResultToResult(vk_result));
        }

        /* Save texture meta data */
        m_texture_info = *texture_info;

        RESULT_RETURN_SUCCESS;
    }

    Result Texture::Initialize(void *gpu_memory_address, const TextureInfo *texture_info) {

        /* Create image by gpu memory address */
        RESULT_TRY(gfx::impl::CreateVkImage(std::addressof(m_vk_image), gpu_memory_address, texture_info));

        /* Transition to general */
        const VkHostImageLayoutTransitionInfo trans_info = {
            .sType = VK_STRUCTURE_TYPE_HOST_IMAGE_LAYOUT_TRANSITION_INFO,
            .image = m_vk_image,
            .oldLayout = VK_IMAGE_LAYOUT_PREINITIALIZED,
            .newLayout = VK_IMAGE_LAYOUT_GENERAL,
            .subresourceRange = {
                .aspectMask = this->GetVkAspectMask(),
                .baseMipLevel = 0,
                .levelCount = VK_REMAINING_MIP_LEVELS,
                .baseArrayLayer = 0,
                .layerCount = VK_REMAINING_ARRAY_LAYERS,
            },
        };
        const VkResult vk_result = ::pfn_vkTransitionImageLayout(impl::Context::GetInstance()->GetVkDevice(), 1, std::addressof(trans_info));
        if (vk_result != VK_SUCCESS) {
            this->Finalize();
            RESULT_RETURN(impl::ConvertVkResultToResult(vk_result));
        }

        /* Save texture meta data */
        m_texture_info = *texture_info;

        RESULT_RETURN_SUCCESS;
    }

    Result Texture::InitializeForDedicatedAllocation(const TextureInfo *texture_info, VkImageUsageFlags vk_image_usage_flags, void *pnext) {

        /* Integrity check */
        VP_ASSERT(texture_info != nullptr);

        /* Create image by gpu memory address */
        VkImageUsageFlags flags = vk_image_usage_flags | vp::res::GfxGpuAccessFlagsToVkImageUsageFlags(static_cast<vp::res::GfxGpuAccessFlags>(texture_info->gpu_access_flags));
        RESULT_TRY(gfx::impl::CreateVkImage(std::addressof(m_vk_image), flags, texture_info, pnext, false));

        m_texture_info = *texture_info;

        RESULT_RETURN_SUCCESS;
    }

    void Texture::Finalize() {

        /* Delete VkImage */
        if (m_vk_image != VK_NULL_HANDLE) {
            gfx::impl::DestroyVkImage(m_vk_image);
            m_vk_image = VK_NULL_HANDLE;
        }

        return;
    }

    void Texture::CopyMemoryToTexture(const void * const*memory_array, const TextureCopyRegion *copy_region_array, u32 copy_region_count, bool is_no_conversion) {
        VkMemoryToImageCopy memory_to_image_array[0x10];
        for (u32 i = 0; i < copy_region_count; ++i) {
            memory_to_image_array[i].sType = VK_STRUCTURE_TYPE_MEMORY_TO_IMAGE_COPY;
            memory_to_image_array[i].pNext = nullptr;
            memory_to_image_array[i].pHostPointer = memory_array[i];
            memory_to_image_array[i].memoryRowLength = 0;
            memory_to_image_array[i].memoryImageHeight = 0;
            memory_to_image_array[i].imageSubresource.aspectMask = this->GetVkAspectMask();
            memory_to_image_array[i].imageSubresource.mipLevel = copy_region_array[i].base_mip_level;
            memory_to_image_array[i].imageSubresource.baseArrayLayer = copy_region_array[i].base_array_layer;
            memory_to_image_array[i].imageSubresource.layerCount = copy_region_array[i].layer_count;
            memory_to_image_array[i].imageOffset.x = copy_region_array[i].base_x;
            memory_to_image_array[i].imageOffset.y = copy_region_array[i].base_y;
            memory_to_image_array[i].imageOffset.z = copy_region_array[i].base_z;
            memory_to_image_array[i].imageExtent.width = copy_region_array[i].width;
            memory_to_image_array[i].imageExtent.height = copy_region_array[i].height;
            memory_to_image_array[i].imageExtent.depth = copy_region_array[i].depth;
            
        }
        const VkCopyMemoryToImageInfo copy_info = {
            .sType = VK_STRUCTURE_TYPE_COPY_MEMORY_TO_IMAGE_INFO,
            .flags = (is_no_conversion) ? static_cast<u32>(VK_HOST_IMAGE_COPY_MEMCPY) : 0,
            .dstImage = m_vk_image,
            .dstImageLayout = VK_IMAGE_LAYOUT_GENERAL,
            .regionCount = copy_region_count,
            .pRegions = memory_to_image_array,
        };
        const VkResult vk_result = ::pfn_vkCopyMemoryToImage(impl::Context::GetInstance()->GetVkDevice(), std::addressof(copy_info));
        RESULT_ABORT_UNLESS(impl::ConvertVkResultToResult(vk_result));
        return;
    }

    void Texture::CopyTextureToMemory(void **out_memory_array, const TextureCopyRegion *copy_region_array, u32 copy_region_count, bool is_no_conversion) {
        VkImageToMemoryCopy image_to_memory_array[0x10];
        for (u32 i = 0; i < copy_region_count; ++i) {
            image_to_memory_array[i].sType = VK_STRUCTURE_TYPE_MEMORY_TO_IMAGE_COPY;
            image_to_memory_array[i].pNext = nullptr;
            image_to_memory_array[i].pHostPointer = out_memory_array[i];
            image_to_memory_array[i].memoryRowLength = 0;
            image_to_memory_array[i].memoryImageHeight = 0;
            image_to_memory_array[i].imageSubresource.aspectMask = this->GetVkAspectMask();
            image_to_memory_array[i].imageSubresource.mipLevel = copy_region_array[i].base_mip_level;
            image_to_memory_array[i].imageSubresource.baseArrayLayer = copy_region_array[i].base_array_layer;
            image_to_memory_array[i].imageSubresource.layerCount = copy_region_array[i].layer_count;
            image_to_memory_array[i].imageOffset.x = copy_region_array[i].base_x;
            image_to_memory_array[i].imageOffset.y = copy_region_array[i].base_y;
            image_to_memory_array[i].imageOffset.z = copy_region_array[i].base_z;
            image_to_memory_array[i].imageExtent.width = copy_region_array[i].width;
            image_to_memory_array[i].imageExtent.height = copy_region_array[i].height;
            image_to_memory_array[i].imageExtent.depth = copy_region_array[i].depth;
            
        }
        const VkCopyImageToMemoryInfo copy_info = {
            .sType = VK_STRUCTURE_TYPE_COPY_IMAGE_TO_MEMORY_INFO,
            .flags = (is_no_conversion) ? static_cast<u32>(VK_HOST_IMAGE_COPY_MEMCPY) : 0,
            .srcImage = m_vk_image,
            .srcImageLayout = VK_IMAGE_LAYOUT_GENERAL,
            .regionCount = copy_region_count,
            .pRegions = image_to_memory_array,
        };
        const VkResult vk_result = ::pfn_vkCopyImageToMemory(impl::Context::GetInstance()->GetVkDevice(), std::addressof(copy_info));
        RESULT_ABORT_UNLESS(impl::ConvertVkResultToResult(vk_result));
        return;
    }

    //void Texture::CopyTextureToTexture(Texture *out_texture, const TextureCopyRegion *dest_copy_region_array, const TextureCopyRegion *source_copy_region_array, u32 copy_region_count, bool is_no_conversion = false) {
    //    const VkImageCopy2 image_copy_array[0x10];
    //    for (u32 i = 0; i < copy_region_count; ++i) {
    //        image_copy_array[i].sType = VK_STRUCTURE_TYPE_MEMORY_TO_IMAGE_COPY;
    //        image_copy_array[i].pNext = nullptr;
    //        image_copy_array[i].pHostPointer = out_memory_array[i];
    //        image_copy_array[i].srcSubresource.aspectMask = this->GetVkAspectMask();
    //        image_copy_array[i].srcSubresource.mipLevel = source_copy_region_array[i].base_mip_level;
    //        image_copy_array[i].srcSubresource.baseArrayLayer = source_copy_region_array[i].base_array_layer;
    //        image_copy_array[i].srcSubresource.layerCount = source_copy_region_array[i].layer_count;
    //        image_copy_array[i].srcOffset.x = source_copy_region_array[i].base_x;
    //        image_copy_array[i].srcOffset.y = source_copy_region_array[i].base_y;
    //        image_copy_array[i].srcOffset.z = source_copy_region_array[i].base_z;
    //        image_copy_array[i].dstSubresource.aspectMask = texture->GetVkAspectMask();
    //        image_copy_array[i].dstSubresource.mipLevel = dest_copy_region_array[i].base_mip_level;
    //        image_copy_array[i].dstSubresource.baseArrayLayer = dest_copy_region_array[i].base_array_layer;
    //        image_copy_array[i].dstSubresource.layerCount = dest_copy_region_array[i].layer_count;
    //        image_copy_array[i].dstOffset.x = dest_copy_region_array[i].base_x;
    //        image_copy_array[i].dstOffset.y = dest_copy_region_array[i].base_y;
    //        image_copy_array[i].dstOffset.z = dest_copy_region_array[i].base_z;
    //        image_copy_array[i].extent.width = vp::util::Min(dest_copy_region_array[i].width, source_copy_region_array[i].width);
    //        image_copy_array[i].extent.height = vp::util::Min(dest_copy_region_array[i].height, source_copy_region_array[i].height);
    //        image_copy_array[i].extent.depth = vp::util::Min(dest_copy_region_array[i].depth, source_copy_region_array[i].depth);
    //    }
    //    const VkCopyImageToMemoryInfo copy_info = {
    //        .sType = VK_STRUCTURE_TYPE_COPY_IMAGE_TO_MEMORY_INFO,
    //        .flags = (is_no_conversion) ? VK_HOST_IMAGE_COPY_MEMCPY : 0,
    //        .srcImage = m_vk_image,
    //        .srcImageLayout = VK_IMAGE_LAYOUT_GENERAL,
    //        .dstImage = texture->GetVkImage(),
    //        .dstImageLayout = VK_IMAGE_LAYOUT_GENERAL,
    //        .regionCount = copy_region_count,
    //        .pRegions = image_copy_array,
    //    };
    //    const VkResult result = ::pfn_vkCopyImageToImage(impl::Context::GetInstance()->GetVkDevice(), std::addressof(copy_info));
    //    VP_ASSERT(result == VK_SUCCESS);
    //    return;
    //}

    GpuMemoryRequirements Texture::GetGpuMemoryRequirements(const TextureInfo *texture_info) {

        /* Get memory requirements */
        const VkImageCreateInfo image_info = {
            .sType                 = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
            .flags                 = vp::res::ResGfxTextureInfoToVkImageCreateFlags(texture_info),
            .imageType             = vp::res::GfxImageStorageDimensionToVkImageType(static_cast<ImageStorageDimension>(texture_info->image_storage_dimension)),
            .format                = vp::res::GfxImageFormatToVkFormat(static_cast<ImageFormat>(texture_info->image_format)),
            .extent = {
                .width             = texture_info->width,
                .height            = texture_info->height,
                .depth             = texture_info->depth
            },
            .mipLevels             = texture_info->mip_level_count,
            .arrayLayers           = texture_info->array_layer_count,
            .samples               = static_cast<VkSampleCountFlagBits>(texture_info->sample_count),
            .tiling                = static_cast<VkImageTiling>(texture_info->tile_mode),
            .usage                 = vp::res::GfxGpuAccessFlagsToVkImageUsageFlags(static_cast<vp::res::GfxGpuAccessFlags>(texture_info->gpu_access_flags)) | VK_IMAGE_USAGE_HOST_TRANSFER_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
            .sharingMode           = VK_SHARING_MODE_CONCURRENT,
            .queueFamilyIndexCount = impl::Context::GetInstance()->GetQueueFamilyCount(),
            .pQueueFamilyIndices   = impl::Context::GetInstance()->GetQueueFamilyIndiceArray(),
            .initialLayout         = VK_IMAGE_LAYOUT_UNDEFINED
        };
        const VkDeviceImageMemoryRequirements memory_requirement_info = {
            .sType       = VK_STRUCTURE_TYPE_DEVICE_IMAGE_MEMORY_REQUIREMENTS,
            .pCreateInfo = std::addressof(image_info),
        };
        VkMemoryRequirements2 memory_requirements_2 = {
            .sType = VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2,
        };
        ::pfn_vkGetDeviceImageMemoryRequirements(impl::Context::GetInstance()->GetVkDevice(), std::addressof(memory_requirement_info), std::addressof(memory_requirements_2));

        return { memory_requirements_2.memoryRequirements.size, static_cast<s32>(memory_requirements_2.memoryRequirements.alignment), };
    }
}
