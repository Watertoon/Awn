#pragma once

namespace awn::gfx {

	VkBuffer CreateVkBuffer(void *gpu_address, BufferInfo *buffer_info);
	VkImage  CreateVkImage(void *gpu_address, TextureInfo *buffer_info);
	VkBuffer CreateVkBuffer(void *gpu_address, VkBufferUsageFlags usage_flags, size_t size);
	VkImage  CreateVkImage(void *gpu_address, VkImageUsageFlags image_usage_flags, gfx::TextureInfo *texture_info);
	
    void FlushCpuCache(void *gpu_address, size_t size);
    void InvalidateCpuCache(void *gpu_address, size_t size);
}
