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
#include <awn.hpp>

namespace awn::gfx {

    void RenderTargetBase::Finalize() {
        if (m_vk_image_view != VK_NULL_HANDLE) {
            ::pfn_vkDestroyImageView(Context::GetInstance()->GetVkDevice(), m_vk_image_view, Context::GetInstance()->GetVkAllocationCallbacks());
            m_vk_image_view = VK_NULL_HANDLE;
        }
        m_vk_image          = VK_NULL_HANDLE;
        m_vk_image_layout   = VK_IMAGE_LAYOUT_UNDEFINED;
        m_vk_aspect_mask    = 0;
        m_array_layer_count = 0;
        m_render_width      = 0;
        m_render_height     = 0;
        return;
    }

    void RenderTargetColor::Initialize(RenderTargetInfo *render_target_info) {

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
                .layerCount     = render_target_info->array_layer_count,
            }
        };
        const u32 result = ::pfn_vkCreateImageView(Context::GetInstance()->GetVkDevice(), std::addressof(image_view_info), Context::GetInstance()->GetVkAllocationCallbacks(), std::addressof(m_vk_image_view));
        VP_ASSERT(result == VK_SUCCESS);

        /* Set Info */
        m_vk_image           = render_target_info->texture->GetVkImage();
        m_vk_image_layout    = VK_IMAGE_LAYOUT_UNDEFINED;
        m_vk_aspect_mask     = image_aspect_mask;
        m_array_layer_count  = render_target_info->array_layer_count;
        m_render_width       = render_target_info->texture->GetWidth();
        m_render_height      = render_target_info->texture->GetHeight();
        m_texture            = render_target_info->texture;

        return;
    }

    void RenderTargetColor::Initialize(RenderTargetImportInfo *render_target_import_info) {

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
                .layerCount     = render_target_import_info->array_layer_count,
            }
        };
        const u32 result = ::pfn_vkCreateImageView(Context::GetInstance()->GetVkDevice(), std::addressof(image_view_info), Context::GetInstance()->GetVkAllocationCallbacks(), std::addressof(m_vk_image_view));
        VP_ASSERT(result == VK_SUCCESS);

        /* Set Info */
        m_vk_image           = render_target_import_info->vk_image;
        m_vk_image_layout    = VK_IMAGE_LAYOUT_UNDEFINED;
        m_vk_aspect_mask     = image_aspect_mask;
        m_array_layer_count       = render_target_import_info->array_layer_count;
        m_render_width       = render_target_import_info->render_width;
        m_render_height      = render_target_import_info->render_height;

        return;
    }

    void RenderTargetDepthStencil::Initialize(RenderTargetInfo *render_target_info) {

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
                .layerCount     = render_target_info->array_layer_count,
            }
        };
        const u32 result = ::pfn_vkCreateImageView(Context::GetInstance()->GetVkDevice(), std::addressof(image_view_info), Context::GetInstance()->GetVkAllocationCallbacks(), std::addressof(m_vk_image_view));
        VP_ASSERT(result == VK_SUCCESS);

        /* Set Info */
        m_vk_image           = render_target_info->texture->GetVkImage();
        m_vk_image_layout    = VK_IMAGE_LAYOUT_UNDEFINED;
        m_vk_aspect_mask     = image_aspect_mask;
        m_array_layer_count       = render_target_info->array_layer_count;
        m_render_width       = render_target_info->texture->GetWidth();
        m_render_height      = render_target_info->texture->GetHeight();
        m_texture            = render_target_info->texture;

        return;
    }

    void RenderTargetDepthStencil::Initialize(RenderTargetImportInfo *render_target_import_info) {

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
                .layerCount     = render_target_import_info->array_layer_count,
            }
        };
        const u32 result = ::pfn_vkCreateImageView(Context::GetInstance()->GetVkDevice(), std::addressof(image_view_info), Context::GetInstance()->GetVkAllocationCallbacks(), std::addressof(m_vk_image_view));
        VP_ASSERT(result == VK_SUCCESS);

        /* Set Info */
        m_vk_image           = render_target_import_info->vk_image;
        m_vk_image_layout    = VK_IMAGE_LAYOUT_UNDEFINED;
        m_vk_aspect_mask     = image_aspect_mask;
        m_array_layer_count       = render_target_import_info->array_layer_count;
        m_render_width       = render_target_import_info->render_width;
        m_render_height      = render_target_import_info->render_height;

        return;
    }
}
