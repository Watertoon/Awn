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

	class Texture {
        private:
            VkImage          m_vk_image;
            TextureInfo      m_texture_info;
        public:
            constexpr ALWAYS_INLINE Texture() : m_vk_image(VK_NULL_HANDLE), m_texture_info() {/*...*/}
            constexpr ALWAYS_INLINE ~Texture() {/*...*/}

            void Initialize(void *gpu_memory_address, TextureInfo *texture_info);
            void Initialize(VkDeviceMemory vk_device_memory, size_t memory_offset, TextureInfo *texture_info);

            void Finalize();

            static GpuMemoryRequirements GetMemoryRequirements(TextureInfo *texture_info);

            constexpr ALWAYS_INLINE u32 GetWidth() const {
                return m_texture_info.width;
            }
            constexpr ALWAYS_INLINE u32 GetHeight() const {
                return m_texture_info.height;
            }
            constexpr ALWAYS_INLINE ImageFormat GetImageFormat() const {
                return static_cast<ImageFormat>(m_texture_info.image_format);
            }
            constexpr ALWAYS_INLINE VkImage GetVkImage() const {
                return m_vk_image;
            }
            constexpr u32 GetVkAspectMask() const {
                return vp::res::GfxImageFormatToVkImageAspectFlags(this->GetImageFormat());
            }
    };
    static_assert(sizeof(Texture) <= cMaxGfxTextureSize);
}
