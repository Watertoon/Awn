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
            VkImage     m_vk_image;
            TextureInfo m_texture_info;
        public:
            constexpr ALWAYS_INLINE Texture() : m_vk_image(VK_NULL_HANDLE), m_texture_info() {/*...*/}
            constexpr ALWAYS_INLINE ~Texture() {/*...*/}

            Result Initialize(mem::GpuMemoryHandle gpu_memory_handle, size_t memory_offset, const TextureInfo *texture_info);
            Result Initialize(void *gpu_memory_address, const TextureInfo *texture_info);
            void Finalize();

            void CopyMemoryToTexture(const void * const*memory_array, const TextureCopyRegion *copy_region_array, u32 copy_region_count, bool is_no_conversion = false);
            void CopyTextureToMemory(void **memory_array, const TextureCopyRegion *copy_region_array, u32 copy_region_count, bool is_no_conversion = false);

            constexpr ALWAYS_INLINE u32 GetWidth() const {
                return m_texture_info.width;
            }
            constexpr ALWAYS_INLINE u32 GetHeight() const {
                return m_texture_info.height;
            }
            constexpr ALWAYS_INLINE u32 GetDepth() const {
                return m_texture_info.depth;
            }
            constexpr ALWAYS_INLINE u32 GetMipLevelCount() const {
                return m_texture_info.mip_level_count;
            }
            constexpr ALWAYS_INLINE u32 GetArrayLayerCount() const {
                return m_texture_info.array_layer_count;
            }
            constexpr ALWAYS_INLINE gfx::ImageFormat GetImageFormat() const {
                return static_cast<gfx::ImageFormat>(m_texture_info.image_format);
            }

        public:
            static GpuMemoryRequirements GetGpuMemoryRequirements(const TextureInfo *texture_info);
        public:
            constexpr ALWAYS_INLINE VkImage GetVkImage() const {
                return m_vk_image;
            }
            constexpr u32 GetVkAspectMask() const {
                return vp::res::GfxImageFormatToVkImageAspectFlags(this->GetImageFormat());
            }

            Result InitializeForDedicatedAllocation(const TextureInfo *texture_info, VkImageUsageFlags vk_image_usage_flags, void *pnext = nullptr);
    };
    static_assert(sizeof(Texture) <= cMaxGfxTextureSize);
}
