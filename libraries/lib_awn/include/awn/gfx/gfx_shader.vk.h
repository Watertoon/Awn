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

    static constexpr inline u32 cMaxShaderStageCount = impl::Context::cTargetMaxShaderStageCount;

    struct PrimitiveShaderInfo {
        union {
            void *code_array[5];
            struct {
                void *code_vertex;
                void *code_tessellation_control;
                void *code_tessellation_evaluation;
                void *code_geometry;
                void *code_fragment;
            };
        };
        union {
            u32 code_size_array[5];
            struct {
                u32   code_size_vertex;
                u32   code_size_tessellation_control;
                u32   code_size_tessellation_evaluation;
                u32   code_size_geometry;
                u32   code_size_fragment;
            };
        };
    };
    struct MeshShaderInfo {
        union {
            void *code_array[3];
            struct {
                void *code_task;
                void *code_mesh;
                void *code_fragment;
            };
        };
        union {
            u32 code_size_array[3];
            struct {
                u32   code_size_task;
                u32   code_size_mesh;
                u32   code_size_fragment;
                
            };
        };
    };
    struct ComputeShaderInfo {
        void *code_compute;
        u32   code_size_compute;
    };

    class Shader {
        public:
            static constexpr u32 cInvalidInterfaceSlot = 0xffff'ffff;
        public:
            friend class CommandBuffer;
        private:
            sys::Event m_init_event;
            VkShaderEXT m_vk_shader_array[cMaxShaderStageCount];
            vp::res::ResBnshShaderProgram *m_res_program;
        private:
            Result InitializeImpl(void **code_array, u32 *code_size_array, const ShaderStage *stage_array, u32 stage_count, ShaderCodeType code_type, VkShaderCreateFlagsEXT create_flags);
        public:
            constexpr Shader() : m_init_event(sys::SignalState::Cleared, sys::ResetMode::Manual), m_vk_shader_array{}, m_res_program() {/*...*/}
            constexpr ~Shader() {/*...*/}

            Result Initialize(MeshShaderInfo *shader_info, ShaderCodeType code_type);
            Result Initialize(PrimitiveShaderInfo *shader_info, ShaderCodeType code_type);
            Result Initialize(ComputeShaderInfo *shader_info, ShaderCodeType code_type);

            Result Initialize(vp::res::ResBnshShaderProgram *res_program);
            void Finalize();

            u32 GetInterfaceSlot(ShaderStage shader_stage, ShaderInterfaceType interface_type, const char *key);
        public:
            constexpr VkShaderEXT *GetVkShaderArray() { return m_vk_shader_array; }
    };
    static_assert(sizeof(Shader) <= cMaxGfxShaderSize);
}
