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

namespace vp::resbui {

    struct BnshShaderInterfaceNode {
        static constexpr u32 cInvalidLocation = 0xffff'ffff;

        u32              location = cInvalidLocation;
        DictionaryString name;
    };


    class BnshShaderProgramNode;
    class BnshBuilder;

    class BnshShaderNode {
        public:
            friend class BnshShaderProgramNode;
            friend class BnshBuilder;
        public:
            static constexpr u32 cInterfaceTableCount = vp::res::cMaxBnshInputAttributeCount + vp::res::cMaxBnshOutputAttributeCount + vp::res::cMaxBnshTextureSamplerCount + vp::res::cMaxBnshUniformBufferCount + vp::res::cMaxBnshStorageBufferCount + vp::res::cMaxBnshImageCount;
            static constexpr u32 cInterfaceTableCountExt = vp::res::cMaxBnshSeparateTextureCount + vp::res::cMaxBnshSeparateSamplerCount;
            static constexpr u32 cTotalInterfaceNodeCount = cInterfaceTableCount + cInterfaceTableCountExt;
        private:
            const void              *m_code;
            const void              *m_control;
            u32                      m_code_size;
            u32                      m_control_size;
            u32                      m_code_offset;
            u32                      m_control_offset;
            u32                      m_workgroup_x;
            u32                      m_workgroup_y;
            u32                      m_workgroup_z;
            BnshShaderInterfaceNode  m_input_attribute_array[vp::res::cMaxBnshInputAttributeCount];
            BnshShaderInterfaceNode  m_output_attribute_array[vp::res::cMaxBnshOutputAttributeCount];
            BnshShaderInterfaceNode  m_texture_sampler_array[vp::res::cMaxBnshTextureSamplerCount];
            BnshShaderInterfaceNode  m_uniform_buffer_array[vp::res::cMaxBnshUniformBufferCount];
            BnshShaderInterfaceNode  m_storage_buffer_array[vp::res::cMaxBnshStorageBufferCount];
            BnshShaderInterfaceNode  m_image_array[vp::res::cMaxBnshImageCount];
            BnshShaderInterfaceNode  m_separate_texture_array[vp::res::cMaxBnshSeparateTextureCount];
            BnshShaderInterfaceNode  m_separate_sampler_array[vp::res::cMaxBnshSeparateSamplerCount];
        public:
            constexpr BnshShaderNode() : m_code(), m_control(), m_code_size(), m_control_size(), m_code_offset(), m_control_offset(), m_workgroup_x(), m_workgroup_y(), m_workgroup_z(), m_input_attribute_array(), m_output_attribute_array(), m_texture_sampler_array(), m_uniform_buffer_array(), m_storage_buffer_array(), m_image_array(), m_separate_texture_array(), m_separate_sampler_array() {/*...*/}
            constexpr ~BnshShaderNode() {/*...*/}

            constexpr void SetShaderCode(const void *code, u32 code_size) {
                m_code = code;
                m_code_size = code_size;
            }
            constexpr void SetShaderControl(const void *control, u32 control_size) {
                m_control = control;
                m_control_size = control_size;
            }

            constexpr void SetWorkgroupSize(u32 x, u32 y, u32 z) {
                m_workgroup_x = x;
                m_workgroup_y = y;
                m_workgroup_z = z;
            }

            constexpr Result SetInterface(res::GfxShaderInterfaceType interface_type, u32 location, const char *name) {
                BnshShaderInterfaceNode *node = nullptr;
                switch (interface_type) {
                    case res::GfxShaderInterfaceType::InputAttribute:
                        RESULT_RETURN_UNLESS(location < res::cMaxBnshInputAttributeCount, ResultOutOfBoundsLocation);
                        node = std::addressof(m_input_attribute_array[location]);
                        break;
                    case res::GfxShaderInterfaceType::OutputAttribute:
                        RESULT_RETURN_UNLESS(location < res::cMaxBnshOutputAttributeCount, ResultOutOfBoundsLocation);
                        node = std::addressof(m_output_attribute_array[location]);
                        break;
                    case res::GfxShaderInterfaceType::TextureSampler:
                        RESULT_RETURN_UNLESS(location < res::cMaxBnshTextureSamplerCount, ResultOutOfBoundsLocation);
                        node = std::addressof(m_texture_sampler_array[location]);
                        break;
                    case res::GfxShaderInterfaceType::UniformBuffer:
                        RESULT_RETURN_UNLESS(location < res::cMaxBnshUniformBufferCount, ResultOutOfBoundsLocation);
                        node = std::addressof(m_uniform_buffer_array[location]);
                        break;
                    case res::GfxShaderInterfaceType::StorageBuffer:
                        RESULT_RETURN_UNLESS(location < res::cMaxBnshStorageBufferCount, ResultOutOfBoundsLocation);
                        node = std::addressof(m_storage_buffer_array[location]);
                        break;
                    case res::GfxShaderInterfaceType::Image:
                        RESULT_RETURN_UNLESS(location < res::cMaxBnshImageCount, ResultOutOfBoundsLocation);
                        node = std::addressof(m_image_array[location]);
                        break;
                    case res::GfxShaderInterfaceType::SeparateTexture:
                        RESULT_RETURN_UNLESS(location < res::cMaxBnshSeparateTextureCount, ResultOutOfBoundsLocation);
                        node = std::addressof(m_separate_texture_array[location]);
                        break;
                    case res::GfxShaderInterfaceType::SeparateSampler:
                        RESULT_RETURN_UNLESS(location < res::cMaxBnshSeparateSamplerCount, ResultOutOfBoundsLocation);
                        node = std::addressof(m_separate_sampler_array[location]);
                        break;
                    default:
                        return ResultInvalidShaderInterfaceType;
                };
                node->location = location;
                node->name.SetString(name);

                RESULT_RETURN_SUCCESS;
            }
    };

