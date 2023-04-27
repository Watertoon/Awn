#pragma once

namespace awn::gfx {

    class TextureView {
        private:
            VkImageView      m_vk_image_view;
            TextureViewInfo  m_image_view_info;
        public:
            constexpr ALWAYS_INLINE TextureView() : m_vk_image_view(VK_NULL_HANDLE), m_image_view_info() {/*...*/}
            constexpr ALWAYS_INLINE ~TextureView() {/*...*/}

            void Initialize(TextureViewInfo *texture_view_info) {

                /* Create VkImageView */
                const VkImageViewCreateInfo image_view_info = {
                    .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
                    .image = texture_view_info->texture->GetVkImage(),
                    .viewType = vp::res::GfxImageDimensionToVkImageViewType(static_cast<ImageDimension>(texture_view_info->image_dimension)),
                    .format   = vp::res::GfxImageFormatToVkFormat(static_cast<ImageFormat>(texture_view_info->image_format)),
                    .components = {
                        .r = vp::res::GfxTextureSwizzleToVkComponentSwizzle(static_cast<TextureSwizzle>(texture_view_info->swizzle_x)),
                        .g = vp::res::GfxTextureSwizzleToVkComponentSwizzle(static_cast<TextureSwizzle>(texture_view_info->swizzle_y)),
                        .b = vp::res::GfxTextureSwizzleToVkComponentSwizzle(static_cast<TextureSwizzle>(texture_view_info->swizzle_z)),
                        .a = vp::res::GfxTextureSwizzleToVkComponentSwizzle(static_cast<TextureSwizzle>(texture_view_info->swizzle_w)),
                    },
                    .subresourceRange = {
                        .aspectMask     = vp::res::GfxTextureViewInfoToVkImageAspectFlags(texture_view_info),
                        .baseMipLevel   = texture_view_info->base_mip_level,
                        .levelCount     = texture_view_info->mip_levels,
                        .baseArrayLayer = texture_view_info->base_array_layer,
                        .layerCount     = texture_view_info->array_layers,
                    }
                };
                const u32 result = ::pfn_vkCreateImageView(Context::GetInstance()->GetVkDevice(), std::addressof(image_view_info), Context::GetInstance()->GetVkAllocationCallbacks(), std::addressof(m_vk_image_view));
                VP_ASSERT(result == VK_SUCCESS);

                /* Set state */
                m_image_view_info = *texture_view_info;

                return;
            }

            void Finalize() {

                if (m_vk_image_view != VK_NULL_HANDLE) {
                    ::pfn_vkDestroyImageView(Context::GetInstance()->GetVkDevice(), m_vk_image_view, Context::GetInstance()->GetVkAllocationCallbacks());
                }
                m_vk_image_view = VK_NULL_HANDLE;
            }
            
            constexpr ALWAYS_INLINE u32 GetLayerCount() const {
                return m_image_view_info.array_layers;
            }
    };
}
