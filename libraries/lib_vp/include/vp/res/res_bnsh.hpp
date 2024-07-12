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
        ResNintendoWareDictionary *separate_texture_dictionary;
        ResNintendoWareDictionary *separate_sampler_dictionary;
        u32                        base_index_separate_texture;
        u32                        base_index_separate_sampler;
        u64                        reserve0[3];
    };
    static_assert(sizeof(ResBnshExtendedInterfaceInfo) == 0x30);

    struct ResBnshShaderInterfaceInfo {
        ResNintendoWareDictionary    *input_attribute_dictionary;
        ResNintendoWareDictionary    *output_attribute_dictionary;
        ResNintendoWareDictionary    *texture_sampler_dictionary;
        ResNintendoWareDictionary    *uniform_buffer_dictionary;
        ResNintendoWareDictionary    *storage_buffer_dictionary;
        u32                           base_index_output_attribute;
        u32                           base_index_texture_sampler;
        u32                           base_index_uniform_buffer;
        u32                           base_index_storage_buffer;
        u32                          *interface_slot_array;
        s32                           work_group_count_x;
        s32                           work_group_count_y;
        s32                           work_group_count_z;
        u32                           base_index_image;
        ResNintendoWareDictionary    *image_dictionary;
        ResBnshExtendedInterfaceInfo *extended_interface_info;
    };
    static_assert(sizeof(ResBnshShaderInterfaceInfo) == 0x60);

    enum class GfxShaderStage : u32 {
        Vertex                 = 0,
        TessellationControl    = 1,
        TessellationEvaluation = 2,
        Geometry               = 3,
        Fragment               = 4,
        Compute                = 5,
        /* Vp ext */
        Task                   = 6,
        Mesh                   = 7,
    };

    static constexpr u32 cMaxBnshShaderStageCount = 0x8;

    enum class GfxShaderCodeType : u8 {
        Binary         = 0x0,
        Intermediate   = 0x1,
        Source         = 0x2,
    };

    static constexpr u32 cMaxBnshShaderCodeTypeCount = 0x3;

    enum class GfxShaderInterfaceType : u32 {
        InputAttribute  = 0x0,
        OutputAttribute = 0x1,
        TextureSampler  = 0x2,
        UniformBuffer   = 0x3,
        StorageBuffer   = 0x4,
        Image           = 0x5,
        SeparateTexture = 0x6,
        SeparateSampler = 0x7,
    };

    static constexpr u32 cMaxBnshInputAttributeCount  = 0x10;
    static constexpr u32 cMaxBnshOutputAttributeCount = 0x10;
    static constexpr u32 cMaxBnshTextureSamplerCount  = 0x20;
    static constexpr u32 cMaxBnshUniformBufferCount   = 0xe;
    static constexpr u32 cMaxBnshStorageBufferCount   = 0x10;
    static constexpr u32 cMaxBnshImageCount           = 0x8;
    static constexpr u32 cMaxBnshSeparateTextureCount = 0x80;
    static constexpr u32 cMaxBnshSeparateSamplerCount = 0x20;

    struct ResBnshShaderInterfaceInfoTable {
        ResBnshShaderInterfaceInfo *interface_info_vertex;
        ResBnshShaderInterfaceInfo *interface_info_tessellation_evaluation;
        ResBnshShaderInterfaceInfo *interface_info_tessellation_control;
        ResBnshShaderInterfaceInfo *interface_info_geometry;
        ResBnshShaderInterfaceInfo *interface_info_fragment;
        ResBnshShaderInterfaceInfo *interface_info_compute;
        union {
            u64 reserve0[2];
            struct {
                /* Vp extensions */
                ResBnshShaderInterfaceInfo *interface_info_task;
                ResBnshShaderInterfaceInfo *interface_info_mesh;
            };
        };
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
        ResBnshShaderCodeInfo           *code_info_vertex;
        ResBnshShaderCodeInfo           *code_info_tessellation_control;
        ResBnshShaderCodeInfo           *code_info_tessellation_evaluation;
        ResBnshShaderCodeInfo           *code_info_geometry;
        ResBnshShaderCodeInfo           *code_info_fragment;
        ResBnshShaderCodeInfo           *code_info_compute;
        union {
            u64                          reserve4[5];
            struct {
                /* Vp extensions */
                ResBnshShaderCodeInfo  *code_info_task;
                ResBnshShaderCodeInfo  *code_info_mesh;
                u64                     reserve5[3];
            };
        };
        u32                              size_of_runtime_shader;
        u32                              reserve6;
        void                            *runtime_shader;
        ResBnshShaderVariation          *parent_variation;
        ResBnshShaderInterfaceInfoTable *shader_interface_info_table;
        u64                              reserve7[4];
    };
    static_assert(sizeof(ResBnshShaderProgram) == 0xa0);

    static constexpr u32 cDefaultBnshRuntimeShaderSize = 0x100;

    struct ResBnshShaderContainer;
    
    struct ResBnshShaderVariation {
        ResBnshShaderProgram   *program_source;
        ResBnshShaderProgram   *program_intermediate;
        ResBnshShaderProgram   *program_binary;
        ResBnshShaderContainer *parent_container;
        u64                     reserve0[4];
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
        Nvn         = 4,
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
        u32                         shader_variation_count;
        ResBnshShaderVariation     *shader_variation_array;
        ResBnshShaderContainerData *shader_container_data;
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
        static constexpr u8  cTargetVersionMinor = 2;
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
    };
    static_assert(sizeof(ResBnsh) == 0x60);
}
