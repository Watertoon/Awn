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
                u8 reserve0 : 4;
            };
        };

        constexpr void SetDefaults() {
            this->is_enable                      = 0;
            this->blend_factor_source_color      = static_cast<u8>(BlendFactor::One);
            this->blend_factor_destination_color = static_cast<u8>(BlendFactor::Zero);
            this->blend_equation_color           = static_cast<u8>(BlendEquation::Add);
            this->blend_factor_source_alpha      = static_cast<u8>(BlendFactor::One);
            this->blend_factor_destination_alpha = static_cast<u8>(BlendFactor::Zero);
            this->blend_equation_alpha           = static_cast<u8>(BlendEquation::Add);
            this->channel_mask                   = 0xf;
        }

        constexpr void SetIsEnabled(bool is_enabled) {
            this->is_enable = is_enabled;
        }
        constexpr void SetColorBlend(BlendFactor blend_factor_source, BlendFactor blend_factor_destination, BlendEquation blend_equation) {
            this->blend_factor_source_color = static_cast<u8>(blend_factor_source);
            this->blend_factor_destination_color = static_cast<u8>(blend_factor_destination);
            this->blend_equation_color = static_cast<u8>(blend_equation);
        }
        constexpr void SetAlphaBlend(BlendFactor blend_factor_source, BlendFactor blend_factor_destination, BlendEquation blend_equation) {
            this->blend_factor_source_alpha = static_cast<u8>(blend_factor_source);
            this->blend_factor_destination_alpha = static_cast<u8>(blend_factor_destination);
            this->blend_equation_alpha = static_cast<u8>(blend_equation);
        }
        constexpr void SetChannelMask(u8 is_r, u8 is_g, u8 is_b, u8 is_a) {
            this->channel_mask = is_r | (is_g << 1) | (is_b << 2) | (is_a << 3);
        }
    };

    struct BlendState {
        u8                 blend_target_state_count;
        u8                 logic_op;
        u8                 is_enable_multi_blend;
        u8                 is_enable_logic_op;
        vp::util::Color4f  blend_constant_color;
        const BlendTargetState  *blend_target_state_array;

        constexpr void SetDefaults() {
            this->blend_target_state_count = 0;
            this->logic_op                 = static_cast<u8>(LogicOperation::NoOp);
            this->is_enable_multi_blend    = 0;
            this->is_enable_logic_op       = 0;
            this->blend_constant_color     = vp::util::cWhite4f;
            this->blend_target_state_array = nullptr;
        }

        constexpr void SetBlendTargetStateArray(const BlendTargetState *state_array, u8 count) {
            this->blend_target_state_count = count;
            this->blend_target_state_array = state_array;
        }
        constexpr void SetLogicOperation(bool is_enable, LogicOperation logic_op) {
            this->is_enable_logic_op = is_enable;
            this->logic_op = static_cast<u8>(logic_op);
        }
        constexpr void SetConstantColor(vp::util::Color4f color) {
            this->blend_constant_color = color;
        }
    };

    struct DepthStencilState {
        union {
            u8 enable_mask;
            struct {
                u8 is_enable_depth_test : 1;
                u8 is_enable_depth_write : 1;
                u8 is_enable_stencil_test : 1;
                u8 reserve0 : 5;
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
            this->enable_mask                            = 0;
            this->depth_comparison_function              = static_cast<u8>(ComparisonFunction::Less);
            this->stencil_value_mask                     = 0xff;
            this->stencil_write_mask                     = 0xff;
            this->front_face_stencil_fail_op             = static_cast<u8>(StencilOperation::Keep);
            this->front_face_stencil_depth_fail_op       = static_cast<u8>(StencilOperation::Keep);
            this->front_face_stencil_depth_pass_op       = static_cast<u8>(StencilOperation::Keep);
            this->front_face_stencil_comparison_function = static_cast<u8>(ComparisonFunction::Less);
            this->front_face_stencil_reference           = 0;
            this->back_face_stencil_fail_op              = static_cast<u8>(StencilOperation::Keep);
            this->back_face_stencil_depth_fail_op        = static_cast<u8>(StencilOperation::Keep);
            this->back_face_stencil_depth_pass_op        = static_cast<u8>(StencilOperation::Keep);
            this->back_face_stencil_comparison_function  = static_cast<u8>(ComparisonFunction::Less);
            this->back_face_stencil_reference            = 0;
        }

        constexpr void SetIsEnableDepthTest(bool is_enable_depth_test) {
            this->is_enable_depth_test = is_enable_depth_test;
        }
        constexpr void SetIsEnableDepthWrite(bool is_enable_depth_write) {
            this->is_enable_depth_write = is_enable_depth_write;
        }
        constexpr void SetDepthComparisonFunction(ComparisonFunction comparison_function) {
            this->depth_comparison_function = static_cast<u8>(comparison_function);
        }
        constexpr void SetIsEnableStencilTest(bool is_enable_stencil_test) {
            this->is_enable_stencil_test = is_enable_stencil_test;
        }
        constexpr void SetStencilMasks(u8 value_mask, u8 write_mask) {
            this->stencil_value_mask = value_mask;
            this->stencil_write_mask = write_mask;
        }
        constexpr void SetFrontFace(StencilOperation fail_op, StencilOperation depth_fail_op, StencilOperation depth_pass_op, ComparisonFunction compare_func, u8 reference) {
            this->front_face_stencil_fail_op             = static_cast<u8>(fail_op);
            this->front_face_stencil_depth_fail_op       = static_cast<u8>(depth_fail_op);
            this->front_face_stencil_depth_pass_op       = static_cast<u8>(depth_pass_op);
            this->front_face_stencil_comparison_function = static_cast<u8>(compare_func);
            this->front_face_stencil_reference           = reference;
        }
        constexpr void SetBackFace(StencilOperation fail_op, StencilOperation depth_fail_op, StencilOperation depth_pass_op, ComparisonFunction compare_func, u8 reference) {
            this->back_face_stencil_fail_op             = static_cast<u8>(fail_op);
            this->back_face_stencil_depth_fail_op       = static_cast<u8>(depth_fail_op);
            this->back_face_stencil_depth_pass_op       = static_cast<u8>(depth_pass_op);
            this->back_face_stencil_comparison_function = static_cast<u8>(compare_func);
            this->back_face_stencil_reference           = reference;
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
                u32 reserve0                          : 27;
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
            this->fill_mode                      = static_cast<u8>(FillMode::Fill);
            this->front_face                     = static_cast<u8>(FrontFace::CounterClockwise);
            this->cull_mode                      = static_cast<u8>(CullMode::Back);
            this->enable_mask                    = (1 << 2);
            this->sample_count                   = 1;
            this->sample_mask                    = 0xffff'ffff;
            this->polygon_offset_clamp           = 0.0f;
            this->polygon_offset_constant_factor = 0.0f;
            this->polygon_offset_slope_factor    = 0.0f;
        }

        constexpr void SetFillMode(FillMode fill_mode) {
            this->fill_mode = static_cast<u8>(fill_mode);
        }
        constexpr void SetFrontFace(FrontFace front_face) {
            this->front_face = static_cast<u8>(front_face);
        }
        constexpr void SetCullMode(CullMode cull_mode) {
            this->cull_mode = static_cast<u8>(cull_mode);
        }
        constexpr void SetSampleCount(u8 sample_count) {
            this->sample_count = sample_count;
        }
        constexpr void SetSampleMask(u32 sample_mask) {
            this->sample_mask = sample_mask;
        }
        constexpr void SetPolygonOffsets(float clamp, float constant_factor, float slope_factor) {
            this->polygon_offset_clamp           = clamp;
            this->polygon_offset_constant_factor = constant_factor;
            this->polygon_offset_slope_factor    = slope_factor;
        }
    };

    struct VertexBufferState {
        u32 stride;
        u32 divisor;

        constexpr void SetDefaults() {
            this->stride = 0;
            this->divisor = 1;
        }
        constexpr void SetStride(u32 stride) {
            this->stride = stride;
        }
        constexpr void SetDivisor(u32 divisor) {
            this->divisor = divisor;
        }
    };

    struct VertexAttributeState {
        static constexpr u32 cInvalidInterfaceSlot = 0xffff'ffff;

        u32         interface_slot;
        u32         binding;
        u32         offset;
        u32         attribute_format;
        const char *name;

        constexpr void SetDefaults() {
            this->interface_slot = cInvalidInterfaceSlot;
            this->binding = 0;
            this->offset = 0;
            this->attribute_format = static_cast<u32>(AttributeFormat::None_Unorm);
            this->name = nullptr;
        }

        constexpr void SetLocation(u32 location) {
            this->interface_slot = location;
        }
        constexpr void SetBufferBinding(u32 binding) {
            this->binding = binding;
        }
        constexpr void SetAttributeOffset(u32 offset) {
            this->offset = offset;
        }
        constexpr void SetAttributeFormat(AttributeFormat attribute_format) {
            this->attribute_format = static_cast<u32>(attribute_format);
        }
        constexpr void SetName(const char *name) {
            this->name = name;
        }
        
        constexpr u32 GetLocation() const {
            return this->interface_slot;
        }
    };

    struct VertexState {
        u32 vertex_attribute_state_count;
        u32 vertex_buffer_state_count;
        const VertexAttributeState *vertex_attribute_state_array;
        const VertexBufferState *vertex_buffer_state_array;

        constexpr void SetDefaults() {
            this->vertex_attribute_state_count = 0;
            this->vertex_buffer_state_count = 0;
            this->vertex_attribute_state_array = nullptr;
            this->vertex_buffer_state_array = nullptr;
        }

        constexpr void SetVertexAttributeStateArray(const VertexAttributeState *vertex_attribute_state_array, u32 count) {
            this->vertex_attribute_state_count = count;
            this->vertex_attribute_state_array = vertex_attribute_state_array;
        }
        constexpr void SetVertexBufferStateArray(const VertexBufferState *vertex_buffer_state_array, u32 count) {
            this->vertex_buffer_state_count = count;
            this->vertex_buffer_state_array = vertex_buffer_state_array;
        }

        //void ResolveInterfaceSlots(Shader *shader) {
        //    for (u32 i = 0; i < this->vertex_attribute_state_count; ++i) {
        //        const char *input_name = this->vertex_attribute_state_array[i].name;
        //        if (input_name == nullptr || *input_name == '\0') { continue; }
        //        this->vertex_attribute_state_array[i].interface_slot = shader->GetInterfaceSlot(ShaderStage::Vertex, ShaderInterfaceType::InputAttribute, input_name);
        //    }
        //}
    };

    struct ViewportState {
        Float2 position;
        Float2 extents;
        Float2 depth_range;

        constexpr void SetDefaults() {
            this->position = VectorConstant<Float2>::cZero;
            this->extents = { 640.0f, 480.0f, };
            this->depth_range = { 0.0f, 1.0f, };
        }

        constexpr void SetPosition(const Float2 &position) {
            this->position = position;
        }
        constexpr void SetSize(const Float2 &extents) {
            this->extents = extents;
        }
        constexpr void SetDepthRange(const Float2 &depth_range) {
            this->depth_range = depth_range;
        }
    };

    struct ScissorState {
        Int2 position;
        Int2 extents;

        constexpr void SetDefaults() {
            this->position = VectorConstant<Int2>::cZero;
            this->extents = VectorConstant<Int2>::cZero;
        }

        constexpr void SetPosition(const Int2 &position) {
            this->position = position;
        }
        constexpr void SetSize(const Int2 &extents) {
            this->extents = extents;
        }
    };

    struct ViewportScissorState {
        bool           is_enable_scissor;
        u16            state_count;
        ViewportState *viewport_state_array;
        ScissorState  *scissor_state_array;

        constexpr void SetDefaults() {
            this->is_enable_scissor = false;
            this->state_count = 0;
            this->viewport_state_array = nullptr;
            this->scissor_state_array = nullptr;
        }

        constexpr void SetViewportScissorStateArray(ViewportState *viewport_state_array, ScissorState *scissor_state_array, u32 state_count) {
            this->is_enable_scissor = (scissor_state_array != nullptr);
            this->state_count = static_cast<u16>(state_count);
            this->viewport_state_array = viewport_state_array;
            this->scissor_state_array = scissor_state_array;
        }
    };

#ifdef VP_TARGET_GRAPHICS_API_vk
    static_assert(sizeof(ViewportState) == sizeof(VkViewport));
    static_assert(sizeof(ScissorState) == sizeof(VkRect2D));
#endif
}
