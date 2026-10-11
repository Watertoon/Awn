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
    struct GpuMemoryRequirements;
}
namespace awn::gfx::impl {

    struct GpuAddress {
        VkDeviceAddress vk_device_address = 0;

        constexpr GpuAddress &operator+=(size_t offset) {
            this->vk_device_address += offset;
            return *this;
        }
        friend constexpr GpuAddress operator+(GpuAddress address, size_t offset) {
            address.vk_device_address += offset;
            return address;
        }

        constexpr VkDeviceAddress GetVkDeviceAddress() const {
            return this->vk_device_address;
        }
        constexpr bool IsValid() const {
            return this->vk_device_address != 0;
        }
        constexpr bool IsAligned(size_t alignment) const {
            return vp::util::IsAligned(this->vk_device_address, alignment);
        }
    };

    static constexpr GpuAddress cInvalidGpuAddress = {};

	Result CreateVkBuffer(VkBuffer *out_buffer, const BufferInfo *buffer_info);
	Result CreateVkBuffer(VkBuffer *out_buffer, VkBufferUsageFlags usage_flags, size_t size);
	//Result CreateVkBuffer(VkBuffer *out_buffer, void *gpu_address, const BufferInfo *buffer_info);
	//Result CreateVkBuffer(VkBuffer *out_buffer, void *gpu_address, VkBufferUsageFlags usage_flags, size_t size);
	Result CreateVkBuffer(VkBuffer *out_buffer, MemoryPool *memory_pool, size_t memory_offset, const BufferInfo *buffer_info);
    Result CreateVkBuffer(VkBuffer *out_buffer, MemoryPool *memory_pool, size_t memory_offset, VkBufferUsageFlags usage_flags, size_t size);

	Result CreateVkImage(VkImage *out_image, const TextureInfo *buffer_info);
	Result CreateVkImage(VkImage *out_image, const TextureInfo *buffer_info, const void *pnext);
	Result CreateVkImage(VkImage *out_image, VkImageUsageFlags image_usage_flags, const gfx::TextureInfo *texture_info);
	Result CreateVkImage(VkImage *out_image, VkImageUsageFlags image_usage_flags, const gfx::TextureInfo *texture_info, const void *pnext);
	Result CreateVkImage(VkImage *out_image, VkImageUsageFlags image_usage_flags, const gfx::TextureInfo *texture_info, const void *pnext, bool is_preinit);
	//Result CreateVkImage(VkImage *out_image, void *gpu_address, const TextureInfo *buffer_info);
	//Result CreateVkImage(VkImage *out_image, void *gpu_address, const TextureInfo *buffer_info, const void *pnext);
	//Result CreateVkImage(VkImage *out_image, void *gpu_address, VkImageUsageFlags image_usage_flags, const gfx::TextureInfo *texture_info);
	//Result CreateVkImage(VkImage *out_image, void *gpu_address, VkImageUsageFlags image_usage_flags, const gfx::TextureInfo *texture_info, const void *pnext);
	Result CreateVkImage(VkImage *out_image, MemoryPool *memory_pool, size_t offset, const TextureInfo *buffer_info);
	Result CreateVkImage(VkImage *out_image, MemoryPool *memory_pool, size_t offset, const TextureInfo *buffer_info, const void *pnext);
	Result CreateVkImage(VkImage *out_image, MemoryPool *memory_pool, size_t offset, VkImageUsageFlags image_usage_flags, const gfx::TextureInfo *texture_info);
	Result CreateVkImage(VkImage *out_image, MemoryPool *memory_pool, size_t offset, VkImageUsageFlags image_usage_flags, const gfx::TextureInfo *texture_info, const void *pnext);

    GpuMemoryRequirements GetVkImageMemoryRequirements(VkImage vk_image);

    Result CreateVkSampler(VkSampler *out_sampler, const SamplerInfo *sampler_info);
    Result CreateVkImageView(VkImageView *out_image_view, const TextureViewInfo *texture_view_info);

    void DestroyVkBuffer(VkBuffer vk_buffer);
    void DestroyVkImage(VkImage vk_image);
    void DestroyVkSampler(VkSampler vk_sampler);
    void DestroyVkImageView(VkImageView vk_image_view);

    GpuAddress GetGpuAddress(VkBuffer vk_buffer);
}
