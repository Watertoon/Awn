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
    
    enum class BfresRenderInfoDataType : u8 {
        S32    = 0x0,
        Float  = 0x1,
        String = 0x2,
    };

    struct ResBfresRenderInfo {
        const char *name;
        u8          data_type;
        u8          reserve0;
        u16         reserve1;
        u32         reserve2;
    };
    static_assert(sizeof(ResBfresRenderInfo) == 0x10);

    enum class BfresShaderParamDataType : u8 {
        Bool = 0x0,
        Bool2 = 0x1,
        Bool3 = 0x2,
        Bool4 = 0x3,
        S32 = 0x4,
        Int2 = 0x5,
        Int3 = 0x6,
        Int4 = 0x7,
        U32 = 0x8,
        UInt2 = 0x9,
        UInt3 = 0xa,
        UInt4 = 0xb,
        Float = 0xc,
        Float2 = 0xd,
        Float3 = 0xe,
        Float4 = 0xf,
        FloatColumnMajor2x2 = 0x11,
        FloatColumnMajor2x3 = 0x12,
        FloatColumnMajor2x4 = 0x13,
        FloatColumnMajor3x2 = 0x15,
        FloatColumnMajor3x3 = 0x16,
        FloatColumnMajor3x4 = 0x17,
        FloatColumnMajor4x2 = 0x19,
        FloatColumnMajor4x3 = 0x1a,
        FloatColumnMajor4x4 = 0x1b,
        Srt2d = 0x1c,
        Srt3d = 0x1d,
        SrtTexture = 0x1e,
        //SrtTextureWithUserPointer = 0x1f,
    };

    struct ResBfresSrt2d {
        util::Float2 scale;
        float rotate;
        util::Float2 translate;

        constexpr void ConvertSrt2d(float *out_matrix) const {
            const vp::util::Float2 sincos = vp::util::SampleSinCos(this->rotate);
            /* 
             * | sr sr t |
             * | sr sr t |
             */
            out_matrix[0] = sincos[1] * this->scale[0];
            out_matrix[1] = sincos[0] * this->scale[0];
            out_matrix[2] = sincos[0] * -this->scale[1];
            out_matrix[3] = sincos[1] * this->scale[1];
            out_matrix[4] = this->translate[0];
            out_matrix[5] = this->translate[1];
            return;
        }
    };

    struct ResBfresSrt3d {
        util::Float3 scale;
        util::Float3 rotate;
        util::Float3 translate;

        constexpr void ConvertSrt3d(vp::util::FloatColumnMajor4x3 *out_matrix) const {
            const vp::util::MatrixRowMajor4x3f matrix = vp::util::SRTEulerXYZToMatrix<vp::util::MatrixRowMajor4x3f>(this->scale, vp::util::Convert<vp::util::Vector3f>(this->rotate), this->translate);
            *out_matrix = vp::util::ConvertMajor<vp::util::FloatColumnMajor4x3>(matrix);
        }
    };

    enum class BfresSrtMode : u32 {
        Maya      = 0x0,
        Max       = 0x1,
        //SoftImage = 0x2, /* Deprecated */
    };

    struct ResBfresTextureSrt {
        BfresSrtMode srt_mode;
        util::Float2 scale;
        float        rotate;
        util::Float2 translate;

        constexpr void ConvertTextureSrt(float *out_matrix) const {
            const vp::util::Float2 sincos = vp::util::SampleSinCos(this->rotate);
            const float sin = sincos[0];
            const float cos = sincos[1];
            if (this->srt_mode == BfresSrtMode::Maya) {
                /* 
                 * | sr sr t |
                 * | sr sr t |
                 */
                out_matrix[0] = cos * this->scale[0];
                out_matrix[1] = sin * -this->scale[1];
                out_matrix[2] = sin * this->scale[0];
                out_matrix[3] = cos * this->scale[1];
                out_matrix[4] = ((cos * -0.5f - (sin * 0.5f + -0.5f)) - this->translate[0]) * this->scale[0];
                out_matrix[5] = ((cos * -0.5f + (sin * 0.5f + -0.5f)) + this->translate[1]) * this->scale[1];
            } else if (this->srt_mode == BfresSrtMode::Max) {
                /* 
                 * | sr sr t |
                 * | sr sr t |
                 */
                out_matrix[0] = cos * this->scale[0];
                out_matrix[1] = sin * -this->scale[1];
                out_matrix[2] = sin * this->scale[0];
                out_matrix[3] = cos * this->scale[1];
                out_matrix[4] = ((sin * this->scale[0]) * (this->translate[1] + -0.5f) - (cos * this->scale[0]) * (this->translate[0] + 0.5f)) + 0.5f;
                out_matrix[5] = ((sin * this->scale[1]) * (this->translate[0] + 0.5f) + (cos * this->scale[1]) * (this->translate[1] + -0.5f)) + 0.5f;
            } else {
                VP_UNEXPECTED_DEFAULT();
            }
        }
    };

    constexpr u16 cBfresShaderParamMaterialSizeTable[0x1f] = {
        sizeof(u32) * 1,
        sizeof(u32) * 2,
        sizeof(u32) * 3,
        sizeof(u32) * 4,
        sizeof(s32) * 1,
        sizeof(s32) * 2,
        sizeof(s32) * 3,
        sizeof(s32) * 4,
        sizeof(u32) * 1,
        sizeof(u32) * 2,
        sizeof(u32) * 3,
        sizeof(u32) * 4,
        sizeof(float) * 1,
        sizeof(float) * 2,
        sizeof(float) * 3,
        sizeof(float) * 4,
        0,
        sizeof(float) * 2 * 2,
        sizeof(float) * 2 * 3,
        sizeof(float) * 2 * 4,
        0,
        sizeof(float) * 3 * 2,
        sizeof(float) * 3 * 3,
        sizeof(float) * 3 * 4,
        0,
        sizeof(float) * 4 * 2,
        sizeof(float) * 4 * 3,
        sizeof(float) * 4 * 4,
        sizeof(ResBfresSrt2d),
        sizeof(ResBfresSrt3d),
        sizeof(ResBfresTextureSrt),
    };

    constexpr u16 GetShaderParamMaterialSize(BfresShaderParamDataType data_type) {
        const u32 type = static_cast<u32>(data_type);
        return cBfresShaderParamMaterialSizeTable[type];
    }

    struct ResBfresShaderParam;

    static constexpr u32 cMaxShaderParamConvertSize = 0x40;
    using BfresShaderParamConvertFunction = u32 (*)(void*, const void*, ResBfresShaderParam*, void*);

    struct ResBfresShaderParam {
        BfresShaderParamConvertFunction  runtime_convert_shader_param_callback;
        const char                      *name;
        u16                              data_offset;
        u8                               data_type;
        u8                               reserve0;
        u32                              reserve1;

        void ConvertShaderParam(void *output, void *input) {

            /* Convert callback */
            if (this->runtime_convert_shader_param_callback != nullptr) {
                u64 conv[8];
                static_assert(sizeof(conv) == cMaxShaderParamConvertSize);
                const u32 size = (this->runtime_convert_shader_param_callback)(conv, input, this, nullptr);
                ::memcpy(output, conv, size);
                return;
            }

            /* Copy vector or matrix */
            const u32 type = this->data_type;
            if (type < 0x10) {
                const u32 size = sizeof(float) + (type & 3) * sizeof(float);
                ::memcpy(output, input, size);
            } else if (type < 0x1c) {
                const u32 count = (type - 0x10);
                const u32 size = sizeof(float) + (type & 3) * sizeof(float);
                uintptr_t out_iter = reinterpret_cast<uintptr_t>(output);
                uintptr_t in_iter = reinterpret_cast<uintptr_t>(input);
                ::memcpy(output, input, size);
                out_iter += sizeof(vp::util::Float4);
                in_iter += size;
                ::memcpy(reinterpret_cast<void*>(out_iter), reinterpret_cast<void*>(in_iter), size);
                if (3 < count) {
                    out_iter += sizeof(vp::util::Float4);
                    in_iter += size;
                    ::memcpy(reinterpret_cast<void*>(out_iter), reinterpret_cast<void*>(in_iter), size);
                    if ((count & (~3)) != 4) {                        
                        out_iter += sizeof(vp::util::Float4);
                        in_iter += size;
                        ::memcpy(reinterpret_cast<void*>(out_iter), reinterpret_cast<void*>(in_iter), size);
                    }
                }
            }

            return;
        }
    };
    static_assert(sizeof(ResBfresShaderParam) == 0x18);

    struct ResBfresShaderReflection {
        const char                *shader_archive_name;
        const char                *shader_name;
        ResBfresRenderInfo        *render_info_array;
        ResNintendoWareDictionary *render_info_dictionary;
        ResBfresShaderParam       *shader_param_array;
        ResNintendoWareDictionary *shader_param_dictionary;
        ResNintendoWareDictionary *vertex_attribute_dictionary;
        ResNintendoWareDictionary *sampler_dictionary;
        ResNintendoWareDictionary *static_shader_option_dictionary;
        u16                        render_info_count;
        u16                        shader_param_count;
        u16                        shader_param_data_size;
        u16                        reserve0;
        u64                        reserve1;
    };
    static_assert(sizeof(ResBfresShaderReflection) == 0x58);

    struct ResBfresStaticShaderVariation {
        ResBfresShaderReflection  *shader_reflection;
        const char               **vertex_attribute_name_array;
        u8                        *vertex_attribute_remap_array;
        const char               **sampler_name_array;
        u8                        *sampler_remap_array;
        u32                       *static_shader_option_packed_bool_value_array;
        const char               **static_shader_option_string_value_array;
        u16                       *static_option_remap_array;
        u32                        reserve0;
        u8                         vertex_attribute_count;
        u8                         sampler_count;
        u16                        static_option_count_bool;
        u16                        total_static_option_count;
        u16                        reserve1;
        u32                        reserve2;
    };
    static_assert(sizeof(ResBfresStaticShaderVariation) == 0x50);

    struct ResBfresMaterial {
        u32                            magic;
        union {
            u32 flags;
            struct {
                u32 is_visible : 1;
                u32 reserve0 : 31;
            };
        };
        const char                     *material_name;
        ResBfresStaticShaderVariation  *static_shader_variation;
        void                          **runtime_texture_view_array;
        const char                    **texture_name_array;
        void                           *runtime_sampler_array;
        ResGfxSamplerInfo              *sampler_info_array;
        ResNintendoWareDictionary      *sampler_dictionary;
        void                           *render_info_value_array;
        u16                            *render_info_value_count_array;
        u16                            *render_info_value_offset_array;
        void                           *shader_param_value_array;
        s32                            *runtime_shader_param_material_buffer_offset_array;
        void                           *reserve1;
        ResGfxUserData                 *user_data_array;
        ResNintendoWareDictionary      *user_data_dictionary;
        u32                            *shader_param_is_convert_bit_array;
        void                           *runtime_user_pointer;
        u64                            *runtime_sampler_descriptor_slot_array;
        u64                            *runtime_texture_descriptor_slot_array;
        u16                             section_index;
        u8                              sampler_count;
        u8                              texture_count;
        u16                             is_force_calculate_material;
        u16                             user_data_count;
        u16                             render_info_value_size;
        u16                             runtime_material_buffer_size;
        u32                             reserve2;

        static constexpr u32    cMagic = util::TCharCode32("FMAT");
        static constexpr u16 cInvalidMaterialBufferOffset = 0xffff;
        static constexpr u16 cInvalidMaterialBufferSize = 0xffff;
        static constexpr size_t cInvalidDescriptorSlot = 0xffff'ffff'ffff'ffff;
        static constexpr u32 cInvalidEntryIndex = 0xffff'ffff;

        void CalculateMaterialBufferOffsets(const vp::res::ResBfshaShadingModel *shading_model) {

            /* Invalid if shader does not use material buffer */
            if (shading_model->material_uniform_buffer_index == vp::res::ResBfshaShadingModel::cInvalidBufferIndex) {
                this->runtime_material_buffer_size = cInvalidMaterialBufferSize;
                return;
            }

            /* Get size */
            const vp::res::ResBfshaBuffer *material_buffer = std::addressof(shading_model->uniform_buffer_array[shading_model->material_uniform_buffer_index]);
            this->runtime_material_buffer_size = material_buffer->size;

            /* Relocate structure offsets */
            const ResBfresStaticShaderVariation *static_variation = this->static_shader_variation;
            if (static_variation == nullptr) { return; }
            const ResBfresShaderReflection *reflection = static_variation->shader_reflection;
            if (reflection == nullptr) { return; }
            const u32 param_count = reflection->shader_param_count;
            const vp::res::ResBfresShaderParam *shader_param_array = reflection->shader_param_array;
            const vp::res::ResNintendoWareDictionary *uniform_dictionary = material_buffer->param_dictionary;
            const vp::res::ResBfshaBufferParam *uniform_array = material_buffer->param_array;
            if (uniform_dictionary != nullptr && uniform_array != nullptr) {
                for (u32 i = 0; i < param_count; ++i) {
                    const u32 entry_index = uniform_dictionary->TryGetEntryIndexByKey(shader_param_array[i].name + 2);
                    this->runtime_shader_param_material_buffer_offset_array[i] = (entry_index != cInvalidEntryIndex) ? uniform_array[entry_index].struct_offset : cInvalidMaterialBufferOffset;
                }
            } else {                
                for (u32 i = 0; i < param_count; ++i) {
                    this->runtime_shader_param_material_buffer_offset_array[i] = cInvalidMaterialBufferOffset;
                }
            }

            return;
        }

        void BindTextures(GfxBindTextureCallback bind_callback, void *user_data) {
            for (u32 i = 0; i < sampler_count; ++i) {
                if (this->runtime_texture_view_array[i] != nullptr && this->runtime_texture_descriptor_slot_array[i] != cInvalidDescriptorSlot) { continue; }

                GfxBindTextureView bind_texture_view = (bind_callback)(texture_name_array[i] + 2, user_data);
                this->runtime_texture_descriptor_slot_array[i] = bind_texture_view.texture_view_decriptor_slot;
                this->runtime_texture_view_array[i] = bind_texture_view.texture_view;
            }
            return;
        }

        constexpr void ReleaseTextures() {
            for (u32 i = 0; i < sampler_count; ++i) {
                this->runtime_texture_descriptor_slot_array[i] = cInvalidDescriptorSlot;
                this->runtime_texture_view_array[i] = nullptr;
            }
            return;
        }

        constexpr bool TrySetTextureByName(const char *name, GfxBindTextureView *bind_texture_view) {

            bool had_success = false;
            for (u32 i = 0; i < sampler_count; ++i) {
                if (!vp::util::StringCompare(texture_name_array[i], name)) { continue; }

                this->runtime_texture_descriptor_slot_array[i] = bind_texture_view->texture_view_decriptor_slot;
                this->runtime_texture_view_array[i] = bind_texture_view->texture_view;

                had_success = true;
            }

            return had_success;
        }

        u32 TryGetStaticShaderOptionChoiceIndex(const char *option_name) const {

            /* Try lookup option in shader reflection */
            const ResBfresStaticShaderVariation *variation = this->static_shader_variation;
            if (variation == nullptr) { return ResNintendoWareDictionary::cInvalidEntryIndex; }
            const ResBfresShaderReflection *reflection = variation->shader_reflection;
            if (reflection == nullptr) { return ResNintendoWareDictionary::cInvalidEntryIndex; }
            const ResNintendoWareDictionary *option_dic = reflection->static_shader_option_dictionary;
            if (option_dic == nullptr) { return ResNintendoWareDictionary::cInvalidEntryIndex; }
            const u32 dic_index = option_dic->TryGetEntryIndexByKey(option_name);
            if (dic_index == ResNintendoWareDictionary::cInvalidEntryIndex) { return dic_index; }

            /* Remap index if necessary */
            const u32 base_index = variation->total_static_option_count + dic_index;
            const u16 *remap_array = variation->static_option_remap_array;
            const u32 option_index = (remap_array != nullptr) ? remap_array[base_index] : dic_index;
            return option_index;
        }

        constexpr bool IsStaticShaderOptionBool(u32 option_index) const {
            const ResBfresStaticShaderVariation *variation = this->static_shader_variation;
            const bool is_value = (variation->static_option_count_bool > option_index);
            return is_value;
        }

        constexpr bool IsStaticShaderOptionString(u32 option_index) const {
            const ResBfresStaticShaderVariation *variation = this->static_shader_variation;
            const bool is_value = (variation->static_option_count_bool < option_index);
            return is_value;
        }

        constexpr u32 TryGetStaticShaderOptionBool(u32 option_index) const {
            const ResBfresStaticShaderVariation *variation = this->static_shader_variation;
            const u32 value = (variation->static_option_count_bool <= option_index) ? -1 : ((variation->static_shader_option_packed_bool_value_array[(option_index / 32)] & 0x7fff'ffff) >> (option_index & 0x1f)) & 1;
            return value;
        }

        constexpr const char *TryGetStaticShaderOptionString(u32 option_index) const {
            const ResBfresStaticShaderVariation *variation = this->static_shader_variation;
            const char *value = (option_index < variation->static_option_count_bool || variation->total_static_option_count <= option_index) ? nullptr : variation->static_shader_option_string_value_array[(option_index - variation->static_option_count_bool)];
            return value;
        }

        const char *TryGetShaderVertexAttributeName(const char *material_attribute_name) const {
            const ResBfresStaticShaderVariation *variation = this->static_shader_variation;
            if (variation == nullptr) { return nullptr; }
            const ResBfresShaderReflection *reflection = variation->shader_reflection;
            if (reflection == nullptr) { return nullptr; }
            for (u32 i = 0; i < variation->vertex_attribute_count; ++i) {
                if (!vp::util::StringCompare(variation->vertex_attribute_name_array[i] + 2, material_attribute_name)) { continue; }
                const u32 entry_index = (variation->vertex_attribute_remap_array != nullptr) ? variation->vertex_attribute_remap_array[i] : i;
                return reflection->vertex_attribute_dictionary->GetKeyByEntryIndex(entry_index);
            }
            return nullptr;
        }

        const char *TryGetShaderSamplerName(const char *material_sampler_name) const {
            const ResBfresStaticShaderVariation *variation = this->static_shader_variation;
            if (variation == nullptr) { return nullptr; }
            const ResBfresShaderReflection *reflection = variation->shader_reflection;
            if (reflection == nullptr) { return nullptr; }
            for (u32 i = 0; i < variation->sampler_count; ++i) {
                if (!vp::util::StringCompare(variation->sampler_name_array[i] + 2, material_sampler_name)) { continue; }
                const u32 entry_index = (variation->sampler_remap_array != nullptr) ? variation->sampler_remap_array[i] : i;
                return reflection->sampler_dictionary->GetKeyByEntryIndex(entry_index);
            }
            return nullptr; 
        }

        const char *TryGetTextureName(const char *sampler_name) const {
            const u32 entry_index = this->sampler_dictionary->TryGetEntryIndexByKey(sampler_name);
            return (entry_index != 0xffff'ffff) ? texture_name_array[entry_index] : nullptr;
        }

        ResGfxSamplerInfo *TryGetSamplerInfo(const char *sampler_name) {
            if (this->sampler_dictionary == nullptr) { return nullptr; }
            const u32 entry_id = this->sampler_dictionary->TryGetEntryIndexByKey(sampler_name);
            if (entry_id == ResNintendoWareDictionary::cInvalidEntryIndex) { return nullptr; }
            return std::addressof(this->sampler_info_array[entry_id]);
        }

        ResGfxUserData *TryGetUserData(const char *user_data_name) {
            if (this->user_data_dictionary == nullptr) { return nullptr; }
            const u32 entry_id = this->user_data_dictionary->TryGetEntryIndexByKey(user_data_name);
            if (entry_id == ResNintendoWareDictionary::cInvalidEntryIndex) { return nullptr; }
            return std::addressof(this->user_data_array[entry_id]);
        }
    };
    static_assert(sizeof(ResBfresMaterial) == 0xb0);
}
