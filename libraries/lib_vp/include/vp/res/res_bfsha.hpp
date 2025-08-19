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
        s32                       *choice_array;
        u16                        choice_count;
        u16                        default_choice_index;
        u16                        struct_index;
        union {
            u8                     flags;
            struct {
                u8 is_applied : 1;
                u8 is_static_option : 1;
                u8 reserve0 : 6;
            };
        };
        u8                         base_key_index_dynamic;
        u32                        key_mask;
        u8                         key_index;
        u8                         key_bit_offset;
        u16                        reserve1;

        static constexpr u16 cInvalidBaseIndex = 0;

        u32 ReadStaticKey(const void *static_key) const;
        void WriteStaticKey(void *static_key, u32 choice_index) const;

        u32 ReadDynamicKey(const void *dynamic_key) const;
        void WriteDynamicKey(void *dynamic_key, u32 choice_index) const;
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

    struct ResBfshaImage {
        u8          location;
        u8          reserve0;
        u16         reserve1;
        u32         reserve2;
    };
    static_assert(sizeof(ResBfshaImage) == 0x8);

    struct ResBfshaBufferParam {
        const char *convert_name;
        u32         index;
        u16         struct_offset;
        u16         buffer_index;
    };
    static_assert(sizeof(ResBfshaBufferParam) == 0x10);

    enum class BfshaBufferType : u8 {
        User = 0,
        Material = 1,
        Shape = 2,
        Skeleton = 3,
        Option = 4,
    };
    
    VP_ENUM_TRAITSV(
        vp::res::BfshaBufferType, BfshaBufferType,
        User,
        Material,
        Shape,
        Skeleton,
        Option
    );

    struct ResBfshaBuffer{
        ResBfshaBufferParam       *param_array;
        ResNintendoWareDictionary *param_dictionary;
        void                      *default_value_array;
        u8                         location;
        u8                         buffer_type;
        u16                        size;
        u32                        param_count;
    };
    static_assert(sizeof(ResBfshaBuffer) == 0x20);

    struct ResBfshaShadingModel;

    struct ResBfshaShaderProgram {
        u32                    *location_array_texture_sampler;
        u32                    *location_array_image;
        u32                    *location_array_uniform_buffer;
        u32                    *location_array_storage_buffer;
        ResBnshShaderVariation *bnsh_shader_variation;
        ResBfshaShadingModel   *parent_shading_model;
        u32                     reserve0; /* Unknown */
        union {
            u16 runtime_flags;
            struct {
                u16 runtime_is_setup              : 1;
                u16 runtime_is_any                : 1;
                u16 runtime_is_binary             : 1;
                u16 runtime_is_source             : 1;
                u16 runtime_is_intermediate       : 1;
                u16 reserve1                      : 11;
            };
        };
        u16                     texture_sampler_count;
        u16                     image_count;
        u16                     uniform_buffer_count;
        u16                     storage_buffer_count;
        u16                     reserve2;
    };
    static_assert(sizeof(ResBfshaShaderProgram) == 0x40);

    struct ResBfshaResourceNameTable {
        const char **texture_sampler_name_array;
        const char **image_name_array;
        const char **uniform_buffer_name_array;
        const char **storage_buffer_name_array;
    };
    static_assert(sizeof(ResBfshaResourceNameTable) == 0x20);

    struct ResBfshaShaderArchive;

    struct BfshaShaderRange {
        void *start_key;
        void *end_key;
    };

    struct ResBfshaShadingModel {
        const char                *name;
        ResBfshaShaderOption      *static_option_array;
        ResNintendoWareDictionary *static_option_dictionary;
        ResBfshaShaderOption      *dynamic_option_array;
        ResNintendoWareDictionary *dynamic_option_dictionary;
        ResBfshaInputAttribute    *input_attribute_array;
        ResNintendoWareDictionary *input_attribute_dictionary;
        ResBfshaTextureSampler    *texture_sampler_array;
        ResNintendoWareDictionary *texture_sampler_dictionary;
        ResBfshaImage             *image_array;
        ResNintendoWareDictionary *image_dictionary;
        ResBfshaBuffer            *uniform_buffer_array;
        ResNintendoWareDictionary *uniform_buffer_dictionary;
        ResBfshaBufferParam       *uniform_param_array;
        ResBfshaBuffer            *storage_buffer_array;
        ResNintendoWareDictionary *storage_buffer_dictionary;
        ResBfshaBufferParam       *storage_param_array;
        ResBfshaShaderProgram     *program_array;
        u32                       *option_key_array;
        ResBfshaShaderArchive     *parent_archive;
        ResBfshaResourceNameTable *resource_name_table;
        ResBnsh                   *bnsh;
        void                      *runtime_mutex;
        u8                        *base_location_array;
        u8                         base_location_count;
        u8                         reserve0;
        u16                        reserve1;
        u32                        reserve2;
        void                      *runtime_user_pointer;
        u64                        reserve3;
        u32                        uniform_param_count;
        u32                        storage_param_count;
        u32                        default_program_index;
        u16                        static_option_count;
        u16                        dynamic_option_count;
        u16                        program_count;
        u8                         static_key_size;
        u8                         dynamic_key_size;
        u8                         input_attribute_count;
        u8                         texture_sampler_count;
        u8                         image_count;
        u8                         uniform_buffer_count;
        u8                         material_uniform_buffer_index;
        u8                         shape_uniform_buffer_index;
        u8                         skeleton_uniform_buffer_index;
        u8                         option_uniform_buffer_index;
        u8                         storage_buffer_count;
        u8                         material_storage_buffer_index;
        u8                         shape_storage_buffer_index;
        u8                         skeleton_storage_buffer_index;
        u8                         option_storage_buffer_index;
        u8                         base_location_array_old[6]; /* Vertex, Geometry, Fragment, Compute, Tess Control, Tess Eval, use if base_location_array is nullptr */
        u8                         shader_stage_count;

        static constexpr u32 cInvalidEntryIndex = 0xffff'ffff;
        static_assert(cInvalidEntryIndex == ResNintendoWareDictionary::cInvalidEntryIndex);
        static constexpr u32 cInvalidDefaultKeyIndex = 0xffff'ffff;
        static constexpr u8 cInvalidBufferIndex = 0xff;
        static constexpr u8 cInvalidBaseLocation = 0xff;

        constexpr bool IsAppliedStaticOption() const {
            for (u32 i = 0; i < this->static_option_count; ++i) {
                if (this->static_option_array[i].is_applied) { return true; }
            }
            return false;
        }

        constexpr u32 GetAppliedStaticOptionCount() const {
            u32 count = 0;
            for (u32 i = 0; i < this->static_option_count; ++i) {
                if (!this->static_option_array[i].is_applied) { continue; }
                ++count;
            }
            return count;
        }

        u32 FindStaticShaderOptionIndex(const char *option_name) const;
        u32 FindDynamicShaderOptionIndex(const char *option_name) const;
        u32 FindInputAttributeIndex(const char *attribute_name) const;
        u32 FindTextureSamplerIndex(const char *texture_sampler_name) const;
        u32 FindImageIndex(const char *image_name) const;
        u32 FindUniformBufferIndex(const char *uniform_buffer_name) const;
        u32 FindStorageBufferIndex(const char *storage_buffer_name) const;

        bool FindProgramRange(BfshaShaderRange *out_range, const void *static_key) const;
        u32 FindProgramIndex(const BfshaShaderRange *range, const void *dynamic_key) const;

        void GetDefaultStaticKey(void *out_default_static_key) const;
        void GetDefaultDynamicKey(void *out_default_dynamic_key) const;
        void GetInvalidDynamicKey(void *out_invalid_dynamic_key) const;
    };                     
    static_assert(sizeof(ResBfshaShadingModel) == 0x100);

    using BfshaProgramInitializeCallback = Result(*)(ResBfshaShadingModel *shading_model, u32 program_index);

    struct ResBfshaShaderArchive {
        const char                     *name;
        const char                     *reserve0;
        ResBfshaShadingModel           *shading_model_array;
        ResNintendoWareDictionary      *shading_model_dictionary;
        void                           *runtime_user_pointer;
        BfshaProgramInitializeCallback  runtime_program_initialize_callback;
        void                           *runtime_work_memory;
        u64                             reserve1;
        u16                             shading_model_count;
        union {
            u16 flags;
            struct {
                u16 reserve2 : 1;
                u16 is_reserve : 1; /* Unknown */
                u16 reserve3 : 1;
                u16 is_binary_available : 1;
                u16 is_baked_options : 1;
                u16 reserve5 : 2;
                u16 runtime_is_setup : 1;
                u16 reserve6 : 8;
            };
        };
        u32                             reserve7;
    };
    static_assert(sizeof(ResBfshaShaderArchive) == 0x48);

    struct ResBfsha : public ResNintendoWareFileHeader {
        ResBfshaShaderArchive     *shader_archive;
        ResNintendoWareStringPool *string_pool;
        u64                        string_pool_size;

        static constexpr inline u64 cMagic = util::TCharCode64("FSHA    ");
        static constexpr inline u32 cTargetVersionMajor = 9;
        static constexpr inline u32 cTargetVersionMinor = 1;
        static constexpr inline u32 cTargetVersionMicro = 1;

        ResBfsha *ResCast(void *file) {
            ResBfsha *bfsha = reinterpret_cast<ResBfsha*>(file);
            if (bfsha == nullptr || !IsValid(file)) { return nullptr; }
            bfsha->Relocate();
            for (u32 i = 0; i < bfsha->shader_archive->shading_model_count; ++i) {
                ResBnsh *bnsh = ResBnsh::ResCast(bfsha->shader_archive->shading_model_array[i].bnsh);
                if (bnsh == nullptr) { return nullptr; }
            }
            return bfsha;
        }

        static bool IsValid(void *file) {
            ResBfsha *bfsha = reinterpret_cast<ResBfsha*>(file);
            return bfsha->ResNintendoWareFileHeader::IsValid(cMagic, cTargetVersionMajor, cTargetVersionMinor, cTargetVersionMicro);
        }
    };
    static_assert(sizeof(ResBfsha) == 0x38);
}
VP_ENUM_TRAITS_AUTO(vp::res::BfshaBufferType);
