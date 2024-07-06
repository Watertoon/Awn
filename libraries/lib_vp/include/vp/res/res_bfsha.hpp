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

    struct ResBfshaShaderOption {
        const char                *name;
        ResNintendoWareDictionary *choice_dictionary;
        u32                       *choice_array;
        u16                        choice_count;
        u16                        default_value;
        u16                        reserve0;
        u8                         is_use_default_dynamic_key;
        u8                         dynamic_base_index;
        u32                        key_mask;
        u8                         key_index;
        u8                         key_bit_offset;
        u16                        reserve1;
    };
    static_assert(sizeof(ResBfshaShaderOption) == 0x28);

    struct ResBfshaInputAttribute {
        u8 location;
        u8 index;
    };
    static_assert(sizeof(ResBfshaInputAttribute) == 0x2);

    struct ResBfshaTextureSampler {
        const char *reserve0;
        u8          location;
        u8          reserve1;
        u16         reserve2;
        u32         reserve3;
    };
    static_assert(sizeof(ResBfshaTextureSampler) == 0x10);

    struct ResBfshaBufferParam {
        const char *name;
        u32         index;
        u16         offset;
        u16         buffer_index;
    };
    static_assert(sizeof(ResBfshaBufferParam) == 0x10);

    struct ResBfshaBuffer{
        ResBfshaBufferParam       *buffer_param_array;
        ResNintendoWareDictionary *buffer_param_dictionary;
        void                      *default_value_array;
        u8                         location;
        u8                         reserve0;
        u16                        size;
        u32                        buffer_param_count;
    };
    static_assert(sizeof(ResBfshaBuffer) == 0x20);

    struct ResBfshaShadingModel;

    struct ResBfshaShaderProgram {
        u32                    *sampler_location_array;
        u32                    *image_location_array;
        u32                    *uniform_buffer_location_array;
        u32                    *storage_buffer_location_array;
        ResBnshShaderVariation *bnsh_shader_variation;
        ResBfshaShadingModel   *parent_shading_model;
        u32                     reserve0;
        union {
            u16 runtime_flags;
            struct {
                u16 runtime_is_require_update     : 1;
                u16 runtime_is_fail_to_initialize : 1;
                u16 runtime_is_binary             : 1;
                u16 runtime_is_source             : 1;
                u16 runtime_is_intermediate       : 1;
                u16 reserve1                      : 11;
            };
        };
        u16                     static_key_size;
        u16                     reserve2;
        u16                     dynamic_key_size;
        u16                     reserve3;
        u16                     reserve4;
    };
    static_assert(sizeof(ResBfshaShaderProgram) == 0x40);

    struct ResBfshaResourceNameTable {
        const char **sampler_name_array;
        const char **image_name_array;
        const char **uniform_buffer_name_array;
        const char **storage_buffer_name_array;
    };
    static_assert(sizeof(ResBfshaResourceNameTable) == 0x20);

    struct ResBfshaShaderArchive;

    struct ResBfshaShadingModel {
        const char                *name;
        ResBfshaShaderOption      *static_shader_option_array;
        ResNintendoWareDictionary *static_shader_option_dictionary;
        ResBfshaShaderOption      *dynamic_shader_option_array;
        ResNintendoWareDictionary *dynamic_shader_option_dictionary;
        ResBfshaInputAttribute    *input_attribute_array;
        ResNintendoWareDictionary *input_attribute_dictionary;
        ResBfshaTextureSampler    *texture_sampler_array;
        ResNintendoWareDictionary *texture_sampler_dictionary;
        u64                       *image_array;
        ResNintendoWareDictionary *image_dictionary;
        ResBfshaBuffer            *uniform_buffer_array;
        ResNintendoWareDictionary *uniform_buffer_dictionary;
        ResBfshaBufferParam       *uniform_array;
        ResBfshaBuffer            *storage_buffer_array;
        ResNintendoWareDictionary *storage_buffer_dictionary;
        ResBfshaBufferParam       *storage_array;
        ResBfshaShaderProgram     *shader_program_array;
        u32                       *key_array;
        ResBfshaShaderArchive     *parent_archive;
        ResBfshaResourceNameTable *resource_name_table;
        ResBnsh                   *bnsh;
        void                      *runtime_mutex;
        u64                        reserve0;
        u64                        reserve1;
        void                      *runtime_user_pointer;
        u64                        reserve2;
        u64                        reserve3;
        u32                        default_key_index;
        u16                        static_option_count;
        u16                        dynamic_option_count;
        u16                        shader_program_count;
        u8                         static_key_size;
        u8                         dynamic_key_size;
        u8                         input_attribute_count;
        u8                         texture_sampler_count;
        u8                         image_count;
        u8                         uniform_buffer_count;
        u8                         material_uniform_buffer_index;
        u8                         reserve4;
        u8                         reserve5;
        u8                         option_uniform_buffer_index;
        u8                         storage_buffer_count;
        u8                         reserve6;
        u8                         reserve7;
        u8                         reserve8;
        u8                         option_storage_buffer_index;
        u8                         base_location_vertex;
        u8                         base_location_geoemtry;
        u8                         base_location_fragment;
        u8                         base_location_compute;
        u8                         base_location_tessellation_control;
        u8                         base_location_tessellation_evaluation;
        u8                         shader_stage_count;

        static constexpr u8 cInvalidBaseLocation = 0xff;
    };                     
    static_assert(sizeof(ResBfshaShadingModel) == 0x100);

    using BfshaProgramInitializeCallback = void(*)(ResBfshaShadingModel *shading_model, u32 program_index);

    struct ResBfshaShaderArchive {
        const char                     *name;
        const char                     *reserve0;
        ResBfshaShadingModel           *shading_model_array;
        ResNintendoWareDictionary      *shading_model_dictionary;
        void                           *runtime_user_pointer;
        BfshaProgramInitializeCallback  runtime_user_program_initialize_callback;
        void                           *runtime_user_work_memory;
        u64                             reserve1;
        u16                             shading_model_count;
        u16                             runtime_flags;
        u32                             reserve2;
    };
    static_assert(sizeof(ResBfshaShaderArchive) == 0x48);

    struct ResBfshaShaderFile : public ResNintendoWareFileHeader {
        ResBfshaShaderArchive     *shader_archive;
        ResNintendoWareStringPool *string_pool;
        u64                        string_pool_size;

        static constexpr inline u64 Magic = util::TCharCode64("FSHA    ");
    };
    static_assert(sizeof(ResBfshaShaderFile) == 0x38);
}
