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

    struct BlendTargetState {
        u8 is_enable;
        u8 blend_factor_source_color;
        u8 blend_factor_destination_color;
        u8 blend_equation_color;
        u8 blend_factor_source_alpha;
        u8 blend_factor_destination_alpha;
        u8 blend_equation_alpha;
        union {
            u8 channel_mask;
            struct {
                u8 is_channel_r : 1;
                u8 is_channel_g : 1;
                u8 is_channel_b : 1;
                u8 is_channel_a : 1;
                u8 reserve0       : 4;
            };
        };
        
        constexpr void SetDefaults() {
            is_enable                      = 0;
            blend_factor_source_color      = static_cast<u8>(BlendFactor::One);
            blend_factor_destination_color = static_cast<u8>(BlendFactor::Zero);
            blend_equation_color           = static_cast<u8>(BlendEquation::Add);
            blend_factor_source_alpha      = static_cast<u8>(BlendFactor::One);
            blend_factor_destination_alpha = static_cast<u8>(BlendFactor::Zero);
            blend_equation_alpha           = static_cast<u8>(BlendEquation::Add);
            channel_mask                   = 0xf;
        }
    };
    struct BlendState {
        u8                 blend_target_state_count;
        u8                 logic_op;
        u8                 is_enable_multi_blend;
        u8                 is_enable_logic_op;
        vp::util::Color4f  blend_constant_color;
        BlendTargetState  *blend_target_state_array;

        constexpr void SetDefaults() {
            blend_target_state_count = 0;
            logic_op                 = static_cast<u8>(LogicOperation::NoOp);
            is_enable_multi_blend    = 0;
            blend_constant_color     = vp::util::cWhite4f;
            blend_target_state_array = nullptr;
        }
    };

    struct DepthStencilState {
        union {
            u8 enable_mask;
            struct {
                u8 is_enable_depth_test   : 1;
                u8 is_enable_depth_write  : 1;
                u8 is_enable_stencil_test : 1;
                u8 is_enable_depth_bounds : 1;
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
            depth_comparison_function              = static_cast<u8>(ComparisonFunction::Less);
            stencil_value_mask                     = 0xff;
            stencil_write_mask                     = 0xff;
            front_face_stencil_fail_op             = static_cast<u8>(StencilOperation::Keep);
            front_face_stencil_depth_fail_op       = static_cast<u8>(StencilOperation::Keep);
            front_face_stencil_depth_pass_op       = static_cast<u8>(StencilOperation::Keep);
            front_face_stencil_comparison_function = static_cast<u8>(ComparisonFunction::Less);
            front_face_stencil_reference           = 0;
            back_face_stencil_fail_op              = static_cast<u8>(StencilOperation::Keep);
            back_face_stencil_depth_fail_op        = static_cast<u8>(StencilOperation::Keep);
            back_face_stencil_depth_pass_op        = static_cast<u8>(StencilOperation::Keep);
            back_face_stencil_comparison_function  = static_cast<u8>(ComparisonFunction::Less);
            back_face_stencil_reference            = 0;
        }
    };

    
    struct RasterizerState {
        union {
            u32 enable_mask;
            struct {
                u32 is_enable_rasterizer_discard      : 1;
                u32 is_enable_multisample             : 1;
                u32 is_enable_depth_clamp             : 1;
                u32 is_enable_conservative_rasterizer : 1;
                u32 is_enable_alpha_to_coverage       : 1;
                u32 reserve0                       : 27;
            };
        };
        u8    fill_mode;
        u8    front_face;
        u8    cull_mode;
        u8    sample_count;
        u32   sample_mask;
        float polygon_offset_clamp;
        float polygon_offset_constant_factor;
        float polygon_offset_slope_factor;
        
        constexpr void SetDefaults() {
            fill_mode                       = static_cast<u8>(FillMode::Line);
            front_face                      = static_cast<u8>(FrontFace::CounterClockwise);
            cull_mode                       = static_cast<u8>(CullMode::Back);
            enable_mask                     = (1 << 2);
            sample_count                    = 0;
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
        static constexpr u32 cInvalidInterfaceSlot = 0xffff'ffff;

        u32         interface_slot = cInvalidInterfaceSlot;
        u32         binding;
        u32         offset;
        u32         attribute_format;
        const char *name;
    };
    struct VertexState {
        u32                   vertex_attribute_state_count;
        u32                   vertex_buffer_state_count;
        VertexAttributeState *vertex_attribute_state_array;
        VertexBufferState    *vertex_buffer_state_array;

        void ResolveInterfaceSlots(Shader *shader) {
            for (u32 i = 0; i < this->vertex_attribute_state_count; ++i) {
                const char *input_name = this->vertex_attribute_state_array[i].name;
                if (input_name == nullptr) { continue; }
                this->vertex_attribute_state_array[i].interface_slot = shader->GetInterfaceSlot(ShaderStage::Vertex, ShaderInterfaceType::InputAttribute, input_name);
            }
        }
    };

    struct ViewportState {
        vp::util::Vector2f position;
        vp::util::Vector2f extents;
        vp::util::Vector2f depth_range;
    };

    struct ScissorState {
        vp::util::Vector2i position;
        vp::util::Vector2i extents;
    };

    struct ViewportScissorState {
        u16            is_enable_scissor;
        u16            viewport_scissor_count;
        ViewportState *viewport_state_array;
        ScissorState  *scissor_state_array;
    };

#ifdef VP_TARGET_GRAPHICS_API_vk
    static_assert(sizeof(ViewportState) == sizeof(VkViewport));
    static_assert(sizeof(ScissorState) == sizeof(VkRect2D));
#endif
}
