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

namespace vp::res {

    constexpr inline VkSamplerAddressMode GfxWrapModeToVkSamplerAddressMode(GfxWrapMode wrap_mode) {
        switch (wrap_mode) {
            case GfxWrapMode::Repeat:
                return VK_SAMPLER_ADDRESS_MODE_REPEAT;
            case GfxWrapMode::MirrorRepeat:
                return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
            case GfxWrapMode::ClampToEdge:
                return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
            case GfxWrapMode::ClampToBorder:
                return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
            case GfxWrapMode::MirrorClampToEdge:
                return VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE;
            default:
                break;
        }
        return VK_SAMPLER_ADDRESS_MODE_REPEAT;
    }

    constexpr inline VkSamplerMipmapMode GfxMipMapFilterToVkSamplerMipMapMode(GfxMipMapFilter mip_map_filter) {
        switch (mip_map_filter) {
            case GfxMipMapFilter::Nearest:
                return VK_SAMPLER_MIPMAP_MODE_NEAREST;
            case GfxMipMapFilter::Linear:
                return VK_SAMPLER_MIPMAP_MODE_LINEAR;
            default:
                break;
        }
        return VK_SAMPLER_MIPMAP_MODE_NEAREST;
    }

    constexpr inline VkFilter GfxMagFilterToVkFilter(GfxMagFilter mag_filter) {
        switch (mag_filter) {
            case GfxMagFilter::Nearest:
                return VK_FILTER_NEAREST;
            case GfxMagFilter::Linear:
                return VK_FILTER_LINEAR;
            default:
                break;
        }
        return VK_FILTER_NEAREST;
    }

    constexpr inline VkFilter GfxMinFilterToVkFilter(GfxMinFilter min_filter) {
        switch (min_filter) {
            case GfxMinFilter::Nearest:
                return VK_FILTER_NEAREST;
            case GfxMinFilter::Linear:
                return VK_FILTER_LINEAR;
            default:
                break;
        }
        return VK_FILTER_NEAREST;
    }

    constexpr inline VkSamplerReductionMode GfxReductionFilterToVkSamplerReductionMode(GfxReductionFilter reduction_filter) {
        switch (reduction_filter) {
            case GfxReductionFilter::Average:
                return VK_SAMPLER_REDUCTION_MODE_WEIGHTED_AVERAGE;
            case GfxReductionFilter::Min:
                return VK_SAMPLER_REDUCTION_MODE_MIN;
            case GfxReductionFilter::Max:
                return VK_SAMPLER_REDUCTION_MODE_MAX;
            default:
                break;
        }
        return VK_SAMPLER_REDUCTION_MODE_WEIGHTED_AVERAGE;
    }

    constexpr inline VkCompareOp GfxCompareOperationToVkCompareOp(GfxCompareOperation compare_op) {
        switch (compare_op) {
            case GfxCompareOperation::Never:
                return VK_COMPARE_OP_NEVER;
            case GfxCompareOperation::LessThan:
                return VK_COMPARE_OP_LESS;
            case GfxCompareOperation::Equal:
                return VK_COMPARE_OP_EQUAL;
            case GfxCompareOperation::LessThanEqual:
                return VK_COMPARE_OP_LESS_OR_EQUAL;
            case GfxCompareOperation::GreaterThan:
                return VK_COMPARE_OP_GREATER;
            case GfxCompareOperation::NotEqual:
                return VK_COMPARE_OP_NOT_EQUAL;
            case GfxCompareOperation::GreaterThanEqual:
                return VK_COMPARE_OP_GREATER_OR_EQUAL;
            case GfxCompareOperation::Always:
                return VK_COMPARE_OP_ALWAYS;
            default:
                break;
        }
        return VK_COMPARE_OP_NEVER;
    }

    constexpr inline VkLogicOp GfxLogicOperationToVkLogicOp(GfxLogicOperation logic_op) {
        return static_cast<VkLogicOp>(logic_op);
    }

    constexpr inline VkBorderColor GfxBorderColorToVkBorderColor(GfxBorderColor border_color) {
        switch (border_color) {
            case GfxBorderColor::White:
                return VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
            case GfxBorderColor::TransparentBlack:
                return VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
            case GfxBorderColor::Black:
                return VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
            default:
                break;
        }
        return VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
    }

