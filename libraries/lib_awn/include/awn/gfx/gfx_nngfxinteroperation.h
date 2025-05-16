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

    using ShaderStage = vp::res::GfxShaderStage;
    using ShaderStageBits = vp::res::GfxShaderStageBits;

    /* Resource sizes */
    constexpr inline const size_t cMaxGfxMemoryPoolSize  = vp::res::cMaxGfxMemoryPoolSize;
    constexpr inline const size_t cMaxGfxBufferSize      = vp::res::cMaxGfxBufferSize;
    constexpr inline const size_t cMaxGfxTextureSize     = vp::res::cMaxGfxTextureSize;
    constexpr inline const size_t cMaxGfxTextureViewSize = vp::res::cMaxGfxTextureViewSize;
    constexpr inline const size_t cMaxGfxSamplerSize     = vp::res::cMaxGfxSamplerSize;
    constexpr inline const size_t cMaxGfxShaderSize      = vp::res::cMaxGfxShaderSize;

    /* Resource flags */
    using GpuAccessFlags      = vp::res::GfxGpuAccessFlags;
    using MemoryPropertyFlags = vp::res::GfxMemoryPoolFlags;

    /* Draw time enums */
    using PrimitiveTopology = vp::res::GfxPrimitiveTopology;
    using IndexFormat       = vp::res::GfxIndexFormat;

    /* Texture enums */
    using ImageFormat           = vp::res::GfxImageFormat;
    using ImageStorageDimension = vp::res::GfxImageStorageDimension;
    using TileMode              = vp::res::GfxTileMode;

    /* TextureView enums */
    using TextureDepthStencilMode = vp::res::GfxTextureDepthStencilMode;
    using TextureSwizzle = vp::res::GfxTextureSwizzle;
    using ImageDimension = vp::res::GfxImageDimension;

    /* Sampler enums */
    using MinFilter        = vp::res::GfxMinFilter;
    using MagFilter        = vp::res::GfxMagFilter;
    using MipMapFilter     = vp::res::GfxMipMapFilter;
    using ReductionFilter  = vp::res::GfxReductionFilter;
    using WrapMode         = vp::res::GfxWrapMode;
    using CompareOperation = vp::res::GfxCompareOperation;
    using BorderColor      = vp::res::GfxBorderColor;

    /* Blend State enums */
    using BlendFactor    = vp::res::GfxBlendFactor;
    using BlendEquation  = vp::res::GfxBlendEquation;
    using LogicOperation = vp::res::GfxLogicOperation;

    /* Depth Stencil State enums */
    using ComparisonFunction = vp::res::GfxComparisonFunction;
    using StencilOperation   = vp::res::GfxStencilOperation;

    /* Rasterizer State enums */
    using CullMode  = vp::res::GfxCullMode;
    using FillMode  = vp::res::GfxFillMode;
    using FrontFace = vp::res::GfxFrontFace;

    /* Vertex State enums */
    using AttributeFormat = vp::res::GfxAttributeFormat;

    /* Shader interface enums */
    using ShaderCodeType      = vp::res::GfxShaderCodeType;
    using ShaderInterfaceType = vp::res::GfxShaderInterfaceType;

    /* Infos */
    using MemoryPoolInfo  = vp::res::ResGfxMemoryPoolInfo;
    using BufferInfo      = vp::res::ResGfxBufferInfo;
    using TextureInfo     = vp::res::ResGfxTextureInfo;
    class Texture;
    using TextureViewInfo = vp::res::ResGfxTextureViewInfo<Texture>;
    using SamplerInfo     = vp::res::ResGfxSamplerInfo;
}
