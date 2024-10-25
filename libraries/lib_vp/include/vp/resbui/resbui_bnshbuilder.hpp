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

        u32 location = cInvalidLocation;
        vp::res::GfxShaderInterfaceType interface_type;
        DictionaryString name;

        constexpr bool IsValid() const {
            if (this->location == cInvalidLocation) { return false; }
            if (this->name.GetString() == nullptr || *this->name.GetString() == '\0') { return false; }
            return true;
        }
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
        public:
            using InterfaceArray = vp::util::FixedPointerArrayAllocator<BnshShaderInterfaceNode, cTotalInterfaceNodeCount>;
        private:
            const void *m_code;
            const void *m_control;
            u32         m_code_size;
            u32         m_control_size;
            u32         m_code_offset;
            u32         m_control_offset;
            u32         m_workgroup_x;
            u32         m_workgroup_y;
            u32         m_workgroup_z;
            InterfaceArray m_interface_array;
        public:
            constexpr BnshShaderNode() : m_code(), m_control(), m_code_size(), m_control_size(), m_code_offset(), m_control_offset(), m_workgroup_x(), m_workgroup_y(), m_workgroup_z(), m_interface_array() {/*...*/}
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

            bool AddInterface(res::GfxShaderInterfaceType interface_type, u32 location, const char *name) {
                BnshShaderInterfaceNode *node = m_interface_array.Allocate();
                if (node == nullptr) { return false; }
                node->location = location;
                node->interface_type = interface_type;
                node->name.SetString(name);
                return true;
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