    constexpr inline VkPrimitiveTopology GfxPrimitiveTopologyToVkPrimitiveTopology(GfxPrimitiveTopology primitive_topology) {
        switch (primitive_topology) {
            case GfxPrimitiveTopology::Points:
                return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
            case GfxPrimitiveTopology::Lines:
                return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
            case GfxPrimitiveTopology::LineStrip:
                return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
            case GfxPrimitiveTopology::Triangles:
                return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
            case GfxPrimitiveTopology::TrianglesStrip:
                return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
            case GfxPrimitiveTopology::LinesAdjacency:
                return VK_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY;
            case GfxPrimitiveTopology::LineStripAdjacency:
                return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP_WITH_ADJACENCY;
            case GfxPrimitiveTopology::TrianglesAdjacency:
                return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST_WITH_ADJACENCY;
            case GfxPrimitiveTopology::TriangleStripAdjacency:
                return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP_WITH_ADJACENCY;
            case GfxPrimitiveTopology::Patches:
                return VK_PRIMITIVE_TOPOLOGY_PATCH_LIST;
            default:
                break;
        }
        return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
    }

    constexpr inline VkIndexType GfxIndexFormatToVkIndexType(GfxIndexFormat index_format) {
        switch (index_format) {
            case GfxIndexFormat::U8:
                return VK_INDEX_TYPE_UINT8_EXT;
            case GfxIndexFormat::U16:
                return VK_INDEX_TYPE_UINT16;
            case GfxIndexFormat::U32:
                return VK_INDEX_TYPE_UINT32;
            default:
                break;
        }
        return VK_INDEX_TYPE_NONE_KHR;
    }

    constexpr inline VkImageType GfxImageStorageDimensionToVkImageType(GfxImageStorageDimension image_storage_dimension) {
        return static_cast<VkImageType>(static_cast<u8>(image_storage_dimension) - 1);
    }

    constexpr inline VkImageCreateFlags ResGfxTextureInfoToVkImageCreateFlags(ResGfxTextureInfo *texture_info) {

        u32 flags = 0;
        //if (texture_info->storage_dimension == static_cast<u8>(GfxImageStorageDimension::Type2D) { flags |= VK_IMAGE_CREATE_2D_VIEW_COMPATIBLE_BIT_EXT; }
        if (1 < texture_info->array_layers) { flags |= VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT | VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT; }
        //if (1 < texture_info->sample_count) { flags |= ; }
        
        return flags;
    }

    constexpr inline VkImageUsageFlags GfxGpuAccessFlagsToVkImageUsageFlags(GfxGpuAccessFlags access_flags) {
        
        u32 flags = 0;
        if ((static_cast<u32>(access_flags) & static_cast<u32>(GfxGpuAccessFlags::TransferDestination)) != 0) { flags |= VK_IMAGE_USAGE_TRANSFER_DST_BIT; }
        if ((static_cast<u32>(access_flags) & static_cast<u32>(GfxGpuAccessFlags::Texture)) != 0)             { flags |= VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE; }
        if ((static_cast<u32>(access_flags) & static_cast<u32>(GfxGpuAccessFlags::RenderTargetColor)) != 0)   { flags |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT; }
        if ((static_cast<u32>(access_flags) & static_cast<u32>(GfxGpuAccessFlags::RenderTargetDepth)) != 0)   { flags |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT; }

        return flags;
    }

    constexpr inline VkBufferUsageFlags GfxGpuAccessFlagsToVkBufferUsageFlags(GfxGpuAccessFlags access_flags) {

        u32 flags = 0;
        if ((static_cast<u32>(access_flags) & static_cast<u32>(GfxGpuAccessFlags::VertexBuffer)) != 0)        { flags |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT; }
        if ((static_cast<u32>(access_flags) & static_cast<u32>(GfxGpuAccessFlags::IndexBuffer)) != 0)         { flags |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT; }
        if ((static_cast<u32>(access_flags) & static_cast<u32>(GfxGpuAccessFlags::UniformBuffer)) != 0)       { flags |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT; }
        if ((static_cast<u32>(access_flags) & static_cast<u32>(GfxGpuAccessFlags::TransferDestination)) != 0) { flags |= VK_IMAGE_USAGE_TRANSFER_DST_BIT; }
        return flags;
    }

