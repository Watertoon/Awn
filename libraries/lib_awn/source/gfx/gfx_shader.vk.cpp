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

    Result Shader::InitializeImpl(void **code_array, u32 *code_size_array, const ShaderStage *stage_array, u32 stage_count, ShaderCodeType code_type, VkShaderCreateFlagsEXT create_flags) {

        /* Build create info from provided stages */
        impl::Context *context = impl::Context::GetInstance();
        const VkShaderCodeTypeEXT vk_code_type = vp::res::GfxShaderCodeTypeToVkShaderCodeType(code_type);
        VkShaderCreateInfoEXT vk_shader_create_info_array[impl::Context::cTargetMaxPrimitiveShaderStageCount];
        u32 info_i = 0;
        for (u32 i = 0; i < stage_count; ++i) {
            if (code_array[i] == nullptr) { continue; }

            const ShaderStage stage = stage_array[i];
            const VkShaderStageFlags vk_stage_flags = vp::res::GfxShaderStageToVkShaderStageFlags(stage);
            u32 desc_count = 0;
            const VkDescriptorSetLayout *desc_layout = context->GetPerStageDescriptorLayoutArray(std::addressof(desc_count), stage);
            vk_shader_create_info_array[info_i].sType                  = VK_STRUCTURE_TYPE_SHADER_CREATE_INFO_EXT;
            vk_shader_create_info_array[info_i].flags                  = create_flags;
            vk_shader_create_info_array[info_i].stage                  = static_cast<VkShaderStageFlagBits>(vk_stage_flags);
            vk_shader_create_info_array[info_i].codeType               = vk_code_type;
            vk_shader_create_info_array[info_i].codeSize               = code_size_array[i];
            vk_shader_create_info_array[info_i].pCode                  = code_array[i];
            vk_shader_create_info_array[info_i].pName                  = "main";
            vk_shader_create_info_array[info_i].setLayoutCount         = desc_count;
            vk_shader_create_info_array[info_i].pSetLayouts            = desc_layout;
            vk_shader_create_info_array[info_i].pushConstantRangeCount = 0;
            vk_shader_create_info_array[info_i].pPushConstantRanges    = nullptr;
            vk_shader_create_info_array[info_i].pSpecializationInfo    = nullptr;

            /* Set last info's next stage */
            if (0 < info_i) {
                vk_shader_create_info_array[info_i - 1].nextStage = vk_stage_flags;
            }
            ++info_i;
        }
        /* Set final info's next stage to none */
        vk_shader_create_info_array[info_i].nextStage = 0;

        /* Create shaders */
        VkShaderEXT vk_shader_array[cMaxShaderStageCount];
        const u32 vk_result = ::pfn_vkCreateShadersEXT(context->GetVkDevice(), info_i, vk_shader_create_info_array, context->GetVkAllocationCallbacks(), vk_shader_array);
        RESULT_RETURN_UNLESS(vk_result == VK_SUCCESS, ResultFailedToCreateShaders);

        /* Set shaders */
        for (u32 i = 0; i < stage_count; ++i) {
            const ShaderStage stage = stage_array[i];
            m_vk_shader_array[static_cast<u32>(stage)] = vk_shader_array[i];
        }

        RESULT_RETURN_SUCCESS;
    }

    Result Shader::Initialize(MeshShaderInfo *shader_info, ShaderCodeType code_type) {

        /* Integrity checks */
        VP_ASSERT((shader_info->code_mesh != nullptr) && (shader_info->code_fragment != nullptr));

        /* Setup mesh, fragment, and the optional task shader info */
        const VkShaderCreateFlagsEXT create_flags = static_cast<VkShaderCreateFlagsEXT>(VK_SHADER_CREATE_LINK_STAGE_BIT_EXT | ((shader_info->code_task == nullptr) ? VK_SHADER_CREATE_NO_TASK_SHADER_BIT_EXT : 0));
        static constexpr const ShaderStage cStageArray[] = {
            ShaderStage::Task,
            ShaderStage::Mesh,
            ShaderStage::Fragment,
        };
        static constexpr u32 cStageCount = sizeof(cStageArray) / sizeof(ShaderStage);
        return this->InitializeImpl(shader_info->code_array, shader_info->code_size_array, cStageArray, cStageCount, code_type, create_flags);
    }

    Result Shader::Initialize(PrimitiveShaderInfo *shader_info, ShaderCodeType code_type) {

        /* Integrity checks */
        VP_ASSERT((shader_info->code_vertex != nullptr) && (shader_info->code_fragment != nullptr));

        /* Setup vertex and fragment shaders, and optional geometry, and tessellation */
        constexpr VkShaderCreateFlagsEXT create_flags = static_cast<VkShaderCreateFlagsEXT>(VK_SHADER_CREATE_LINK_STAGE_BIT_EXT);
        static constexpr const ShaderStage cStageArray[] = {
            ShaderStage::Vertex,
            ShaderStage::TessellationControl,
            ShaderStage::TessellationEvaluation,
            ShaderStage::Geometry,
            ShaderStage::Fragment,
        };
        static constexpr u32 cStageCount = sizeof(cStageArray) / sizeof(ShaderStage);
        return this->InitializeImpl(shader_info->code_array, shader_info->code_size_array, cStageArray, cStageCount, code_type, create_flags);
    }

    Result Shader::Initialize(ComputeShaderInfo *shader_info, ShaderCodeType code_type) {

        /* Integrity checks */
        VP_ASSERT(shader_info->code_compute != nullptr);

        /* Setup compute shader */
        constexpr VkShaderCreateFlagsEXT create_flags = static_cast<VkShaderCreateFlagsEXT>(VK_SHADER_CREATE_LINK_STAGE_BIT_EXT);
        static constexpr const ShaderStage cStageArray[] = {
            ShaderStage::Compute,
        };
        static constexpr u32 cStageCount = sizeof(cStageArray) / sizeof(ShaderStage);
        return this->InitializeImpl(std::addressof(shader_info->code_compute), std::addressof(shader_info->code_size_compute), cStageArray, cStageCount, code_type, create_flags);
    }

    Result Shader::Initialize(vp::res::ResBnshShaderProgram *res_program) {

        /* Set the shader program atomically, handle someone else getting here first */
        vp::res::ResBnshShaderProgram *prev_program;
        const bool is_set_program = vp::util::InterlockedCompareExchange(std::addressof(prev_program), std::addressof(m_res_program), res_program, static_cast<vp::res::ResBnshShaderProgram*>(nullptr));
        if (!is_set_program) {
            if (res_program == prev_program) {
                m_init_event.Wait();
                prev_program = m_res_program;
            }
            RESULT_RETURN_IF(res_program != prev_program, ResultInitializedByDifferentProgram);
            RESULT_RETURN_SUCCESS;
        }

        /* Ensure the program is either binary or spirv */
        const vp::res::GfxShaderCodeType code_type = static_cast<vp::res::GfxShaderCodeType>(res_program->code_type);
        RESULT_RETURN_IF(code_type != vp::res::GfxShaderCodeType::Binary && code_type != vp::res::GfxShaderCodeType::Intermediate, ResultInvalidShaderCodeType);
        const VkShaderCodeTypeEXT vk_code_type = (code_type == vp::res::GfxShaderCodeType::Binary) ? VK_SHADER_CODE_TYPE_BINARY_EXT : VK_SHADER_CODE_TYPE_SPIRV_EXT;

        /* Setup mesh, fragment, and the optional task shader info */
        vp::res::ResBnshShaderCodeInfo **code_info_array = res_program->code_info_array;
        u32 compile_count = 0;
        gfx::ShaderStage stage_array[cMaxShaderStageCount];
        VkShaderCreateInfoEXT vk_shader_create_info_array[cMaxShaderStageCount];
        const u32 max_shader_count = cMaxShaderStageCount - (2 * (!gfx::IsOptionalFeature(OptionalFeature::SupportsMeshShaders)));
        for (u32 i = 0; i < max_shader_count; ++i) {

            /* Increment reference  */
            vp::res::ResBnshShaderCodeInfo *code_info = code_info_array[i];
            if (code_info == nullptr) { continue; }
            
            /* Resolve stage and next stage */
            const ShaderStage shader_stage = static_cast<ShaderStage>(i);
            stage_array[compile_count] = shader_stage;
            const VkShaderStageFlags vk_shader_stage = vp::res::GfxShaderStageToVkShaderStageFlags(shader_stage);
            VkShaderStageFlags vk_next_shader_stage = 0;
            u32 flags = VK_SHADER_CREATE_LINK_STAGE_BIT_EXT;
            if (i < 4) {
                for (u32 j = i; j < 5; ++j) {
                    if (code_info_array[j] == nullptr) { continue; }
                    const ShaderStage next_shader_stage = static_cast<ShaderStage>(j);
                    vk_next_shader_stage = vp::res::GfxShaderStageToVkShaderStageFlags(next_shader_stage);
                    break;
                }
            } else if (i == 6) {
                vk_next_shader_stage = VK_SHADER_STAGE_MESH_BIT_EXT;
            } else if (i == 7) {
                if (compile_count < 1 || stage_array[compile_count - 1] != ShaderStage::Task) {
                    flags |= VK_SHADER_CREATE_NO_TASK_SHADER_BIT_EXT;
                }
                vk_next_shader_stage = VK_SHADER_STAGE_FRAGMENT_BIT;
            }

            u32 desc_count = 0;
            const VkDescriptorSetLayout *desc_layout = impl::Context::GetInstance()->GetPerStageDescriptorLayoutArray(std::addressof(desc_count), shader_stage);

            vk_shader_create_info_array[i].sType                  = VK_STRUCTURE_TYPE_SHADER_CREATE_INFO_EXT;
            vk_shader_create_info_array[i].pNext                  = nullptr;
            vk_shader_create_info_array[i].flags                  = flags;
            vk_shader_create_info_array[i].stage                  = static_cast<VkShaderStageFlagBits>(vk_shader_stage);
            vk_shader_create_info_array[i].nextStage              = vk_next_shader_stage;
            vk_shader_create_info_array[i].codeType               = vk_code_type;
            vk_shader_create_info_array[i].codeSize               = code_info->code_size;
            vk_shader_create_info_array[i].pCode                  = code_info->code;
            vk_shader_create_info_array[i].pName                  = "main";
            vk_shader_create_info_array[i].setLayoutCount         = desc_count;
            vk_shader_create_info_array[i].pSetLayouts            = desc_layout;
            vk_shader_create_info_array[i].pushConstantRangeCount = 0;
            vk_shader_create_info_array[i].pPushConstantRanges    = nullptr;
            vk_shader_create_info_array[i].pSpecializationInfo    = nullptr;

            ++compile_count;
        }

        /* Create shaders */
        VkShaderEXT vk_shader_array[cMaxShaderStageCount] = {};
        const u32 vk_result = ::pfn_vkCreateShadersEXT(impl::Context::GetInstance()->GetVkDevice(), compile_count, vk_shader_create_info_array, impl::Context::GetInstance()->GetVkAllocationCallbacks(), vk_shader_array);
        RESULT_RETURN_UNLESS(vk_result == VK_SUCCESS, ResultFailedToCreateShaders);

        /* Write back shader handles */
        for (u32 i = 0; i < compile_count; ++i) {
            m_vk_shader_array[static_cast<u32>(stage_array[i])] = vk_shader_array[i];
        }

        /* Set state */
        m_init_event.Signal();

        RESULT_RETURN_SUCCESS;
    }

    void Shader::Finalize() {

        /* Reset program */
        if (m_res_program != nullptr) {
            m_init_event.Clear();
            m_res_program = nullptr;
        }
        
        /* Delete shaders */
        for (u32 i = 0; i < cMaxShaderStageCount; ++i) {
            if (m_vk_shader_array[i] == VK_NULL_HANDLE) { continue; }
            ::pfn_vkDestroyShaderEXT(impl::Context::GetInstance()->GetVkDevice(), m_vk_shader_array[i], impl::Context::GetInstance()->GetVkAllocationCallbacks());
        }

        /* Reset shader array */
        static_assert(VK_NULL_HANDLE == 0);
        ::memset(m_vk_shader_array, 0, cMaxShaderStageCount * sizeof(VkShaderEXT));

        return;
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
        vp::res::ResNintendoWareDictionary *dictionary = nullptr;
        u32 base_index = 0;
        const u32 interface_type_index = static_cast<u32>(interface_type);
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
