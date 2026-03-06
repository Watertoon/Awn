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

    struct TextureCopyRegion {
        u32 base_x;
        u32 base_y;
        u32 base_z;
        u32 width = 1;
        u32 height = 1;
        u32 depth = 1;
        u16 base_mip_level;
        u16 reserve0;
        u32 base_array_layer;
        u32 layer_count = 1;
    };

	class Texture {
        public:
            friend class CommandBuffer;
        private:
            VkImage m_vk_image;
            TextureInfo m_texture_info;
            size_t m_memory_offset;
        public:
            constexpr ALWAYS_INLINE Texture() : m_vk_image(VK_NULL_HANDLE), m_texture_info(), m_memory_offset() {/*...*/}
            constexpr ALWAYS_INLINE ~Texture() {/*...*/}

            Result Initialize(MemoryPool *memory_pool, size_t memory_offset, const TextureInfo *texture_info);
            Result Initialize(void *gpu_memory_address, const TextureInfo *texture_info);
            void Finalize();

            void CopyMemoryToTexture(const void * const*memory_array, const TextureCopyRegion *copy_region_array, u32 copy_region_count, bool is_no_conversion = false);
            void CopyTextureToMemory(void **memory_array, const TextureCopyRegion *copy_region_array, u32 copy_region_count, bool is_no_conversion = false);

            constexpr bool IsValid() const {
                return m_vk_image != VK_NULL_HANDLE;
            }
            constexpr u32 GetWidth() const {
                return m_texture_info.width;
            }
            constexpr u32 GetHeight() const {
                return m_texture_info.height;
            }
            constexpr u32 GetDepth() const {
                return m_texture_info.depth;
            }
            constexpr Int2 GetSize2d() const {
                return Int2 { static_cast<s32>(m_texture_info.width), static_cast<s32>(m_texture_info.height), };
            }
            constexpr Int3 GetSize3d() const {
                return Int3 { static_cast<s32>(m_texture_info.width), static_cast<s32>(m_texture_info.height), static_cast<s32>(m_texture_info.depth), };
            }
            constexpr u32 GetMipLevelCount() const {
                return m_texture_info.mip_level_count;
            }
            constexpr u32 GetArrayLayerCount() const {
                return m_texture_info.array_layer_count;
            }
            constexpr gfx::ImageFormat GetImageFormat() const {
                return static_cast<gfx::ImageFormat>(m_texture_info.image_format);
            }

            constexpr uintptr_t GetMemoryPoolOffset() const {
                return m_memory_offset;
            }
        public:
            static GpuMemoryRequirements GetGpuMemoryRequirements(const TextureInfo *texture_info);
        public:
            constexpr bool IsSameInfo(const TextureInfo *texture_info) const {
                if (m_texture_info.texture_flags != texture_info->texture_flags) { return false; }
                if (m_texture_info.image_storage_dimension != texture_info->image_storage_dimension) { return false; }
                if (m_texture_info.tile_mode != texture_info->tile_mode) { return false; }
                if (m_texture_info.swizzle != texture_info->swizzle) { return false; }
                if (m_texture_info.mip_level_count != texture_info->mip_level_count) { return false; }
                if (m_texture_info.texture_flags != texture_info->texture_flags) { return false; }
                if (m_texture_info.sample_count != texture_info->sample_count) { return false; }
                if (m_texture_info.image_format != texture_info->image_format) { return false; }
                if (m_texture_info.gpu_access_flags != texture_info->gpu_access_flags) { return false; }
                if (m_texture_info.width != texture_info->width) { return false; }
                if (m_texture_info.height != texture_info->height) { return false; }
                if (m_texture_info.depth != texture_info->depth) { return false; }
                if (m_texture_info.array_layer_count != texture_info->array_layer_count) { return false; }
                if (m_texture_info.packaged_texture_layout != texture_info->packaged_texture_layout) { return false; }
                return true;
            }
            constexpr ALWAYS_INLINE VkImage GetVkImage() const {
                return m_vk_image;
            }
            constexpr u32 GetVkAspectMask() const {
                const bool is_depth = vp::res::IsDepthFormat(this->GetImageFormat());
                const bool is_stencil = vp::res::IsStencilFormat(this->GetImageFormat());
                u32 aspect_mask;
                if (!is_depth && !is_stencil) {
                    aspect_mask = VK_IMAGE_ASPECT_COLOR_BIT;
                } else {
                    aspect_mask = 0;
                    if (is_depth) {                
                        aspect_mask = VK_IMAGE_ASPECT_DEPTH_BIT;
                    }
                    if (is_stencil) {
                        aspect_mask = VK_IMAGE_ASPECT_STENCIL_BIT;
                    }
                }
                return aspect_mask;
            }

            Result InitializeForDedicatedAllocation(const TextureInfo *texture_info, VkImageUsageFlags vk_image_usage_flags, void *pnext = nullptr);
    };
    static_assert(sizeof(Texture) <= cMaxGfxTextureSize);
}
