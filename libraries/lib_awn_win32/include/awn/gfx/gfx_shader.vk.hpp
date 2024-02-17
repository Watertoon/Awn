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

    enum class ShaderStage : u32 {
        Vertex                 = (1 << 1),
        TessellationEvaluation = (1 << 2),
        TessellationControl    = (1 << 3),
        Geometry               = (1 << 4),
        Fragment               = (1 << 5),
        Compute                = (1 << 6),
        Task                   = (1 << 7),
        Mesh                   = (1 << 8),
        Ray                    = (1 << 9),
        AnyHit                 = (1 << 10),
        ClosestHit             = (1 << 11),
        Miss                   = (1 << 12),
        Callable               = (1 << 13),
    };

    struct PrimitiveShaderInfo {
        u32   vertex_code_size;
        u32   tessellation_control_code_size;
        u32   tessellation_evaluation_code_size;
        u32   geometry_code_size;
        u32   fragment_code_size;
        void *vertex_code;
        void *tessellation_control_code;
        void *tessellation_evaluation_code;
        void *geometry_code;
        void *fragment_code;
    };
    struct MeshShaderInfo {
        u32   task_code_size;
        u32   mesh_code_size;
        u32   fragment_code_size;
        void *task_code;
        void *mesh_code;
        void *fragment_code;
    };
    struct ComputeShaderInfo {
        u32   compute_code_size;
        void *compute_code;
    };

    class Shader {
        public:
            static constexpr u32 cInvalidInterfaceSlot = 0xffff'ffff;
        public:
            friend class CommandBuffer;
        private:
            u32                            m_shader_count;
            u32                            m_shader_stage_mask;
            VkShaderStageFlagBits          m_vk_shader_stage_flag_array[Context::cTargetMaxSimultaneousShaderStageCount];
            VkShaderEXT                    m_vk_shader_array[Context::cTargetMaxSimultaneousShaderStageCount];
            vp::res::ResBnshShaderProgram *m_res_program;
        public:
            constexpr ALWAYS_INLINE Shader() : m_shader_count(0), m_shader_stage_mask(), m_vk_shader_stage_flag_array{}, m_vk_shader_array{} {/*...*/}
            constexpr ~Shader() {/*...*/}

            void Initialize(MeshShaderInfo *shader_info, const bool is_binary = false);
            void Initialize(PrimitiveShaderInfo *shader_info, const bool is_binary = true);
            void Initialize(ComputeShaderInfo *shader_info, const bool is_binary = true);

            void Initialize(vp::res::ResBnshShaderProgram *res_program);
            void Finalize();

            u32 TryGetInterfaceSlot(gfx::ShaderStage shader_stage, const char *key);
        public:
            u32 SetupForCommandList(VkShaderEXT **out_shader_array, VkShaderStageFlagBits **out_shader_stage_array);
    };
}
