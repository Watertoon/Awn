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

    /* Resource sizes */
    constexpr inline const size_t cMaxGfxMemoryPoolSize  = 0x120;
    constexpr inline const size_t cMaxGfxBufferSize      = 0x48;
    constexpr inline const size_t cMaxGfxTextureSize     = 0xd8;
    constexpr inline const size_t cMaxGfxTextureViewSize = 0x48;
    constexpr inline const size_t cMaxGfxSamplerSize     = 0x78;

    /* Resource flags */
    using GpuAccessFlags      = vp::res::GfxGpuAccessFlags;
    using MemoryPropertyFlags = vp::res::GfxMemoryPoolFlags;

    /* Draw time enums */
    using PrimitiveTopology  = vp::res::GfxPrimitiveTopology;
    using IndexFormat        = vp::res::GfxIndexFormat;

    /* Texture enums */
    using ImageFormat           = vp::res::GfxImageFormat;
    using ImageStorageDimension = vp::res::GfxImageStorageDimension;

    /* TextureView enums */
    using TextureSwizzle     = vp::res::GfxTextureSwizzle;
    using ImageDimension     = vp::res::GfxImageDimension;

    /* Sampler enums */
    using MinFilter          = vp::res::GfxMinFilter;
    using MagFilter          = vp::res::GfxMagFilter;
    using MipMapFilter       = vp::res::GfxMipMapFilter;
    using ReductionFilter    = vp::res::GfxReductionFilter;
    using WrapMode           = vp::res::GfxWrapMode;
    using CompareOperation   = vp::res::GfxCompareOperation;
    using BorderColor        = vp::res::GfxBorderColor;

    /* Blend State enums */
    using BlendFactor        = vp::res::GfxBlendFactor;
    using BlendEquation      = vp::res::GfxBlendEquation;
    using LogicOperation     = vp::res::GfxLogicOperation;

    /* Depth Stencil State enums */
    using ComparisonFunction = vp::res::GfxComparisonFunction;
    using StencilOperation   = vp::res::GfxStencilOperation;

    /* Rasterizer State enums */
    using CullMode           = vp::res::GfxCullMode;
    using FillMode           = vp::res::GfxFillMode;
    using FrontFace          = vp::res::GfxFrontFace;

    /* Vertex State enums */
    using AttributeFormat    = vp::res::GfxAttributeFormat;

    /* Infos */
    using BufferInfo         = vp::res::ResGfxBufferInfo;
    using TextureInfo        = vp::res::ResGfxTextureInfo;
    class Texture;
    using TextureViewInfo    = vp::res::ResGfxTextureViewInfo<Texture>;
    using SamplerInfo        = vp::res::ResGfxSamplerInfo;
}