    constexpr inline VkMemoryPropertyFlags GfxMemoryPoolFlagsToVkMemoryPropertyFlags(GfxMemoryPoolFlags memory_pool_flags) {

        u32 flags = 0;
        if ((static_cast<u32>(memory_pool_flags) & (static_cast<u32>(GfxMemoryPoolFlags::GpuUncached) | static_cast<u32>(GfxMemoryPoolFlags::GpuCached))) != 0) { flags |= VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT; }
        if ((static_cast<u32>(memory_pool_flags) & (static_cast<u32>(GfxMemoryPoolFlags::CpuUncached) | static_cast<u32>(GfxMemoryPoolFlags::CpuCached))) != 0) { flags |= VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT; }
        if ((static_cast<u32>(memory_pool_flags) & (static_cast<u32>(GfxMemoryPoolFlags::CpuUncached) | static_cast<u32>(GfxMemoryPoolFlags::CpuCached))) != 0) { flags |= VK_MEMORY_PROPERTY_HOST_COHERENT_BIT; }
        if ((static_cast<u32>(memory_pool_flags) &  static_cast<u32>(GfxMemoryPoolFlags::CpuCached)) != 0)                                                      { flags |= VK_MEMORY_PROPERTY_HOST_CACHED_BIT; }

        return flags;
    }

    constexpr inline VkImageViewType GfxImageDimensionToVkImageViewType(GfxImageDimension image_dimension) {

        switch (image_dimension) {
            case GfxImageDimension::Type1D:
                return VK_IMAGE_VIEW_TYPE_1D;
            case GfxImageDimension::Type2D:
                return VK_IMAGE_VIEW_TYPE_2D;
            case GfxImageDimension::Type3D:
                return VK_IMAGE_VIEW_TYPE_3D;
            case GfxImageDimension::TypeCube:
                return VK_IMAGE_VIEW_TYPE_CUBE;
            case GfxImageDimension::Type1DArray:
                return VK_IMAGE_VIEW_TYPE_1D_ARRAY;
            case GfxImageDimension::Type2DArray:
                return VK_IMAGE_VIEW_TYPE_2D_ARRAY;
            case GfxImageDimension::TypeCubeArray:
                return VK_IMAGE_VIEW_TYPE_CUBE_ARRAY;
            case GfxImageDimension::Type2DMultisample:
            case GfxImageDimension::Type2DMultisampleArray:
            case GfxImageDimension::TypeRectangle:
                break;
        };
        return VK_IMAGE_VIEW_TYPE_2D;
    }

    constexpr ALWAYS_INLINE VkComponentSwizzle GfxTextureSwizzleToVkComponentSwizzle(GfxTextureSwizzle swizzle) {
        return static_cast<VkComponentSwizzle>(swizzle);
    }

    template <typename T>
    constexpr inline VkImageAspectFlags GfxTextureViewInfoToVkImageAspectFlags(ResGfxTextureViewInfo<T> *texture_view_info) {

        u32 flags = 0;
        if (static_cast<GfxTypeFormat>(texture_view_info->image_format & 0xff) != GfxTypeFormat::Depth) {
            flags |= VK_IMAGE_ASPECT_COLOR_BIT;
        } else if (texture_view_info->depth_stencil_mode == static_cast<u8>(GfxTextureDepthStencilMode::Depth)) {
            flags |= VK_IMAGE_ASPECT_DEPTH_BIT;
        } else {
            flags |= VK_IMAGE_ASPECT_STENCIL_BIT;
        }
        return flags;
    }

    constexpr inline VkImageAspectFlags GfxImageFormatToVkImageAspectFlags(GfxImageFormat image_format) {

        u32 flags = 0;
        if (static_cast<GfxTypeFormat>(static_cast<u32>(image_format) & 0xff) != GfxTypeFormat::Depth) {
            flags |= VK_IMAGE_ASPECT_COLOR_BIT;
        } else {
            flags |= VK_IMAGE_ASPECT_DEPTH_BIT;
        }

        return flags;
    }

