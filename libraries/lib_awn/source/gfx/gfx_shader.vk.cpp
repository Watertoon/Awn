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

    void Shader::Initialize(MeshShaderInfo *shader_info, const bool is_binary) {

        /* Integrity checks */
        VP_ASSERT((shader_info->mesh_code != nullptr) & (shader_info->fragment_code != nullptr));

        /* Get descriptor set layouts */
        impl::Context *context = impl::Context::GetInstance();
        const VkDescriptorSetLayout *set_layout_array = context->GetVkDescriptorSetLayoutArrayGraphics();

        /* Setup mesh, fragment, and the optional task shader info */
        VkShaderCreateFlagsEXT  create_flags = static_cast<VkShaderCreateFlagsEXT>(VK_SHADER_CREATE_LINK_STAGE_BIT_EXT | ((shader_info->task_code == nullptr) ? VK_SHADER_CREATE_NO_TASK_SHADER_BIT_EXT : 0));
        const VkShaderCreateInfoEXT vk_shader_create_info_array[impl::Context::cTargetMaxMeshShaderStageCount] = {
            {
                .sType                  = VK_STRUCTURE_TYPE_SHADER_CREATE_INFO_EXT,
                .flags                  = create_flags,
                .stage                  = VK_SHADER_STAGE_MESH_BIT_EXT,
                .nextStage              = VK_SHADER_STAGE_FRAGMENT_BIT,
                .codeType               = (is_binary == false) ? VK_SHADER_CODE_TYPE_SPIRV_EXT : VK_SHADER_CODE_TYPE_BINARY_EXT,
                .codeSize               = shader_info->mesh_code_size,
                .pCode                  = shader_info->mesh_code,
                .pName                  = "main",
                .setLayoutCount         = impl::Context::cTargetDescriptorSetLayoutCountGraphics,
                .pSetLayouts            = set_layout_array,
                //.pushConstantRangeCount = impl::Context::cTargetAllStagePushConstantRangeCount,
                //.pPushConstantRanges    = context->GetVkPushConstantRangeArray(),
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
                .setLayoutCount         = impl::Context::cTargetDescriptorSetLayoutCountGraphics,
                .pSetLayouts            = set_layout_array,
                //.pushConstantRangeCount = impl::Context::cTargetAllStagePushConstantRangeCount,
                //.pPushConstantRanges    = context->GetVkPushConstantRangeArray(),
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
                .setLayoutCount         = impl::Context::cTargetDescriptorSetLayoutCountGraphics,
                .pSetLayouts            = set_layout_array,
                //.pushConstantRangeCount = impl::Context::cTargetAllStagePushConstantRangeCount,
                //.pPushConstantRanges    = context->GetVkPushConstantRangeArray(),
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
        VkShaderStageFlags next_stage_array[impl::Context::cTargetMaxPrimitiveShaderStageCount] = {
            VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT,
            VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT,
            VK_SHADER_STAGE_GEOMETRY_BIT,
            VK_SHADER_STAGE_FRAGMENT_BIT,
        };
        u32 code_size_array[impl::Context::cTargetMaxPrimitiveShaderStageCount] = {
            shader_info->vertex_code_size,
            shader_info->tessellation_control_code_size,
            shader_info->tessellation_evaluation_code_size,
            shader_info->geometry_code_size,
            shader_info->fragment_code_size,
        };
        void *code_array[impl::Context::cTargetMaxPrimitiveShaderStageCount] = {
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
        impl::Context *context = impl::Context::GetInstance();
        const VkDescriptorSetLayout *set_layout_array = context->GetVkDescriptorSetLayoutArrayGraphics();

        /* Setup mesh, fragment, and the optional task shader info */
        const VkShaderCreateInfoEXT vk_shader_create_info_array[impl::Context::cTargetMaxPrimitiveShaderStageCount] = {
            {
                .sType                  = VK_STRUCTURE_TYPE_SHADER_CREATE_INFO_EXT,
                .flags                  = VK_SHADER_CREATE_LINK_STAGE_BIT_EXT,
                .stage                  = static_cast<VkShaderStageFlagBits>(m_vk_shader_stage_flag_array[0]),
                .nextStage              = next_stage_array[0],
                .codeType               = (is_binary == false) ? VK_SHADER_CODE_TYPE_SPIRV_EXT : VK_SHADER_CODE_TYPE_BINARY_EXT,
                .codeSize               = code_size_array[0],
                .pCode                  = code_array[0],
                .pName                  = "main",
                .setLayoutCount         = impl::Context::cTargetDescriptorSetLayoutCountGraphics,
                .pSetLayouts            = set_layout_array,
                //.pushConstantRangeCount = impl::Context::cTargetAllStagePushConstantRangeCount,
                //.pPushConstantRanges    = context->GetVkPushConstantRangeArray(),
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
                .setLayoutCount         = impl::Context::cTargetDescriptorSetLayoutCountGraphics,
                .pSetLayouts            = set_layout_array,
                //.pushConstantRangeCount = impl::Context::cTargetAllStagePushConstantRangeCount,
                //.pPushConstantRanges    = context->GetVkPushConstantRangeArray(),
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
                .setLayoutCount         = impl::Context::cTargetDescriptorSetLayoutCountGraphics,
                .pSetLayouts            = set_layout_array,
                //.pushConstantRangeCount = impl::Context::cTargetAllStagePushConstantRangeCount,
                //.pPushConstantRanges    = context->GetVkPushConstantRangeArray(),
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
                .setLayoutCount         = impl::Context::cTargetDescriptorSetLayoutCountGraphics,
                .pSetLayouts            = set_layout_array,
                //.pushConstantRangeCount = impl::Context::cTargetAllStagePushConstantRangeCount,
                //.pPushConstantRanges    = context->GetVkPushConstantRangeArray(),
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
                .setLayoutCount         = impl::Context::cTargetDescriptorSetLayoutCountGraphics,
                .pSetLayouts            = set_layout_array,
                //.pushConstantRangeCount = impl::Context::cTargetAllStagePushConstantRangeCount,
                //.pPushConstantRanges    = context->GetVkPushConstantRangeArray(),
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
        impl::Context *context = impl::Context::GetInstance();
        const VkDescriptorSetLayout *set_layout_array = context->GetVkDescriptorSetLayoutArrayCompute();

        /* Setup compute shader info */
        const VkShaderCreateInfoEXT vk_shader_create_info_array[impl::Context::cTargetMaxComputeShaderStageCount] = {
            {
                .sType                  = VK_STRUCTURE_TYPE_SHADER_CREATE_INFO_EXT,
                .flags                  = VK_SHADER_CREATE_LINK_STAGE_BIT_EXT,
                .stage                  = VK_SHADER_STAGE_COMPUTE_BIT,
                .nextStage              = 0,
                .codeType               = (is_binary == false) ? VK_SHADER_CODE_TYPE_SPIRV_EXT : VK_SHADER_CODE_TYPE_BINARY_EXT,
                .codeSize               = shader_info->compute_code_size,
                .pCode                  = shader_info->compute_code,
                .pName                  = "main",
                .setLayoutCount         = impl::Context::cTargetDescriptorSetLayoutCountCompute,
                .pSetLayouts            = set_layout_array,
                //.pushConstantRangeCount = impl::Context::cTargetAllStagePushConstantRangeCount,
                //.pPushConstantRanges    = impl::Context::GetInstance()->GetVkPushConstantRangeArray(),
                .pSpecializationInfo    = nullptr
            },
        };

        /* Create shaders */
        const u32 vk_result = ::pfn_vkCreateShadersEXT(impl::Context::GetInstance()->GetVkDevice(), impl::Context::cTargetMaxComputeShaderStageCount, vk_shader_create_info_array, impl::Context::GetInstance()->GetVkAllocationCallbacks(), m_vk_shader_array);
        VP_ASSERT(vk_result == VK_SUCCESS);

        /* Set state */
        m_shader_count                  = impl::Context::cTargetMaxComputeShaderStageCount;
        m_shader_stage_mask             = static_cast<u32>(ShaderStage::Compute);
        m_vk_shader_stage_flag_array[0] = VK_SHADER_STAGE_COMPUTE_BIT;

        return;
    }

    bool Shader::Initialize(vp::res::ResBnshShaderProgram *res_program) {

        /* Ensure the program is either binary or spirv */
        if (res_program->code_type != static_cast<u8>(vp::res::GfxShaderCodeType::Binary) && res_program->code_type != static_cast<u8>(vp::res::GfxShaderCodeType::Intermediate)) { return false; }
        const VkShaderCodeTypeEXT code_type = (res_program->code_type == static_cast<u8>(vp::res::GfxShaderCodeType::Binary)) ? VK_SHADER_CODE_TYPE_BINARY_EXT : VK_SHADER_CODE_TYPE_SPIRV_EXT;

        /* Get code info */
        vp::res::ResBnshShaderCodeInfo *vertex_code_info                  = res_program->code_info_vertex;
        vp::res::ResBnshShaderCodeInfo *tessellation_control_code_info    = res_program->code_info_tessellation_control;
        vp::res::ResBnshShaderCodeInfo *tessellation_evaluation_code_info = res_program->code_info_tessellation_evaluation;
        vp::res::ResBnshShaderCodeInfo *geometry_code_info                = res_program->code_info_geometry;
        vp::res::ResBnshShaderCodeInfo *fragment_code_info                = res_program->code_info_fragment;
        vp::res::ResBnshShaderCodeInfo *compute_code_info                 = res_program->code_info_compute;
        vp::res::ResBnshShaderCodeInfo *task_code_info                    = res_program->code_info_task;
        vp::res::ResBnshShaderCodeInfo *mesh_code_info                    = res_program->code_info_mesh;

        /* Calculate shader stages */
        VkShaderStageFlags next_stage_array[impl::Context::cTargetMaxPrimitiveShaderStageCount] = {};
        u32 shader_map[impl::Context::cTargetMaxPrimitiveShaderStageCount] = {};
        u32 code_size_array[impl::Context::cTargetMaxPrimitiveShaderStageCount] = {};
        void *code_array[impl::Context::cTargetMaxPrimitiveShaderStageCount] = {};
        vp::res::ResBnshShaderCodeInfo *code_info_array[impl::Context::cTargetMaxPrimitiveShaderStageCount] = {};
        VkShaderEXT vk_shader_array[impl::Context::cTargetMaxPrimitiveShaderStageCount] = {};

        u32 last_stage        = 0xffff'ffff;
        u32 shader_count      = 0;
        u32 compile_count     = 0;
        u32 shader_stage_mask = 0;

        /* Primitive pipeline */
        const auto set_code_info_lambda = [&](vp::res::ResBnshShaderCodeInfo *code_info, gfx::ShaderStage shader_stage, VkShaderStageFlagBits vk_stage_flags) {
            if (code_info == nullptr) { return; }

            /* Set last stage */
            if (last_stage != 0xffff'ffff) { next_stage_array[shader_count - 1] = last_stage; }
            m_vk_shader_stage_flag_array[shader_count] = vk_stage_flags;
            shader_stage_mask |= (1u << static_cast<u32>(shader_stage));
            last_stage = vk_stage_flags;

            if (vp::util::InterlockedIncrement(std::addressof(code_info->runtime_ref_count)) == 1) {
                /* Set compile state */
                code_info_array[compile_count] = code_info;
                code_array[compile_count] = code_info->code;
                code_size_array[compile_count] = code_info->code_size;
                shader_map[compile_count] = shader_count;
                ++compile_count;
            } else {
                m_vk_shader_array[shader_count] = reinterpret_cast<VkShaderEXT>(code_info->runtime_user_shader_code_buffer_address);
            }
            ++shader_count;
        };
        set_code_info_lambda(vertex_code_info, ShaderStage::Vertex, VK_SHADER_STAGE_VERTEX_BIT);
        set_code_info_lambda(tessellation_control_code_info, ShaderStage::TessellationControl, VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT);
        set_code_info_lambda(tessellation_evaluation_code_info, ShaderStage::TessellationEvaluation, VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT);
        set_code_info_lambda(geometry_code_info, ShaderStage::Geometry, VK_SHADER_STAGE_GEOMETRY_BIT);
        set_code_info_lambda(fragment_code_info, ShaderStage::Fragment, VK_SHADER_STAGE_FRAGMENT_BIT);
        set_code_info_lambda(compute_code_info, ShaderStage::Compute, VK_SHADER_STAGE_COMPUTE_BIT);
        set_code_info_lambda(task_code_info, ShaderStage::Task, VK_SHADER_STAGE_TASK_BIT_EXT);
        set_code_info_lambda(mesh_code_info, ShaderStage::Mesh, VK_SHADER_STAGE_MESH_BIT_EXT);

        /* Get set layout set */
        impl::Context *context = impl::Context::GetInstance();
        const u32 set_count = (last_stage != VK_SHADER_STAGE_COMPUTE_BIT) ? impl::Context::cTargetDescriptorSetLayoutCountGraphics : impl::Context::cTargetDescriptorSetLayoutCountCompute;
        const VkDescriptorSetLayout *set_layout_array = (last_stage != VK_SHADER_STAGE_COMPUTE_BIT) ? context->GetVkDescriptorSetLayoutArrayGraphics() : context->GetVkDescriptorSetLayoutArrayCompute();

        /* Setup mesh, fragment, and the optional task shader info */
        VkShaderCreateInfoEXT vk_shader_create_info_array[impl::Context::cTargetMaxPrimitiveShaderStageCount];
        for (u32 i = 0; i < compile_count; ++i) {
            vk_shader_create_info_array[i].sType                  = VK_STRUCTURE_TYPE_SHADER_CREATE_INFO_EXT;
            vk_shader_create_info_array[i].pNext                  = nullptr;
            vk_shader_create_info_array[i].flags                  = VK_SHADER_CREATE_LINK_STAGE_BIT_EXT;
            vk_shader_create_info_array[i].stage                  = static_cast<VkShaderStageFlagBits>(m_vk_shader_stage_flag_array[i]);
            vk_shader_create_info_array[i].nextStage              = next_stage_array[i];
            vk_shader_create_info_array[i].codeType               = code_type;
            vk_shader_create_info_array[i].codeSize               = code_size_array[i];
            vk_shader_create_info_array[i].pCode                  = code_array[i];
            vk_shader_create_info_array[i].pName                  = "main";
            vk_shader_create_info_array[i].setLayoutCount         = set_count;
            vk_shader_create_info_array[i].pSetLayouts            = set_layout_array;
            vk_shader_create_info_array[i].pushConstantRangeCount = 0;
            vk_shader_create_info_array[i].pPushConstantRanges    = nullptr;
            vk_shader_create_info_array[i].pSpecializationInfo    = nullptr;
        }

        /* Create shaders */
        const u32 vk_result = ::pfn_vkCreateShadersEXT(impl::Context::GetInstance()->GetVkDevice(), shader_count, vk_shader_create_info_array, impl::Context::GetInstance()->GetVkAllocationCallbacks(), vk_shader_array);
        if (vk_result != VK_SUCCESS) { return false; }

        /* Write back shader handles */
        for (u32 i = 0; i < compile_count; ++i) {
            if (code_info_array[i]->runtime_user_shader_code_buffer_address != nullptr) { continue; }
            code_info_array[i]->runtime_user_shader_code_buffer_address = vk_shader_array[i];
            m_vk_shader_array[shader_map[i]] = vk_shader_array[i];
        }

        /* Set state */
        m_shader_count      = shader_count;
        m_shader_stage_mask = shader_stage_mask;
        m_res_program       = res_program;

        return true;
    }

    void Shader::Finalize() {

        /* Delete shaders */
        if (m_res_program != nullptr) {
            const auto finalize_code_info_lambda = [](vp::res::ResBnshShaderCodeInfo *code_info) {                
                if (code_info == nullptr) { return; }
                const u32 ref_count = vp::util::InterlockedDecrement(std::addressof(code_info->runtime_ref_count));
                if (0 < ref_count) { return; }
                VkShaderEXT vk_shader = reinterpret_cast<VkShaderEXT>(code_info->runtime_user_shader_code_buffer_address);
                ::pfn_vkDestroyShaderEXT(impl::Context::GetInstance()->GetVkDevice(), vk_shader, impl::Context::GetInstance()->GetVkAllocationCallbacks());
                code_info->runtime_user_shader_code_buffer_address = nullptr;
            };
            finalize_code_info_lambda(m_res_program->code_info_vertex);
            finalize_code_info_lambda(m_res_program->code_info_tessellation_control);
            finalize_code_info_lambda(m_res_program->code_info_tessellation_evaluation);
            finalize_code_info_lambda(m_res_program->code_info_geometry);
            finalize_code_info_lambda(m_res_program->code_info_fragment);
            finalize_code_info_lambda(m_res_program->code_info_compute);
            finalize_code_info_lambda(m_res_program->code_info_task);
            finalize_code_info_lambda(m_res_program->code_info_mesh);
            m_res_program = nullptr;
        } else {
            for (u32 i = 0; i < m_shader_count; ++i) {
                ::pfn_vkDestroyShaderEXT(impl::Context::GetInstance()->GetVkDevice(), m_vk_shader_array[i], impl::Context::GetInstance()->GetVkAllocationCallbacks());
            }
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

    u32 Shader::GetInterfaceSlot(ShaderStage shader_stage, ShaderInterfaceType interface_type, const char *key) {

        /* Fail if no res program */
        if (m_res_program == nullptr) { return cInvalidInterfaceSlot; }

        /* Select interface if our shader stage is not out of bounds */
        const u32 stage_index = static_cast<u32>(shader_stage);
        if (7 < stage_index) { return cInvalidInterfaceSlot; }
        vp::res::ResBnshShaderInterfaceInfoTable *interface_info_table = m_res_program->shader_interface_info_table;
        if (interface_info_table == nullptr) { return cInvalidInterfaceSlot; }
        vp::res::ResBnshShaderInterfaceInfo *interface_info = std::addressof(interface_info_table->interface_info_vertex)[stage_index];
        if (interface_info == nullptr) { return cInvalidInterfaceSlot; }

        /* Select base index and dictionary for interface type */
        vp::res::ResNintendoWareDictionary *dictionary           = nullptr;
        u32                                 base_index           = 0;
        const u32                           interface_type_index = static_cast<u32>(interface_type);
        if (interface_type_index < static_cast<u32>(ShaderInterfaceType::Image)) {
            dictionary = std::addressof(interface_info->input_attribute_dictionary)[interface_type_index];
            base_index = (interface_type_index == 0) ? 0 : std::addressof(interface_info->base_index_output_attribute)[interface_type_index - 1];
        } else if (interface_type == ShaderInterfaceType::Image) {
            dictionary = interface_info->image_dictionary;
            base_index = interface_info->base_index_image;
        } else if (interface_type_index <= static_cast<u32>(ShaderInterfaceType::SeparateSampler)) {
            if (interface_info->extended_interface_info == nullptr) { return cInvalidInterfaceSlot; }
            const u32 adj_index = interface_type_index - static_cast<u32>(ShaderInterfaceType::Image);
            dictionary = std::addressof(interface_info->extended_interface_info->separate_texture_dictionary)[adj_index];
            base_index = std::addressof(interface_info->extended_interface_info->base_index_separate_texture)[adj_index];
        }
        if (dictionary == nullptr) { return false; }

        /* Lookup local slot index */
        const u32 index = dictionary->TryGetEntryIndexByKey(key);
        if (index == vp::res::ResNintendoWareDictionary::cInvalidEntryIndex) { return false; }

        /* Get slot from global slot array */
        const u32 slot_index = base_index + index;
        return interface_info->interface_slot_array[slot_index];
    }
}
