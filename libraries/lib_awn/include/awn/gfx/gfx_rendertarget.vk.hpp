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

    /* Todo; support stencil */

    struct RenderTargetInfo {
        u32      image_format;
        u16      image_dimension;
        u16      base_mip_level;
        u32      base_array_layer;
        u32      array_layer_count;
        Texture *texture;

        constexpr void SetDefaults() {
            image_format      = 0;
            image_dimension   = static_cast<u16>(ImageDimension::Type2d);
            base_mip_level    = 0;
            base_array_layer  = 0;
            array_layer_count = 1;
            texture           = nullptr;
        }

        constexpr void SetImageFormat(ImageFormat image_format) {
            this->image_format = static_cast<u32>(image_format);
        }
        constexpr void SetImageDimension(ImageDimension image_dimension) {
            this->image_dimension = static_cast<u16>(image_dimension);
        }
        constexpr void SetBaseMipLevel(u16 base_mip_level) {
            this->base_mip_level = base_mip_level;
        }
        constexpr void SetArrayRange(u32 base_array_layer, u32 array_layer_count) {
            this->base_array_layer = base_array_layer;
            this->array_layer_count = array_layer_count;
        }
        constexpr void SetTexture(Texture *texture) {
            this->texture = texture;
        }
    };

    class CommandBufferBase;

    class RenderTargetBase {
        public:
            friend class CommandBuffer;
        protected:
            VkImageView   m_vk_image_view;
            VkImageLayout m_vk_image_layout;
            u32           m_vk_aspect_mask;
            u32           m_array_layer_count;
            u32           m_render_width;
            u32           m_render_height;
            Texture       *m_texture;
        protected:
            void SetVkImageLayout(VkImageLayout vk_image_layout) { m_vk_image_layout = vk_image_layout; }
        public:
            constexpr RenderTargetBase() : m_vk_image_view(VK_NULL_HANDLE), m_vk_image_layout(), m_array_layer_count(), m_render_width(), m_render_height(), m_texture() {/*...*/}
            constexpr ~RenderTargetBase() {/*...*/}

            void Finalize();

            constexpr u32 GetViewCount()    const { return m_array_layer_count; }
            constexpr u32 GetViewMask()     const { return 0; }
            constexpr u32 GetRenderWidth()  const { return m_render_width; }
            constexpr u32 GetRenderHeight() const { return m_render_height; }

            constexpr Texture       *GetTexture()       const { return m_texture; }
            constexpr VkImageView    GetVkImageView()   const { return m_vk_image_view; }
            constexpr VkImage        GetVkImage()       const { return m_texture->GetVkImage(); }
            constexpr VkImageLayout  GetVkImageLayout() const { return m_vk_image_layout; }
            constexpr u32            GetVkAspectMask()  const { return m_vk_aspect_mask; }
    };

    class RenderTargetColor : public RenderTargetBase {
        public:
            constexpr RenderTargetColor() : RenderTargetBase() {/*...*/}
            constexpr ~RenderTargetColor() {/*...*/}

            void Initialize(RenderTargetInfo *render_target_info);
    };

    class RenderTargetDepthStencil : public RenderTargetBase {
        public:
            constexpr RenderTargetDepthStencil() : RenderTargetBase() {/*...*/}
            constexpr ~RenderTargetDepthStencil() {/*...*/}

            void Initialize(RenderTargetInfo *render_target_info);

            constexpr bool IsDepthFormat() { return true; }
            constexpr bool IsStencilFormat() { return false; }
    };
}
