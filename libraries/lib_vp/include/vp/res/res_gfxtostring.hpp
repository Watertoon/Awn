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

    constexpr inline const char *GfxShaderStageToString(GfxShaderStage shader_stage) {
        switch (shader_stage) {
            case GfxShaderStage::Vertex:
                return "Vertex";
            case GfxShaderStage::TessellationControl:
                return "TessellationControl";
            case GfxShaderStage::TessellationEvaluation:
                return "TessellationEvaluation";
            case GfxShaderStage::Geometry:
                return "Geometry";
            case GfxShaderStage::Fragment:
                return "Fragment";
            case GfxShaderStage::Compute:
                return "Compute";
            case GfxShaderStage::Task:
                return "Task";
            case GfxShaderStage::Mesh:
                return "Mesh";
            default:
                break;
        };
        return "";
    }

    constexpr inline const char *GfxShaderCodeTypeToString(GfxShaderCodeType shader_code_type) {
        switch (shader_code_type) {
            case GfxShaderCodeType::Binary:
                return "Binary";
            case GfxShaderCodeType::Intermediate:
                return "Intermediate";
            case GfxShaderCodeType::Source:
                return "Source";
            default:
                break;
        };
        return "";
    }

    constexpr inline void GfxMemoryPoolFlagsToString(vp::util::FixedString<0x80> *out_string, GfxMemoryPoolFlags memory_pool_flags) {

        /* Reset string */
        out_string->Clear();

        /* Build string */
        bool is_first = true;
        const auto print_or_lambda = [&]() {
            if (is_first == false) {
                out_string->Append(" | ");
            } else {                
                is_first = false;
            }
        };
        
        if ((memory_pool_flags & GfxMemoryPoolFlags::CpuNoAccess) == GfxMemoryPoolFlags::CpuNoAccess) {
            is_first = false;
            out_string->Append("CpuNoAccess");
        }
        if ((memory_pool_flags & GfxMemoryPoolFlags::CpuUncached) == GfxMemoryPoolFlags::CpuUncached) {
            print_or_lambda();
            out_string->Append("CpuUncached");
        }
        if ((memory_pool_flags & GfxMemoryPoolFlags::CpuCached) == GfxMemoryPoolFlags::CpuCached) {
            print_or_lambda();
            out_string->Append("CpuCached");
        }
        if ((memory_pool_flags & GfxMemoryPoolFlags::GpuNoAccess) == GfxMemoryPoolFlags::GpuNoAccess) {
            print_or_lambda();
            out_string->Append("GpuNoAccess");
        }
        if ((memory_pool_flags & GfxMemoryPoolFlags::GpuUncached) == GfxMemoryPoolFlags::GpuUncached) {
            print_or_lambda();
            out_string->Append("GpuUncached");
        }
        if ((memory_pool_flags & GfxMemoryPoolFlags::GpuCached) == GfxMemoryPoolFlags::GpuCached) {
            print_or_lambda();
            out_string->Append("GpuCached");
        }
        if ((memory_pool_flags & GfxMemoryPoolFlags::ShaderCode) == GfxMemoryPoolFlags::ShaderCode) {
            print_or_lambda();
            out_string->Append("ShaderCode");
        }
        if ((memory_pool_flags & GfxMemoryPoolFlags::Compressible) == GfxMemoryPoolFlags::Compressible) {
            print_or_lambda();
            out_string->Append("Compressible");
        }
        if ((memory_pool_flags & GfxMemoryPoolFlags::Physical) == GfxMemoryPoolFlags::Physical) {
            print_or_lambda();
            out_string->Append("Physical");
        }
        if ((memory_pool_flags & GfxMemoryPoolFlags::Virtual) == GfxMemoryPoolFlags::Virtual) {
            print_or_lambda();
            out_string->Append("Virtual");
        }

        return;
    }

    constexpr inline const char *GfxWrapModeToString(GfxWrapMode wrap_mode) {
        switch (wrap_mode) {
            case GfxWrapMode::Repeat:
                return "Repeat";
            case GfxWrapMode::MirrorRepeat:
                return "MirrorRepeat";
            case GfxWrapMode::ClampToEdge:
                return "ClampToEdge";
            case GfxWrapMode::ClampToBorder:
                return "ClamptToBorder";
            case GfxWrapMode::MirrorClampToEdge:
                return "MirrorClampToEdge";
            default:
                break;
        }
        return "Invalid";
    }

    constexpr inline const char *GfxMipMapFilterToString(GfxMipMapFilter mip_map_filter) {
        switch (mip_map_filter) {
            case GfxMipMapFilter::None:
                return "None";
            case GfxMipMapFilter::Nearest:
                return "Nearest";
            case GfxMipMapFilter::Linear:
                return "Linear";
            default:
                break;
        }
        return "Invalid";
    }

    constexpr inline const char *GfxMagFilterToString(GfxMagFilter mag_filter) {
        switch (mag_filter) {
            case GfxMagFilter::Nearest:
                return "Nearest";
            case GfxMagFilter::Linear:
                return "Linear";
            default:
                break;
        }
        return "Invalid";
    }

    constexpr inline const char *GfxMinFilterToString(GfxMinFilter min_filter) {
        switch (min_filter) {
            case GfxMinFilter::Nearest:
                return "Nearest";
            case GfxMinFilter::Linear:
                return "Linear";
            default:
                break;
        }
        return "Invalid";
    }

    constexpr inline const char *GfxReductionFilterToString(GfxReductionFilter reduction_filter) {
        switch (reduction_filter) {
            case GfxReductionFilter::Average:
                return "Average";
            case GfxReductionFilter::Min:
                return "Min";
            case GfxReductionFilter::Max:
                return "Max";
            default:
                break;
        }
        return "Invalid";
    }

    constexpr inline const char *GfxCompareOperationToString(GfxCompareOperation compare_op) {
        switch (compare_op) {
            case GfxCompareOperation::Never:
                return "Never";
            case GfxCompareOperation::LessThan:
                return "LessThan";
            case GfxCompareOperation::Equal:
                return "Equal";
            case GfxCompareOperation::LessThanEqual:
                return "LessThanEqual";
            case GfxCompareOperation::GreaterThan:
                return "GreaterThan";
            case GfxCompareOperation::NotEqual:
                return "NotEqual";
            case GfxCompareOperation::GreaterThanEqual:
                return "GreaterThanEqual";
            case GfxCompareOperation::Always:
                return "Always";
            default:
                break;
        }
        return "Invalid";
    }

    constexpr inline const char *GfxBorderColorToString(GfxBorderColor border_color) {
        switch (border_color) {
            case GfxBorderColor::White:
                return "White";
            case GfxBorderColor::TransparentBlack:
                return "TransparentBlack";
            case GfxBorderColor::Black:
                return "Black";
            default:
                break;
        }
        return "Invalid";
    }

    constexpr inline const char *GfxPrimitiveTopologyToString(GfxPrimitiveTopology primitive_topology) {
        switch (primitive_topology) {
            case GfxPrimitiveTopology::Points:
                return "Points";
            case GfxPrimitiveTopology::Lines:
                return "Lines";
            case GfxPrimitiveTopology::LineStrip:
                return "LineStrip";
            case GfxPrimitiveTopology::Triangles:
                return "Triangles";
            case GfxPrimitiveTopology::TrianglesStrip:
                return "TrianglesStrip";
            case GfxPrimitiveTopology::LinesAdjacency:
                return "LinesAdjacency";
            case GfxPrimitiveTopology::LineStripAdjacency:
                return "LineStripAdjacency";
            case GfxPrimitiveTopology::TrianglesAdjacency:
                return "TrianglesAdjacency";
            case GfxPrimitiveTopology::TriangleStripAdjacency:
                return "TriangleStripAdjacency";
            case GfxPrimitiveTopology::Patches:
                return "Patches";
            default:
                break;
        }
        return "Invalid";
    }

    constexpr inline const char *GfxIndexFormatToString(GfxIndexFormat index_format) {
        switch (index_format) {
            case GfxIndexFormat::U8:
                return "U8";
            case GfxIndexFormat::U16:
                return "U16";
            case GfxIndexFormat::U32:
                return "U32";
            default:
                break;
        }
        return "Invalid";
    }

    constexpr inline const char *GfxImageFormatToString(GfxImageFormat image_format) {
        switch (image_format) {
            case GfxImageFormat::R8_Unorm:
                return "R8_Unorm";
            case GfxImageFormat::R8_Snorm:
                return "R8_Snorm";
            case GfxImageFormat::R8_UInt:
                return "R8_UInt";
            case GfxImageFormat::R8_SInt:
                return "R8_SInt";
            case GfxImageFormat::R4G4B4A4_Unorm:
                return "R4G4B4A4_Unorm";
            case GfxImageFormat::R5G5B5A1_Unorm:
                return "R5G5B5A1_Unorm";
            case GfxImageFormat::A1B5G5R5_Unorm:
                return "A1B5G5R5_Unorm";
            case GfxImageFormat::R5G6B5_Unorm:
                return "R5G6B5_Unorm";
            case GfxImageFormat::B5G6R5_Unorm:
                return "B5G6R5_Unorm";
            case GfxImageFormat::R8G8_Unorm:
                return "R8G8_Unorm";
            case GfxImageFormat::R8G8_Snorm:
                return "R8G8_Snorm";
            case GfxImageFormat::R8G8_UInt:
                return "R8G8_UInt";
            case GfxImageFormat::R8G8_SInt:
                return "R8G8_SInt";
            case GfxImageFormat::R16_Unorm:
                return "R16_Unorm";
            case GfxImageFormat::R16_Snorm:
                return "R16_Snorm";
            case GfxImageFormat::R16_UInt:
                return "R16_UInt";
            case GfxImageFormat::R16_SInt:
                return "R16_SInt";
            case GfxImageFormat::R16_Float:
                return "R16_Float";
            case GfxImageFormat::Z16_Depth:
                return "Z16_Depth";
            case GfxImageFormat::R8G8B8A8_Unorm:
                return "R8G8B8A8_Unorm";
            case GfxImageFormat::R8G8B8A8_Snorm:
                return "R8G8B8A8_Snorm";
            case GfxImageFormat::R8G8B8A8_UInt:
                return "R8G8B8A8_UInt";
            case GfxImageFormat::R8G8B8A8_SInt:
                return "R8G8B8A8_SInt";
            case GfxImageFormat::R8G8B8A8_SRGB:
                return "R8G8B8A8_SRGB";
            case GfxImageFormat::B8G8R8A8_Unorm:
                return "B8G8R8A8_Unorm";
            case GfxImageFormat::B8G8R8A8_SRGB:
                return "B8G8R8A8_SRGB";
            case GfxImageFormat::R9G9B9E5F_SharedFloat:
                return "R9G9B9E5F_SharedFloat";
            case GfxImageFormat::R10G10B10A2_Unorm:
                return "R10G10B10A2_Unorm";
            case GfxImageFormat::R10G10B10A2_UInt:
                return "R10G10B10A2_UInt";
            case GfxImageFormat::R11G11B10F_Float:
                return "R11G11B10F_Float";
            case GfxImageFormat::R16G16_Unorm:
                return "R16G16_Unorm";
            case GfxImageFormat::R16G16_Snorm:
                return "R16G16_Snorm";
            case GfxImageFormat::R16G16_UInt:
                return "R16G16_UInt";
            case GfxImageFormat::R16G16_SInt:
                return "R16G16_SInt";
            case GfxImageFormat::R16G16_Float:
                return "R16G16_Float";
            case GfxImageFormat::D24S8_Depth:
                return "D24S8_Depth";
            case GfxImageFormat::R32_UInt:
                return "R32_UInt";
            case GfxImageFormat::R32_SInt:
                return "R32_SInt";
            case GfxImageFormat::R32_Float:
                return "R32_Float";
            case GfxImageFormat::D32F_Depth:
                return "D32F_Depth";
            case GfxImageFormat::R16G16B16A16_Unorm:
                return "R16G16B16A16_Unorm";
            case GfxImageFormat::R16G16B16A16_Snorm:
                return "R16G16B16A16_Snorm";
            case GfxImageFormat::R16G16B16A16_UInt:
                return "R16G16B16A16_UInt";
            case GfxImageFormat::R16G16B16A16_SInt:
                return "R16G16B16A16_SInt";
            case GfxImageFormat::R16G16B16A16_Float:
                return "R16G16B16A16_Float";
            case GfxImageFormat::D32FS8_Depth:
                return "D32FS8_Depth";
            case GfxImageFormat::R32G32_UInt:
                return "R32G32_UInt";
            case GfxImageFormat::R32G32_SInt:
                return "R32G32_SInt";
            case GfxImageFormat::R32G32_Float:
                return "R32G32_Float";
            case GfxImageFormat::R32G32B32_UInt:
                return "R32G32B32_UInt";
            case GfxImageFormat::R32G32B32_SInt:
                return "R32G32B32_SInt";
            case GfxImageFormat::R32G32B32_Float:
                return "R32G32B32_Float";
            case GfxImageFormat::R32G32B32A32_UInt:
                return "R32G32B32A32_UInt";
            case GfxImageFormat::R32G32B32A32_SInt:
                return "R32G32B32A32_SInt";
            case GfxImageFormat::R32G32B32A32_Float:
                return "R32G32B32A32_Float";
            case GfxImageFormat::BC1_Unorm:
                return "BC1_Unorm";
            case GfxImageFormat::BC1_SRGB:
                return "BC1_SRGB";
            case GfxImageFormat::BC2_Unorm:
                return "BC2_Unorm";
            case GfxImageFormat::BC2_SRGB:
                return "BC2_SRGB";
            case GfxImageFormat::BC3_Unorm:
                return "BC3_Unorm";
            case GfxImageFormat::BC3_SRGB:
                return "BC3_SRGB";
            case GfxImageFormat::BC4_Unorm:
                return "BC4_Unorm";
            case GfxImageFormat::BC4_Snorm:
                return "BC4_Snorm";
            case GfxImageFormat::BC5_Unorm:
                return "BC5_Unorm";
            case GfxImageFormat::BC5_Snorm:
                return "BC5_Snorm";
            case GfxImageFormat::BC6H_SF16:
                return "BC6H_SF16";
            case GfxImageFormat::BC6H_UF16:
                return "BC6H_UF16";
            case GfxImageFormat::BC7U_Unorm:
                return "BC7U_Unorm";
            case GfxImageFormat::BC7U_SRGB:
                return "BC7U_SRGB";
            case GfxImageFormat::ASTC_4x4_Unorm:
                return "ASTC_4x4_Unorm";
            case GfxImageFormat::ASTC_4x4_SRGB:
                return "ASTC_4x4_SRGB";
            case GfxImageFormat::ASTC_5x4_Unorm:
                return "ASTC_5x4_Unorm";
            case GfxImageFormat::ASTC_5x4_SRGB:
                return "ASTC_5x4_SRGB";
            case GfxImageFormat::ASTC_5x5_Unorm:
                return "ASTC_5x5_Unorm";
            case GfxImageFormat::ASTC_5x5_SRGB:
                return "ASTC_5x5_SRGB";
            case GfxImageFormat::ASTC_6x5_Unorm:
                return "ASTC_6x5_Unorm";
            case GfxImageFormat::ASTC_6x5_SRGB:
                return "ASTC_6x5_SRGB";
            case GfxImageFormat::ASTC_6x6_Unorm:
                return "ASTC_6x6_Unorm";
            case GfxImageFormat::ASTC_6x6_SRGB:
                return "ASTC_6x6_SRGB";
            case GfxImageFormat::ASTC_8x5_Unorm:
                return "ASTC_8x5_Unorm";
            case GfxImageFormat::ASTC_8x5_SRGB:
                return "ASTC_8x5_SRGB";
            case GfxImageFormat::ASTC_8x6_Unorm:
                return "ASTC_8x6_Unorm";
            case GfxImageFormat::ASTC_8x6_SRGB:
                return "ASTC_8x6_SRGB";
            case GfxImageFormat::ASTC_8x8_Unorm:
                return "ASTC_8x8_Unorm";
            case GfxImageFormat::ASTC_8x8_SRGB:
                return "ASTC_8x8_SRGB";
            case GfxImageFormat::ASTC_10x5_Unorm:
                return "ASTC_10x5_Unorm";
            case GfxImageFormat::ASTC_10x5_SRGB:
                return "ASTC_10x5_SRGB";
            case GfxImageFormat::ASTC_10x6_Unorm:
                return "ASTC_10x6_Unorm";
            case GfxImageFormat::ASTC_10x6_SRGB:
                return "ASTC_10x6_SRGB";
            case GfxImageFormat::ASTC_10x8_Unorm:
                return "ASTC_10x8_Unorm";
            case GfxImageFormat::ASTC_10x8_SRGB:
                return "ASTC_10x8_SRGB";
            case GfxImageFormat::ASTC_10x10_Unorm:
                return "ASTC_10x10_Unorm";
            case GfxImageFormat::ASTC_10x10_SRGB:
                return "ASTC_10x10_SRGB";
            case GfxImageFormat::ASTC_12x10_Unorm:
                return "ASTC_12x10_Unorm";
            case GfxImageFormat::ASTC_12x10_SRGB:
                return "ASTC_12x10_SRGB";
            case GfxImageFormat::ASTC_12x12_Unorm:
                return "ASTC_12x12_Unorm";
            case GfxImageFormat::ASTC_12x12_SRGB:
                return "ASTC_12x12_SRGB";
            case GfxImageFormat::B5G5R5A1_Unorm:
                return "B5G5R5A1_Unorm";
            default:
                break;
        }
        return "Invalid";
    }

    constexpr inline const char *GfxAttributeFormatToString(GfxAttributeFormat attribute_format) {
        switch (attribute_format) {
            case GfxAttributeFormat::None_Unorm:
                return "None_Unorm";
            case GfxAttributeFormat::R8_Unorm:
                return "R8_Unorm";
            case GfxAttributeFormat::R8_Snorm:
                return "R8_Snorm";
            case GfxAttributeFormat::R8_UInt:
                return "R8_UInt";
            case GfxAttributeFormat::R8_SInt:
                return "R8_SInt";
            case GfxAttributeFormat::R8_UScaled:
                return "R8_UScaled";
            case GfxAttributeFormat::R8_SScaled:
                return "R8_SScaled";
            case GfxAttributeFormat::R8G8_Unorm:
                return "R8G8_Unorm";
            case GfxAttributeFormat::R8G8_Snorm:
                return "R8G8_Snorm";
            case GfxAttributeFormat::R8G8_UInt:
                return "R8G8_UInt";
            case GfxAttributeFormat::R8G8_SInt:
                return "R8G8_SInt";
            case GfxAttributeFormat::R8G8_UScaled:
                return "R8G8_UScaled";
            case GfxAttributeFormat::R8G8_SScaled:
                return "R8G8_SScaled";
            case GfxAttributeFormat::R16_Unorm:
                return "R16_Unorm";
            case GfxAttributeFormat::R16_Snorm:
                return "R16_Snorm";
            case GfxAttributeFormat::R16_UInt:
                return "R16_UInt";
            case GfxAttributeFormat::R16_SInt:
                return "R16_SInt";
            case GfxAttributeFormat::R16_Float:
                return "R16_Float";
            case GfxAttributeFormat::R16_UScaled:
                return "R16_UScaled";
            case GfxAttributeFormat::R16_SScaled:
                return "R16_SScaled";
            case GfxAttributeFormat::R8G8B8A8_Unorm:
                return "R8G8B8A8_Unorm";
            case GfxAttributeFormat::R8G8B8A8_Snorm:
                return "R8G8B8A8_Snorm";
            case GfxAttributeFormat::R8G8B8A8_UInt:
                return "R8G8B8A8_UInt";
            case GfxAttributeFormat::R8G8B8A8_SInt:
                return "R8G8B8A8_SInt";
            case GfxAttributeFormat::R8G8B8A8_UScaled:
                return "R8G8B8A8_UScaled";
            case GfxAttributeFormat::R8G8B8A8_SScaled:
                return "R8G8B8A8_SScaled";
            case GfxAttributeFormat::R10G10B10A2_Unorm:
                return "R10G10B10A2_Unorm";
            case GfxAttributeFormat::R10G10B10A2_Snorm:
                return "R10G10B10A2_Snorm";
            case GfxAttributeFormat::R10G10B10A2_UInt:
                return "R10G10B10A2_UInt";
            case GfxAttributeFormat::R10G10B10A2_SInt:
                return "R10G10B10A2_SInt";
            case GfxAttributeFormat::R16G16_Unorm:
                return "R16G16_Unorm";
            case GfxAttributeFormat::R16G16_Snorm:
                return "R16G16_Snorm";
            case GfxAttributeFormat::R16G16_UInt:
                return "R16G16_UInt";
            case GfxAttributeFormat::R16G16_SInt:
                return "R16G16_SInt";
            case GfxAttributeFormat::R16G16_Float:
                return "R16G16_Float";
            case GfxAttributeFormat::R16G16_UScaled:
                return "R16G16_UScaled";
            case GfxAttributeFormat::R16G16_SScaled:
                return "R16G16_SScaled";
            case GfxAttributeFormat::R32_UInt:
                return "R32_UInt";
            case GfxAttributeFormat::R32_SInt:
                return "R32_SInt";
            case GfxAttributeFormat::R32_Float:
                return "R32_Float";
            case GfxAttributeFormat::R16G16B16A16_Unorm:
                return "R16G16B16A16_Unorm";
            case GfxAttributeFormat::R16G16B16A16_Snorm:
                return "R16G16B16A16_Snorm";
            case GfxAttributeFormat::R16G16B16A16_UInt:
                return "R16G16B16A16_UInt";
            case GfxAttributeFormat::R16G16B16A16_SInt:
                return "R16G16B16A16_SInt";
            case GfxAttributeFormat::R16G16B16A16_Float:
                return "R16G16B16A16_Float";
            case GfxAttributeFormat::R16G16B16A16_UScaled:
                return "R16G16B16A16_UScaled";
            case GfxAttributeFormat::R16G16B16A16_SScaled:
                return "R16G16B16A16_SScaled";
            case GfxAttributeFormat::R32G32_UInt:
                return "R32G32_UInt";
            case GfxAttributeFormat::R32G32_SInt:
                return "R32G32_SInt";
            case GfxAttributeFormat::R32G32_Float:
                return "R32G32_Float";
            case GfxAttributeFormat::R32G32B32_UInt:
                return "R32G32B32_UInt";
            case GfxAttributeFormat::R32G32B32_SInt:
                return "R32G32B32_SInt";
            case GfxAttributeFormat::R32G32B32_Float:
                return "R32G32B32_Float";
            case GfxAttributeFormat::R32G32B32A32_UInt:
                return "R32G32B32A32_UInt";
            case GfxAttributeFormat::R32G32B32A32_SInt:
                return "R32G32B32A32_SInt";
            case GfxAttributeFormat::R32G32B32A32_Float:
                return "R32G32B32A32_Float";
            default:
                break;
        }
        return "Invalid";
    }
}
