#pragma once

namespace awn::gfx {

    struct BlendTargetStateInfo {
        u8 enable_blend;
        u8 blend_factor_source_color;
        u8 blend_factor_destination_color;
        u8 blend_equation_color;
        u8 blend_factor_source_alpha;
        u8 blend_factor_destination_alpha;
        u8 blend_equation_alpha;
        union {
            u8 channel_mask;
            struct {
                u8 channel_mask_r : 1;
                u8 channel_mask_g : 1;
                u8 channel_mask_b : 1;
                u8 channel_mask_a : 1;
                u8 reserve0       : 4;
            };
        };
        
        constexpr void SetDefaults() {
            enable_blend                   = 0;
            blend_factor_source_color      = res::GfxBlendFunction::One;
            blend_factor_destination_color = res::GfxBlendFunction::Zero;
            blend_equation_color           = res::GfxBlendEquation::Add;
            blend_factor_source_alpha      = res::GfxBlendFunction::One;
            blend_factor_destination_alpha = res::GfxBlendFunction::Zero;
            blend_equation_alpha           = res::GfxBlendEquation::Add;
            channel_mask                   = 0xf;
        }
    };
    struct BlendState {
        u8                    blend_target_state_count;
        u8                    logic_op;
        u8                    enable_multi_blend;
        u8                    reserve0;
        vp::util::Color4f     blend_constant_color;
        BlendTargetStateInfo *blend_target_state_info_array;

        constexpr void SetDefaults() {
            blend_target_state_count      = 0;
            logic_op                      = res::GfxLogicOperation::NoOp;
            enable_multi_blend            = 0;
            blend_constant_color          = vp:util::cWhite;
            blend_target_state_info_array = nullptr;
        }
    };

    struct DepthStencilState {
        union {
            u8 enable_mask;
            struct {
                u8 enable_depth_test   : 1;
                u8 enable_depth_write  : 1;
                u8 enable_stencil_test : 1;
                u8 enable_depth_bounds : 1;
                u8 reserve0            : 4;
            };
        };
        u8 depth_comparison_function;
        u8 stencil_value_mask;
        u8 stencil_write_mask;
        u8 front_face_stencil_fail_op;
        u8 front_face_stencil_depth_fail_op;
        u8 front_face_stencil_depth_pass_op;
        u8 front_face_stencil_comparison_function;
        u8 front_face_stencil_reference;
        u8 back_face_stencil_fail_op;
        u8 back_face_stencil_depth_fail_op;
        u8 back_face_stencil_depth_pass_op;
        u8 back_face_stencil_comparison_function;
        u8 back_face_stencil_reference;

        constexpr void SetDefaults() {
            enable_mask                            = 0;
            depth_comparison_function              = res::GfxComparisonFunction::Less;
            stencil_value_mask                     = 0xff;
            stencil_write_mask                     = 0xff;
            front_face_stencil_fail_op             = res::GfxStencilOperation::Keep;
            front_face_stencil_depth_fail_op       = res::GfxStencilOperation::Keep;
            front_face_stencil_depth_pass_op       = res::GfxStencilOperation::Keep;
            front_face_stencil_comparison_function = res::GfxComparisonFunction::Less;
            front_face_stencil_reference           = 0;
            back_face_stencil_fail_op              = res::GfxStencilOperation::Keep;
            back_face_stencil_depth_fail_op        = res::GfxStencilOperation::Keep;
            back_face_stencil_depth_pass_op        = res::GfxStencilOperation::Keep;
            back_face_stencil_comparison_function  = res::GfxComparisonFunction::Less;
            back_face_stencil_reference            = 0;
        }
    };

    
    struct RasterizerState {
        union {
            u32 enable_mask;
            struct {
                u32 enable_rasterizer_discard      : 1;
                u32 enable_multisample             : 1;
                u32 enable_depth_clamp             : 1;
                u32 enable_conservative_rasterizer : 1;
                u32 enable_alpha_to_coverage       : 1;
                u32 reserve0                       : 27;
            };
        };
        u8 fill_mode;
        u8 front_face;
        u8 cull_mode;
        u8 rasterization_samples;
        float sample_mask;
        float polygon_offset_clamp;
        float polygon_offset_constant_factor;
        float polygon_offset_slope_factor;
        
        constexpr void SetDefaults() {
            fill_mode                       = res::GfxFillMode::;
            front_face                      = res::GfxFrontFace::;
            cull_mode                       = res::GfxCullMode::;
            enable_mask                     = (1 << 2);
            sample_mask                     = 0xffff'ffff;
            polygon_offset_clamp            = 0.0f;
            polygon_offset_constant_factor  = 0.0f;
            polygon_offset_slope_factor     = 0.0f;
        }
    };

    struct VertexBufferState {
        u32 stride;
        u32 divisor;
    };
    struct VertexAttributeState {
        u16         interface_slot;
        u16         binding;
        u32         offset;
        u32         attribute_format;
        u32         reserve0;
    };
    struct VertexState {
        u32                   vertex_attribute_state_count;
        u32                   vertex_buffer_state_count;
        VertexAttributeState *vertex_attribute_state_array;
        VertexBufferState    *vertex_buffer_state_array;
    };


    struct ViewportState {
        vp::util::Vector2f position;
        vp::util::Vector2f extents;
        vp::util::Vector2f depth_clamp;
    };
    static_assert(sizeof(ViewportState) == sizeof(VkViewport));
    struct ScissorState {
        vp::util::Vector2f position;
        vp::util::Vector2f extents;
    };
    static_assert(sizeof(ViewportState) == sizeof(VkRect2d));
    struct ViewportScissorState {
        u16            enable_scissors;
        u16            viewport_scissor_count;
        u32            reserve0;
        ViewportState *viewport_state_array;
        ScissorState  *scissor_state_array;
    };
}
