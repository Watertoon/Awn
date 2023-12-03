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

    VkBuffer CreateVkBuffer(void *gpu_address, VkBufferUsageFlags usage_flags, size_t size) {

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
        mem::GpuRootHeapContext *gpu_root_heap_context = mem::FindGpuRootHeapContextFromAddress(gpu_address);
        const u32 result1 = ::pfn_vkBindBufferMemory(gfx::Context::GetInstance()->GetVkDevice(), buffer, gpu_root_heap_context->vk_device_memory, gpu_root_heap_context->GetVkDeviceMemoryOffset(gpu_address));
        VP_ASSERT(result1 == VK_SUCCESS);
        
        return buffer;
    }

    VkImage CreateVkImage(void *gpu_address, VkImageUsageFlags image_usage_flags, gfx::TextureInfo *texture_info) {

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
        mem::GpuRootHeapContext *gpu_root_heap_context = mem::FindGpuRootHeapContextFromAddress(gpu_address);
        const u32 result1 = ::pfn_vkBindImageMemory(gfx::Context::GetInstance()->GetVkDevice(), image, gpu_root_heap_context->vk_device_memory, gpu_root_heap_context->GetVkDeviceMemoryOffset(gpu_address));
        VP_ASSERT(result1 == VK_SUCCESS);

        return image;
    }

	VkBuffer CreateVkBuffer(void *gpu_address, BufferInfo *buffer_info) {
        return CreateVkBuffer(gpu_address, vp::res::GfxGpuAccessFlagsToVkBufferUsageFlags(static_cast<vp::res::GfxGpuAccessFlags>(buffer_info->gpu_access_flags)), buffer_info->size);
    }
	VkImage  CreateVkImage(void *gpu_address, TextureInfo *texture_info) {
        return CreateVkImage(gpu_address, vp::res::GfxGpuAccessFlagsToVkImageUsageFlags(static_cast<vp::res::GfxGpuAccessFlags>(texture_info->gpu_access_flags)), texture_info);
    }

    void FlushCpuCache(void *gpu_address, size_t size) {
        mem::FindGpuRootHeapContextFromAddress(gpu_address)->FlushCpuCache(gpu_address, size);
    }

    void InvalidateCpuCache(void *gpu_address, size_t size) {
        mem::FindGpuRootHeapContextFromAddress(gpu_address)->InvalidateCpuCache(gpu_address, size);
    }
}
