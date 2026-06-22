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

    struct ResBfresKeyShapeAnim {
        const NWString8 *key_shape_name;
        u8               anim_curve_index;
        u8               fast_bind_id;
        u16              reserve0;
        u32              reserve1;

        static constexpr u8 cInvalidId = 0xff;
    };
    static_assert(sizeof(ResBfresKeyShapeAnim) == 0x10);

    struct ResBfresVertexShapeAnim {
        const NWString8      *shape_name;
        ResBfresAnimCurve    *anim_curve_array;
        float                *default_result_array;
        ResBfresKeyShapeAnim *key_shape_anim_array;
        u16                   anim_curve_count;
        u16                   key_shape_anim_count;
        u32                   base_cache_index;
        u32                   base_result_index;
        u32                   reserve0;

        constexpr void SetResultDefaults(float *out_default_array) const {
            for (u32 i = 0; i < this->key_shape_anim_count; ++i) {
                out_default_array[i] = this->default_result_array[i];
            }
            return;
        }

        void Evaluate(float *out_result_array, const u8 *key_shape_index_array, float frame) const {
            for (u32 i = 0; i < this->key_shape_anim_count; ++i) {
                if (key_shape_index_array[i] == ResBfresKeyShapeAnim::cInvalidId) { continue; }
                const ResBfresKeyShapeAnim *key_shape_anim = std::addressof(this->key_shape_anim_array[i]);
                const u8 curve_index = key_shape_anim->anim_curve_index;
                if (curve_index == ResBfresKeyShapeAnim::cInvalidId) { continue; }
                BfresAnimFrameCache cache = {};
                const float result = this->anim_curve_array[curve_index].EvaluateFloat(std::addressof(cache), frame);
                out_result_array[i] = result;
            }
            return;
        }

        void Evaluate(float *out_result_array, const u8 *key_shape_index_array, float frame, BfresAnimFrameCache *cache_array) const {
            for (u32 i = 0; i < this->key_shape_anim_count; ++i) {
                if (key_shape_index_array[i] == ResBfresKeyShapeAnim::cInvalidId) { continue; }
                const ResBfresKeyShapeAnim *key_shape_anim = std::addressof(this->key_shape_anim_array[i]);
                const u8 curve_index = key_shape_anim->anim_curve_index;
                if (curve_index == ResBfresKeyShapeAnim::cInvalidId) { continue; }
                BfresAnimFrameCache *cache = std::addressof(cache_array[curve_index]);
                const float result = this->anim_curve_array[curve_index].EvaluateFloat(cache, frame);
                out_result_array[i] = result;
            }
            return;
        }

        constexpr bool BuildKeyShapeIndices(u8 *out_key_shape_index_array, ResBfresShape *shape) const {
            if (shape->key_shape_count < 1) { return false; }
            bool result = false;
            for (u32 i = 0; i < this->key_shape_anim_count; ++i) {
                const u32 entry_id = (shape->key_shape_dictionary != nullptr) ? shape->key_shape_dictionary->TryGetEntryIndexByKey(key_shape_anim_array[i].key_shape_name) : ResNintendoWareDictionary::cInvalidEntryIndex;
                out_key_shape_index_array[i + this->base_result_index] = (entry_id != ResNintendoWareDictionary::cInvalidEntryIndex) ? entry_id : 0xff;
                result |= (entry_id != ResNintendoWareDictionary::cInvalidEntryIndex);
            }
            return result;
        }
    };
    static_assert(sizeof(ResBfresVertexShapeAnim) == 0x30);

    struct ResBfresShapeAnim {
        u32                        magic;
        union {
            u32 flags;
            struct {                
                u32 is_baked   : 1;
                u32 reserve0   : 1;
                u32 is_looping : 1;
                u32 reserve1   : 29;
            };
        };
        const NWString8           *name;
        const NWString8           *reserve2;
        ResBfresModel             *runtime_bound_model;
        u16                       *fast_bind_table;
        ResBfresVertexShapeAnim   *vertex_shape_anim_array;
        ResGfxUserData            *user_data_array;
        ResNintendoWareDictionary *user_data_dictionary;
        u32                        frame_count;
        u32                        baked_size;
        u16                        user_data_count;
        u16                        vertex_shape_anim_count;
        u16                        total_key_shape_anim_count;
        u16                        total_anim_curve_count;

        static constexpr u32 cMagic = util::TCharCode32("FSHA");

        ResGfxUserData *TryGetUserData(const char *user_data_name) {
            if (user_data_dictionary == nullptr) { return nullptr; }
            const u32 entry_id = user_data_dictionary->TryGetEntryIndexByKey(user_data_name);
            if (entry_id == ResNintendoWareDictionary::cInvalidEntryIndex) { return nullptr; }
            return std::addressof(user_data_array[entry_id]);
        }
    };
    static_assert(sizeof(ResBfresShapeAnim) == 0x50);
}