    class BnshShaderProgramNode {
        public:
            friend class BnshBuilder;
        private:
            BnshShaderNode *m_shader_array[res::cMaxBnshShaderStageCount];
        public:
            constexpr BnshShaderProgramNode() : m_shader_array() {/*...*/}
            constexpr ~BnshShaderProgramNode() {/*...*/}

            constexpr Result SetShader(res::GfxShaderStage shader_stage, BnshShaderNode *shader_node) {
                const u32 stage_index = static_cast<u32>(shader_stage);
                RESULT_RETURN_UNLESS(stage_index < res::cMaxBnshShaderStageCount, ResultOutOfBoundsShaderStage);
                m_shader_array[stage_index] = shader_node;
                RESULT_RETURN_SUCCESS;
            }
    };

    class BnshShaderVariationNode {
        public:
            friend class BnshBuilder;
        private:
            vp::util::IntrusiveListNode m_builder_node;
            BnshShaderProgramNode *m_program_array[res::cMaxBnshShaderCodeTypeCount];
        public:
            constexpr BnshShaderVariationNode() : m_builder_node(), m_program_array() {/*...*/}
            constexpr ~BnshShaderVariationNode() {/*...*/}

            constexpr Result SetProgram(res::GfxShaderCodeType code_type, BnshShaderProgramNode *program) {
                const u32 code_type_index = static_cast<u32>(code_type);
                RESULT_RETURN_UNLESS(code_type_index < res::cMaxBnshShaderCodeTypeCount, ResultOutOfBoundsCodeType);
                m_program_array[code_type_index] = program;
                RESULT_RETURN_SUCCESS;
            }
    };

    struct BnshBuilderMemoryInfo {
        size_t         total_size;
        size_t         max_alignment;
        u32            total_section_count;
        u32            total_relocation_count;
        BufferLocation location_header;
        BufferLocation location_container;
        BufferLocation location_variation_array;
        BufferLocation location_program_array;
        BufferLocation location_container_data;
        BufferLocation location_gpu_region;
        BufferLocation location_string_pool;
        BufferLocation location_relocation_table;
        NintendoWareStringPoolBuilderMemoryInfo string_pool_memory_info;
    };

    class BnshBuilder {
        public:
            using VariationList = vp::util::IntrusiveListTraits<BnshShaderVariationNode, &BnshShaderVariationNode::m_builder_node>::List;
        private:
            NintendoWareStringPoolBuilder m_string_builder;
            VariationList                 m_variation_list;
            StringPoolString              m_file_name;
            u32                           m_api_type;
            u32                           m_api_version;
            u32                           m_code_type;
            u32                           m_compiler_version;
        public:
            constexpr BnshBuilder() : m_string_builder(), m_variation_list(), m_file_name("dummy"), m_api_type(static_cast<u32>(vp::res::BnshTargetApiType::VpExtVulkan)), m_api_version(1), m_code_type(static_cast<u32>(vp::res::GfxShaderCodeType::Binary)), m_compiler_version(1) {/*...*/}
            constexpr ~BnshBuilder() {/*...*/}

            constexpr void SetTargetApi(u32 api_type, u32 api_version) {
                m_api_type = api_type;
                m_api_version = api_version;
            }
            constexpr void SetTargetCodeType(u32 code_type) {
                m_code_type = code_type;
            }
            constexpr void SetCompilerVersion(u32 compiler_version) {
                m_compiler_version = compiler_version;
            }
            constexpr void SetFileName(const char *name) {
                m_file_name.SetString(name);
            }

            void AddShaderVariation(BnshShaderVariationNode *variation_node) {
                m_variation_list.PushBack(*variation_node);
            }

            void Serialize(void *out_bnsh, size_t output_size, const BnshBuilderMemoryInfo *memory_info);

            Result CalculateMemoryInfo(BnshBuilderMemoryInfo *out_memory_info);
    };
}
