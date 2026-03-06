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

namespace vp::res {

    struct ResBnshShaderCodeInfo {
        void *runtime_user_shader_code_buffer_address;
        void *control;
        void *code;
        u32   code_size;
        u32   control_size;
        void *control_extra_info;
        u32   scratch_memory_size_recommended;
        u32   scratch_memory_size_per_warp;
        void *shader_ext_data;
        u64   reserve0;
    };
    static_assert(sizeof(ResBnshShaderCodeInfo) == 0x40);

    struct ResBnshExtendedInterfaceInfo {
        union {
            ResNintendoWareDictionary *ext_dictionary_array[2];
            struct {
                ResNintendoWareDictionary *separate_texture_dictionary;
                ResNintendoWareDictionary *separate_sampler_dictionary;
            };
        };
        union {
            u32 ext_base_index_array[2];
            struct {
                u32 base_index_separate_texture;
                u32 base_index_separate_sampler;
            };
        };
        u32 *reserve0;
        u64 reserve1[2];
    };
    static_assert(sizeof(ResBnshExtendedInterfaceInfo) == 0x30);

    struct ResBnshShaderInterfaceInfo {
        union {
            ResNintendoWareDictionary *base_dictionary_array[5];
            struct {
                ResNintendoWareDictionary *input_attribute_dictionary;
                ResNintendoWareDictionary *output_attribute_dictionary;
                ResNintendoWareDictionary *texture_sampler_dictionary;
                ResNintendoWareDictionary *uniform_buffer_dictionary;
                ResNintendoWareDictionary *storage_buffer_dictionary;
            };
        };
        union {
            u32 base_base_index_array[4];
            struct {
                u32 base_index_output_attribute;
                u32 base_index_texture_sampler;
                u32 base_index_uniform_buffer;
                u32 base_index_storage_buffer;
            };
        };
        u32                          *interface_slot_array;
        s32                           work_group_count_x;
        s32                           work_group_count_y;
        s32                           work_group_count_z;
        u32                           base_index_image;
        ResNintendoWareDictionary    *image_dictionary;
        ResBnshExtendedInterfaceInfo *extended_interface_info;
        
        static constexpr u32 cInvalidInterfaceSlot = 0xffff'ffff;

