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
        u32 base_slot = 0;
        u32 compile_count = 0;
        [[indeterminate]] gfx::ShaderStage stage_array[cMaxShaderStageCount];
        [[indeterminate]] VkShaderCreateInfoEXT vk_shader_create_info_array[cMaxShaderStageCount];
        [[indeterminate]] VkShaderDescriptorSetAndBindingMappingInfoEXT binding_info_array[cMaxShaderStageCount];
        [[indeterminate]] VkDescriptorSetAndBindingMappingEXT mapping_array_storage[230 * cMaxShaderStageCount];
        const u32 desc_count = 0;
        const VkDescriptorSetLayout *desc_layout = nullptr;
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
            flags |= VK_SHADER_CREATE_DESCRIPTOR_HEAP_BIT_EXT;
            /* Build descriptor heap interface */
            u32 mapping_count = 0;
            VkDescriptorSetAndBindingMappingEXT *mapping_array = mapping_array_storage + (i * 230);
            vp::res::ResBnshShaderInterfaceInfo *interface_info = (res_program->shader_interface_info_table != nullptr) ? res_program->shader_interface_info_table->GetShaderInterfaceInfo(shader_stage) : nullptr;
            if (interface_info != nullptr) {

                static constexpr ShaderInterfaceType cGfxShaderInterfaceType[] = {
                    ShaderInterfaceType::TextureSampler,
                    ShaderInterfaceType::UniformBuffer,
                    ShaderInterfaceType::StorageBuffer,
                    ShaderInterfaceType::Image,
                    ShaderInterfaceType::SeparateTexture,
                    ShaderInterfaceType::SeparateSampler,
                };
                static constexpr VkDescriptorType cGfxShaderInterfaceTypeToVkDescriptorType[] = {
                    VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                    VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                    VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                    VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
                    VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
                    VK_DESCRIPTOR_TYPE_SAMPLER,
                };
                static constexpr VkSpirvResourceTypeFlagBitsEXT cGfxShaderInterfaceTypeToVkSpirvResourceTypeFlagBitsEXT[] = {
                    VK_SPIRV_RESOURCE_TYPE_COMBINED_SAMPLED_IMAGE_BIT_EXT,
                    VK_SPIRV_RESOURCE_TYPE_UNIFORM_BUFFER_BIT_EXT,
                    static_cast<VkSpirvResourceTypeFlagBitsEXT>(VK_SPIRV_RESOURCE_TYPE_READ_ONLY_STORAGE_BUFFER_BIT_EXT | VK_SPIRV_RESOURCE_TYPE_READ_WRITE_STORAGE_BUFFER_BIT_EXT),
                    static_cast<VkSpirvResourceTypeFlagBitsEXT>(VK_SPIRV_RESOURCE_TYPE_READ_ONLY_IMAGE_BIT_EXT | VK_SPIRV_RESOURCE_TYPE_READ_WRITE_IMAGE_BIT_EXT),
                    VK_SPIRV_RESOURCE_TYPE_SAMPLED_IMAGE_BIT_EXT,
                    VK_SPIRV_RESOURCE_TYPE_SAMPLER_BIT_EXT,
                };
                static constexpr VkDescriptorMappingSourceEXT cGfxShaderInterfaceTypeToVkDescriptorMappingSource[] = {
                    VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_INDIRECT_INDEX_EXT,
                    VK_DESCRIPTOR_MAPPING_SOURCE_INDIRECT_ADDRESS_EXT,
                    VK_DESCRIPTOR_MAPPING_SOURCE_INDIRECT_ADDRESS_EXT,
                    VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_INDIRECT_INDEX_EXT,
                    VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_INDIRECT_INDEX_EXT,
                    VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_INDIRECT_INDEX_EXT,
                };
                static_assert(6 == sizeof(cGfxShaderInterfaceType) / sizeof(ShaderInterfaceType));
                static_assert(6 == sizeof(cGfxShaderInterfaceTypeToVkDescriptorType) / sizeof(VkDescriptorType));
                static_assert(6 == sizeof(cGfxShaderInterfaceTypeToVkDescriptorMappingSource) / sizeof(VkDescriptorMappingSourceEXT));
                u32 max_slot = 0;
                for (u32 j = 0; j < 6; ++j) {
                    const ShaderInterfaceType interface_type = cGfxShaderInterfaceType[j];
                    const u32 interface_count = interface_info->GetInterfaceCount(interface_type);
                    if (0 < interface_count) {
                        const u32 *slot_array = interface_info->GetInterfaceSlotArray(interface_type);
                        const VkDescriptorMappingSourceEXT source = cGfxShaderInterfaceTypeToVkDescriptorMappingSource[j];
                        for (u32 k = 0; k < interface_count; ++k) {
                            const u32 location = slot_array[k];
                            if (location == vp::res::ResBnshShaderInterfaceInfo::cInvalidInterfaceSlot) { continue; }
                            VP_ASSERT(mapping_count < 230);
                            max_slot = vp::util::Max(max_slot, location + 1);
                            const u32 slot = base_slot + location;
                            VkDescriptorSetAndBindingMappingEXT *mapping = mapping_array + mapping_count;
                            ++mapping_count;
                            mapping->sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_AND_BINDING_MAPPING_EXT;
                            mapping->pNext = nullptr;
                            mapping->descriptorSet = 0;
                            mapping->firstBinding = location;
                            mapping->bindingCount = 1;
                            mapping->resourceMask = cGfxShaderInterfaceTypeToVkSpirvResourceTypeFlagBitsEXT[j];
                            mapping->source = source;
                            if (source == VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_INDIRECT_INDEX_EXT) {
                                mapping->sourceData.indirectIndex.heapOffset = 0;
                                mapping->sourceData.indirectIndex.pushOffset = 0;
                                mapping->sourceData.indirectIndex.addressOffset = static_cast<u32>(slot * sizeof(u64));
                                mapping->sourceData.indirectIndex.heapIndexStride = impl::Context::cTargetMaxSeparateTextureDescriptorSize;
                                mapping->sourceData.indirectIndex.heapArrayStride = impl::Context::cTargetMaxSeparateTextureDescriptorSize;
                                mapping->sourceData.indirectIndex.pEmbeddedSampler = nullptr;
                                mapping->sourceData.indirectIndex.useCombinedImageSamplerIndex = VK_TRUE;
                                mapping->sourceData.indirectIndex.samplerHeapOffset = 0;
                                mapping->sourceData.indirectIndex.samplerPushOffset = 0;
                                mapping->sourceData.indirectIndex.samplerAddressOffset = static_cast<u32>(slot * sizeof(u64));
                                mapping->sourceData.indirectIndex.samplerHeapIndexStride = impl::Context::cTargetMaxSeparateSamplerDescriptorSize;
                                mapping->sourceData.indirectIndex.samplerHeapArrayStride = impl::Context::cTargetMaxSeparateSamplerDescriptorSize;
                                if (VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER != cGfxShaderInterfaceTypeToVkDescriptorType[j]) {
                                    mapping->sourceData.indirectIndex.useCombinedImageSamplerIndex = VK_FALSE;
                                }
                            } else {
                                mapping->sourceData.indirectAddress.pushOffset = 0;
                                mapping->sourceData.indirectAddress.addressOffset = static_cast<u32>(slot * sizeof(u64));
                            }
                        }
                    }
                }
                base_slot += max_slot;
            }
            binding_info_array[compile_count].sType = VK_STRUCTURE_TYPE_SHADER_DESCRIPTOR_SET_AND_BINDING_MAPPING_INFO_EXT;
            binding_info_array[compile_count].pNext = nullptr;
            binding_info_array[compile_count].mappingCount = mapping_count;
            binding_info_array[compile_count].pMappings = mapping_array;
            pnext = std::addressof(binding_info_array[compile_count]);

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

    Result Shader::Initialize(vp::res::ResBfshaShaderProgram *program) {

        vp::res::ResBnshShaderProgram *res_program = program->bnsh_shader_variation->program_intermediate;
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
        u8 base_location_array[cMaxShaderStageCount];
        program->parent_shading_model->GetBaseLocationArray(base_location_array, sizeof(base_location_array));

        /* Ensure the program is either binary or spirv */
        const vp::res::GfxShaderCodeType code_type = static_cast<vp::res::GfxShaderCodeType>(res_program->code_type);
        RESULT_RETURN_IF(code_type != vp::res::GfxShaderCodeType::Binary && code_type != vp::res::GfxShaderCodeType::Intermediate, ResultInvalidShaderCodeType);
        const VkShaderCodeTypeEXT vk_code_type = (code_type == vp::res::GfxShaderCodeType::Binary) ? VK_SHADER_CODE_TYPE_BINARY_EXT : VK_SHADER_CODE_TYPE_SPIRV_EXT;

        /* Setup shader info */
        const u32 stage_count = program->parent_shading_model->shader_stage_count;
        u32 base_slot = 0;
        u32 compile_count = 0;
        [[indeterminate]] gfx::ShaderStage stage_array[cMaxShaderStageCount];
        [[indeterminate]] VkShaderCreateInfoEXT vk_shader_create_info_array[cMaxShaderStageCount];
        [[indeterminate]] VkShaderDescriptorSetAndBindingMappingInfoEXT binding_info_array[cMaxShaderStageCount];
        [[indeterminate]] VkDescriptorSetAndBindingMappingEXT mapping_array_storage[70 * cMaxShaderStageCount];
        const u32 desc_count = 0;
        const VkDescriptorSetLayout *desc_layout = nullptr;
        for (u32 i = 0; i < cMaxShaderStageCount; ++i) {

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
            flags |= VK_SHADER_CREATE_DESCRIPTOR_HEAP_BIT_EXT;
            /* Build descriptor heap interface */
            u32 mapping_count = 0;
            VkDescriptorSetAndBindingMappingEXT *mapping_array = mapping_array_storage + (i * 70);
            static constexpr VkDescriptorType cBfshaInterfaceToVkDescriptorType[] = {
                VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
                VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            };
            static constexpr VkSpirvResourceTypeFlagBitsEXT cBfshaInterfaceToVkSpirvResourceTypeFlagBitsEXT[] = {
                VK_SPIRV_RESOURCE_TYPE_COMBINED_SAMPLED_IMAGE_BIT_EXT,
                static_cast<VkSpirvResourceTypeFlagBitsEXT>(VK_SPIRV_RESOURCE_TYPE_READ_ONLY_IMAGE_BIT_EXT | VK_SPIRV_RESOURCE_TYPE_READ_WRITE_IMAGE_BIT_EXT),
                VK_SPIRV_RESOURCE_TYPE_UNIFORM_BUFFER_BIT_EXT,
                static_cast<VkSpirvResourceTypeFlagBitsEXT>(VK_SPIRV_RESOURCE_TYPE_READ_ONLY_STORAGE_BUFFER_BIT_EXT | VK_SPIRV_RESOURCE_TYPE_READ_WRITE_STORAGE_BUFFER_BIT_EXT),
            };
            static constexpr VkDescriptorMappingSourceEXT cBfshaInterfaceToVkDescriptorMappingSource[] = {
                VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_INDIRECT_INDEX_EXT,
                VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_INDIRECT_INDEX_EXT,
                VK_DESCRIPTOR_MAPPING_SOURCE_INDIRECT_ADDRESS_EXT,
                VK_DESCRIPTOR_MAPPING_SOURCE_INDIRECT_ADDRESS_EXT,
            };
            u32 max_slot = 0;
            for (u32 j = 0; j < 4; ++j) {
                u32 *location_array = program->location_array[j];
                if (location_array == nullptr) { continue; }
                const u32 interface_count = program->location_count_array[j];
                const VkDescriptorMappingSourceEXT source = cBfshaInterfaceToVkDescriptorMappingSource[j];
                for (u32 k = 0; k < interface_count; ++k) {
                    const u32 location = location_array[k * stage_count + base_location_array[i]];
                    if (location == vp::res::ResBfshaShaderProgram::cInvalidLocation) { continue; }
                    max_slot = vp::util::Max(max_slot, location + 1);
                    const u32 slot = base_slot + location;
                    VP_ASSERT(mapping_count < 70);
                    VkDescriptorSetAndBindingMappingEXT *mapping = mapping_array + mapping_count;
                    ++mapping_count;
                    mapping->sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_AND_BINDING_MAPPING_EXT;
                    mapping->pNext = nullptr;
                    mapping->descriptorSet = 0;
                    mapping->firstBinding = location;
                    mapping->bindingCount = 1;
                    mapping->resourceMask = cBfshaInterfaceToVkSpirvResourceTypeFlagBitsEXT[j];
                    mapping->source = source;
                    if (source == VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_INDIRECT_INDEX_EXT) {
                        mapping->sourceData.indirectIndex.heapOffset = 0;
                        mapping->sourceData.indirectIndex.pushOffset = 0;
                        mapping->sourceData.indirectIndex.addressOffset = static_cast<u32>(slot * sizeof(u64));
                        mapping->sourceData.indirectIndex.heapIndexStride = impl::Context::cTargetMaxSeparateTextureDescriptorSize;
                        mapping->sourceData.indirectIndex.heapArrayStride = impl::Context::cTargetMaxSeparateTextureDescriptorSize;
                        mapping->sourceData.indirectIndex.pEmbeddedSampler = nullptr;
                        mapping->sourceData.indirectIndex.useCombinedImageSamplerIndex = VK_TRUE;
                        mapping->sourceData.indirectIndex.samplerHeapOffset = 0;
                        mapping->sourceData.indirectIndex.samplerPushOffset = 0;
                        mapping->sourceData.indirectIndex.samplerAddressOffset = static_cast<u32>(slot * sizeof(u64));
                        mapping->sourceData.indirectIndex.samplerHeapIndexStride = impl::Context::cTargetMaxSeparateSamplerDescriptorSize;
                        mapping->sourceData.indirectIndex.samplerHeapArrayStride = impl::Context::cTargetMaxSeparateSamplerDescriptorSize;
                        if (VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER != cBfshaInterfaceToVkDescriptorType[j]) {
                            mapping->sourceData.indirectIndex.useCombinedImageSamplerIndex = VK_FALSE;
                        }
                    } else {
                        mapping->sourceData.indirectAddress.pushOffset = 0;
                        mapping->sourceData.indirectAddress.addressOffset = static_cast<u32>(slot * sizeof(u64));
                    }
                }
            }
            base_slot += max_slot;
            binding_info_array[compile_count].sType = VK_STRUCTURE_TYPE_SHADER_DESCRIPTOR_SET_AND_BINDING_MAPPING_INFO_EXT;
            binding_info_array[compile_count].pNext = nullptr;
            binding_info_array[compile_count].mappingCount = mapping_count;
            binding_info_array[compile_count].pMappings = mapping_array;
            pnext = std::addressof(binding_info_array[compile_count]);

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
            m_vk_shader_array[i] = VK_NULL_HANDLE;
        }
        static_assert(VK_NULL_HANDLE == 0);

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
