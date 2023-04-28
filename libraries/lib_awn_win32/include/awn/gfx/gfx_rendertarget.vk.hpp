#pragma once

namespace awn::gfx {

    /* Todo; support stencil */

    struct RenderTargetInfo {
        u32      image_format;
        u16      image_dimension;
        u16      base_mip_level;
        u32      base_array_layer;
        u32      array_layers;
        Texture *texture;

        constexpr void SetDefaults() {
            image_format     = 0;
            image_dimension  = static_cast<u16>(ImageDimension::Type2D);
            base_mip_level   = 0;
            base_array_layer = 0;
            array_layers     = 1;
            texture          = nullptr;
        }
    };

    class RenderTargetBase {
        protected:
            VkImageView      m_vk_image_view;
            RenderTargetInfo m_render_target_info;
        public:
            constexpr RenderTargetBase() : m_vk_image_view(VK_NULL_HANDLE), m_render_target_info() {/*...*/}

            void Finalize() {

                if (m_vk_image_view != VK_NULL_HANDLE) {
                    ::pfn_vkDestroyImageView(Context::GetInstance()->GetVkDevice(), m_vk_image_view, Context::GetInstance()->GetVkAllocationCallbacks());
                }
                m_vk_image_view = VK_NULL_HANDLE;
                m_render_target_info.SetDefaults();
            }

            constexpr u32 GetViewCount()    const { return m_render_target_info.array_layers; }
            constexpr u32 GetViewMask()     const { return 0xffff'ffff; }
            constexpr u32 GetRenderWidth()  const { return m_render_target_info.texture->GetWidth(); }
            constexpr u32 GetRenderHeight() const { return m_render_target_info.texture->GetHeight(); }

            constexpr VkImageView GetVkImageView() const { return m_vk_image_view; }
    };

    class RenderTargetColor : public RenderTargetBase {
        public:
            constexpr RenderTargetColor() : RenderTargetBase() {/*...*/}

            void Initialize(RenderTargetInfo *render_target_info) {

                /* Create VkImageView */
                const VkImageViewCreateInfo image_view_info = {
                    .sType    = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
                    .image    = render_target_info->texture->GetVkImage(),
                    .viewType = vp::res::GfxImageDimensionToVkImageViewType(static_cast<ImageDimension>(render_target_info->image_dimension)),
                    .format   = vp::res::GfxImageFormatToVkFormat(static_cast<ImageFormat>(render_target_info->image_format)),
                    .components = {
                        .r = VK_COMPONENT_SWIZZLE_R,
                        .g = VK_COMPONENT_SWIZZLE_G,
                        .b = VK_COMPONENT_SWIZZLE_B,
                        .a = VK_COMPONENT_SWIZZLE_A,
                    },
                    .subresourceRange = {
                        .aspectMask     = vp::res::GfxImageFormatToVkImageAspectFlags(static_cast<ImageFormat>(render_target_info->image_format)),
                        .baseMipLevel   = render_target_info->base_mip_level,
                        .levelCount     = 1,
                        .baseArrayLayer = render_target_info->base_array_layer,
                        .layerCount     = render_target_info->array_layers,
                    }
                };
                const u32 result = ::pfn_vkCreateImageView(Context::GetInstance()->GetVkDevice(), std::addressof(image_view_info), Context::GetInstance()->GetVkAllocationCallbacks(), std::addressof(m_vk_image_view));
                VP_ASSERT(result == VK_SUCCESS);

                /* Set Info */
                m_render_target_info = *render_target_info;

                return;
            }
    };

    class RenderTargetDepthStencil : public RenderTargetBase {
        public:
            constexpr RenderTargetDepthStencil() : RenderTargetBase() {/*...*/}

            void Initialize(RenderTargetInfo *render_target_info) {

                /* Create VkImageView */
                const VkImageViewCreateInfo image_view_info = {
                    .sType    = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
                    .image    = render_target_info->texture->GetVkImage(),
                    .viewType = vp::res::GfxImageDimensionToVkImageViewType(static_cast<ImageDimension>(render_target_info->image_dimension)),
                    .format   = vp::res::GfxImageFormatToVkFormat(static_cast<ImageFormat>(render_target_info->image_format)),
                    .components = {
                        .r = VK_COMPONENT_SWIZZLE_R,
                        .g = VK_COMPONENT_SWIZZLE_G,
                        .b = VK_COMPONENT_SWIZZLE_B,
                        .a = VK_COMPONENT_SWIZZLE_A,
                    },
                    .subresourceRange = {
                        .aspectMask     = vp::res::GfxImageFormatToVkImageAspectFlags(static_cast<ImageFormat>(render_target_info->image_format)),
                        .baseMipLevel   = render_target_info->base_mip_level,
                        .levelCount     = 1,
                        .baseArrayLayer = render_target_info->base_array_layer,
                        .layerCount     = render_target_info->array_layers,
                    }
                };
                const u32 result = ::pfn_vkCreateImageView(Context::GetInstance()->GetVkDevice(), std::addressof(image_view_info), Context::GetInstance()->GetVkAllocationCallbacks(), std::addressof(m_vk_image_view));
                VP_ASSERT(result == VK_SUCCESS);

                /* Set Info */
                m_render_target_info = *render_target_info;

                return;
            }

            constexpr bool IsDepthFormat() { return true; }
            constexpr bool IsStencilFormat() { return false; }
    };
}