        ResNintendoWareDictionary *GetDictionary(GfxShaderInterfaceType interface_type) {
            /* Get dictionary and base index */
            ResNintendoWareDictionary *dictionary;
            const u32 interface_type_index = static_cast<u32>(interface_type);
            if (interface_type_index < static_cast<u32>(GfxShaderInterfaceType::Image)) {
                dictionary = this->base_dictionary_array[interface_type_index];
            } else if (interface_type == GfxShaderInterfaceType::Image) {
                dictionary = this->image_dictionary;
            } else if (interface_type_index <= static_cast<u32>(GfxShaderInterfaceType::SeparateSampler)) {
                if (this->extended_interface_info == nullptr) { return nullptr; }
                const u32 adj_index = interface_type_index - static_cast<u32>(GfxShaderInterfaceType::Image);
                dictionary = this->extended_interface_info->ext_dictionary_array[adj_index];
            } else {                
                VP_UNEXPECTED_DEFAULT();
            }
            return dictionary;
        }
        u32 *GetInterfaceSlotArray(GfxShaderInterfaceType interface_type) {
            /* Get base index */
            u32 base_index;
            const u32 interface_type_index = static_cast<u32>(interface_type);
            if (interface_type_index < static_cast<u32>(GfxShaderInterfaceType::Image)) {
                base_index = (interface_type_index == 0) ? 0 : this->base_base_index_array[interface_type_index - 1];
            } else if (interface_type == GfxShaderInterfaceType::Image) {
                base_index = this->base_index_image;
            } else if (interface_type_index <= static_cast<u32>(GfxShaderInterfaceType::SeparateSampler)) {
                if (this->extended_interface_info == nullptr) { return nullptr; }
                const u32 adj_index = interface_type_index - static_cast<u32>(GfxShaderInterfaceType::Image);
                base_index = this->extended_interface_info->ext_base_index_array[adj_index];
            } else {                
                VP_UNEXPECTED_DEFAULT();
            }
            /* Get slot */
            return std::addressof(this->interface_slot_array[base_index]);
        }
        u32 GetInterfaceCount(GfxShaderInterfaceType interface_type) {
            ResNintendoWareDictionary *dictionary = this->GetDictionary(interface_type);
            return (dictionary != nullptr) ? dictionary->node_count : 0;
        }
        u32 GetInterfaceSlot(GfxShaderInterfaceType interface_type, const char *key) {
            /* Get dictionary and base index */
            u32 base_index;
            ResNintendoWareDictionary *dictionary;
            const u32 interface_type_index = static_cast<u32>(interface_type);
            if (interface_type_index < static_cast<u32>(GfxShaderInterfaceType::Image)) {
                base_index = (interface_type_index == 0) ? 0 : this->base_base_index_array[interface_type_index - 1];
                dictionary = this->base_dictionary_array[interface_type_index];
            } else if (interface_type == GfxShaderInterfaceType::Image) {
                base_index = this->base_index_image;
                dictionary = this->image_dictionary;
            } else if (interface_type_index <= static_cast<u32>(GfxShaderInterfaceType::SeparateSampler)) {
                if (this->extended_interface_info == nullptr) { return cInvalidInterfaceSlot; }
                const u32 adj_index = interface_type_index - static_cast<u32>(GfxShaderInterfaceType::Image);
                base_index = this->extended_interface_info->ext_base_index_array[adj_index];
                dictionary = this->extended_interface_info->ext_dictionary_array[adj_index];
            } else {                
                VP_UNEXPECTED_DEFAULT();
            }
            /* Get slot */
            if (dictionary == nullptr) { return cInvalidInterfaceSlot; }
            const u32 index = dictionary->TryGetEntryIndexByKey(key);
            if (index == vp::res::ResNintendoWareDictionary::cInvalidEntryIndex) { return cInvalidInterfaceSlot; }
            const u32 slot_index = base_index + index;
            return this->interface_slot_array[slot_index];
        }
    };
    static_assert(sizeof(ResBnshShaderInterfaceInfo) == 0x60);

    static constexpr u32 cMaxBnshShaderStageCount = 0x9;

    static constexpr u32 cMaxBnshShaderCodeTypeCount = 0x3;

    static constexpr u32 cMaxBnshInputAttributeCount  = 0x10;
    static constexpr u32 cMaxBnshOutputAttributeCount = 0x10;
    static constexpr u32 cMaxBnshTextureSamplerCount  = 0x20;
    static constexpr u32 cMaxBnshUniformBufferCount   = 0xe;
    static constexpr u32 cMaxBnshStorageBufferCount   = 0x10;
    static constexpr u32 cMaxBnshImageCount           = 0x8;
    static constexpr u32 cMaxBnshSeparateTextureCount = 0x80;
    static constexpr u32 cMaxBnshSeparateSamplerCount = 0x20;

    struct ResBnshShaderInterfaceInfoTableExt {
        /* Vp ext */
        union {
            ResBnshShaderInterfaceInfo *interface_info_array[3];
            struct {
                ResBnshShaderInterfaceInfo *interface_info_kernel;
                ResBnshShaderInterfaceInfo *interface_info_task;
                ResBnshShaderInterfaceInfo *interface_info_mesh;
            };
        };
        u64 reserve0;
    };
    struct ResBnshShaderInterfaceInfoTable {
        union {
            ResBnshShaderInterfaceInfo *interface_info_array[6];
            struct {
                ResBnshShaderInterfaceInfo *interface_info_vertex;
                ResBnshShaderInterfaceInfo *interface_info_tessellation_control;
                ResBnshShaderInterfaceInfo *interface_info_tessellation_evaluation;
                ResBnshShaderInterfaceInfo *interface_info_geometry;
                ResBnshShaderInterfaceInfo *interface_info_fragment;
                ResBnshShaderInterfaceInfo *interface_info_compute;
            };
        };
        /* Vp ext */
        ResBnshShaderInterfaceInfoTableExt *ext_table;
        u64 reserve0;

