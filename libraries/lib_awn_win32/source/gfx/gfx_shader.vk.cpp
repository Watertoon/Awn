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
#include <awn.hpp>

namespace awn::gfx {

    namespace {

        void BuildDescriptorLayoutSetGraphics(Context *context, VkDescriptorSetLayout *set_layout_array) {
            set_layout_array[Context::cTargetTextureLayoutSet] = context->GetTextureVkDescriptorSetLayout();
            set_layout_array[Context::cTargetSamplerLayoutSet] = context->GetSamplerVkDescriptorSetLayout();

            set_layout_array[Context::cTargetPerStageLayoutSetTextureSamplerVertex]                         = context->GetPerStageVkDescriptorSetLayoutTextureSamplerVertex();
            set_layout_array[Context::cTargetPerStageLayoutSetTextureSamplerGeometry]                       = context->GetPerStageVkDescriptorSetLayoutTextureSamplerGeometry();
            set_layout_array[Context::cTargetPerStageLayoutSetTextureSamplerFragment]                       = context->GetPerStageVkDescriptorSetLayoutTextureSamplerFragment();
            set_layout_array[Context::cTargetPerStageLayoutSetTextureSamplerTask]                           = context->GetPerStageVkDescriptorSetLayoutTextureSamplerTask();
            set_layout_array[Context::cTargetPerStageLayoutSetTextureSamplerMesh]                           = context->GetPerStageVkDescriptorSetLayoutTextureSamplerMesh();
            set_layout_array[Context::cTargetPerStageLayoutSetImageVertex]                                  = context->GetPerStageVkDescriptorSetLayoutImageVertex();
            set_layout_array[Context::cTargetPerStageLayoutSetImageGeometry]                                = context->GetPerStageVkDescriptorSetLayoutImageGeometry();
            set_layout_array[Context::cTargetPerStageLayoutSetImageFragment]                                = context->GetPerStageVkDescriptorSetLayoutImageFragment();
            set_layout_array[Context::cTargetPerStageLayoutSetImageTask]                                    = context->GetPerStageVkDescriptorSetLayoutImageTask();
            set_layout_array[Context::cTargetPerStageLayoutSetImageMesh]                                    = context->GetPerStageVkDescriptorSetLayoutImageMesh();
            set_layout_array[Context::cTargetPerStageLayoutSetUniformBufferVertex]                          = context->GetPerStageVkDescriptorSetLayoutUniformBufferVertex();
            set_layout_array[Context::cTargetPerStageLayoutSetUniformBufferGeometry]                        = context->GetPerStageVkDescriptorSetLayoutUniformBufferGeometry();
            set_layout_array[Context::cTargetPerStageLayoutSetUniformBufferFragment]                        = context->GetPerStageVkDescriptorSetLayoutUniformBufferFragment();
            set_layout_array[Context::cTargetPerStageLayoutSetUniformBufferTask]                            = context->GetPerStageVkDescriptorSetLayoutUniformBufferTask();
            set_layout_array[Context::cTargetPerStageLayoutSetUniformBufferMesh]                            = context->GetPerStageVkDescriptorSetLayoutUniformBufferMesh();
            set_layout_array[Context::cTargetPerStageLayoutSetStorageBufferVertex]                          = context->GetPerStageVkDescriptorSetLayoutStorageBufferVertex();
            set_layout_array[Context::cTargetPerStageLayoutSetStorageBufferGeometry]                        = context->GetPerStageVkDescriptorSetLayoutStorageBufferGeometry();
            set_layout_array[Context::cTargetPerStageLayoutSetStorageBufferFragment]                        = context->GetPerStageVkDescriptorSetLayoutStorageBufferFragment();
            set_layout_array[Context::cTargetPerStageLayoutSetStorageBufferTask]                            = context->GetPerStageVkDescriptorSetLayoutStorageBufferTask();
            set_layout_array[Context::cTargetPerStageLayoutSetStorageBufferMesh]                            = context->GetPerStageVkDescriptorSetLayoutStorageBufferMesh();
            set_layout_array[Context::cTargetPerStageLayoutSetCombinedTessellationEvaluation]               = context->GetPerStageVkDescriptorSetLayoutCombinedTessellationEvaluation();
            set_layout_array[Context::cTargetPerStageLayoutSetCombinedTessellationControl]                  = context->GetPerStageVkDescriptorSetLayoutCombinedTessellationControl();
            set_layout_array[Context::cTargetPerStageLayoutSetSeparateTextureSamplerVertex]                 = context->GetPerStageVkDescriptorSetLayoutSeparateTextureSamplerVertex();
            set_layout_array[Context::cTargetPerStageLayoutSetSeparateTextureSamplerTessellationEvaluation] = context->GetPerStageVkDescriptorSetLayoutSeparateTextureSamplerTessellationEvaluation();
            set_layout_array[Context::cTargetPerStageLayoutSetSeparateTextureSamplerTessellationControl]    = context->GetPerStageVkDescriptorSetLayoutSeparateTextureSamplerTessellationControl();
            set_layout_array[Context::cTargetPerStageLayoutSetSeparateTextureSamplerGeometry]               = context->GetPerStageVkDescriptorSetLayoutSeparateTextureSamplerGeometry();
            set_layout_array[Context::cTargetPerStageLayoutSetSeparateTextureSamplerFragment]               = context->GetPerStageVkDescriptorSetLayoutSeparateTextureSamplerFragment();
            set_layout_array[Context::cTargetPerStageLayoutSetSeparateTextureSamplerTask]                   = context->GetPerStageVkDescriptorSetLayoutSeparateTextureSamplerTask();
            set_layout_array[Context::cTargetPerStageLayoutSetSeparateTextureSamplerMesh]                   = context->GetPerStageVkDescriptorSetLayoutSeparateTextureSamplerMesh();

            return;
        }
        void BuildDescriptorLayoutSetCompute(Context *context, VkDescriptorSetLayout *set_layout_array) {
            set_layout_array[Context::cTargetTextureLayoutSet] = context->GetTextureVkDescriptorSetLayout();
            set_layout_array[Context::cTargetSamplerLayoutSet] = context->GetSamplerVkDescriptorSetLayout();

            set_layout_array[Context::cTargetPerStageLayoutSetTextureSamplerCompute]         = context->GetPerStageVkDescriptorSetLayoutTextureSamplerCompute();
            set_layout_array[Context::cTargetPerStageLayoutSetImageCompute]                  = context->GetPerStageVkDescriptorSetLayoutImageCompute();
            set_layout_array[Context::cTargetPerStageLayoutSetUniformBufferCompute]          = context->GetPerStageVkDescriptorSetLayoutUniformBufferCompute();
            set_layout_array[Context::cTargetPerStageLayoutSetStorageBufferCompute]          = context->GetPerStageVkDescriptorSetLayoutStorageBufferCompute();
            set_layout_array[Context::cTargetPerStageLayoutSetSeparateTextureSamplerCompute] = context->GetPerStageVkDescriptorSetLayoutSeparateTextureSamplerCompute();

            return;
        }
    }

