#pragma once

namespace awn::gfx {

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