        ResBnshShaderInterfaceInfo *GetShaderInterfaceInfo(GfxShaderStage shader_stage) {
            vp::res::ResBnshShaderInterfaceInfo *interface_info;
            const u32 stage_index = static_cast<u32>(shader_stage);
            if (stage_index < 6) {
                 interface_info = this->interface_info_array[stage_index];
            } else {
                /* Non-canon extension for other shader stages (currently ifdefed out on nx) */
                interface_info = (this->ext_table != nullptr) ? this->ext_table->interface_info_array[stage_index-6] : nullptr;
            }
            return interface_info;
        }
    };
    static_assert(sizeof(ResBnshShaderInterfaceInfoTable) == 0x40);

    struct ResBnshShaderVariation;

    struct ResBnshShaderProgram {
        union {
            u8 option;
            struct {
                u8 reserve0               : 1;
                u8 is_nvn_program_ext_api : 1;
                u8 reserve1               : 6;
            };
        };
        u8                               code_type;
        u16                              reserve2;
        u32                              reserve3;
        union {
            ResBnshShaderCodeInfo *code_info_array[9];
            struct {
                ResBnshShaderCodeInfo *code_info_vertex;
                ResBnshShaderCodeInfo *code_info_tessellation_control;
                ResBnshShaderCodeInfo *code_info_tessellation_evaluation;
                ResBnshShaderCodeInfo *code_info_geometry;
                ResBnshShaderCodeInfo *code_info_fragment;
                ResBnshShaderCodeInfo *code_info_compute;
                union {
                    u64 reserve4[3];
                    struct {
                        ResBnshShaderCodeInfo  *code_info_kernel;
                        ResBnshShaderCodeInfo  *code_info_mesh;
                        ResBnshShaderCodeInfo  *code_info_task;
                    };
                };
            };
        };
        u64                              reserve5[2];
        u32                              size_of_runtime_shader;
        u32                              reserve6;
        void                            *runtime_shader;
        ResBnshShaderVariation          *parent_variation;
        ResBnshShaderInterfaceInfoTable *shader_interface_info_table;
        u64                              reserve7[4];

        static constexpr u32 cInvalidInterfaceSlot = ResBnshShaderInterfaceInfo::cInvalidInterfaceSlot;

        u32 GetInterfaceSlot(GfxShaderStage shader_stage, GfxShaderInterfaceType interface_type, const char *key) {
            const u32 stage_index = static_cast<u32>(shader_stage);
            if (cMaxBnshShaderStageCount <= stage_index) { return cInvalidInterfaceSlot; }
            vp::res::ResBnshShaderInterfaceInfoTable *interface_info_table = this->shader_interface_info_table;
            if (interface_info_table == nullptr) { return cInvalidInterfaceSlot; }
            vp::res::ResBnshShaderInterfaceInfo *interface_info = interface_info_table->GetShaderInterfaceInfo(shader_stage);
            if (interface_info == nullptr) { return cInvalidInterfaceSlot; }
            return interface_info->GetInterfaceSlot(interface_type, key);
        }

        ResBnshShaderCodeInfo *GetShaderCodeInfo(GfxShaderStage shader_stage) {
            const u32 stage_id = static_cast<u32>(shader_stage);
            VP_ASSERT(stage_id < vp::util::TEnumTraits<GfxShaderStage>::cEnumCount);
            return this->code_info_array[stage_id];
        }
    };
    static_assert(sizeof(ResBnshShaderProgram) == 0xa0);