    void Shader::Initialize(MeshShaderInfo *shader_info, const bool is_binary) {

        /* Integrity checks */
        VP_ASSERT((shader_info->mesh_code != nullptr) & (shader_info->fragment_code != nullptr));

        /* Get descriptor set layouts */
        Context               *context = Context::GetInstance();
        VkDescriptorSetLayout  set_layout_array[Context::cTargetDescriptorSetLayoutCountGraphics];
        BuildDescriptorLayoutSetGraphics(context, set_layout_array);

        /* Setup mesh, fragment, and the optional task shader info */
        VkShaderCreateFlagsEXT  create_flags = static_cast<VkShaderCreateFlagsEXT>(VK_SHADER_CREATE_LINK_STAGE_BIT_EXT | ((shader_info->task_code == nullptr) ? VK_SHADER_CREATE_NO_TASK_SHADER_BIT_EXT : 0));
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
                .setLayoutCount         = Context::cTargetDescriptorSetLayoutCountGraphics,
                .pSetLayouts            = set_layout_array,
                .pushConstantRangeCount = Context::cTargetAllStagePushConstantRangeCount,
                .pPushConstantRanges    = context->GetVkPushConstantRangeArray(),
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
                .setLayoutCount         = Context::cTargetDescriptorSetLayoutCountGraphics,
                .pSetLayouts            = set_layout_array,
                .pushConstantRangeCount = Context::cTargetAllStagePushConstantRangeCount,
                .pPushConstantRanges    = context->GetVkPushConstantRangeArray(),
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
                .setLayoutCount         = Context::cTargetDescriptorSetLayoutCountGraphics,
                .pSetLayouts            = set_layout_array,
                .pushConstantRangeCount = Context::cTargetAllStagePushConstantRangeCount,
                .pPushConstantRanges    = context->GetVkPushConstantRangeArray(),
                .pSpecializationInfo    = nullptr
            },
        };

        /* Create shaders */
        const u32 shader_count = (shader_info->task_code == nullptr) ? 2 : 3;
        const u32 vk_result    = ::pfn_vkCreateShadersEXT(context->GetVkDevice(), shader_count, vk_shader_create_info_array, context->GetVkAllocationCallbacks(), m_vk_shader_array);
        VP_ASSERT(vk_result == VK_SUCCESS);

        /* Set state */
        m_shader_count                  = shader_count;
        m_shader_stage_mask             = static_cast<u32>(ShaderStage::Mesh) | static_cast<u32>(ShaderStage::Fragment) | ((shader_info->task_code) ? static_cast<u32>(ShaderStage::Task) : 0);
        m_vk_shader_stage_flag_array[0] = VK_SHADER_STAGE_MESH_BIT_EXT;
        m_vk_shader_stage_flag_array[1] = VK_SHADER_STAGE_FRAGMENT_BIT;
        m_vk_shader_stage_flag_array[2] = VK_SHADER_STAGE_TASK_BIT_EXT;

        return;
    }

    void Shader::Initialize(PrimitiveShaderInfo *shader_info, const bool is_binary) {

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

        /* Get descriptor set layouts */
        Context               *context = Context::GetInstance();
        VkDescriptorSetLayout  set_layout_array[Context::cTargetDescriptorSetLayoutCountGraphics];
        BuildDescriptorLayoutSetGraphics(context, set_layout_array);

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
                .setLayoutCount         = Context::cTargetDescriptorSetLayoutCountGraphics,
                .pSetLayouts            = set_layout_array,
                .pushConstantRangeCount = Context::cTargetAllStagePushConstantRangeCount,
                .pPushConstantRanges    = context->GetVkPushConstantRangeArray(),
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
                .setLayoutCount         = Context::cTargetDescriptorSetLayoutCountGraphics,
                .pSetLayouts            = set_layout_array,
                .pushConstantRangeCount = Context::cTargetAllStagePushConstantRangeCount,
                .pPushConstantRanges    = context->GetVkPushConstantRangeArray(),
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
                .setLayoutCount         = Context::cTargetDescriptorSetLayoutCountGraphics,
                .pSetLayouts            = set_layout_array,
                .pushConstantRangeCount = Context::cTargetAllStagePushConstantRangeCount,
                .pPushConstantRanges    = context->GetVkPushConstantRangeArray(),
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
                .setLayoutCount         = Context::cTargetDescriptorSetLayoutCountGraphics,
                .pSetLayouts            = set_layout_array,
                .pushConstantRangeCount = Context::cTargetAllStagePushConstantRangeCount,
                .pPushConstantRanges    = context->GetVkPushConstantRangeArray(),
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
                .setLayoutCount         = Context::cTargetDescriptorSetLayoutCountGraphics,
                .pSetLayouts            = set_layout_array,
                .pushConstantRangeCount = Context::cTargetAllStagePushConstantRangeCount,
                .pPushConstantRanges    = context->GetVkPushConstantRangeArray(),
                .pSpecializationInfo    = nullptr
            },
        };

        /* Create shaders */
        const u32 vk_result = ::pfn_vkCreateShadersEXT(context->GetVkDevice(), shader_count, vk_shader_create_info_array, context->GetVkAllocationCallbacks(), m_vk_shader_array);
        VP_ASSERT(vk_result == VK_SUCCESS);

        /* Set state */
        m_shader_count      = shader_count;
        m_shader_stage_mask = shader_stage_mask;

        return;
    }

    void Shader::Initialize(ComputeShaderInfo *shader_info, const bool is_binary) {

        /* Integrity checks */
        VP_ASSERT(shader_info->compute_code != nullptr);

        /* Get descriptor set layouts */
        Context               *context = Context::GetInstance();
        VkDescriptorSetLayout  set_layout_array[Context::cTargetDescriptorSetLayoutCountCompute];
        BuildDescriptorLayoutSetCompute(context, set_layout_array);

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
                .setLayoutCount         = Context::cTargetDescriptorSetLayoutCountCompute,
                .pSetLayouts            = set_layout_array,
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

    void Shader::Initialize(vp::res::ResBnshShaderProgram *res_program) {

        /* Integrity checks */
        VP_ASSERT(res_program->code_type == static_cast<u8>(vp::res::GfxShaderCodeType::Binary));

        /* Get code info */
        vp::res::ResBnshShaderCodeInfo *vertex_code_info                  = res_program->vertex_info;
        vp::res::ResBnshShaderCodeInfo *tessellation_control_code_info    = res_program->tessellation_control_info;
        vp::res::ResBnshShaderCodeInfo *tessellation_evaluation_code_info = res_program->tessellation_evaluation_info;
        vp::res::ResBnshShaderCodeInfo *geometry_code_info                = res_program->geometry_info;
        vp::res::ResBnshShaderCodeInfo *fragment_code_info                = res_program->fragment_info;
        vp::res::ResBnshShaderCodeInfo *compute_code_info                 = res_program->compute_info;

        /* Get graphics descriptor set layouts */
        Context               *context = Context::GetInstance();
        VkDescriptorSetLayout  set_layout_array_graphics[Context::cTargetDescriptorSetLayoutCountGraphics];
        VkDescriptorSetLayout  set_layout_array_compute[Context::cTargetDescriptorSetLayoutCountCompute];
        BuildDescriptorLayoutSetGraphics(context, set_layout_array_graphics);

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
            code_array[shader_count]                    = vertex_code_info->code;
            code_size_array[shader_count]               = vertex_code_info->code_size;

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
            code_array[shader_count]                    = tessellation_control_code_info->code;
            code_size_array[shader_count]               = tessellation_control_code_info->code_size;

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
            code_array[shader_count]                    = tessellation_evaluation_code_info->code;
            code_size_array[shader_count]               = tessellation_evaluation_code_info->code_size;

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
            code_array[shader_count]                    = geometry_code_info->code;
            code_size_array[shader_count]               = geometry_code_info->code_size;

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
            code_array[shader_count]                    = fragment_code_info->code;
            code_size_array[shader_count]               = fragment_code_info->code_size;

            /* Advance shader iter */
            ++shader_count;
            last_stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        }
        if (compute_code_info != nullptr) {

            /* Build compute set layout */
            BuildDescriptorLayoutSetCompute(context, set_layout_array_compute);

            /* Set last stage */
            if (last_stage != 0xffff'ffff) { next_stage_array[shader_count - 1] = last_stage; }

            /* Set shader state */
            m_vk_shader_stage_flag_array[shader_count]  = VK_SHADER_STAGE_COMPUTE_BIT;
            shader_stage_mask                          |= static_cast<u32>(ShaderStage::Compute);
            code_array[shader_count]                    = compute_code_info->code;
            code_size_array[shader_count]               = compute_code_info->code_size;

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
                .setLayoutCount         = (m_vk_shader_stage_flag_array[0] != VK_SHADER_STAGE_COMPUTE_BIT) ? Context::cTargetDescriptorSetLayoutCountGraphics : Context::cTargetDescriptorSetLayoutCountCompute,
                .pSetLayouts            = (m_vk_shader_stage_flag_array[0] != VK_SHADER_STAGE_COMPUTE_BIT) ? set_layout_array_graphics : set_layout_array_compute,
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
                .setLayoutCount         = (m_vk_shader_stage_flag_array[1] != VK_SHADER_STAGE_COMPUTE_BIT) ? Context::cTargetDescriptorSetLayoutCountGraphics : Context::cTargetDescriptorSetLayoutCountCompute,
                .pSetLayouts            = (m_vk_shader_stage_flag_array[1] != VK_SHADER_STAGE_COMPUTE_BIT) ? set_layout_array_graphics : set_layout_array_compute,
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
                .setLayoutCount         = (m_vk_shader_stage_flag_array[2] != VK_SHADER_STAGE_COMPUTE_BIT) ? Context::cTargetDescriptorSetLayoutCountGraphics : Context::cTargetDescriptorSetLayoutCountCompute,
                .pSetLayouts            = (m_vk_shader_stage_flag_array[2] != VK_SHADER_STAGE_COMPUTE_BIT) ? set_layout_array_graphics : set_layout_array_compute,
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
                .setLayoutCount         = (m_vk_shader_stage_flag_array[3] != VK_SHADER_STAGE_COMPUTE_BIT) ? Context::cTargetDescriptorSetLayoutCountGraphics : Context::cTargetDescriptorSetLayoutCountCompute,
                .pSetLayouts            = (m_vk_shader_stage_flag_array[3] != VK_SHADER_STAGE_COMPUTE_BIT) ? set_layout_array_graphics : set_layout_array_compute,
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
                .setLayoutCount         = (m_vk_shader_stage_flag_array[4] != VK_SHADER_STAGE_COMPUTE_BIT) ? Context::cTargetDescriptorSetLayoutCountGraphics : Context::cTargetDescriptorSetLayoutCountCompute,
                .pSetLayouts            = (m_vk_shader_stage_flag_array[4] != VK_SHADER_STAGE_COMPUTE_BIT) ? set_layout_array_graphics : set_layout_array_compute,
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

    void Shader::Finalize() {

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

    u32 Shader::SetupForCommandList(VkShaderEXT **out_shader_array, VkShaderStageFlagBits **out_shader_stage_array) {
        *out_shader_array       = m_vk_shader_array;
        *out_shader_stage_array = m_vk_shader_stage_flag_array;
        return m_shader_count;
    }

    u32 TryGetInterfaceSlot(u32 shader_stage, const char *key);
}
