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

#ifndef AWN_ENABLE_VK_DESCRIPTOR_HEAP
    Result Shader::InitializeImpl(void **code_array, u32 *code_size_array, const ShaderStage *stage_array, u32 stage_count, ShaderCodeType code_type, VkShaderCreateFlagsEXT create_flags) {

        /* Build create info from provided stages */
        impl::Context *context = impl::Context::GetInstance();
        const VkShaderCodeTypeEXT vk_code_type = vp::res::GfxShaderCodeTypeToVkShaderCodeType(code_type);
        VkShaderCreateInfoEXT vk_shader_create_info_array[impl::Context::cTargetMaxPrimitiveShaderStageCount];
        const u32 desc_count = impl::Context::cMaxCombinedSetCountGraphics;
        const VkDescriptorSetLayout *desc_layout = context->GetPerStageDescriptorLayoutArray();
        u32 info_i = 0;
        for (u32 i = 0; i < stage_count; ++i) {
            if (code_array[i] == nullptr) { continue; }

            const ShaderStage stage = stage_array[i];
            const VkShaderStageFlags vk_stage_flags = vp::res::GfxShaderStageToVkShaderStageFlags(stage);
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
        const VkResult vk_result = ::pfn_vkCreateShadersEXT(context->GetVkDevice(), info_i, vk_shader_create_info_array, context->GetVkAllocationCallbacks(), vk_shader_array);
        RESULT_TRY(impl::ConvertVkResultToResult(vk_result));

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
#endif

    Result Shader::Initialize(vp::res::ResBnshShaderProgram *res_program) {

        /* Set the shader program atomically, handle someone else getting here first */
        vp::res::ResBnshShaderProgram *prev_program;
        const bool is_set_program = vp::util::InterlockedCompareExchangeRelaxed(std::addressof(prev_program), std::addressof(m_res_program), res_program, static_cast<vp::res::ResBnshShaderProgram*>(nullptr));
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
        u32 compile_count = 0;
        gfx::ShaderStage stage_array[cMaxShaderStageCount];
        VkShaderCreateInfoEXT vk_shader_create_info_array[cMaxShaderStageCount];
    #ifdef AWN_ENABLE_VK_DESCRIPTOR_HEAP
        VkShaderDescriptorSetAndBindingMappingInfoEXT binding_info_array[cMaxShaderStageCount];
        VkDescriptorSetAndBindingMappingEXT mapping_array_storage[230 * cMaxShaderStageCount];
        const u32 desc_count = 0;
        const VkDescriptorSetLayout *desc_layout = nullptr;
    #else
        const u32 desc_count = impl::Context::cMaxCombinedSetCountGraphics;
        const VkDescriptorSetLayout *desc_layout = impl::Context::GetInstance()->GetPerStageDescriptorLayoutArray();
    #endif
        const u32 max_shader_count = cMaxShaderStageCount - (2 * (!gfx::IsOptionalFeature(OptionalFeature::SupportsMeshShaders)));
        for (u32 i = 0; i < max_shader_count; ++i) {

            /* Increment reference  */
            const ShaderStage shader_stage = static_cast<ShaderStage>(i);
            vp::res::ResBnshShaderCodeInfo *code_info = res_program->GetShaderCodeInfo(shader_stage);
            if (code_info == nullptr) { continue; }
            
            /* Resolve stage and next stage */
            stage_array[compile_count] = shader_stage;
            const VkShaderStageFlags vk_shader_stage = vp::res::GfxShaderStageToVkShaderStageFlags(shader_stage);
            VkShaderStageFlags vk_next_shader_stage = 0;
            u32 flags = VK_SHADER_CREATE_LINK_STAGE_BIT_EXT;
            if (i < 4) {
                for (u32 j = i + 1; j < 5; ++j) {
                    const ShaderStage next_shader_stage = static_cast<ShaderStage>(j);
                    if (res_program->GetShaderCodeInfo(next_shader_stage) == nullptr) { continue; }
                    vk_next_shader_stage = vp::res::GfxShaderStageToVkShaderStageFlags(next_shader_stage);
                    break;
                }
            } else if (i == 7) {
                vk_next_shader_stage = VK_SHADER_STAGE_MESH_BIT_EXT;
            } else if (i == 8) {
                if (compile_count < 1 || stage_array[compile_count - 1] != ShaderStage::Task) {
                    flags |= VK_SHADER_CREATE_NO_TASK_SHADER_BIT_EXT;
                }
                vk_next_shader_stage = VK_SHADER_STAGE_FRAGMENT_BIT;
            }

            const void *pnext = nullptr;
    #ifdef AWN_ENABLE_VK_DESCRIPTOR_HEAP
            flags |= VK_SHADER_CREATE_DESCRIPTOR_HEAP_BIT_EXT;
            /* Build descriptor heap interface */
            u32 mapping_count = 0;
            VkDescriptorSetAndBindingMappingEXT *mapping_array = mapping_array_storage + (i * 230);
            vp::res::ResBnshShaderInterfaceInfo *interface_info = (res_program->shader_interface_info_table != nullptr) ? res_program->shader_interface_info_table->GetShaderInterfaceInfo(shader_stage) : nullptr;
            if (interface_info != nullptr) {

                static constexpr VkDescriptorType cGfxShaderInterfaceTypeToVkDescriptorType[] = {
                    VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                    VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                    VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                    VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
                    VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
                    VK_DESCRIPTOR_TYPE_SAMPLER,
                };
                static constexpr VkDescriptorMappingSourceEXT cGfxShaderInterfaceTypeToVkDescriptorMappingSource[] = {
                    VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_INDIRECT_INDEX_EXT,
                    VK_DESCRIPTOR_MAPPING_SOURCE_INDIRECT_ADDRESS_EXT,
                    VK_DESCRIPTOR_MAPPING_SOURCE_INDIRECT_ADDRESS_EXT,
                    VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_INDIRECT_INDEX_EXT,
                    VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_INDIRECT_INDEX_EXT,
                    VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_INDIRECT_INDEX_EXT,
                };
                static_assert(vp::util::TEnumTraits<ShaderInterfaceType>::cEnumCount - 2 == sizeof(cGfxShaderInterfaceTypeToVkDescriptorType) / sizeof(VkDescriptorType));
                for (u32 j = 2; j < vp::util::TEnumTraits<ShaderInterfaceType>::cEnumCount; ++j) {
                    const ShaderInterfaceType interface_type = vp::util::TEnumTraits<ShaderInterfaceType>::GetValueByIndex(j);
                    const u32 interface_count = interface_info->GetInterfaceCount(interface_type);
                    if (0 < interface_count) {
                        const u32 *slot_array = interface_info->GetInterfaceSlotArray(interface_type);
                        const VkDescriptorMappingSourceEXT source = cGfxShaderInterfaceTypeToVkDescriptorMappingSource[j-2];
                        for (u32 k = 0; k < interface_count; ++k) {
                            const u32 slot = slot_array[k];
                            if (slot == vp::res::ResBnshShaderInterfaceInfo::cInvalidInterfaceSlot) { continue; }
                            VP_ASSERT(mapping_count < 230);
                            VkDescriptorSetAndBindingMappingEXT *mapping = mapping_array + mapping_count;
                            ++mapping_count;
                            mapping->sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_AND_BINDING_MAPPING_EXT;
                            mapping->descriptorSet = i;
                            mapping->firstBinding = slot;
                            mapping->bindingCount = 1;
                            mapping->resourceMask = VK_SPIRV_RESOURCE_TYPE_ALL_EXT;
                            mapping->source = source;
                            if (source == VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_INDIRECT_INDEX_EXT) {
                                if (VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER == cGfxShaderInterfaceTypeToVkDescriptorType[j-2]) {
                                    mapping->sourceData = VkDescriptorMappingSourceDataEXT{
                                        .indirectIndex = {
                                            .heapOffset = 0,
                                            .pushOffset = static_cast<u32>(i * sizeof(u64)),
                                            .addressOffset = static_cast<u32>(slot * sizeof(u64)),
                                            .heapIndexStride = sizeof(u64),
                                            .heapArrayStride = 1,
                                            .useCombinedImageSamplerIndex = VK_TRUE,
                                            .samplerHeapOffset = 0,
                                            .samplerPushOffset = static_cast<u32>(i * sizeof(u64)),
                                            .samplerAddressOffset = static_cast<u32>(slot * sizeof(u64)),
                                            .samplerHeapIndexStride = sizeof(u64),
                                            .samplerHeapArrayStride = 1,
                                        },
                                    };
                                } else {
                                    mapping->sourceData = VkDescriptorMappingSourceDataEXT{
                                        .indirectIndex = {
                                            .heapOffset = 0,
                                            .pushOffset = static_cast<u32>(i * sizeof(u64)),
                                            .addressOffset = static_cast<u32>(slot * sizeof(u64)),
                                            .heapIndexStride = sizeof(u64),
                                            .heapArrayStride = 1,
                                        },
                                    };
                                }
                            } else {
                                mapping->sourceData = VkDescriptorMappingSourceDataEXT{
                                    .indirectAddress = {
                                        .pushOffset = static_cast<u32>(i * sizeof(u64)),
                                        .addressOffset = static_cast<u32>(slot * sizeof(u64)),
                                    },
                                };
                            }
                        }
                    }
                }
            }
            binding_info_array[compile_count].sType = VK_STRUCTURE_TYPE_SHADER_DESCRIPTOR_SET_AND_BINDING_MAPPING_INFO_EXT;
            binding_info_array[compile_count].pNext = nullptr;
            binding_info_array[compile_count].mappingCount = mapping_count;
            binding_info_array[compile_count].pMappings = mapping_array;
            pnext = std::addressof(binding_info_array[compile_count]);
    #endif

            /* Write shader create info */
            vk_shader_create_info_array[compile_count].sType                  = VK_STRUCTURE_TYPE_SHADER_CREATE_INFO_EXT;
            vk_shader_create_info_array[compile_count].pNext                  = pnext;
            vk_shader_create_info_array[compile_count].flags                  = flags;
            vk_shader_create_info_array[compile_count].stage                  = static_cast<VkShaderStageFlagBits>(vk_shader_stage);
            vk_shader_create_info_array[compile_count].nextStage              = vk_next_shader_stage;
            vk_shader_create_info_array[compile_count].codeType               = vk_code_type;
            vk_shader_create_info_array[compile_count].codeSize               = code_info->code_size;
            vk_shader_create_info_array[compile_count].pCode                  = code_info->code;
            vk_shader_create_info_array[compile_count].pName                  = "main";
            vk_shader_create_info_array[compile_count].setLayoutCount         = desc_count;
            vk_shader_create_info_array[compile_count].pSetLayouts            = desc_layout;
            vk_shader_create_info_array[compile_count].pushConstantRangeCount = 0;
            vk_shader_create_info_array[compile_count].pPushConstantRanges    = nullptr;
            vk_shader_create_info_array[compile_count].pSpecializationInfo    = nullptr;

            ++compile_count;
        }

        /* Create shaders */
        VkShaderEXT vk_shader_array[cMaxShaderStageCount] = {};
        const VkResult vk_result = ::pfn_vkCreateShadersEXT(impl::Context::GetInstance()->GetVkDevice(), compile_count, vk_shader_create_info_array, impl::Context::GetInstance()->GetVkAllocationCallbacks(), vk_shader_array);
        RESULT_TRY(impl::ConvertVkResultToResult(vk_result));

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
        if (m_res_program == nullptr) { return cInvalidInterfaceSlot; }
        return m_res_program->GetInterfaceSlot(shader_stage, interface_type, key);
    }

    Result Shader::GetBinarySize(size_t *out_size, ShaderStage stage) {
        const u32 i = static_cast<u32>(stage);
        RESULT_RETURN_IF(m_vk_shader_array[i] == nullptr, ResultInvalidShaderStage);
        impl::Context *context = impl::Context::GetInstance();
        size_t size;
        const VkResult vk_result = ::pfn_vkGetShaderBinaryDataEXT(context->GetVkDevice(), m_vk_shader_array[i], std::addressof(size), nullptr);
        RESULT_TRY(impl::ConvertVkResultToResult(vk_result));
        *out_size += size;
        RESULT_RETURN_SUCCESS;
    }

    Result Shader::GetBinary(void *data, size_t data_size, ShaderStage stage) {
        const u32 i = static_cast<u32>(stage);
        RESULT_RETURN_IF(m_vk_shader_array[i] == nullptr, ResultInvalidShaderStage);
        impl::Context *context = impl::Context::GetInstance();
        const VkResult vk_result = ::pfn_vkGetShaderBinaryDataEXT(context->GetVkDevice(), m_vk_shader_array[i], std::addressof(data_size), data);
        RESULT_TRY(impl::ConvertVkResultToResult(vk_result));
        RESULT_RETURN_SUCCESS;
    }
}