    constexpr inline VkPolygonMode GfxFillModeToVkPolygonMode(GfxFillMode fill_mode) {

        if (fill_mode == GfxFillMode::Fill) {
            return VK_POLYGON_MODE_FILL;
        } else if (fill_mode == GfxFillMode::Point) {
            return VK_POLYGON_MODE_POINT;
        }

        /* Line mode optimization */
        return static_cast<VkPolygonMode>(fill_mode);
    }

    constexpr inline VkFrontFace GfxFrontFaceToVkFrontFace(GfxFrontFace front_face) {
        return static_cast<VkFrontFace>(front_face);
    }

    constexpr inline VkCullModeFlags GfxCullModeToVkCullModeFlags(GfxCullMode cull_mode) {
        return static_cast<VkCullModeFlags>(cull_mode);
    }

    constexpr inline VkBlendFactor GfxBlendFactorToVkBlendFactor(GfxBlendFactor blend_factor) {
        return static_cast<VkBlendFactor>(blend_factor);
    }

    constexpr inline VkBlendOp GfxBlendEquationToVkBlendOp(GfxBlendEquation blend_equation) {
        return static_cast<VkBlendOp>(blend_equation);
    }

    constexpr inline VkFormat GfxAttributeFormatToVkFormat(GfxAttributeFormat attribute_format) {
        switch (attribute_format) {
            case GfxAttributeFormat::None_Unorm:
                break;
            case GfxAttributeFormat::R8_Unorm:
                return VK_FORMAT_R8_UNORM;
            case GfxAttributeFormat::R8_Snorm:
                return VK_FORMAT_R8_SNORM;
            case GfxAttributeFormat::R8_UInt:
                return VK_FORMAT_R8_UINT;
            case GfxAttributeFormat::R8_SInt:
                return VK_FORMAT_R8_SINT;
            case GfxAttributeFormat::R8_UScaled:
                return VK_FORMAT_R8_USCALED;
            case GfxAttributeFormat::R8_SScaled:
                return VK_FORMAT_R8_SSCALED;
            case GfxAttributeFormat::R8G8_Unorm:
                return VK_FORMAT_R8G8_UNORM;
            case GfxAttributeFormat::R8G8_Snorm:
                return VK_FORMAT_R8G8_SNORM;
            case GfxAttributeFormat::R8G8_UInt:
                return VK_FORMAT_R8G8_UINT;
            case GfxAttributeFormat::R8G8_SInt:
                return VK_FORMAT_R8G8_SINT;
            case GfxAttributeFormat::R8G8_UScaled:
                return VK_FORMAT_R8G8_USCALED;
            case GfxAttributeFormat::R8G8_SScaled:
                return VK_FORMAT_R8G8_SSCALED;
            case GfxAttributeFormat::R16_Unorm:
                return VK_FORMAT_R16_UNORM;
            case GfxAttributeFormat::R16_Snorm:
                return VK_FORMAT_R16_SNORM;
            case GfxAttributeFormat::R16_UInt:
                return VK_FORMAT_R16_UINT;
            case GfxAttributeFormat::R16_SInt:
                return VK_FORMAT_R16_SINT;
            case GfxAttributeFormat::R16_Float:
                return VK_FORMAT_R16_SFLOAT;
            case GfxAttributeFormat::R16_UScaled:
                return VK_FORMAT_R16_USCALED;
            case GfxAttributeFormat::R16_SScaled:
                return VK_FORMAT_R16_SSCALED;
            case GfxAttributeFormat::R8G8B8A8_Unorm:
                return VK_FORMAT_R8G8B8A8_UNORM;
            case GfxAttributeFormat::R8G8B8A8_Snorm:
                return VK_FORMAT_R8G8B8A8_SNORM;
            case GfxAttributeFormat::R8G8B8A8_UInt:
                return VK_FORMAT_R8G8B8A8_UINT;
            case GfxAttributeFormat::R8G8B8A8_SInt:
                return VK_FORMAT_R8G8B8A8_SINT;
            case GfxAttributeFormat::R8G8B8A8_UScaled:
                return VK_FORMAT_R8G8B8A8_USCALED;
            case GfxAttributeFormat::R8G8B8A8_SScaled:
                return VK_FORMAT_R8G8B8A8_SSCALED;
            case GfxAttributeFormat::R10G10B10A2_Unorm:
                return VK_FORMAT_A2B10G10R10_UNORM_PACK32;
            case GfxAttributeFormat::R10G10B10A2_Snorm:
                return VK_FORMAT_A2B10G10R10_SNORM_PACK32;
            case GfxAttributeFormat::R10G10B10A2_UInt:
                return VK_FORMAT_A2B10G10R10_UINT_PACK32;
            case GfxAttributeFormat::R10G10B10A2_SInt:
                return VK_FORMAT_A2B10G10R10_SINT_PACK32;
            case GfxAttributeFormat::R16G16_Unorm:
                return VK_FORMAT_R16G16_UNORM;
            case GfxAttributeFormat::R16G16_Snorm:
                return VK_FORMAT_R16G16_SNORM;
            case GfxAttributeFormat::R16G16_UInt:
                return VK_FORMAT_R16G16_UINT;
            case GfxAttributeFormat::R16G16_SInt:
                return VK_FORMAT_R16G16_SINT;
            case GfxAttributeFormat::R16G16_Float:
                return VK_FORMAT_R16G16_SFLOAT;
            case GfxAttributeFormat::R16G16_UScaled:
                return VK_FORMAT_R16G16_USCALED;
            case GfxAttributeFormat::R16G16_SScaled:
                return VK_FORMAT_R16G16_SSCALED;
            case GfxAttributeFormat::R32_UInt:
                return VK_FORMAT_R32_UINT;
            case GfxAttributeFormat::R32_SInt:
                return VK_FORMAT_R32_SINT;
            case GfxAttributeFormat::R32_Float:
                return VK_FORMAT_R32_SFLOAT;
            case GfxAttributeFormat::R16G16B16A16_Unorm:
                return VK_FORMAT_R16G16B16A16_UNORM;
            case GfxAttributeFormat::R16G16B16A16_Snorm:
                return VK_FORMAT_R16G16B16A16_SNORM;
            case GfxAttributeFormat::R16G16B16A16_UInt:
                return VK_FORMAT_R16G16B16A16_UINT;
            case GfxAttributeFormat::R16G16B16A16_SInt:
                return VK_FORMAT_R16G16B16A16_SINT;
            case GfxAttributeFormat::R16G16B16A16_Float:
                return VK_FORMAT_R16G16B16A16_SFLOAT;
            case GfxAttributeFormat::R16G16B16A16_UScaled:
                return VK_FORMAT_R16G16B16A16_USCALED;
            case GfxAttributeFormat::R16G16B16A16_SScaled:
                return VK_FORMAT_R16G16B16A16_SSCALED;
            case GfxAttributeFormat::R32G32_UInt:
                return VK_FORMAT_R32G32_UINT;
            case GfxAttributeFormat::R32G32_SInt:
                return VK_FORMAT_R32G32_SINT;
            case GfxAttributeFormat::R32G32_Float:
                return VK_FORMAT_R32G32_SFLOAT;
            case GfxAttributeFormat::R32G32B32_UInt:
                return VK_FORMAT_R32G32B32_UINT;
            case GfxAttributeFormat::R32G32B32_SInt:
                return VK_FORMAT_R32G32B32_SINT;
            case GfxAttributeFormat::R32G32B32_Float:
                return VK_FORMAT_R32G32B32_SFLOAT;
            case GfxAttributeFormat::R32G32B32A32_UInt:
                return VK_FORMAT_R32G32B32A32_UINT;
            case GfxAttributeFormat::R32G32B32A32_SInt:
                return VK_FORMAT_R32G32B32A32_SINT;
            case GfxAttributeFormat::R32G32B32A32_Float:
                return VK_FORMAT_R32G32B32A32_SFLOAT;
            default:
                break;
        }
        return VK_FORMAT_UNDEFINED;
    }

