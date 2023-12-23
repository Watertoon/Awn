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

            void Initialize(MeshShaderInfo *shader_info, const bool is_binary = false) {

                /* Integrity checks */
                VP_ASSERT((shader_info->mesh_code != nullptr) & (shader_info->fragment_code != nullptr));

                /* Setup mesh, fragment, and the optional task shader info */
                VkShaderCreateFlagsEXT create_flags = static_cast<VkShaderCreateFlagsEXT>(VK_SHADER_CREATE_LINK_STAGE_BIT_EXT | ((shader_info->task_code == nullptr) ? VK_SHADER_CREATE_NO_TASK_SHADER_BIT_EXT : 0));
                const VkShaderCreateInfoEXT vk_shader_create_info_array[Context::cTargetMaxMeshShaderStageCount] = {
                    {
                        .sType                  = VK_STRUCTURE_TYPE_SHADER_CREATE_INFO_EXT,
                        .flags                  = create_flags,
                        .stage                  = VK_SHADER_STAGE_MESH_BIT_EXT,
                        .nextStage              = VK_SHADER_STAGE_FRAGMENT_BIT,
                        .codeType               = (is_binary == false) ? VK_SHADER_CODE_TYPE_SPIRV_EXT : VK_SHADER_CODE_TYPE_BINARY_EXT,
                        .codeSize               = shader_info->mesh_code_size,
                        .pCode                  = shader_info->mesh_code,
                        .pName                  = "main",
                        .setLayoutCount         = Context::cTargetDescriptorSetLayoutCount,
                        .pSetLayouts            = Context::GetInstance()->GetVkDescriptorSetLayoutArray(),
                        .pushConstantRangeCount = Context::cTargetAllStagePushConstantRangeCount,
                        .pPushConstantRanges    = Context::GetInstance()->GetVkPushConstantRangeArray(),
                        .pSpecializationInfo    = nullptr
                    },
                    {
                        .sType                  = VK_STRUCTURE_TYPE_SHADER_CREATE_INFO_EXT,
                        .flags                  = create_flags,
                        .stage                  = VK_SHADER_STAGE_FRAGMENT_BIT,
                        .nextStage              = 0,
                        .codeType               = (is_binary == false) ? VK_SHADER_CODE_TYPE_SPIRV_EXT : VK_SHADER_CODE_TYPE_BINARY_EXT,
                        .codeSize               = shader_info->fragment_code_size,
                        .pCode                  = shader_info->fragment_code,
                        .pName                  = "main",
                        .setLayoutCount         = Context::cTargetDescriptorSetLayoutCount,
                        .pSetLayouts            = Context::GetInstance()->GetVkDescriptorSetLayoutArray(),
                        .pushConstantRangeCount = Context::cTargetAllStagePushConstantRangeCount,
                        .pPushConstantRanges    = Context::GetInstance()->GetVkPushConstantRangeArray(),
                        .pSpecializationInfo    = nullptr
                    },
                    {
                        .sType                  = VK_STRUCTURE_TYPE_SHADER_CREATE_INFO_EXT,
                        .flags                  = create_flags,
                        .stage                  = VK_SHADER_STAGE_TASK_BIT_EXT,
                        .nextStage              = VK_SHADER_STAGE_MESH_BIT_EXT,
                        .codeType               = (is_binary == false) ? VK_SHADER_CODE_TYPE_SPIRV_EXT : VK_SHADER_CODE_TYPE_BINARY_EXT,
                        .codeSize               = shader_info->task_code_size,
                        .pCode                  = shader_info->task_code,
                        .pName                  = "main",
                        .setLayoutCount         = Context::cTargetDescriptorSetLayoutCount,
                        .pSetLayouts            = Context::GetInstance()->GetVkDescriptorSetLayoutArray(),
                        .pushConstantRangeCount = Context::cTargetAllStagePushConstantRangeCount,
                        .pPushConstantRanges    = Context::GetInstance()->GetVkPushConstantRangeArray(),
                        .pSpecializationInfo    = nullptr
                    },
                };

                /* Create shaders */
                const u32 shader_count = (shader_info->task_code == nullptr) ? 2 : 3;
                const u32 vk_result    = ::pfn_vkCreateShadersEXT(Context::GetInstance()->GetVkDevice(), shader_count, vk_shader_create_info_array, Context::GetInstance()->GetVkAllocationCallbacks(), m_vk_shader_array);
                VP_ASSERT(vk_result == VK_SUCCESS);

                /* Set state */
                m_shader_count                  = shader_count;
                m_shader_stage_mask             = static_cast<u32>(ShaderStage::Mesh) | static_cast<u32>(ShaderStage::Fragment) | ((shader_info->task_code) ? static_cast<u32>(ShaderStage::Task) : 0);
                m_vk_shader_stage_flag_array[0] = VK_SHADER_STAGE_MESH_BIT_EXT;
                m_vk_shader_stage_flag_array[1] = VK_SHADER_STAGE_FRAGMENT_BIT;
                m_vk_shader_stage_flag_array[2] = VK_SHADER_STAGE_TASK_BIT_EXT;

                return;
            }

            void Initialize(PrimitiveShaderInfo *shader_info, const bool is_binary = true) {

                /* Integrity checks */
                VP_ASSERT((shader_info->vertex_code != nullptr) & (shader_info->fragment_code != nullptr));

                /* Calculate shader stages */
                VkShaderStageFlags next_stage_array[Context::cTargetMaxPrimitiveShaderStageCount] = {
                    VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT,
                    VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT,
                    VK_SHADER_STAGE_GEOMETRY_BIT,
                    VK_SHADER_STAGE_FRAGMENT_BIT,
                };
                u32 code_size_array[Context::cTargetMaxPrimitiveShaderStageCount] = {
                    shader_info->vertex_code_size,
                    shader_info->tessellation_control_code_size,
                    shader_info->tessellation_evaluation_code_size,
                    shader_info->geometry_code_size,
                    shader_info->fragment_code_size,
                };
                void *code_array[Context::cTargetMaxPrimitiveShaderStageCount] = {
                    shader_info->vertex_code,
                    shader_info->tessellation_control_code,
                    shader_info->tessellation_evaluation_code,
                    shader_info->geometry_code,
                    shader_info->fragment_code,
                };

                /* TODO; refactor to reduce redundancy? */
                m_vk_shader_stage_flag_array[0] = VK_SHADER_STAGE_VERTEX_BIT;
                m_vk_shader_stage_flag_array[1] = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
                m_vk_shader_stage_flag_array[2] = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
                m_vk_shader_stage_flag_array[3] = VK_SHADER_STAGE_GEOMETRY_BIT;
                m_vk_shader_stage_flag_array[4] = VK_SHADER_STAGE_FRAGMENT_BIT;

                u32 shader_count      = 5;
                u32 shader_stage_mask = static_cast<u32>(ShaderStage::Vertex) | static_cast<u32>(ShaderStage::TessellationControl) | static_cast<u32>(ShaderStage::TessellationEvaluation) | static_cast<u32>(ShaderStage::Geometry) | static_cast<u32>(ShaderStage::Fragment);
                if (shader_info->geometry_code == nullptr && (shader_info->tessellation_control_code == nullptr || shader_info->tessellation_evaluation_code == nullptr)) {
                    m_vk_shader_stage_flag_array[1] = VK_SHADER_STAGE_FRAGMENT_BIT;
                    next_stage_array[1]             = 0;
                    code_size_array[1]              = shader_info->fragment_code_size;
                    code_array[1]                   = shader_info->fragment_code;
                    shader_count                    = 2;
                    shader_stage_mask               = static_cast<u32>(ShaderStage::Vertex) | static_cast<u32>(ShaderStage::Fragment);
                } else if (shader_info->tessellation_control_code == nullptr || shader_info->tessellation_evaluation_code == nullptr) {
                    m_vk_shader_stage_flag_array[1] = VK_SHADER_STAGE_GEOMETRY_BIT;
                    m_vk_shader_stage_flag_array[2] = VK_SHADER_STAGE_FRAGMENT_BIT;
                    next_stage_array[1]             = VK_SHADER_STAGE_FRAGMENT_BIT;
                    next_stage_array[2]             = 0;
                    code_size_array[1]              = shader_info->geometry_code_size;
                    code_size_array[2]              = shader_info->fragment_code_size;
                    code_array[1]                   = shader_info->geometry_code;
                    code_array[2]                   = shader_info->fragment_code;
                    shader_count                    = 3;
                    shader_stage_mask               = static_cast<u32>(ShaderStage::Vertex) | static_cast<u32>(ShaderStage::Geometry) | static_cast<u32>(ShaderStage::Fragment);
                } else if (shader_info->geometry_code == nullptr) {
                    m_vk_shader_stage_flag_array[3] = VK_SHADER_STAGE_FRAGMENT_BIT;
                    next_stage_array[3]             = 0;
                    code_size_array[3]              = shader_info->fragment_code_size;
                    code_array[3]                   = shader_info->fragment_code;
                    shader_count                    = 4;
                    shader_stage_mask               = static_cast<u32>(ShaderStage::Vertex) | static_cast<u32>(ShaderStage::TessellationControl) | static_cast<u32>(ShaderStage::TessellationEvaluation) | static_cast<u32>(ShaderStage::Fragment);
                }

                /* Setup mesh, fragment, and the optional task shader info */
                const VkShaderCreateInfoEXT vk_shader_create_info_array[Context::cTargetMaxPrimitiveShaderStageCount] = {
                    {
                        .sType                  = VK_STRUCTURE_TYPE_SHADER_CREATE_INFO_EXT,
                        .flags                  = VK_SHADER_CREATE_LINK_STAGE_BIT_EXT,
                        .stage                  = static_cast<VkShaderStageFlagBits>(m_vk_shader_stage_flag_array[0]),
                        .nextStage              = next_stage_array[0],
                        .codeType               = (is_binary == false) ? VK_SHADER_CODE_TYPE_SPIRV_EXT : VK_SHADER_CODE_TYPE_BINARY_EXT,
                        .codeSize               = code_size_array[0],
                        .pCode                  = code_array[0],
                        .pName                  = "main",
                        .setLayoutCount         = Context::cTargetDescriptorSetLayoutCount,
                        .pSetLayouts            = Context::GetInstance()->GetVkDescriptorSetLayoutArray(),
                        .pushConstantRangeCount = Context::cTargetAllStagePushConstantRangeCount,
                        .pPushConstantRanges    = Context::GetInstance()->GetVkPushConstantRangeArray(),
                        .pSpecializationInfo    = nullptr
                    },
                    {
                        .sType                  = VK_STRUCTURE_TYPE_SHADER_CREATE_INFO_EXT,
                        .flags                  = VK_SHADER_CREATE_LINK_STAGE_BIT_EXT,
                        .stage                  = static_cast<VkShaderStageFlagBits>(m_vk_shader_stage_flag_array[1]),
                        .nextStage              = next_stage_array[1],
                        .codeType               = (is_binary == false) ? VK_SHADER_CODE_TYPE_SPIRV_EXT : VK_SHADER_CODE_TYPE_BINARY_EXT,
                        .codeSize               = code_size_array[1],
                        .pCode                  = code_array[1],
                        .pName                  = "main",
                        .setLayoutCount         = Context::cTargetDescriptorSetLayoutCount,
                        .pSetLayouts            = Context::GetInstance()->GetVkDescriptorSetLayoutArray(),
                        .pushConstantRangeCount = Context::cTargetAllStagePushConstantRangeCount,
                        .pPushConstantRanges    = Context::GetInstance()->GetVkPushConstantRangeArray(),
                        .pSpecializationInfo    = nullptr
                    },
                    {
                        .sType                  = VK_STRUCTURE_TYPE_SHADER_CREATE_INFO_EXT,
                        .flags                  = VK_SHADER_CREATE_LINK_STAGE_BIT_EXT,
                        .stage                  = static_cast<VkShaderStageFlagBits>(m_vk_shader_stage_flag_array[2]),
                        .nextStage              = next_stage_array[2],
                        .codeType               = (is_binary == false) ? VK_SHADER_CODE_TYPE_SPIRV_EXT : VK_SHADER_CODE_TYPE_BINARY_EXT,
                        .codeSize               = code_size_array[2],
                        .pCode                  = code_array[2],
                        .pName                  = "main",
                        .setLayoutCount         = Context::cTargetDescriptorSetLayoutCount,
                        .pSetLayouts            = Context::GetInstance()->GetVkDescriptorSetLayoutArray(),
                        .pushConstantRangeCount = Context::cTargetAllStagePushConstantRangeCount,
                        .pPushConstantRanges    = Context::GetInstance()->GetVkPushConstantRangeArray(),
                        .pSpecializationInfo    = nullptr
                    },
                    {
                        .sType                  = VK_STRUCTURE_TYPE_SHADER_CREATE_INFO_EXT,
                        .flags                  = VK_SHADER_CREATE_LINK_STAGE_BIT_EXT,
                        .stage                  = static_cast<VkShaderStageFlagBits>(m_vk_shader_stage_flag_array[3]),
                        .nextStage              = next_stage_array[3],
                        .codeType               = (is_binary == false) ? VK_SHADER_CODE_TYPE_SPIRV_EXT : VK_SHADER_CODE_TYPE_BINARY_EXT,
                        .codeSize               = code_size_array[3],
                        .pCode                  = code_array[3],
                        .pName                  = "main",
                        .setLayoutCount         = Context::cTargetDescriptorSetLayoutCount,
                        .pSetLayouts            = Context::GetInstance()->GetVkDescriptorSetLayoutArray(),
                        .pushConstantRangeCount = Context::cTargetAllStagePushConstantRangeCount,
                        .pPushConstantRanges    = Context::GetInstance()->GetVkPushConstantRangeArray(),
                        .pSpecializationInfo    = nullptr
                    },
                    {
                        .sType                  = VK_STRUCTURE_TYPE_SHADER_CREATE_INFO_EXT,
                        .flags                  = VK_SHADER_CREATE_LINK_STAGE_BIT_EXT,
                        .stage                  = static_cast<VkShaderStageFlagBits>(m_vk_shader_stage_flag_array[4]),
                        .nextStage              = next_stage_array[4],
                        .codeType               = (is_binary == false) ? VK_SHADER_CODE_TYPE_SPIRV_EXT : VK_SHADER_CODE_TYPE_BINARY_EXT,
                        .codeSize               = code_size_array[4],
                        .pCode                  = code_array[4],
                        .pName                  = "main",
                        .setLayoutCount         = Context::cTargetDescriptorSetLayoutCount,
                        .pSetLayouts            = Context::GetInstance()->GetVkDescriptorSetLayoutArray(),
                        .pushConstantRangeCount = Context::cTargetAllStagePushConstantRangeCount,
                        .pPushConstantRanges    = Context::GetInstance()->GetVkPushConstantRangeArray(),
                        .pSpecializationInfo    = nullptr
                    },
                };

                /* Create shaders */
                const u32 vk_result = ::pfn_vkCreateShadersEXT(Context::GetInstance()->GetVkDevice(), shader_count, vk_shader_create_info_array, Context::GetInstance()->GetVkAllocationCallbacks(), m_vk_shader_array);
                VP_ASSERT(vk_result == VK_SUCCESS);

                /* Set state */
                m_shader_count      = shader_count;
                m_shader_stage_mask = shader_stage_mask;

                return;
            }

            void Initialize(ComputeShaderInfo *shader_info, const bool is_binary = true) {

                /* Integrity checks */
                VP_ASSERT(shader_info->compute_code != nullptr);

                /* Setup compute shader info */
                const VkShaderCreateInfoEXT vk_shader_create_info_array[Context::cTargetMaxComputeShaderStageCount] = {
                    {
                        .sType                  = VK_STRUCTURE_TYPE_SHADER_CREATE_INFO_EXT,
                        .flags                  = VK_SHADER_CREATE_LINK_STAGE_BIT_EXT,
                        .stage                  = VK_SHADER_STAGE_COMPUTE_BIT,
                        .nextStage              = 0,
                        .codeType               = (is_binary == false) ? VK_SHADER_CODE_TYPE_SPIRV_EXT : VK_SHADER_CODE_TYPE_BINARY_EXT,
                        .codeSize               = shader_info->compute_code_size,
                        .pCode                  = shader_info->compute_code,
                        .pName                  = "main",
                        .setLayoutCount         = Context::cTargetDescriptorSetLayoutCount,
                        .pSetLayouts            = Context::GetInstance()->GetVkDescriptorSetLayoutArray(),
                        .pushConstantRangeCount = Context::cTargetAllStagePushConstantRangeCount,
                        .pPushConstantRanges    = Context::GetInstance()->GetVkPushConstantRangeArray(),
                        .pSpecializationInfo    = nullptr
                    }
                };

                /* Create shaders */
                const u32 vk_result = ::pfn_vkCreateShadersEXT(Context::GetInstance()->GetVkDevice(), Context::cTargetMaxComputeShaderStageCount, vk_shader_create_info_array, Context::GetInstance()->GetVkAllocationCallbacks(), m_vk_shader_array);
                VP_ASSERT(vk_result == VK_SUCCESS);

                /* Set state */
                m_shader_count                  = Context::cTargetMaxComputeShaderStageCount;
                m_shader_stage_mask             = static_cast<u32>(ShaderStage::Compute);
                m_vk_shader_stage_flag_array[0] = VK_SHADER_STAGE_COMPUTE_BIT;

                return;
            }

            void Initialize(vp::res::ResBnshShaderProgram *res_program) {
    
                /* Integrity checks */
                VP_ASSERT(res_program->code_type == static_cast<u8>(vp::res::GfxShaderCodeType::Binary));

                /* Get code info */
                vp::res::ResBnshShaderCodeInfo *vertex_code_info                  = res_program->vertex_info;
                vp::res::ResBnshShaderCodeInfo *tessellation_control_code_info    = res_program->tessellation_control_info;
                vp::res::ResBnshShaderCodeInfo *tessellation_evaluation_code_info = res_program->tessellation_evaluation_info;
                vp::res::ResBnshShaderCodeInfo *geometry_code_info                = res_program->geometry_info;
                vp::res::ResBnshShaderCodeInfo *fragment_code_info                = res_program->fragment_info;
                vp::res::ResBnshShaderCodeInfo *compute_code_info                 = res_program->compute_info;

                /* Calculate shader stages */
                VkShaderStageFlags  next_stage_array[Context::cTargetMaxPrimitiveShaderStageCount] = {};
                u32                 code_size_array[Context::cTargetMaxPrimitiveShaderStageCount]  = {};
                void               *code_array[Context::cTargetMaxPrimitiveShaderStageCount]       = {};

                u32 last_stage        = 0xffff'ffff;
                u32 shader_count      = 0;
                u32 shader_stage_mask = 0;

                if (vertex_code_info != nullptr) {

                    /* Set shader state */
                    m_vk_shader_stage_flag_array[shader_count]  = VK_SHADER_STAGE_VERTEX_BIT;
                    shader_stage_mask                          |= static_cast<u32>(ShaderStage::Vertex);
                    code_array[shader_count]                    = vertex_code_info->shader_code;
                    code_size_array[shader_count]               = vertex_code_info->shader_code_size;

                    /* Advance shader iter */
                    ++shader_count;
                    last_stage = VK_SHADER_STAGE_VERTEX_BIT;
                }
                if (tessellation_control_code_info != nullptr) {

                    /* Set last stage */
                    if (last_stage != 0xffff'ffff) { next_stage_array[shader_count - 1] = last_stage; }

                    /* Set shader state */
                    m_vk_shader_stage_flag_array[shader_count]  = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
                    shader_stage_mask                          |= static_cast<u32>(ShaderStage::TessellationControl);
                    code_array[shader_count]                    = tessellation_control_code_info->shader_code;
                    code_size_array[shader_count]               = tessellation_control_code_info->shader_code_size;

                    /* Advance shader iter */
                    ++shader_count;
                    last_stage = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
                }
                if (tessellation_evaluation_code_info != nullptr) {

                    /* Set last stage */
                    if (last_stage != 0xffff'ffff) { next_stage_array[shader_count - 1] = last_stage; }

                    /* Set shader state */
                    m_vk_shader_stage_flag_array[shader_count]  = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
                    shader_stage_mask                          |= static_cast<u32>(ShaderStage::TessellationEvaluation);
                    code_array[shader_count]                    = tessellation_evaluation_code_info->shader_code;
                    code_size_array[shader_count]               = tessellation_evaluation_code_info->shader_code_size;

                    /* Advance shader iter */
                    ++shader_count;
                    last_stage = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
                }
                if (geometry_code_info != nullptr) {

                    /* Set last stage */
                    if (last_stage != 0xffff'ffff) { next_stage_array[shader_count - 1] = last_stage; }

                    /* Set shader state */
                    m_vk_shader_stage_flag_array[shader_count]  = VK_SHADER_STAGE_GEOMETRY_BIT;
                    shader_stage_mask                          |= static_cast<u32>(ShaderStage::Geometry);
                    code_array[shader_count]                    = geometry_code_info->shader_code;
                    code_size_array[shader_count]               = geometry_code_info->shader_code_size;

                    /* Advance shader iter */
                    ++shader_count;
                    last_stage = VK_SHADER_STAGE_GEOMETRY_BIT;
                }
                if (fragment_code_info != nullptr) {

                    /* Set last stage */
                    if (last_stage != 0xffff'ffff) { next_stage_array[shader_count - 1] = last_stage; }

                    /* Set shader state */
                    m_vk_shader_stage_flag_array[shader_count]  = VK_SHADER_STAGE_FRAGMENT_BIT;
                    shader_stage_mask                          |= static_cast<u32>(ShaderStage::Fragment);
                    code_array[shader_count]                    = fragment_code_info->shader_code;
                    code_size_array[shader_count]               = fragment_code_info->shader_code_size;

                    /* Advance shader iter */
                    ++shader_count;
                    last_stage = VK_SHADER_STAGE_FRAGMENT_BIT;
                }
                if (compute_code_info != nullptr) {

                    /* Set last stage */
                    if (last_stage != 0xffff'ffff) { next_stage_array[shader_count - 1] = last_stage; }

                    /* Set shader state */
                    m_vk_shader_stage_flag_array[shader_count]  = VK_SHADER_STAGE_COMPUTE_BIT;
                    shader_stage_mask                          |= static_cast<u32>(ShaderStage::Compute);
                    code_array[shader_count]                    = compute_code_info->shader_code;
                    code_size_array[shader_count]               = compute_code_info->shader_code_size;

                    /* Advance shader iter */
                    ++shader_count;
                    last_stage = VK_SHADER_STAGE_COMPUTE_BIT;
                }
                VP_ASSERT(shader_count < Context::cTargetMaxPrimitiveShaderStageCount);

                /* Setup mesh, fragment, and the optional task shader info */
                const VkShaderCreateInfoEXT vk_shader_create_info_array[Context::cTargetMaxPrimitiveShaderStageCount] = {
                    {
                        .sType                  = VK_STRUCTURE_TYPE_SHADER_CREATE_INFO_EXT,
                        .flags                  = VK_SHADER_CREATE_LINK_STAGE_BIT_EXT,
                        .stage                  = static_cast<VkShaderStageFlagBits>(m_vk_shader_stage_flag_array[0]),
                        .nextStage              = next_stage_array[0],
                        .codeType               = VK_SHADER_CODE_TYPE_BINARY_EXT,
                        .codeSize               = code_size_array[0],
                        .pCode                  = code_array[0],
                        .pName                  = "main",
                        .setLayoutCount         = Context::cTargetDescriptorSetLayoutCount,
                        .pSetLayouts            = Context::GetInstance()->GetVkDescriptorSetLayoutArray(),
                        .pushConstantRangeCount = Context::cTargetAllStagePushConstantRangeCount,
                        .pPushConstantRanges    = Context::GetInstance()->GetVkPushConstantRangeArray(),
                        .pSpecializationInfo    = nullptr
                    },
                    {
                        .sType                  = VK_STRUCTURE_TYPE_SHADER_CREATE_INFO_EXT,
                        .flags                  = VK_SHADER_CREATE_LINK_STAGE_BIT_EXT,
                        .stage                  = static_cast<VkShaderStageFlagBits>(m_vk_shader_stage_flag_array[1]),
                        .nextStage              = next_stage_array[1],
                        .codeType               = VK_SHADER_CODE_TYPE_BINARY_EXT,
                        .codeSize               = code_size_array[1],
                        .pCode                  = code_array[1],
                        .pName                  = "main",
                        .setLayoutCount         = Context::cTargetDescriptorSetLayoutCount,
                        .pSetLayouts            = Context::GetInstance()->GetVkDescriptorSetLayoutArray(),
                        .pushConstantRangeCount = Context::cTargetAllStagePushConstantRangeCount,
                        .pPushConstantRanges    = Context::GetInstance()->GetVkPushConstantRangeArray(),
                        .pSpecializationInfo    = nullptr
                    },
                    {
                        .sType                  = VK_STRUCTURE_TYPE_SHADER_CREATE_INFO_EXT,
                        .flags                  = VK_SHADER_CREATE_LINK_STAGE_BIT_EXT,
                        .stage                  = static_cast<VkShaderStageFlagBits>(m_vk_shader_stage_flag_array[2]),
                        .nextStage              = next_stage_array[2],
                        .codeType               = VK_SHADER_CODE_TYPE_BINARY_EXT,
                        .codeSize               = code_size_array[2],
                        .pCode                  = code_array[2],
                        .pName                  = "main",
                        .setLayoutCount         = Context::cTargetDescriptorSetLayoutCount,
                        .pSetLayouts            = Context::GetInstance()->GetVkDescriptorSetLayoutArray(),
                        .pushConstantRangeCount = Context::cTargetAllStagePushConstantRangeCount,
                        .pPushConstantRanges    = Context::GetInstance()->GetVkPushConstantRangeArray(),
                        .pSpecializationInfo    = nullptr
                    },
                    {
                        .sType                  = VK_STRUCTURE_TYPE_SHADER_CREATE_INFO_EXT,
                        .flags                  = VK_SHADER_CREATE_LINK_STAGE_BIT_EXT,
                        .stage                  = static_cast<VkShaderStageFlagBits>(m_vk_shader_stage_flag_array[3]),
                        .nextStage              = next_stage_array[3],
                        .codeType               = VK_SHADER_CODE_TYPE_BINARY_EXT,
                        .codeSize               = code_size_array[3],
                        .pCode                  = code_array[3],
                        .pName                  = "main",
                        .setLayoutCount         = Context::cTargetDescriptorSetLayoutCount,
                        .pSetLayouts            = Context::GetInstance()->GetVkDescriptorSetLayoutArray(),
                        .pushConstantRangeCount = Context::cTargetAllStagePushConstantRangeCount,
                        .pPushConstantRanges    = Context::GetInstance()->GetVkPushConstantRangeArray(),
                        .pSpecializationInfo    = nullptr
                    },
                    {
                        .sType                  = VK_STRUCTURE_TYPE_SHADER_CREATE_INFO_EXT,
                        .flags                  = VK_SHADER_CREATE_LINK_STAGE_BIT_EXT,
                        .stage                  = static_cast<VkShaderStageFlagBits>(m_vk_shader_stage_flag_array[4]),
                        .nextStage              = next_stage_array[4],
                        .codeType               = VK_SHADER_CODE_TYPE_BINARY_EXT,
                        .codeSize               = code_size_array[4],
                        .pCode                  = code_array[4],
                        .pName                  = "main",
                        .setLayoutCount         = Context::cTargetDescriptorSetLayoutCount,
                        .pSetLayouts            = Context::GetInstance()->GetVkDescriptorSetLayoutArray(),
                        .pushConstantRangeCount = Context::cTargetAllStagePushConstantRangeCount,
                        .pPushConstantRanges    = Context::GetInstance()->GetVkPushConstantRangeArray(),
                        .pSpecializationInfo    = nullptr
                    },
                };

                /* Create shaders */
                const u32 vk_result = ::pfn_vkCreateShadersEXT(Context::GetInstance()->GetVkDevice(), shader_count, vk_shader_create_info_array, Context::GetInstance()->GetVkAllocationCallbacks(), m_vk_shader_array);
                VP_ASSERT(vk_result == VK_SUCCESS);

                /* Set state */
                m_shader_count      = shader_count;
                m_shader_stage_mask = shader_stage_mask;
                m_res_program       = res_program;

                return;
            }

            void Finalize() {

                /* Delete shaders */
                for (u32 i = 0; i < m_shader_count; ++i) {
                    ::pfn_vkDestroyShaderEXT(Context::GetInstance()->GetVkDevice(), m_vk_shader_array[i], Context::GetInstance()->GetVkAllocationCallbacks());
                }

                /* Reset state */
                m_vk_shader_array[0] = VK_NULL_HANDLE;
                m_vk_shader_array[1] = VK_NULL_HANDLE;
                m_vk_shader_array[2] = VK_NULL_HANDLE;
                m_vk_shader_array[3] = VK_NULL_HANDLE;
                m_vk_shader_array[4] = VK_NULL_HANDLE;
                m_shader_count       = 0;
                m_shader_stage_mask  = 0;
            }

            u32 SetupForCommandList(VkShaderEXT **out_shader_array, VkShaderStageFlagBits **out_shader_stage_array) {
                *out_shader_array       = m_vk_shader_array;
                *out_shader_stage_array = m_vk_shader_stage_flag_array;
                return m_shader_count;
            }

            u32 TryGetInterfaceSlot(u32 shader_stage, const char *key);
    };
}
