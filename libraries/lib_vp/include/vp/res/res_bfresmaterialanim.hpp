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

    struct ResBfresShaderParamAnim {
        const NWString8 *shader_param_name;
        u16              base_curve;
        u16              curve_count_float;
        u16              curve_count_int;
        u16              base_constant;
        u16              constant_count;
        u16              fast_bind_shader_param_id;
        u32              reserve0;
    };
    static_assert(sizeof(ResBfresShaderParamAnim) == 0x18);

    struct ResBfresTexturePatternAnim {
        const NWString8 *sampler_name;
        u16              base_curve;
        u16              base_constant;
        u8               fast_bind_sampler_id;
        u8               reserve0;
        u16              reserve1;
    };
    static_assert(sizeof(ResBfresTexturePatternAnim) == 0x10);

    struct ResBfresConstantEntry {
        u32 result_offset;
        u32 value;
    };

    struct ResBfresPerMaterialAnim {
        const NWString8            *material_name;
        ResBfresShaderParamAnim    *shader_param_anim_array;
        ResBfresTexturePatternAnim *texture_pattern_anim_array;
        ResBfresAnimCurve          *anim_curve_array;
        ResBfresConstantEntry      *constant_array;
        u16                         base_result_offset_shader_param;
        u16                         base_result_offset_texture_pattern;
        u16                         base_result_offset_visibility_curve;
        u16                         visibility_curve_index;
        u16                         visibility_constant_index;
        u16                         shader_param_anim_count;
        u16                         texture_pattern_anim_count;
        u16                         constant_count;
        u16                         anim_curve_count;
        u16                         reserve0;
        u32                         reserve1;

        static constexpr u16 cInvalidEntryIndex = 0xffff;
        static constexpr u16 cInvalidResultOffset = 0xffff;

        void EvaluateShaderParamAnim(void *out_result, const u16 *bind_table, float frame, BfresAnimFrameCache *cache_array) const {

            /* Calculate shader param anim values */
            const u32 anim_count = this->shader_param_anim_count;
            for (u32 i = 0; i < anim_count; ++i) {

                /* Ensure param bound */
                ResBfresShaderParamAnim *param_anim = std::addressof(this->shader_param_anim_array[i]);
                if (bind_table[i] == cInvalidEntryIndex) { continue; }

                /* Calculate int and float value curves */
                const u32 base_curve = param_anim->base_curve;
                ResBfresAnimCurve *curve_array = std::addressof(this->anim_curve_array[base_curve]);
                const u32 count_int = param_anim->curve_count_int;
                const u32 count_float = param_anim->curve_count_float;
                s32 *result_array_int = reinterpret_cast<s32*>(reinterpret_cast<uintptr_t>(out_result) + sizeof(s32) * base_curve);
                for (u32 i = 0; i < count_int; ++i) {
                    const s32 result = curve_array->EvaluateInt(cache_array, frame);
                    ++cache_array;
                    *result_array_int = result;
                    ++result_array_int;
                    ++curve_array;
                }
                float *result_array_float = reinterpret_cast<float*>(reinterpret_cast<uintptr_t>(out_result) + sizeof(s32) * (base_curve + count_int));
                for (u32 i = 0; i < count_float; ++i) {
                    const float result = curve_array->EvaluateFloat(cache_array, frame);
                    ++cache_array;
                    *result_array_float = result;
                    ++result_array_float;
                    ++curve_array;
                }
            }

            return;
        }

        void EvaluateTexturePatternAnim(void *out_result, const u16 *bind_table, float frame, BfresAnimFrameCache *cache_array) const {

            /* Calculate texture pattern anim texture indices for sampler */
            const u32 anim_count = this->texture_pattern_anim_count;
            s32 *result_array = reinterpret_cast<s32*>(out_result);
            for (u32 i = 0; i < anim_count; ++i) {

                /* Ensure sampler bound */
                ResBfresTexturePatternAnim *pattern_anim = std::addressof(this->texture_pattern_anim_array[i]);
                if (bind_table[i] == cInvalidEntryIndex) { continue; }

                /* Ensure valid curve */
                const u16 base_curve = pattern_anim->base_curve;
                if (base_curve == cInvalidEntryIndex) { continue; }

                /* Calculate texture index curve */
                const s32 result = this->anim_curve_array[base_curve].EvaluateInt(cache_array, frame);
                ++cache_array;
                *result_array = result;
                ++result_array;
            }

            return;
        }

        void EvaluateVisibilityAnim(void *out_result, float frame, BfresAnimFrameCache *cache) const {

            /* Calculate visibilty curve */
            const u16 base_curve = this->visibility_curve_index;
            if (base_curve == cInvalidEntryIndex) { return; }
            s32 *result_array = reinterpret_cast<s32*>(out_result);
            const s32 result = this->anim_curve_array[base_curve].EvaluateInt(cache, frame);
            *result_array = result;

            return;
        }

        void EvaluateShaderParamAnim(void *out_result, const u16 *bind_table, float frame) const {

            /* Calculate shader param anim values */
            BfresAnimFrameCache cache;
            const u32 anim_count = this->shader_param_anim_count;
            for (u32 i = 0; i < anim_count; ++i) {

                /* Ensure material bound */
                ResBfresShaderParamAnim *param_anim = std::addressof(this->shader_param_anim_array[i]);
                if (bind_table[i] == cInvalidEntryIndex) { continue; }

                /* Calculate int and float value curves */
                const u32 base_curve = param_anim->base_curve;
                ResBfresAnimCurve *curve_array = std::addressof(this->anim_curve_array[base_curve]);
                const u32 count_int = param_anim->curve_count_int;
                const u32 count_float = param_anim->curve_count_float;
                s32 *result_array_int = reinterpret_cast<s32*>(reinterpret_cast<uintptr_t>(out_result) + sizeof(s32) * base_curve);
                for (u32 i = 0; i < count_int; ++i) {
                    cache.SetDefaults();
                    const s32 result = curve_array->EvaluateInt(std::addressof(cache), frame);
                    *result_array_int = result;
                    ++result_array_int;
                    ++curve_array;
                }
                float *result_array_float = reinterpret_cast<float*>(reinterpret_cast<uintptr_t>(out_result) + sizeof(s32) * (base_curve + count_int));
                for (u32 i = 0; i < count_float; ++i) {
                    cache.SetDefaults();
                    const float result = curve_array->EvaluateFloat(std::addressof(cache), frame);
                    *result_array_float = result;
                    ++result_array_float;
                    ++curve_array;
                }
            }

            return;
        }

        void EvaluateTexturePatternAnim(void *out_result, const u16 *bind_table, float frame) const {

            /* Calculate texture pattern anim texture indices for sampler */
            BfresAnimFrameCache cache;
            const u32 anim_count = this->texture_pattern_anim_count;
            s32 *result_array = reinterpret_cast<s32*>(out_result);
            for (u32 i = 0; i < anim_count; ++i) {

                /* Ensure material bound */
                ResBfresTexturePatternAnim *pattern_anim = std::addressof(this->texture_pattern_anim_array[i]);
                if (bind_table[i] == cInvalidEntryIndex) { continue; }

                /* Ensure valid curve */
                const u16 base_curve = pattern_anim->base_curve;
                if (base_curve == cInvalidEntryIndex) { continue; }

                /* Calculate texture index curve */
                cache.SetDefaults();
                const s32 result = this->anim_curve_array[base_curve].EvaluateInt(std::addressof(cache), frame);
                *result_array = result;
                ++result_array;
            }

            return;
        }

        void EvaluateVisibilityAnim(void *out_result, float frame) const {

            /* Calculate visibilty curve */
            const u16 base_curve = this->visibility_curve_index;
            if (base_curve == cInvalidEntryIndex) { return; }
            BfresAnimFrameCache cache;
            s32 *result_array = reinterpret_cast<s32*>(out_result);
            cache.SetDefaults();
            const s32 result = this->anim_curve_array[base_curve].EvaluateInt(std::addressof(cache), frame);
            *result_array = result;

            return;
        }
    };
    static_assert(sizeof(ResBfresPerMaterialAnim) == 0x40);

    struct ResBfresMaterialAnim  {
        u32                         magic;
        union {
            u32 flags;
            struct {
                u32 is_baked   : 1;
                u32 reserve0   : 1;
                u32 is_looping : 1;
                u32 reserve1   : 29;                
            };
        };
        const NWString8            *name;
        const NWString8            *reserve2;
        ResBfresModel              *runtime_bound_model;
        u16                        *fast_bind_table;
        ResBfresPerMaterialAnim    *per_material_anim_array;
        void                      **runtime_texture_view_array;
        const NWString8           **texture_name_array;
        ResGfxUserData             *user_data_array;
        ResNintendoWareDictionary  *user_data_dictionary;
        u64                        *runtime_texture_descriptor_slot_array;
        u32                         frame_count;
        u32                         baked_size;
        u16                         user_data_count;
        u16                         per_material_anim_count;
        u16                         total_anim_curve_count;
        u16                         shader_param_anim_count;
        u16                         texture_pattern_anim_count;
        u16                         material_visibility_anim_count;
        u16                         texture_count;
        u16                         reserve3;

        static constexpr u32    cMagic = util::TCharCode32("FMAA");
        static constexpr size_t cInvalidDescriptorSlot = 0xffff'ffff'ffff'ffff;

        void BindTextures(GfxBindTextureCallback bind_callback, void *user_data) {
            for (u32 i = 0; i < texture_count; ++i) {
                if (runtime_texture_view_array[i] != nullptr && runtime_texture_descriptor_slot_array[i] != cInvalidDescriptorSlot) { continue; }
        
                GfxBindTextureView bind_texture_view  = (bind_callback)(texture_name_array[i], user_data);
                runtime_texture_view_array[i] = bind_texture_view.texture_view;
                runtime_texture_descriptor_slot_array[i] = bind_texture_view.texture_view_decriptor_slot;
            }
        }

        constexpr void ReleaseTextures() {
            for (u32 i = 0; i < texture_count; ++i) {
                runtime_texture_descriptor_slot_array[i] = cInvalidDescriptorSlot;
                runtime_texture_view_array[i] = nullptr;
            }
        }

        bool TrySetTextureByName(const char *name, GfxBindTextureView *bind_texture_view) {

            bool had_success = false;
            for (u32 i = 0; i < texture_count; ++i) {
                if (!vp::util::StringCompare(texture_name_array[i]->GetString(), name)) { continue; }

                runtime_texture_descriptor_slot_array[i] = bind_texture_view->texture_view_decriptor_slot;
                runtime_texture_view_array[i] = bind_texture_view->texture_view;

                had_success = true;
            }

            return had_success;
        }

        constexpr ResGfxUserData *TryGetUserData(const char *user_data_name) {
            if (user_data_dictionary == nullptr) { return nullptr; }
            const u32 entry_id = user_data_dictionary->TryGetEntryIndexByKey(user_data_name);
            if (entry_id == ResNintendoWareDictionary::cInvalidEntryIndex) { return nullptr; }
            return std::addressof(user_data_array[entry_id]);
        }
    };
    static_assert(sizeof(ResBfresMaterialAnim) == 0x70);
}
