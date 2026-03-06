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

    constexpr inline const size_t cMaxGfxMemoryPoolSize  = 0x120;
    constexpr inline const size_t cMaxGfxBufferSize      = 0x48;
    constexpr inline const size_t cMaxGfxTextureSize     = 0xd8;
    constexpr inline const size_t cMaxGfxTextureViewSize = 0x48;
    constexpr inline const size_t cMaxGfxSamplerSize     = 0x78;
    constexpr inline const size_t cMaxGfxShaderSize      = 0xe8;

    struct ResBntx;

    struct GfxBindTextureView {
        void *texture_view;
        u64   texture_view_decriptor_slot;
    };

    using GfxBindTextureCallback = GfxBindTextureView (*) (const char *texture_name, void *user_data);

    enum class GfxMemoryPoolFlags : u32 {
        CpuNoAccess  = (1 << 0),
        CpuUncached  = (1 << 1),
        CpuCached    = (1 << 2),
        GpuNoAccess  = (1 << 3),
        GpuUncached  = (1 << 4),
        GpuCached    = (1 << 5),
        ShaderCode   = (1 << 6),
        Compressible = (1 << 7),
        Physical     = (1 << 8),
        Virtual      = (1 << 9),
    };
    VP_ENUM_FLAG_TRAITS(GfxMemoryPoolFlags);

    enum class GfxGpuAccessFlags : u32 {
        VertexBuffer        = (1 << 2),
        IndexBuffer         = (1 << 3),
        UniformBuffer       = (1 << 4),
        Texture             = (1 << 5),
        StorageBuffer       = (1 << 6),
        RenderTargetColor   = (1 << 7),
        RenderTargetDepth   = (1 << 8),
        IndirectDraw        = (1 << 9),
        DisplayTexture      = (1 << 10),
        Counter             = (1 << 11),
    };
    VP_ENUM_FLAG_TRAITS(GfxGpuAccessFlags);

    struct ResGfxMemoryPoolInfo {
        u32   memory_pool_flags;
        u32   size;
        void *gpu_region_base;
        
        constexpr void SetDefaults() {
            this->memory_pool_flags = static_cast<u32>(GfxMemoryPoolFlags::CpuUncached | GfxMemoryPoolFlags::GpuCached);
            this->size = 0;
            this->gpu_region_base = nullptr;
        }
        constexpr void SetMemoryPropertyFlags(GfxMemoryPoolFlags property_flags) {
            this->memory_pool_flags = static_cast<u32>(property_flags);
        }
        constexpr void SetSize(u32 size) {
            this->size = size;
        }
        constexpr void SetGpuRegionBase(void *address) {
            this->gpu_region_base = address;
        }

        constexpr GfxMemoryPoolFlags GetMemoryPropertyFlags() const {
            return static_cast<GfxMemoryPoolFlags>(this->memory_pool_flags);
        }
    };
    static_assert(sizeof(ResGfxMemoryPoolInfo) == 0x10);

    struct ResGfxBufferInfo {
        u32 size;
        u32 gpu_access_flags;
        u32 reserve0;
        u32 reserve1;

        constexpr void SetDefaults() {
            this->size = 0;
            this->gpu_access_flags = 0;
        }

        constexpr void SetSize(u32 size) {
            this->size = size;
        }
        constexpr void SetGpuAccessFlags(GfxGpuAccessFlags gpu_access_flags) {
            this->gpu_access_flags = static_cast<u32>(gpu_access_flags);
        }
    };
    static_assert(sizeof(ResGfxBufferInfo) == 0x10);

    enum class GfxChannelSource : u8 {
        Zero  = 0x0,
        One   = 0x1,
        Red   = 0x2,
        Green = 0x3,
        Blue  = 0x4,
        Alpha = 0x5,
    };

    enum class GfxChannelFormat : u8 {
        None         = 0x1,
        R8           = 0x2,
        R4G4B4A4     = 0x3,
        R5G5B5A1     = 0x5,
        A1B5G5R5     = 0x6,
        R5G6B5       = 0x7,
        B5G6R5       = 0x8,
        R8G8         = 0x9,
        R16          = 0xa,
        R8G8B8A8     = 0xb,
        B8G8R8A8     = 0xc,
        R9G9B9E5F    = 0xd,
        R10G10B10A2  = 0xe,
        R11G11B10F   = 0xf,
        R16G16       = 0x12,
        D24S8        = 0x13,
        R32          = 0x14,
        R16G16B16A16 = 0x15,
        D32FS8       = 0x16,
        R32G32       = 0x17,
        R32G32B32    = 0x18,
        R32G32B32A32 = 0x19,
        BC1          = 0x1a,
        BC2          = 0x1b,
        BC3          = 0x1c,
        BC4          = 0x1d,
        BC5          = 0x1e,
        BC6H         = 0x1f,
        BC7U         = 0x20,
        ASTC_4x4     = 0x2d,
        ASTC_5x4     = 0x2e,
        ASTC_5x5     = 0x2f,
        ASTC_6x5     = 0x30,
        ASTC_6x6     = 0x31,
        ASTC_8x5     = 0x32,
        ASTC_8x6     = 0x33,
        ASTC_8x8     = 0x34,
        ASTC_10x5    = 0x35,
        ASTC_10x6    = 0x36,
        ASTC_10x8    = 0x37,
        ASTC_10x10   = 0x38,
        ASTC_12x10   = 0x39,
        ASTC_12x12   = 0x3a,
        B5G5R5A1     = 0x3b,
    };

    static constexpr inline u32 cVariableBlockWidthTable[] = {
        0x10, 0x08, 0x10, 0x08,
        0x08, 0x04, 0x04, 0x05,
        0x05, 0x06, 0x06, 0x08,
        0x08, 0x08, 0x0a, 0x0a,
        0x0a, 0x0a, 0x0c, 0x0c,
    };
    static constexpr inline u32 cVariableBlockHeightTable[] = {
        0x08, 0x08, 0x08, 0x08,
        0x04, 0x04, 0x04, 0x04,
        0x05, 0x05, 0x06, 0x05,
        0x06, 0x08, 0x05, 0x06,
        0x08, 0x0a, 0x0a, 0x0c,
    };

    static constexpr inline u32 cPackagedTextureTexelSizeTable[] = {
        0x08, 0x10, 0x10, 0x08,
        0x10, 0x10, 0x10, 0x08,
        0x10, 0x08, 0x08, 0x08,
        0x10, 0x20, 0x20, 0x20,
        0x20, 0x08, 0x08, 0x10,
        0x10, 0x10, 0x10, 0x10,
        0x10, 0x10, 0x10, 0x10,
        0x10, 0x10, 0x10, 0x10,
        0x10, 0x02,
    };

    constexpr u32 CalculateGfxImageTexelSize(GfxChannelFormat channel_format) {
        u32 format = static_cast<u32>(channel_format);
        u32 package_format = format - 0x1a;
        u32 texel_size = 0;
        if (package_format < 0x22) {
            texel_size = cPackagedTextureTexelSizeTable[package_format];
        } else if (format < 0x3) {
            texel_size = 1;
        } else if (format < 0xb) {
            texel_size = 2;
        } else if (format < 0x15) {
            texel_size = 4;
        } else if (format < 0x18) {
            texel_size = 8;
        } else {
            texel_size = 12;
            if (format != 0x18) {
                texel_size = 16;
            }
        }
        return texel_size;
    }

    constexpr u32 CalculateGfxAttributeSize(GfxChannelFormat channel_format, u32 component_count) {

        u32 format = static_cast<u32>(channel_format);
        u32 attribute_size = 0;
        if (format < 0x3) {
            attribute_size = 1;
        } else if (format < 0xb) {
            attribute_size = 2;
        } else if (format < 0x15) {
            attribute_size = 4;
        } else if (format < 0x18) {
            attribute_size = 8;
        } else {
            attribute_size = 12;
            if (format != 0x18) {
                attribute_size = 16;
            }
        }

        return attribute_size * component_count;
    }

    constexpr u32 CalculateGfxImageSize(GfxChannelFormat channel_format, u32 width, u32 height, u32 depth) {
        const u32 format = static_cast<u32>(channel_format);

        /* Adjust width and height for compressed formats */
        const u32 package_format = format - 0x1a;
        if (package_format < 0x21) {
            const u32 variable_format = format - 0x27;
            u32 block_width  = 4;
            u32 block_height = 4;
            if (variable_format < 0x14) {
                block_width  = cVariableBlockWidthTable[variable_format];
                block_height = cVariableBlockHeightTable[variable_format];
            }

            width  = (block_width != 0)  ? (width  + block_width  - 1) / block_width  : 0;
            height = (block_height != 0) ? (height + block_height - 1) / block_height : 0;
        }

        /* Calculate size */
        const u32 texel_size = CalculateGfxImageTexelSize(channel_format);
        return texel_size * width * height * depth;
    }

    constexpr u32 CalculateLinearTextureStride(u32 width, vp::res::GfxChannelFormat channel_format) {
        const u32 format = static_cast<u32>(channel_format);

        /* Adjust width for linear format */
        const u32 package_format = format - 0x1a;
        if (package_format < 0x21) {
            const u32 variable_format = format - 0x27;
            const u32 block_width = (variable_format < 0x14) ? cVariableBlockWidthTable[variable_format] : 4;

            width = (block_width != 0) ? (width + block_width - 1) / block_width : 0;
        }

        /* Calculate size */
        const u32 texel_size = CalculateGfxImageTexelSize(channel_format);
        return width * texel_size;
    }

    constexpr vp::util::Int2 CalculateGfxImageBlockSize(GfxChannelFormat channel_format) {
        u32 format = static_cast<u32>(channel_format);
        u32 block_width     = 1;
        u32 block_height    = 1;
        u32 package_format = format - 0x1a;
        if (package_format < 0x21) {
            u32 variable_format = format - 0x27;
            block_width  = 4;
            block_height = 4;
            if (variable_format < 0x14) {
                block_width  = cVariableBlockWidthTable[variable_format];
                block_height = cVariableBlockHeightTable[variable_format];
            }
        }
        return { static_cast<s32>(block_width), static_cast<s32>(block_height), };
    }

    enum class GfxTypeFormat : u8 {
        Unorm   = 0x1,
        Snorm   = 0x2,
        UInt    = 0x3,
        SInt    = 0x4,
        Float   = 0x5,
        SRGB    = 0x6,
        Depth   = 0x7, /* (Unorm) */
        UScaled = 0x8,
        SScaled = 0x9,
        UFloat  = 0xa,
    };

    #define GFX_MAKE_IMAGE_FORMAT(channel_format, type_format) \
        (((static_cast<u32>(channel_format) & 0xff) << 8) | (static_cast<u32>(type_format) & 0xff))

    enum class GfxImageFormat : u32 {
        None_Unorm            = GFX_MAKE_IMAGE_FORMAT(GfxChannelFormat::None,         GfxTypeFormat::Unorm),
        R8_Unorm              = GFX_MAKE_IMAGE_FORMAT(GfxChannelFormat::R8,           GfxTypeFormat::Unorm),
        R8_Snorm              = GFX_MAKE_IMAGE_FORMAT(GfxChannelFormat::R8,           GfxTypeFormat::Snorm),
        R8_UInt               = GFX_MAKE_IMAGE_FORMAT(GfxChannelFormat::R8,           GfxTypeFormat::UInt),
        R8_SInt               = GFX_MAKE_IMAGE_FORMAT(GfxChannelFormat::R8,           GfxTypeFormat::SInt),
        R4G4B4A4_Unorm        = GFX_MAKE_IMAGE_FORMAT(GfxChannelFormat::R4G4B4A4,     GfxTypeFormat::Unorm),
        R5G5B5A1_Unorm        = GFX_MAKE_IMAGE_FORMAT(GfxChannelFormat::R5G5B5A1,     GfxTypeFormat::Unorm),
        A1B5G5R5_Unorm        = GFX_MAKE_IMAGE_FORMAT(GfxChannelFormat::A1B5G5R5,     GfxTypeFormat::Unorm),
        R5G6B5_Unorm          = GFX_MAKE_IMAGE_FORMAT(GfxChannelFormat::R5G6B5,       GfxTypeFormat::Unorm),
        B5G6R5_Unorm          = GFX_MAKE_IMAGE_FORMAT(GfxChannelFormat::B5G6R5,       GfxTypeFormat::Unorm),
        R8G8_Unorm            = GFX_MAKE_IMAGE_FORMAT(GfxChannelFormat::R8G8,         GfxTypeFormat::Unorm),
        R8G8_Snorm            = GFX_MAKE_IMAGE_FORMAT(GfxChannelFormat::R8G8,         GfxTypeFormat::Snorm),
        R8G8_UInt             = GFX_MAKE_IMAGE_FORMAT(GfxChannelFormat::R8G8,         GfxTypeFormat::UInt),
        R8G8_SInt             = GFX_MAKE_IMAGE_FORMAT(GfxChannelFormat::R8G8,         GfxTypeFormat::SInt),
        R16_Unorm             = GFX_MAKE_IMAGE_FORMAT(GfxChannelFormat::R16,          GfxTypeFormat::Unorm),
        R16_Snorm             = GFX_MAKE_IMAGE_FORMAT(GfxChannelFormat::R16,          GfxTypeFormat::Snorm),
        R16_UInt              = GFX_MAKE_IMAGE_FORMAT(GfxChannelFormat::R16,          GfxTypeFormat::UInt),
        R16_SInt              = GFX_MAKE_IMAGE_FORMAT(GfxChannelFormat::R16,          GfxTypeFormat::SInt),
        R16_Float             = GFX_MAKE_IMAGE_FORMAT(GfxChannelFormat::R16,          GfxTypeFormat::Float),
        Z16_Depth             = GFX_MAKE_IMAGE_FORMAT(GfxChannelFormat::R16,          GfxTypeFormat::Depth),
        R8G8B8A8_Unorm        = GFX_MAKE_IMAGE_FORMAT(GfxChannelFormat::R8G8B8A8,     GfxTypeFormat::Unorm),
        R8G8B8A8_Snorm        = GFX_MAKE_IMAGE_FORMAT(GfxChannelFormat::R8G8B8A8,     GfxTypeFormat::Snorm),
        R8G8B8A8_UInt         = GFX_MAKE_IMAGE_FORMAT(GfxChannelFormat::R8G8B8A8,     GfxTypeFormat::UInt),
        R8G8B8A8_SInt         = GFX_MAKE_IMAGE_FORMAT(GfxChannelFormat::R8G8B8A8,     GfxTypeFormat::SInt),
        R8G8B8A8_SRGB         = GFX_MAKE_IMAGE_FORMAT(GfxChannelFormat::R8G8B8A8,     GfxTypeFormat::SRGB),
        B8G8R8A8_Unorm        = GFX_MAKE_IMAGE_FORMAT(GfxChannelFormat::B8G8R8A8,     GfxTypeFormat::Unorm),
        B8G8R8A8_SRGB         = GFX_MAKE_IMAGE_FORMAT(GfxChannelFormat::B8G8R8A8,     GfxTypeFormat::SRGB),
        R9G9B9E5F_SharedFloat = GFX_MAKE_IMAGE_FORMAT(GfxChannelFormat::R9G9B9E5F,    GfxTypeFormat::Float),
        R10G10B10A2_Unorm     = GFX_MAKE_IMAGE_FORMAT(GfxChannelFormat::R10G10B10A2,  GfxTypeFormat::Unorm),
        R10G10B10A2_UInt      = GFX_MAKE_IMAGE_FORMAT(GfxChannelFormat::R10G10B10A2,  GfxTypeFormat::UInt),
        R11G11B10F_Float      = GFX_MAKE_IMAGE_FORMAT(GfxChannelFormat::R11G11B10F,   GfxTypeFormat::Float),
        R16G16_Unorm          = GFX_MAKE_IMAGE_FORMAT(GfxChannelFormat::R16G16,       GfxTypeFormat::Unorm),
        R16G16_Snorm          = GFX_MAKE_IMAGE_FORMAT(GfxChannelFormat::R16G16,       GfxTypeFormat::Snorm),
        R16G16_UInt           = GFX_MAKE_IMAGE_FORMAT(GfxChannelFormat::R16G16,       GfxTypeFormat::UInt),
        R16G16_SInt           = GFX_MAKE_IMAGE_FORMAT(GfxChannelFormat::R16G16,       GfxTypeFormat::SInt),
        R16G16_Float          = GFX_MAKE_IMAGE_FORMAT(GfxChannelFormat::R16G16,       GfxTypeFormat::Float),
        D24S8_Depth           = GFX_MAKE_IMAGE_FORMAT(GfxChannelFormat::D24S8,        GfxTypeFormat::Depth),
        R32_UInt              = GFX_MAKE_IMAGE_FORMAT(GfxChannelFormat::R32,          GfxTypeFormat::UInt),
        R32_SInt              = GFX_MAKE_IMAGE_FORMAT(GfxChannelFormat::R32,          GfxTypeFormat::SInt),
        R32_Float             = GFX_MAKE_IMAGE_FORMAT(GfxChannelFormat::R32,          GfxTypeFormat::Float),
        D32F_Depth            = GFX_MAKE_IMAGE_FORMAT(GfxChannelFormat::R32,          GfxTypeFormat::Depth),
        R16G16B16A16_Unorm    = GFX_MAKE_IMAGE_FORMAT(GfxChannelFormat::R16G16B16A16, GfxTypeFormat::Unorm),
        R16G16B16A16_Snorm    = GFX_MAKE_IMAGE_FORMAT(GfxChannelFormat::R16G16B16A16, GfxTypeFormat::Snorm),
        R16G16B16A16_UInt     = GFX_MAKE_IMAGE_FORMAT(GfxChannelFormat::R16G16B16A16, GfxTypeFormat::UInt),
        R16G16B16A16_SInt     = GFX_MAKE_IMAGE_FORMAT(GfxChannelFormat::R16G16B16A16, GfxTypeFormat::SInt),
        R16G16B16A16_Float    = GFX_MAKE_IMAGE_FORMAT(GfxChannelFormat::R16G16B16A16, GfxTypeFormat::Float),
        D32FS8_Depth          = GFX_MAKE_IMAGE_FORMAT(GfxChannelFormat::D32FS8,       GfxTypeFormat::Depth),
        R32G32_UInt           = GFX_MAKE_IMAGE_FORMAT(GfxChannelFormat::R32G32,       GfxTypeFormat::UInt),
        R32G32_SInt           = GFX_MAKE_IMAGE_FORMAT(GfxChannelFormat::R32G32,       GfxTypeFormat::SInt),
        R32G32_Float          = GFX_MAKE_IMAGE_FORMAT(GfxChannelFormat::R32G32,       GfxTypeFormat::Float),
        R32G32B32_UInt        = GFX_MAKE_IMAGE_FORMAT(GfxChannelFormat::R32G32B32,    GfxTypeFormat::UInt),
        R32G32B32_SInt        = GFX_MAKE_IMAGE_FORMAT(GfxChannelFormat::R32G32B32,    GfxTypeFormat::SInt),
        R32G32B32_Float       = GFX_MAKE_IMAGE_FORMAT(GfxChannelFormat::R32G32B32,    GfxTypeFormat::Float),
        R32G32B32A32_UInt     = GFX_MAKE_IMAGE_FORMAT(GfxChannelFormat::R32G32B32A32, GfxTypeFormat::UInt),
        R32G32B32A32_SInt     = GFX_MAKE_IMAGE_FORMAT(GfxChannelFormat::R32G32B32A32, GfxTypeFormat::SInt),
        R32G32B32A32_Float    = GFX_MAKE_IMAGE_FORMAT(GfxChannelFormat::R32G32B32A32, GfxTypeFormat::Float),
        BC1_Unorm             = GFX_MAKE_IMAGE_FORMAT(GfxChannelFormat::BC1,          GfxTypeFormat::Unorm),
        BC1_SRGB              = GFX_MAKE_IMAGE_FORMAT(GfxChannelFormat::BC1,          GfxTypeFormat::SRGB),
        BC2_Unorm             = GFX_MAKE_IMAGE_FORMAT(GfxChannelFormat::BC2,          GfxTypeFormat::Unorm),
        BC2_SRGB              = GFX_MAKE_IMAGE_FORMAT(GfxChannelFormat::BC2,          GfxTypeFormat::SRGB),
        BC3_Unorm             = GFX_MAKE_IMAGE_FORMAT(GfxChannelFormat::BC3,          GfxTypeFormat::Unorm),
        BC3_SRGB              = GFX_MAKE_IMAGE_FORMAT(GfxChannelFormat::BC3,          GfxTypeFormat::SRGB),
        BC4_Unorm             = GFX_MAKE_IMAGE_FORMAT(GfxChannelFormat::BC4,          GfxTypeFormat::Unorm),
        BC4_Snorm             = GFX_MAKE_IMAGE_FORMAT(GfxChannelFormat::BC4,          GfxTypeFormat::Snorm),
        BC5_Unorm             = GFX_MAKE_IMAGE_FORMAT(GfxChannelFormat::BC5,          GfxTypeFormat::Unorm),
        BC5_Snorm             = GFX_MAKE_IMAGE_FORMAT(GfxChannelFormat::BC5,          GfxTypeFormat::Snorm),
        BC6H_SF16             = GFX_MAKE_IMAGE_FORMAT(GfxChannelFormat::BC6H,         GfxTypeFormat::Float),
        BC6H_UF16             = GFX_MAKE_IMAGE_FORMAT(GfxChannelFormat::BC6H,         GfxTypeFormat::UFloat),
        BC7U_Unorm            = GFX_MAKE_IMAGE_FORMAT(GfxChannelFormat::BC7U,         GfxTypeFormat::Unorm),
        BC7U_SRGB             = GFX_MAKE_IMAGE_FORMAT(GfxChannelFormat::BC7U,         GfxTypeFormat::SRGB),
        ASTC_4x4_Unorm        = GFX_MAKE_IMAGE_FORMAT(GfxChannelFormat::ASTC_4x4,     GfxTypeFormat::Unorm),
        ASTC_4x4_SRGB         = GFX_MAKE_IMAGE_FORMAT(GfxChannelFormat::ASTC_4x4,     GfxTypeFormat::SRGB),
        ASTC_5x4_Unorm        = GFX_MAKE_IMAGE_FORMAT(GfxChannelFormat::ASTC_5x4,     GfxTypeFormat::Unorm),
        ASTC_5x4_SRGB         = GFX_MAKE_IMAGE_FORMAT(GfxChannelFormat::ASTC_5x4,     GfxTypeFormat::SRGB),
        ASTC_5x5_Unorm        = GFX_MAKE_IMAGE_FORMAT(GfxChannelFormat::ASTC_5x5,     GfxTypeFormat::Unorm),
        ASTC_5x5_SRGB         = GFX_MAKE_IMAGE_FORMAT(GfxChannelFormat::ASTC_5x5,     GfxTypeFormat::SRGB),
        ASTC_6x5_Unorm        = GFX_MAKE_IMAGE_FORMAT(GfxChannelFormat::ASTC_6x5,     GfxTypeFormat::Unorm),
        ASTC_6x5_SRGB         = GFX_MAKE_IMAGE_FORMAT(GfxChannelFormat::ASTC_6x5,     GfxTypeFormat::SRGB),
        ASTC_6x6_Unorm        = GFX_MAKE_IMAGE_FORMAT(GfxChannelFormat::ASTC_6x6,     GfxTypeFormat::Unorm),
        ASTC_6x6_SRGB         = GFX_MAKE_IMAGE_FORMAT(GfxChannelFormat::ASTC_6x6,     GfxTypeFormat::SRGB),
        ASTC_8x5_Unorm        = GFX_MAKE_IMAGE_FORMAT(GfxChannelFormat::ASTC_8x5,     GfxTypeFormat::Unorm),
        ASTC_8x5_SRGB         = GFX_MAKE_IMAGE_FORMAT(GfxChannelFormat::ASTC_8x5,     GfxTypeFormat::SRGB),
        ASTC_8x6_Unorm        = GFX_MAKE_IMAGE_FORMAT(GfxChannelFormat::ASTC_8x6,     GfxTypeFormat::Unorm),
        ASTC_8x6_SRGB         = GFX_MAKE_IMAGE_FORMAT(GfxChannelFormat::ASTC_8x6,     GfxTypeFormat::SRGB),
        ASTC_8x8_Unorm        = GFX_MAKE_IMAGE_FORMAT(GfxChannelFormat::ASTC_8x8,     GfxTypeFormat::Unorm),
        ASTC_8x8_SRGB         = GFX_MAKE_IMAGE_FORMAT(GfxChannelFormat::ASTC_8x8,     GfxTypeFormat::SRGB),
        ASTC_10x5_Unorm       = GFX_MAKE_IMAGE_FORMAT(GfxChannelFormat::ASTC_10x5,    GfxTypeFormat::Unorm),
        ASTC_10x5_SRGB        = GFX_MAKE_IMAGE_FORMAT(GfxChannelFormat::ASTC_10x5,    GfxTypeFormat::SRGB),
        ASTC_10x6_Unorm       = GFX_MAKE_IMAGE_FORMAT(GfxChannelFormat::ASTC_10x6,    GfxTypeFormat::Unorm),
        ASTC_10x6_SRGB        = GFX_MAKE_IMAGE_FORMAT(GfxChannelFormat::ASTC_10x6,    GfxTypeFormat::SRGB),
        ASTC_10x8_Unorm       = GFX_MAKE_IMAGE_FORMAT(GfxChannelFormat::ASTC_10x8,    GfxTypeFormat::Unorm),
        ASTC_10x8_SRGB        = GFX_MAKE_IMAGE_FORMAT(GfxChannelFormat::ASTC_10x8,    GfxTypeFormat::SRGB),
        ASTC_10x10_Unorm      = GFX_MAKE_IMAGE_FORMAT(GfxChannelFormat::ASTC_10x10,   GfxTypeFormat::Unorm),
        ASTC_10x10_SRGB       = GFX_MAKE_IMAGE_FORMAT(GfxChannelFormat::ASTC_10x10,   GfxTypeFormat::SRGB),
        ASTC_12x10_Unorm      = GFX_MAKE_IMAGE_FORMAT(GfxChannelFormat::ASTC_12x10,   GfxTypeFormat::Unorm),
        ASTC_12x10_SRGB       = GFX_MAKE_IMAGE_FORMAT(GfxChannelFormat::ASTC_12x10,   GfxTypeFormat::SRGB),
        ASTC_12x12_Unorm      = GFX_MAKE_IMAGE_FORMAT(GfxChannelFormat::ASTC_12x12,   GfxTypeFormat::Unorm),
        ASTC_12x12_SRGB       = GFX_MAKE_IMAGE_FORMAT(GfxChannelFormat::ASTC_12x12,   GfxTypeFormat::SRGB),
        B5G5R5A1_Unorm        = GFX_MAKE_IMAGE_FORMAT(GfxChannelFormat::B5G5R5A1,     GfxTypeFormat::Unorm),
    };

    constexpr bool IsDepthFormat(vp::res::GfxImageFormat format) {
        switch (format) {
            case GfxImageFormat::Z16_Depth:
                return true;
            case GfxImageFormat::D24S8_Depth:
                return true;
            case GfxImageFormat::D32F_Depth:
                return true;
            case GfxImageFormat::D32FS8_Depth:
                return true;
            default:
                break;
        };
        return false;
    }
    constexpr bool IsStencilFormat(vp::res::GfxImageFormat format) {
        switch (format) {
            case GfxImageFormat::D24S8_Depth:
                return true;
            case GfxImageFormat::D32FS8_Depth:
                return true;
            default:
                break;
        };
        return false;
    }

    enum class GfxAttributeFormat : u32 {
        None_Unorm           = GFX_MAKE_IMAGE_FORMAT(GfxChannelFormat::None,         GfxTypeFormat::Unorm),
        R8_Unorm             = GFX_MAKE_IMAGE_FORMAT(GfxChannelFormat::R8,           GfxTypeFormat::Unorm),
        R8_Snorm             = GFX_MAKE_IMAGE_FORMAT(GfxChannelFormat::R8,           GfxTypeFormat::Snorm),
        R8_UInt              = GFX_MAKE_IMAGE_FORMAT(GfxChannelFormat::R8,           GfxTypeFormat::UInt),
        R8_SInt              = GFX_MAKE_IMAGE_FORMAT(GfxChannelFormat::R8,           GfxTypeFormat::SInt),
        R8_UScaled           = GFX_MAKE_IMAGE_FORMAT(GfxChannelFormat::R8,           GfxTypeFormat::UScaled),
        R8_SScaled           = GFX_MAKE_IMAGE_FORMAT(GfxChannelFormat::R8,           GfxTypeFormat::SScaled),
        R8G8_Unorm           = GFX_MAKE_IMAGE_FORMAT(GfxChannelFormat::R8G8,         GfxTypeFormat::Unorm),
        R8G8_Snorm           = GFX_MAKE_IMAGE_FORMAT(GfxChannelFormat::R8G8,         GfxTypeFormat::Snorm),
        R8G8_UInt            = GFX_MAKE_IMAGE_FORMAT(GfxChannelFormat::R8G8,         GfxTypeFormat::UInt),
        R8G8_SInt            = GFX_MAKE_IMAGE_FORMAT(GfxChannelFormat::R8G8,         GfxTypeFormat::SInt),
        R8G8_UScaled         = GFX_MAKE_IMAGE_FORMAT(GfxChannelFormat::R8G8,         GfxTypeFormat::UScaled),
        R8G8_SScaled         = GFX_MAKE_IMAGE_FORMAT(GfxChannelFormat::R8G8,         GfxTypeFormat::SScaled),
        R16_Unorm            = GFX_MAKE_IMAGE_FORMAT(GfxChannelFormat::R16,          GfxTypeFormat::Unorm),
        R16_Snorm            = GFX_MAKE_IMAGE_FORMAT(GfxChannelFormat::R16,          GfxTypeFormat::Snorm),
        R16_UInt             = GFX_MAKE_IMAGE_FORMAT(GfxChannelFormat::R16,          GfxTypeFormat::UInt),
        R16_SInt             = GFX_MAKE_IMAGE_FORMAT(GfxChannelFormat::R16,          GfxTypeFormat::SInt),
        R16_Float            = GFX_MAKE_IMAGE_FORMAT(GfxChannelFormat::R16,          GfxTypeFormat::Float),
        R16_UScaled          = GFX_MAKE_IMAGE_FORMAT(GfxChannelFormat::R16,          GfxTypeFormat::UScaled),
        R16_SScaled          = GFX_MAKE_IMAGE_FORMAT(GfxChannelFormat::R16,          GfxTypeFormat::SScaled),
        R8G8B8A8_Unorm       = GFX_MAKE_IMAGE_FORMAT(GfxChannelFormat::R8G8B8A8,     GfxTypeFormat::Unorm),
        R8G8B8A8_Snorm       = GFX_MAKE_IMAGE_FORMAT(GfxChannelFormat::R8G8B8A8,     GfxTypeFormat::Snorm),
        R8G8B8A8_UInt        = GFX_MAKE_IMAGE_FORMAT(GfxChannelFormat::R8G8B8A8,     GfxTypeFormat::UInt),
        R8G8B8A8_SInt        = GFX_MAKE_IMAGE_FORMAT(GfxChannelFormat::R8G8B8A8,     GfxTypeFormat::SInt),
        R8G8B8A8_UScaled     = GFX_MAKE_IMAGE_FORMAT(GfxChannelFormat::R8G8B8A8,     GfxTypeFormat::UScaled),
        R8G8B8A8_SScaled     = GFX_MAKE_IMAGE_FORMAT(GfxChannelFormat::R8G8B8A8,     GfxTypeFormat::SScaled),
        R10G10B10A2_Unorm    = GFX_MAKE_IMAGE_FORMAT(GfxChannelFormat::R10G10B10A2,  GfxTypeFormat::Unorm),
        R10G10B10A2_Snorm    = GFX_MAKE_IMAGE_FORMAT(GfxChannelFormat::R10G10B10A2,  GfxTypeFormat::Snorm),
        R10G10B10A2_UInt     = GFX_MAKE_IMAGE_FORMAT(GfxChannelFormat::R10G10B10A2,  GfxTypeFormat::UInt),
        R10G10B10A2_SInt     = GFX_MAKE_IMAGE_FORMAT(GfxChannelFormat::R10G10B10A2,  GfxTypeFormat::SInt),
        R16G16_Unorm         = GFX_MAKE_IMAGE_FORMAT(GfxChannelFormat::R16G16,        GfxTypeFormat::Unorm),
        R16G16_Snorm         = GFX_MAKE_IMAGE_FORMAT(GfxChannelFormat::R16G16,        GfxTypeFormat::Snorm),
        R16G16_UInt          = GFX_MAKE_IMAGE_FORMAT(GfxChannelFormat::R16G16,        GfxTypeFormat::UInt),
        R16G16_SInt          = GFX_MAKE_IMAGE_FORMAT(GfxChannelFormat::R16G16,        GfxTypeFormat::SInt),
        R16G16_Float         = GFX_MAKE_IMAGE_FORMAT(GfxChannelFormat::R16G16,        GfxTypeFormat::Float),
        R16G16_UScaled       = GFX_MAKE_IMAGE_FORMAT(GfxChannelFormat::R16G16,        GfxTypeFormat::UScaled),
        R16G16_SScaled       = GFX_MAKE_IMAGE_FORMAT(GfxChannelFormat::R16G16,        GfxTypeFormat::SScaled),
        R32_UInt             = GFX_MAKE_IMAGE_FORMAT(GfxChannelFormat::R32,           GfxTypeFormat::UInt),
        R32_SInt             = GFX_MAKE_IMAGE_FORMAT(GfxChannelFormat::R32,           GfxTypeFormat::SInt),
        R32_Float            = GFX_MAKE_IMAGE_FORMAT(GfxChannelFormat::R32,           GfxTypeFormat::Float),
        R16G16B16A16_Unorm   = GFX_MAKE_IMAGE_FORMAT(GfxChannelFormat::R16G16B16A16,  GfxTypeFormat::Unorm),
        R16G16B16A16_Snorm   = GFX_MAKE_IMAGE_FORMAT(GfxChannelFormat::R16G16B16A16,  GfxTypeFormat::Snorm),
        R16G16B16A16_UInt    = GFX_MAKE_IMAGE_FORMAT(GfxChannelFormat::R16G16B16A16,  GfxTypeFormat::UInt),
        R16G16B16A16_SInt    = GFX_MAKE_IMAGE_FORMAT(GfxChannelFormat::R16G16B16A16,  GfxTypeFormat::SInt),
        R16G16B16A16_Float   = GFX_MAKE_IMAGE_FORMAT(GfxChannelFormat::R16G16B16A16,  GfxTypeFormat::Float),
        R16G16B16A16_UScaled = GFX_MAKE_IMAGE_FORMAT(GfxChannelFormat::R16G16B16A16,  GfxTypeFormat::UScaled),
        R16G16B16A16_SScaled = GFX_MAKE_IMAGE_FORMAT(GfxChannelFormat::R16G16B16A16,  GfxTypeFormat::SScaled),
        R32G32_UInt          = GFX_MAKE_IMAGE_FORMAT(GfxChannelFormat::R32G32,        GfxTypeFormat::UInt),
        R32G32_SInt          = GFX_MAKE_IMAGE_FORMAT(GfxChannelFormat::R32G32,        GfxTypeFormat::SInt),
        R32G32_Float         = GFX_MAKE_IMAGE_FORMAT(GfxChannelFormat::R32G32,        GfxTypeFormat::Float),
        R32G32B32_UInt       = GFX_MAKE_IMAGE_FORMAT(GfxChannelFormat::R32G32B32,     GfxTypeFormat::UInt),
        R32G32B32_SInt       = GFX_MAKE_IMAGE_FORMAT(GfxChannelFormat::R32G32B32,     GfxTypeFormat::SInt),
        R32G32B32_Float      = GFX_MAKE_IMAGE_FORMAT(GfxChannelFormat::R32G32B32,     GfxTypeFormat::Float),
        R32G32B32A32_UInt    = GFX_MAKE_IMAGE_FORMAT(GfxChannelFormat::R32G32B32A32,  GfxTypeFormat::UInt),
        R32G32B32A32_SInt    = GFX_MAKE_IMAGE_FORMAT(GfxChannelFormat::R32G32B32A32,  GfxTypeFormat::SInt),
        R32G32B32A32_Float   = GFX_MAKE_IMAGE_FORMAT(GfxChannelFormat::R32G32B32A32,  GfxTypeFormat::Float),
    };

    constexpr GfxChannelFormat GetGfxChannelFormat(GfxImageFormat image_format) {
        return static_cast<GfxChannelFormat>(static_cast<u32>(image_format) >> 0x8);
    }
    constexpr GfxTypeFormat GetGfxTypeFormat(GfxImageFormat image_format) {
        return static_cast<GfxTypeFormat>(static_cast<u32>(image_format) & 0xff);
    }
    constexpr GfxChannelFormat GetGfxChannelFormat(GfxAttributeFormat attribute_format) {
        return static_cast<GfxChannelFormat>(static_cast<u32>(attribute_format) >> 0x8);
    }
    constexpr GfxTypeFormat GetGfxTypeFormat(GfxAttributeFormat attribute_format) {
        return static_cast<GfxTypeFormat>(static_cast<u32>(attribute_format) & 0xff);
    }

    enum class GfxImageStorageDimension : u8 { 
        Type1d = 1,
        Type2d = 2,
        Type3d = 3,
    };

    enum class GfxTileMode : u16 {
        Optimal = 0,
        Linear = 1,
    };

    struct ResGfxTextureInfo {
        union {
            u8 texture_flags;
            struct {
                u8  is_packaged_texture_layout : 1;
                u8  is_sparse_binding   : 1;
                u8  is_sparse           : 1;
                u8  is_res_texture      : 1;
                u8  reserve0            : 4;
            };
        };
        u8  image_storage_dimension;
        u16 tile_mode;
        u16 swizzle;
        u16 mip_level_count;
        u16 sample_count;
        u16 reserve1;
        u32 image_format;
        u32 gpu_access_flags;
        u32 width;
        u32 height;
        u32 depth;
        u32 array_layer_count;
        union {
            u32 packaged_texture_layout;
            struct {
                u32 tile_h : 4;
                u32 tile_d : 4;
                u32 tile_num : 4;
                u32 reserve2 : 20;
            };
        };

        constexpr void SetDefaults() {
            this->texture_flags = 0;
            this->image_storage_dimension = static_cast<u8>(GfxImageStorageDimension::Type2d);
            this->tile_mode = static_cast<u16>(GfxTileMode::Optimal);
            this->swizzle = 0;
            this->mip_level_count = 1;
            this->sample_count = 1;
            this->image_format = static_cast<u32>(GfxImageFormat::None_Unorm);
            this->gpu_access_flags = 0;
            this->width = 1;
            this->height = 1;
            this->depth = 1;
            this->array_layer_count = 1;
            this->packaged_texture_layout = 0;
        }

        constexpr void SetImageStorageDimension(GfxImageStorageDimension image_storage_dimension) {
            this->image_storage_dimension = static_cast<u8>(image_storage_dimension);
        }
        constexpr void SetTileMode(GfxTileMode tile_mode) {
            this->tile_mode = static_cast<u32>(tile_mode);
        }
        constexpr void SetMipLevelCount(u16 mip_level_count) {
            this->mip_level_count = mip_level_count;
        }
        constexpr void SetSampleCount(u16 sample_count) {
            this->sample_count = sample_count;
        }
        constexpr void SetImageFormat(GfxImageFormat image_format) {
            this->image_format = static_cast<u32>(image_format);
        }
        constexpr void SetGpuAccessFlags(GfxGpuAccessFlags gpu_access_flags) {
            this->gpu_access_flags = static_cast<u32>(gpu_access_flags);
        }
        constexpr void SetWidth(u32 width) {
            this->width = width;
        }
        constexpr void SetHeight(u32 height) {
            this->height = height;
        }
        constexpr void SetDepth(u32 depth) {
            this->depth = depth;
        }
        template <typename T>
            requires (T::cIsVectorType && 2 <= T::cCount)
        constexpr void SetSize2d(const T &vec) {
            this->width = static_cast<u32>(vec[0]);
            this->height = static_cast<u32>(vec[1]);
            this->depth = 1;
        }
        template <typename T>
            requires (T::cIsVectorType && 3 <= T::cCount)
        constexpr void SetSize3d(const T &vec) {
            this->width = static_cast<u32>(vec[0]);
            this->height = static_cast<u32>(vec[1]);
            this->depth = static_cast<u32>(vec[2]);
        }
        constexpr void SetArrayLayerCount(u32 array_layer_count) {
            this->array_layer_count = array_layer_count;
        }
    };
    static_assert(sizeof(ResGfxTextureInfo) == 0x28);

    enum class GfxImageDimension : u8 {
        Type1d                  = 0,
        Type2d                  = 1,
        Type3d                  = 2,
        TypeCube                = 3,
        Type1dArray             = 4,
        Type2dArray             = 5,
        Type2dMultisample       = 6,
        Type2dMultisampleArray  = 7,
        TypeCubeArray           = 8,
        TypeRectangle           = 9,
    };

    constexpr u32 AdjustHeight(u32 height, GfxImageDimension image_dimension) {
        switch (image_dimension) {
            case GfxImageDimension::Type1dArray:
                height = 1;
                break;
            default:
                break;
        };
        return height;
    }
    constexpr u32 AdjustDepth(u32 height, u32 depth, GfxImageDimension image_dimension) {
        switch (image_dimension) {
            case GfxImageDimension::TypeCube:
                depth = 6;
                break;
            case GfxImageDimension::Type1dArray:
                if (1 < height) {
                    depth = height;
                }
                break;
            default:
                break;
        };
        return depth;
    }
    constexpr bool IsArray(GfxImageDimension image_dimension) {
        switch (image_dimension) {
            case GfxImageDimension::TypeCube:
            case GfxImageDimension::TypeCubeArray:
            case GfxImageDimension::Type1dArray:
            case GfxImageDimension::Type2dArray:
                return true;
            default:
                break;
        };
        return false;
    }
    constexpr GfxImageStorageDimension GetStorageDimension(GfxImageDimension image_dimension) {
        u32 dimension = 2;
        switch (image_dimension) {
            case GfxImageDimension::Type1d:
            case GfxImageDimension::Type1dArray:
                dimension = 1;
                break;
            case GfxImageDimension::Type3d:
                dimension = 3;
                break;
            default:
                break;
        };
        return static_cast<GfxImageStorageDimension>(dimension);
    }

    enum class GfxTextureSwizzle : u8 {
        Zero = 0,
        One  = 1,
        R    = 2,
        G    = 3,
        B    = 4,
        A    = 5,
    };

    enum class GfxTextureDepthStencilMode : u8 {
        Depth   = 0,
        Stencil = 1,
        /* Vp ext */
        DepthAndStencil = 2,
    };

    template <typename T>
        requires (sizeof(T*) == sizeof(u64))
    struct ResGfxTextureViewInfo {
        u8   image_dimension;
        u8   depth_stencil_mode;
        u16  reserve0;
        u32  image_format;
        u8   swizzle_x;
        u8   swizzle_y;
        u8   swizzle_z;
        u8   swizzle_w;
        u32  reserve1;
        u16  base_mip_level;
        u16  mip_level_count;
        u32  reserve2[3];
        u32  base_array_layer;
        u32  array_layer_count;
        u32  reserve3[2];
        T   *texture;

        constexpr void SetDefaults() {
            this->image_dimension = static_cast<u8>(GfxImageDimension::Type2d);
            this->depth_stencil_mode = static_cast<u8>(GfxTextureDepthStencilMode::Depth);
            this->reserve0 = 0;
            this->image_format = static_cast<u32>(GfxImageFormat::None_Unorm);
            this->swizzle_x = static_cast<u8>(GfxTextureSwizzle::R);
            this->swizzle_y = static_cast<u8>(GfxTextureSwizzle::G);
            this->swizzle_z = static_cast<u8>(GfxTextureSwizzle::B);
            this->swizzle_w = static_cast<u8>(GfxTextureSwizzle::A);
            this->reserve1 = 0;
            this->base_mip_level = 0;
            this->mip_level_count = 1;
            this->reserve2[0] = 0;
            this->reserve2[1] = 0;
            this->reserve2[2] = 0;
            this->base_array_layer = 0;
            this->array_layer_count = 1;
            this->reserve3[0] = 0;
            this->reserve3[1] = 0;
            this->texture = nullptr;
        }

        constexpr void SetImageDimension(GfxImageDimension image_dimension) {
            this->image_dimension = static_cast<u8>(image_dimension);
        }
        constexpr void SetDepthStencilMode(GfxTextureDepthStencilMode depth_stencil_mode) {
            this->depth_stencil_mode = static_cast<u8>(depth_stencil_mode);
        }
        constexpr void SetImageFormat(GfxImageFormat image_format) {
            this->image_format = static_cast<u32>(image_format);
        }
        constexpr void SetSwizzle(GfxTextureSwizzle x, GfxTextureSwizzle y, GfxTextureSwizzle z, GfxTextureSwizzle w) {
            this->swizzle_x = static_cast<u8>(x);
            this->swizzle_y = static_cast<u8>(y);
            this->swizzle_z = static_cast<u8>(z);
            this->swizzle_w = static_cast<u8>(w);
        }
        constexpr void SetMipLevelRange(u32 base_mip_level, u32 mip_level_count) {
            this->base_mip_level = base_mip_level;
            this->mip_level_count = mip_level_count;
        }
        constexpr void SetArrayLayerRange(u32 base_array_layer, u32 array_layer_count) {
            this->base_array_layer = base_array_layer;
            this->array_layer_count = array_layer_count;
        }
        constexpr void SetTexture(T *texture) {
            this->texture = texture;
        }
    };
    static_assert(sizeof(ResGfxTextureViewInfo<void*>) == 0x38);

    enum class GfxWrapMode : u8 {
        Repeat            = 0,
        MirrorRepeat      = 1,
        ClampToEdge       = 2,
        ClampToBorder     = 3,
        MirrorClampToEdge = 4,
    };

    enum class GfxCompareOperation : u8 {
        Never            = 0,
        LessThan         = 1,
        Equal            = 2,
        LessThanEqual    = 3,
        GreaterThan      = 4,
        NotEqual         = 5,
        GreaterThanEqual = 6,
        Always           = 7,
    };

    enum class GfxBorderColor : u8 {
        White            = 0,
        TransparentBlack = 1,
        Black            = 2,
    };

    enum class GfxMipMapFilter : u8 {
        None    = 0,
        Nearest = 1,
        Linear  = 2,
    };

    enum class GfxMinFilter : u8 {
        Invalid = 0,
        Nearest = 1,
        Linear  = 2,
    };

    enum class GfxMagFilter : u8 {
        Invalid = 0,
        Nearest = 1,
        Linear  = 2,
    };

    enum class GfxReductionFilter : u8 {
        Average = 0,
        Min     = 1,
        Max     = 2,
    };

    struct ResGfxSamplerInfo {
        u8    wrap_mode_u;
        u8    wrap_mode_v;
        u8    wrap_mode_w;
        u8    compare_op;
        u8    border_color;
        u8    max_anisotropy;
        union {
            u16 packed_options;
            struct {
                u16   mip_map_filter          : 2;
                u16   mag_filter              : 2;
                u16   min_filter              : 2;
                u16   is_enable_anisotropy    : 1;
                u16   is_compare_r_to_texture : 1;
                u16   reduction_filter        : 2;
                u16   reserve0                : 6;
            };
        };
        float lod_clamp_min;
        float lod_clamp_max;
        float lod_bias;
        u32   reserve1;
        u32   reserve2;
        u32   reserve3;
        
        constexpr void SetDefaults() {
            this->wrap_mode_u       =  static_cast<u8>(GfxWrapMode::ClampToEdge);
            this->wrap_mode_v       =  static_cast<u8>(GfxWrapMode::ClampToEdge);
            this->wrap_mode_w       =  static_cast<u8>(GfxWrapMode::ClampToEdge);
            this->compare_op        =  static_cast<u8>(GfxCompareOperation::Never);
            this->border_color      =  static_cast<u8>(GfxBorderColor::White);
            this->max_anisotropy    =  1;
            this->mip_map_filter    =  static_cast<u8>(GfxMipMapFilter::Linear);
            this->mag_filter        =  static_cast<u8>(GfxMagFilter::Linear);
            this->min_filter        =  static_cast<u8>(GfxMinFilter::Linear);
            this->is_enable_anisotropy = false;
            this->is_compare_r_to_texture = false;
            this->reduction_filter  =  static_cast<u8>(GfxReductionFilter::Average);
            this->reserve0          =  0;
            this->lod_clamp_min     = -1000.0;
            this->lod_clamp_max     =  1000.0;
            this->lod_bias          =  0.0;
            this->reserve1          =  0;
            this->reserve2          =  0;
            this->reserve3          =  0;
        }

        constexpr void SetWrapModes(GfxWrapMode u, GfxWrapMode v, GfxWrapMode w) {
            this->wrap_mode_u = static_cast<u8>(u);
            this->wrap_mode_v = static_cast<u8>(v);
            this->wrap_mode_w = static_cast<u8>(w);
        }
        constexpr void SetWrapModeU(GfxWrapMode u) {
            this->wrap_mode_u = static_cast<u8>(u);
        }
        constexpr void SetWrapModeV(GfxWrapMode v) {
            this->wrap_mode_v = static_cast<u8>(v);
        }
        constexpr void SetWrapModeW(GfxWrapMode w) {
            this->wrap_mode_w = static_cast<u8>(w);
        }
        constexpr void SetCompareOperation(GfxCompareOperation compare_op) {
            this->compare_op = static_cast<u8>(compare_op);
        }
        constexpr void SetBorderColor(GfxBorderColor border_color) {
            this->border_color = static_cast<u8>(border_color);
        }
        constexpr void SetAnisotropy(bool is_enable_anisotropy, u8 max_anisotropy) {
            this->is_enable_anisotropy = is_enable_anisotropy;
            this->max_anisotropy = max_anisotropy;
        }
        constexpr void SetFilters(GfxMipMapFilter mip_map_filter, GfxMagFilter mag_filter, GfxMinFilter min_filter) {
            this->mip_map_filter = static_cast<u8>(mip_map_filter);
            this->mag_filter = static_cast<u8>(mag_filter);
            this->min_filter = static_cast<u8>(min_filter);
        }
        constexpr void SetMipMapFilter(GfxMipMapFilter mip_map_filter) {
            this->mip_map_filter = static_cast<u8>(mip_map_filter);
        }
        constexpr void SetMinFilter(GfxMinFilter min_filter) {
            this->min_filter = static_cast<u8>(min_filter);
        }
        constexpr void SetMaxFilter(GfxMagFilter mag_filter) {
            this->mag_filter = static_cast<u8>(mag_filter);
        }
        constexpr void SetIsCompareRToTexture(bool is_compare_r_to_texture) {
            this->is_compare_r_to_texture = is_compare_r_to_texture;
        }
        constexpr void SetReductionFilter(GfxReductionFilter reduction_filter) {
            this->reduction_filter = static_cast<u8>(reduction_filter);
        }
        constexpr void SetLodClamp(float lod_clamp_min, float lod_clamp_max) {
            this->lod_clamp_min = lod_clamp_min;
            this->lod_clamp_max = lod_clamp_max;
        }
        constexpr void SetLodBias(float lod_bias) {
            this->lod_bias = lod_bias;
        }
    };
    static_assert(sizeof(ResGfxSamplerInfo) == 0x20);

    constexpr u16 ConstructSamplerPackedOptions(GfxMipMapFilter mip_map_filter, GfxMagFilter mag_filter, GfxMinFilter min_filter, bool enable_anisotropy, bool enable_compare_op, GfxReductionFilter reduction_filter) {
        return (static_cast<u16>(mip_map_filter) & 3) | ((static_cast<u16>(mag_filter) & 3) << 2) | ((static_cast<u16>(min_filter) & 3) << 4) | ((static_cast<u16>(enable_anisotropy) & 1) << 6) | ((static_cast<u16>(enable_compare_op) & 1) << 7) | ((static_cast<u16>(reduction_filter) & 3) << 8);
    }

    enum class GfxPrimitiveTopology {
        Points                 = 0,
        Lines                  = 1,
        LineStrip              = 2,
        Triangles              = 3,
        TrianglesStrip         = 4,
        LinesAdjacency         = 5,
        LineStripAdjacency     = 6,
        TrianglesAdjacency     = 7,
        TriangleStripAdjacency = 8,
        Patches                = 9,
    };

    enum class GfxIndexFormat {
        U8  = 0,
        U16 = 1,
        U32 = 2,
    };

    constexpr u32 GfxIndexFormatToSize(GfxIndexFormat index_format) {
        return (1 << static_cast<u32>(index_format));
    }

    enum class GfxComparisonFunction {
        Never            = 0,
        Less             = 1,
        Equal            = 2,
        LessThanEqual    = 3,
        Greater          = 4,
        NotEqual         = 5,
        GreaterThanEqual = 6,
        Always           = 7,
    };

    enum class GfxStencilOperation {
        Keep          = 0,
        Zero          = 1,
        Replace       = 2,
        Increment     = 3,
        Decrement     = 4,
        Invert        = 5,
        IncrementWrap = 6,
        DecrementWrap = 7,
    };

    enum class GfxBlendFactor : u8 {
        Zero                     = 0,
        One                      = 1,
        SourceColor              = 2,
        OneMinusSourceColor      = 3,
        DestinationColor         = 4,
        OneMinusDestinationColor = 5,
        SourceAlpha              = 6,
        OneMinusSourceAlpha      = 7,
        DestinationAlpha         = 8,
        OneMinusDestinationAlpha = 9,
        ConstantColor            = 10,
        OneMinusConstantColor    = 11,
        ConstantAlpha            = 12,
        OneMinusConstantAlpha    = 13,
        SourceAlphaSaturate      = 14,
        SourceOneColor           = 15,
        OneMinusSourceOneColor   = 16,
        SourceOneAlpha           = 17,
        OneMinusSourceOneAlpha   = 18,
    };

    enum class GfxBlendEquation : u8 {
        Add        = 0,
        Sub        = 1,
        ReverseSub = 2,
        Min        = 3,
        Max        = 4,
    };

    enum class GfxLogicOperation : u8 {
        Clear        = 0,
        And          = 1,
        AndReverse   = 2,
        Copy         = 3,
        AndInverted  = 4,
        NoOp         = 5,
        Xor          = 6,
        Or           = 7,
        Nor          = 8,
        Equivalent   = 9,
        Invert       = 10,
        OrReverse    = 11,
        CopyInverted = 12,
        OrInverted   = 13,
        Nand         = 14,
        Set          = 15,
    };

    enum class GfxCullMode : u8 {
        None  = 0,
        Front = 1,
        Back  = 2,
    };
    VP_ENUM_FLAG_TRAITS(GfxCullMode);

    enum class GfxFillMode : u8 {
        Point = 0,
        Line  = 1,
        Fill  = 2,
    };

    enum class GfxFrontFace : u8 {
        CounterClockwise = 0,
        Clockwise        = 1,
    };

    enum class GfxUserDataType : u8 {
        S32    = 0,
        Float  = 1,
        String = 2,
        Byte   = 3,
    };

    struct ResGfxUserData {
        const char *name;
        void       *value;
        s32         data_count;
        u8          data_type;
        u8          reserve0[0x2b];
    };
    static_assert(sizeof(ResGfxUserData) == 0x40);

    enum class GfxShaderStage : u32 {
        Vertex                 = 0,
        TessellationControl    = 1,
        TessellationEvaluation = 2,
        Geometry               = 3,
        Fragment               = 4,
        Compute                = 5,
        /* Vp ext (Mesh shaders confirmed 8) */
        Kernel                 = 6,
        Task                   = 7,
        Mesh                   = 8,
    };
    static constexpr u32 cMaxShaderStageCount = vp::util::TEnumTraits<GfxShaderStage>::cEnumCount;

    enum class GfxShaderStageBits : u32 {
        Vertex                 = (1 << 0),
        TessellationControl    = (1 << 1),
        TessellationEvaluation = (1 << 2),
        Geometry               = (1 << 3),
        Fragment               = (1 << 4),
        Compute                = (1 << 5),
        /* Vp ext */
        Kernel                 = (1 << 6),
        Task                   = (1 << 7),
        Mesh                   = (1 << 8),
    };
    VP_ENUM_FLAG_TRAITS(GfxShaderStageBits);

    enum class GfxShaderCodeType : u8 {
        Binary       = 0x0,
        Intermediate = 0x1,
        Source       = 0x2,
    };

    enum class GfxShaderInterfaceType : u32 {
        InputAttribute  = 0x0,
        OutputAttribute = 0x1,
        TextureSampler  = 0x2,
        UniformBuffer   = 0x3,
        StorageBuffer   = 0x4,
        Image           = 0x5,
        SeparateTexture = 0x6,
        SeparateSampler = 0x7,
    };
}