    static constexpr u32 cDefaultBnshRuntimeShaderSize = 0x100;

    struct ResBnshShaderContainer;

    struct ResBnshShaderVariation {
        union {
            ResBnshShaderProgram *program_array[3];
            struct {                    
                ResBnshShaderProgram   *program_source;
                ResBnshShaderProgram   *program_intermediate;
                ResBnshShaderProgram   *program_binary;
            };
        };
        ResBnshShaderContainer *parent_container;
        u64                     reserve0[4];

        ResBnshShaderProgram *GetShaderProgram(GfxShaderCodeType shader_code_type) {
            switch (shader_code_type) {
                case GfxShaderCodeType::Binary:
                    return this->program_binary;
                case GfxShaderCodeType::Intermediate:
                    return this->program_intermediate;
                case GfxShaderCodeType::Source:
                    return this->program_source;
                default:
                    VP_UNEXPECTED_DEFAULT();
            };
            return nullptr;
        }
    };

    struct ResBnshShaderContainerData {
        ResGfxMemoryPoolInfo  memory_pool_info;
        u64                   reserve0;
        u64                   reserve1;
        void                 *runtime_memory_pool;
        u64                   reserve2[5];
    };
    static_assert(sizeof(ResBnshShaderContainerData) == 0x50);

    static constexpr u32 cDefaultBnshRuntimeMemoryPoolSize = 0x140;

    enum class BnshTargetApiType : u16 {
        Nvn = 4,
        /* Vp extensions */
        VpExtVulkan = 0x100,
    };

    struct ResBnshShaderContainer : public ResNintendoWareSubHeader {
        u16                         target_api_type;
        u16                         target_api_version;
        u8                          target_code_type;
        u8                          reserve0;
        u16                         reserve1;
        u32                         compiler_version;
        u32                         variation_count;
        ResBnshShaderVariation     *variation_array;
        ResBnshShaderContainerData *container_data;
        u16                         reserve2[4];
        u64                         reserve3[5];

        static constexpr u32 cMagic = util::TCharCode32("grsc");

        constexpr bool IsCodeBinary() const {
            return target_code_type == static_cast<u16>(GfxShaderCodeType::Binary);
        }
        constexpr bool IsCodeIntermediate() const {
            return target_code_type == static_cast<u16>(GfxShaderCodeType::Intermediate);
        }
        constexpr bool IsApiNvn() const {
            return target_api_type == static_cast<u16>(BnshTargetApiType::Nvn);
        }
        constexpr bool IsApiVpExtVulkan() const {
            return target_api_type == static_cast<u16>(BnshTargetApiType::VpExtVulkan);
        }
    };
    static_assert(sizeof(ResBnshShaderContainer) == 0x60);

    struct ResBnsh : public ResNintendoWareFileHeader {
        char reserve0[0x40];

        static constexpr u64 cMagic              = util::TCharCode32("BNSH");
        static constexpr u16 cTargetVersionMajor = 2;
        static constexpr u8  cTargetVersionMinor = 3;
        static constexpr u8  cTargetVersionMicro = 1;

        static ResBnsh *ResCast(void *file) {
            ResBnsh *bnsh = reinterpret_cast<ResBnsh*>(file);
            if (bnsh == nullptr || !IsValid(file)) { return nullptr; }
            bnsh->Relocate();
            return bnsh;
        }

        static bool IsValid(void *file) {
            ResBnsh *bnsh = reinterpret_cast<ResBnsh*>(file);
            return bnsh->ResNintendoWareFileHeader::IsValid(cMagic, cTargetVersionMajor, cTargetVersionMinor, cTargetVersionMicro);
        }
        
        ResBnshShaderContainer *GetContainer() {
            void *block = this->GetFirstBlock();
            return (block != nullptr) ? reinterpret_cast<ResBnshShaderContainer*>(block) : nullptr;
        }
    };
    static_assert(sizeof(ResBnsh) == 0x60);
}
