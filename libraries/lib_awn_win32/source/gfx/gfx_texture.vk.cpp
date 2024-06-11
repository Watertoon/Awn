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

    bool Texture::Initialize(void *gpu_memory_address, const TextureInfo *texture_info) {

        /* Create image by gpu memory address */
        m_vk_image = gfx::CreateVkImage(gpu_memory_address, texture_info);
        if (m_vk_image == nullptr) { return false; }

        m_texture_info = *texture_info;

        return true;
    }

    bool Texture::InitializeForDedicatedAllocation(const TextureInfo *texture_info) {

        /* Integrity check */
        VP_ASSERT(texture_info != nullptr);

        /* Create image by gpu memory address */
        m_vk_image = gfx::CreateVkImage(texture_info);
        if (m_vk_image == nullptr) { return false; }

        m_texture_info = *texture_info;

        return true;
    }

    void Texture::Finalize() {

        /* Delete VkImage */
        if (m_vk_image != VK_NULL_HANDLE) {
            ::pfn_vkDestroyImage(Context::GetInstance()->GetVkDevice(), m_vk_image, Context::GetInstance()->GetVkAllocationCallbacks());
            m_vk_image = VK_NULL_HANDLE;
        }

        return;
    }

    GpuMemoryRequirements Texture::GetMemoryRequirements(const TextureInfo *texture_info) {

        /* Get memory requirements */
        const VkImageCreateInfo image_info = {
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
            .arrayLayers           = texture_info->array_layer_count,
            .samples               = static_cast<VkSampleCountFlagBits>(texture_info->sample_count),
            .tiling                = static_cast<VkImageTiling>(texture_info->tile_mode),
            .usage                 = vp::res::GfxGpuAccessFlagsToVkImageUsageFlags(static_cast<vp::res::GfxGpuAccessFlags>(texture_info->gpu_access_flags)),
            .sharingMode           = VK_SHARING_MODE_CONCURRENT,
            .queueFamilyIndexCount = Context::GetInstance()->GetQueueFamilyCount(),
            .pQueueFamilyIndices   = Context::GetInstance()->GetQueueFamilyIndiceArray(),
            .initialLayout         = VK_IMAGE_LAYOUT_UNDEFINED
        };
        const VkDeviceImageMemoryRequirements memory_requirement_info = {
            .sType       = VK_STRUCTURE_TYPE_DEVICE_IMAGE_MEMORY_REQUIREMENTS,
            .pCreateInfo = std::addressof(image_info),
        };
        VkMemoryRequirements2 memory_requirements_2 = {
            .sType = VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2,
        };
        ::pfn_vkGetDeviceImageMemoryRequirements(Context::GetInstance()->GetVkDevice(), std::addressof(memory_requirement_info), std::addressof(memory_requirements_2));

        return { memory_requirements_2.memoryRequirements.size, memory_requirements_2.memoryRequirements.alignment };
    }
}
