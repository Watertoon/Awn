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
#pragma once

namespace awn::gfx {

	VkBuffer CreateVkBuffer(void *gpu_address, const BufferInfo *buffer_info);
	VkBuffer CreateVkBuffer(const BufferInfo *buffer_info);
	VkBuffer CreateVkBuffer(void *gpu_address, VkBufferUsageFlags usage_flags, size_t size);
    
	VkImage  CreateVkImage(void *gpu_address, const TextureInfo *buffer_info);
	VkImage  CreateVkImage(const TextureInfo *buffer_info);
	VkImage  CreateVkImage(void *gpu_address, VkImageUsageFlags image_usage_flags, const gfx::TextureInfo *texture_info);

    VkSampler   CreateVkSampler(const SamplerInfo *sampler_info);
    VkImageView CreateVkImageView(const TextureViewInfo *texture_view_info);

    void DestroyVkBuffer(VkBuffer vk_buffer);
    void DestroyVkImage(VkImage vk_image);
    void DestroyVkSampler(VkSampler vk_sampler);
    void DestroyVkImageView(VkImageView vk_image_view);

    VkDeviceAddress GetVkDeviceAddress(VkBuffer vk_buffer);

    void FlushCpuCache(void *gpu_address, size_t size);
    void InvalidateCpuCache(void *gpu_address, size_t size);
}