	constexpr inline VkFormat GfxImageFormatToVkFormat(GfxImageFormat image_format) {
		switch (image_format) {
            case GfxImageFormat::R8_Unorm:
                return VK_FORMAT_R8_UNORM;
            case GfxImageFormat::R8_Snorm:
                return VK_FORMAT_R8_SNORM;
            case GfxImageFormat::R8_UInt:
                return VK_FORMAT_R8_UINT;
            case GfxImageFormat::R8_SInt:
                return VK_FORMAT_R8_SINT;
            case GfxImageFormat::R4G4B4A4_Unorm:
                return VK_FORMAT_R4G4B4A4_UNORM_PACK16;
            case GfxImageFormat::R5G5B5A1_Unorm:
                return VK_FORMAT_R5G5B5A1_UNORM_PACK16;
            case GfxImageFormat::A1B5G5R5_Unorm:
                return VK_FORMAT_A1R5G5B5_UNORM_PACK16;
            case GfxImageFormat::R5G6B5_Unorm:
                return VK_FORMAT_R5G6B5_UNORM_PACK16;
            case GfxImageFormat::B5G6R5_Unorm:
                return VK_FORMAT_B5G6R5_UNORM_PACK16;
            case GfxImageFormat::R8G8_Unorm:
                return VK_FORMAT_R8G8_UNORM;
            case GfxImageFormat::R8G8_Snorm:
                return VK_FORMAT_R8G8_SNORM;
            case GfxImageFormat::R8G8_UInt:
                return VK_FORMAT_R8G8_UINT;
            case GfxImageFormat::R8G8_SInt:
                return VK_FORMAT_R8G8_SINT;
            case GfxImageFormat::R16_Unorm:
                return VK_FORMAT_R16_UNORM;
            case GfxImageFormat::R16_Snorm:
                return VK_FORMAT_R16_SNORM;
            case GfxImageFormat::R16_UInt:
                return VK_FORMAT_R16_UINT;
            case GfxImageFormat::R16_SInt:
                return VK_FORMAT_R16_SINT;
            case GfxImageFormat::R16_Float:
                return VK_FORMAT_R16_SFLOAT;
            case GfxImageFormat::Z16_Depth:
                return VK_FORMAT_D16_UNORM;
            case GfxImageFormat::R8G8B8A8_Unorm:
                return VK_FORMAT_R8G8B8A8_UNORM;
            case GfxImageFormat::R8G8B8A8_Snorm:
                return VK_FORMAT_R8G8B8A8_SNORM;
            case GfxImageFormat::R8G8B8A8_UInt:
                return VK_FORMAT_R8G8B8A8_UINT;
            case GfxImageFormat::R8G8B8A8_SInt:
                return VK_FORMAT_R8G8B8A8_SINT;
            case GfxImageFormat::R8G8B8A8_SRGB:
                return VK_FORMAT_R8G8B8A8_SRGB;
            case GfxImageFormat::B8G8R8A8_Unorm:
                return VK_FORMAT_B8G8R8A8_UNORM;
            case GfxImageFormat::B8G8R8A8_SRGB:
                return VK_FORMAT_B8G8R8A8_SRGB;
            case GfxImageFormat::R9G9B9E5F_SharedFloat:
                return VK_FORMAT_E5B9G9R9_UFLOAT_PACK32;
            case GfxImageFormat::R10G10B10A2_Unorm:
                return VK_FORMAT_A2B10G10R10_UNORM_PACK32;
            case GfxImageFormat::R10G10B10A2_UInt:
                return VK_FORMAT_A2B10G10R10_UINT_PACK32;
            case GfxImageFormat::R11G11B10F_Float:
                return VK_FORMAT_B10G11R11_UFLOAT_PACK32;
            case GfxImageFormat::R16G16_Unorm:
                return VK_FORMAT_R16_UNORM;
            case GfxImageFormat::R16G16_Snorm:
                return VK_FORMAT_R16_SNORM;
            case GfxImageFormat::R16G16_UInt:
                return VK_FORMAT_R16_UINT;
            case GfxImageFormat::R16G16_SInt:
                return VK_FORMAT_R16_SINT;
            case GfxImageFormat::R16G16_Float:
                return VK_FORMAT_R16_SFLOAT;
            case GfxImageFormat::D24S8_Depth:
                return VK_FORMAT_D24_UNORM_S8_UINT;
            case GfxImageFormat::R32_UInt:
                return VK_FORMAT_R32_UINT;
            case GfxImageFormat::R32_SInt:
                return VK_FORMAT_R32_SINT;
            case GfxImageFormat::R32_Float:
                return VK_FORMAT_R32_SFLOAT;
            case GfxImageFormat::D32F_Depth:
                return VK_FORMAT_D32_SFLOAT;
            case GfxImageFormat::R16G16B16A16_Unorm:
                return VK_FORMAT_R16G16B16A16_UNORM;
            case GfxImageFormat::R16G16B16A16_Snorm:
                return VK_FORMAT_R16G16B16A16_SNORM;
            case GfxImageFormat::R16G16B16A16_UInt:
                return VK_FORMAT_R16G16B16A16_UINT;
            case GfxImageFormat::R16G16B16A16_SInt:
                return VK_FORMAT_R16G16B16A16_SINT;
            case GfxImageFormat::R16G16B16A16_Float:
                return VK_FORMAT_R16G16B16A16_SFLOAT;
            case GfxImageFormat::D32FS8_Depth:
                return VK_FORMAT_D32_SFLOAT_S8_UINT;
            case GfxImageFormat::R32G32_UInt:
                return VK_FORMAT_R32G32_UINT;
            case GfxImageFormat::R32G32_SInt:
                return VK_FORMAT_R32G32_SINT;
            case GfxImageFormat::R32G32_Float:
                return VK_FORMAT_R32G32_SFLOAT;
            case GfxImageFormat::R32G32B32_UInt:
                return VK_FORMAT_R32G32B32_UINT;
            case GfxImageFormat::R32G32B32_SInt:
                return VK_FORMAT_R32G32B32_SINT;
            case GfxImageFormat::R32G32B32_Float:
                return VK_FORMAT_R32G32B32_SFLOAT;
            case GfxImageFormat::R32G32B32A32_UInt:
                return VK_FORMAT_R32G32B32A32_UINT;
            case GfxImageFormat::R32G32B32A32_SInt:
                return VK_FORMAT_R32G32B32A32_SINT;
            case GfxImageFormat::R32G32B32A32_Float:
                return VK_FORMAT_R32G32B32A32_SFLOAT;
            case GfxImageFormat::BC1_Unorm:
                return VK_FORMAT_BC1_RGBA_UNORM_BLOCK;
            case GfxImageFormat::BC1_SRGB:
                return VK_FORMAT_BC1_RGBA_SRGB_BLOCK;
            case GfxImageFormat::BC2_Unorm:
                return VK_FORMAT_BC2_UNORM_BLOCK;
            case GfxImageFormat::BC2_SRGB:
                return VK_FORMAT_BC2_SRGB_BLOCK;
            case GfxImageFormat::BC3_Unorm:
                return VK_FORMAT_BC3_UNORM_BLOCK;
            case GfxImageFormat::BC3_SRGB:
                return VK_FORMAT_BC3_SRGB_BLOCK;
            case GfxImageFormat::BC4_Unorm:
                return VK_FORMAT_BC4_UNORM_BLOCK;
            case GfxImageFormat::BC4_Snorm:
                return VK_FORMAT_BC4_SNORM_BLOCK;
            case GfxImageFormat::BC5_Unorm:
                return VK_FORMAT_BC5_UNORM_BLOCK;
            case GfxImageFormat::BC5_Snorm:
                return VK_FORMAT_BC5_SNORM_BLOCK;
            case GfxImageFormat::BC6H_SF16:
                return VK_FORMAT_BC6H_SFLOAT_BLOCK;
            case GfxImageFormat::BC6H_UF16:
                return VK_FORMAT_BC6H_UFLOAT_BLOCK;
            case GfxImageFormat::BC7U_Unorm:
                return VK_FORMAT_BC7_UNORM_BLOCK;
            case GfxImageFormat::BC7U_SRGB:
                return VK_FORMAT_BC7_SRGB_BLOCK;
            case GfxImageFormat::ASTC_4X4_Unorm:
                return VK_FORMAT_ASTC_4x4_UNORM_BLOCK;
            case GfxImageFormat::ASTC_4X4_SRGB:
                return VK_FORMAT_ASTC_4x4_SRGB_BLOCK;
            case GfxImageFormat::ASTC_5X4_Unorm:
                return VK_FORMAT_ASTC_5x4_UNORM_BLOCK;
            case GfxImageFormat::ASTC_5X4_SRGB:
                return VK_FORMAT_ASTC_5x4_SRGB_BLOCK;
            case GfxImageFormat::ASTC_5X5_Unorm:
                return VK_FORMAT_ASTC_5x5_UNORM_BLOCK;
            case GfxImageFormat::ASTC_5X5_SRGB:
                return VK_FORMAT_ASTC_5x5_SRGB_BLOCK;
            case GfxImageFormat::ASTC_6X5_Unorm:
                return VK_FORMAT_ASTC_6x5_UNORM_BLOCK;
            case GfxImageFormat::ASTC_6X5_SRGB:
                return VK_FORMAT_ASTC_6x5_SRGB_BLOCK;
            case GfxImageFormat::ASTC_6X6_Unorm:
                return VK_FORMAT_ASTC_6x6_UNORM_BLOCK;
            case GfxImageFormat::ASTC_6X6_SRGB:
                return VK_FORMAT_ASTC_6x6_SRGB_BLOCK;
            case GfxImageFormat::ASTC_8X5_Unorm:
                return VK_FORMAT_ASTC_8x5_UNORM_BLOCK;
            case GfxImageFormat::ASTC_8X5_SRGB:
                return VK_FORMAT_ASTC_8x5_SRGB_BLOCK;
            case GfxImageFormat::ASTC_8X6_Unorm:
                return VK_FORMAT_ASTC_8x6_UNORM_BLOCK;
            case GfxImageFormat::ASTC_8X6_SRGB:
                return VK_FORMAT_ASTC_8x6_SRGB_BLOCK;
            case GfxImageFormat::ASTC_8X8_Unorm:
                return VK_FORMAT_ASTC_8x8_UNORM_BLOCK;
            case GfxImageFormat::ASTC_8X8_SRGB:
                return VK_FORMAT_ASTC_8x8_SRGB_BLOCK;
            case GfxImageFormat::ASTC_10X5_Unorm:
                return VK_FORMAT_ASTC_10x5_UNORM_BLOCK;
            case GfxImageFormat::ASTC_10X5_SRGB:
                return VK_FORMAT_ASTC_10x5_SRGB_BLOCK;
            case GfxImageFormat::ASTC_10X6_Unorm:
                return VK_FORMAT_ASTC_10x6_UNORM_BLOCK;
            case GfxImageFormat::ASTC_10X6_SRGB:
                return VK_FORMAT_ASTC_10x6_SRGB_BLOCK;
            case GfxImageFormat::ASTC_10X8_Unorm:
                return VK_FORMAT_ASTC_10x8_UNORM_BLOCK;
            case GfxImageFormat::ASTC_10X8_SRGB:
                return VK_FORMAT_ASTC_10x8_SRGB_BLOCK;
            case GfxImageFormat::ASTC_10X10_Unorm:
                return VK_FORMAT_ASTC_10x10_UNORM_BLOCK;
            case GfxImageFormat::ASTC_10X10_SRGB:
                return VK_FORMAT_ASTC_10x10_SRGB_BLOCK;
            case GfxImageFormat::ASTC_12X10_Unorm:
                return VK_FORMAT_ASTC_12x10_UNORM_BLOCK;
            case GfxImageFormat::ASTC_12X10_SRGB:
                return VK_FORMAT_ASTC_12x10_SRGB_BLOCK;
            case GfxImageFormat::ASTC_12X12_Unorm:
                return VK_FORMAT_ASTC_12x12_UNORM_BLOCK;
            case GfxImageFormat::ASTC_12X12_SRGB:
                return VK_FORMAT_ASTC_12x12_SRGB_BLOCK;
            case GfxImageFormat::B5G5R5A1_Unorm:
                return VK_FORMAT_B5G5R5A1_UNORM_PACK16;
            default:
                break;
		}
        return VK_FORMAT_UNDEFINED;
	}
}
