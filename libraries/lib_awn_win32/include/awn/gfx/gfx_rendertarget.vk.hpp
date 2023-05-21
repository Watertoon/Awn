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

    struct RenderTargetImportInfo {
        u32      image_format;
        u16      image_dimension;
        u16      base_mip_level;
        u32      base_array_layer;
        u32      array_layers;
        u32      render_width;
        u32      render_height;
        VkImage  vk_image;

        constexpr void SetDefaults() {
            image_format     = 0;
            image_dimension  = static_cast<u16>(ImageDimension::Type2D);
            base_mip_level   = 0;
            base_array_layer = 0;
            array_layers     = 1;
            render_width     = 640;
            render_height    = 480;
            vk_image         = nullptr;
        }
    };

    class CommandBufferBase;

    class RenderTargetBase {
        public:
            friend class CommandBufferBase;
        protected:
            VkImageView   m_vk_image_view;
            VkImage       m_vk_image;
            VkImageLayout m_vk_image_layout;
            u32           m_vk_aspect_mask;
            u32           m_array_layers;
            u32           m_render_width;
            u32           m_render_height;
        protected:
            void SetVkImageLayout(VkImageLayout vk_image_layout) { m_vk_image_layout = vk_image_layout; }
        public:
            constexpr RenderTargetBase() : m_vk_image_view(VK_NULL_HANDLE), m_vk_image(VK_NULL_HANDLE), m_vk_image_layout(VK_IMAGE_LAYOUT_UNDEFINED), m_array_layers(0), m_render_width(0), m_render_height(0) {/*...*/}

            void Finalize() {

                if (m_vk_image_view != VK_NULL_HANDLE) {
                    ::pfn_vkDestroyImageView(Context::GetInstance()->GetVkDevice(), m_vk_image_view, Context::GetInstance()->GetVkAllocationCallbacks());
                }
                m_vk_image_view   = VK_NULL_HANDLE;
                m_vk_image        = VK_NULL_HANDLE;
                m_vk_image_layout = VK_IMAGE_LAYOUT_UNDEFINED;
                m_vk_aspect_mask  = 0;
                m_array_layers    = 0;
                m_render_width    = 0;
                m_render_height   = 0;
            }

            constexpr u32 GetViewCount()    const { return m_array_layers; }
            constexpr u32 GetViewMask()     const { return 0xffff'ffff; }
            constexpr u32 GetRenderWidth()  const { return m_render_width; }
            constexpr u32 GetRenderHeight() const { return m_render_height; }

            constexpr VkImageView   GetVkImageView()   const { return m_vk_image_view; }
            constexpr VkImage       GetVkImage()       const { return m_vk_image; }
            constexpr VkImageLayout GetVkImageLayout() const { return m_vk_image_layout; }
            constexpr u32           GetVkAspectMask()  const { return m_vk_aspect_mask; }
    };

    class RenderTargetColor : public RenderTargetBase {
        public:
            constexpr RenderTargetColor() : RenderTargetBase() {/*...*/}

            void Initialize(RenderTargetInfo *render_target_info) {

                /* Create VkImageView */
                const u32 image_aspect_mask = vp::res::GfxImageFormatToVkImageAspectFlags(static_cast<ImageFormat>(render_target_info->image_format));
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
                        .aspectMask     = image_aspect_mask,
                        .baseMipLevel   = render_target_info->base_mip_level,
                        .levelCount     = 1,
                        .baseArrayLayer = render_target_info->base_array_layer,
                        .layerCount     = render_target_info->array_layers,
                    }
                };
                const u32 result = ::pfn_vkCreateImageView(Context::GetInstance()->GetVkDevice(), std::addressof(image_view_info), Context::GetInstance()->GetVkAllocationCallbacks(), std::addressof(m_vk_image_view));
                VP_ASSERT(result == VK_SUCCESS);

                /* Set Info */
                m_vk_image           = render_target_info->texture->GetVkImage();
                m_vk_image_layout    = VK_IMAGE_LAYOUT_UNDEFINED;
                m_vk_aspect_mask     = image_aspect_mask;
                m_array_layers       = render_target_info->array_layers;
                m_render_width       = render_target_info->texture->GetWidth();
                m_render_height      = render_target_info->texture->GetHeight();

                return;
            }

            void Initialize(RenderTargetImportInfo *render_target_import_info) {

                /* Create VkImageView */
                const u32 image_aspect_mask = vp::res::GfxImageFormatToVkImageAspectFlags(static_cast<ImageFormat>(render_target_import_info->image_format));
                const VkImageViewCreateInfo image_view_info = {
                    .sType    = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
                    .image    = render_target_import_info->vk_image,
                    .viewType = vp::res::GfxImageDimensionToVkImageViewType(static_cast<ImageDimension>(render_target_import_info->image_dimension)),
                    .format   = vp::res::GfxImageFormatToVkFormat(static_cast<ImageFormat>(render_target_import_info->image_format)),
                    .components = {
                        .r = VK_COMPONENT_SWIZZLE_R,
                        .g = VK_COMPONENT_SWIZZLE_G,
                        .b = VK_COMPONENT_SWIZZLE_B,
                        .a = VK_COMPONENT_SWIZZLE_A,
                    },
                    .subresourceRange = {
                        .aspectMask     = image_aspect_mask,
                        .baseMipLevel   = render_target_import_info->base_mip_level,
                        .levelCount     = 1,
                        .baseArrayLayer = render_target_import_info->base_array_layer,
                        .layerCount     = render_target_import_info->array_layers,
                    }
                };
                const u32 result = ::pfn_vkCreateImageView(Context::GetInstance()->GetVkDevice(), std::addressof(image_view_info), Context::GetInstance()->GetVkAllocationCallbacks(), std::addressof(m_vk_image_view));
                VP_ASSERT(result == VK_SUCCESS);

                /* Set Info */
                m_vk_image           = render_target_import_info->vk_image;
                m_vk_image_layout    = VK_IMAGE_LAYOUT_UNDEFINED;
                m_vk_aspect_mask     = image_aspect_mask;
                m_array_layers       = render_target_import_info->array_layers;
                m_render_width       = render_target_import_info->render_width;
                m_render_height      = render_target_import_info->render_height;

                return;
            }
    };

    class RenderTargetDepthStencil : public RenderTargetBase {
        public:
            constexpr RenderTargetDepthStencil() : RenderTargetBase() {/*...*/}

            void Initialize(RenderTargetInfo *render_target_info) {

                /* Create VkImageView */
                const u32 image_aspect_mask = vp::res::GfxImageFormatToVkImageAspectFlags(static_cast<ImageFormat>(render_target_info->image_format));
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
                        .aspectMask     = image_aspect_mask,
                        .baseMipLevel   = render_target_info->base_mip_level,
                        .levelCount     = 1,
                        .baseArrayLayer = render_target_info->base_array_layer,
                        .layerCount     = render_target_info->array_layers,
                    }
                };
                const u32 result = ::pfn_vkCreateImageView(Context::GetInstance()->GetVkDevice(), std::addressof(image_view_info), Context::GetInstance()->GetVkAllocationCallbacks(), std::addressof(m_vk_image_view));
                VP_ASSERT(result == VK_SUCCESS);

                /* Set Info */
                m_vk_image           = render_target_info->texture->GetVkImage();
                m_vk_image_layout    = VK_IMAGE_LAYOUT_UNDEFINED;
                m_vk_aspect_mask     = image_aspect_mask;
                m_array_layers       = render_target_info->array_layers;
                m_render_width       = render_target_info->texture->GetWidth();
                m_render_height      = render_target_info->texture->GetHeight();

                return;
            }

            void Initialize(RenderTargetImportInfo *render_target_import_info) {

                /* Create VkImageView */
                const u32 image_aspect_mask = vp::res::GfxImageFormatToVkImageAspectFlags(static_cast<ImageFormat>(render_target_import_info->image_format));
                const VkImageViewCreateInfo image_view_info = {
                    .sType    = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
                    .image    = render_target_import_info->vk_image,
                    .viewType = vp::res::GfxImageDimensionToVkImageViewType(static_cast<ImageDimension>(render_target_import_info->image_dimension)),
                    .format   = vp::res::GfxImageFormatToVkFormat(static_cast<ImageFormat>(render_target_import_info->image_format)),
                    .components = {
                        .r = VK_COMPONENT_SWIZZLE_R,
                        .g = VK_COMPONENT_SWIZZLE_G,
                        .b = VK_COMPONENT_SWIZZLE_B,
                        .a = VK_COMPONENT_SWIZZLE_A,
                    },
                    .subresourceRange = {
                        .aspectMask     = image_aspect_mask,
                        .baseMipLevel   = render_target_import_info->base_mip_level,
                        .levelCount     = 1,
                        .baseArrayLayer = render_target_import_info->base_array_layer,
                        .layerCount     = render_target_import_info->array_layers,
                    }
                };
                const u32 result = ::pfn_vkCreateImageView(Context::GetInstance()->GetVkDevice(), std::addressof(image_view_info), Context::GetInstance()->GetVkAllocationCallbacks(), std::addressof(m_vk_image_view));
                VP_ASSERT(result == VK_SUCCESS);

                /* Set Info */
                m_vk_image           = render_target_import_info->vk_image;
                m_vk_image_layout    = VK_IMAGE_LAYOUT_UNDEFINED;
                m_vk_aspect_mask     = image_aspect_mask;
                m_array_layers       = render_target_import_info->array_layers;
                m_render_width       = render_target_import_info->render_width;
                m_render_height      = render_target_import_info->render_height;

                return;
            }

            constexpr bool IsDepthFormat() { return true; }
            constexpr bool IsStencilFormat() { return false; }
    };
}
