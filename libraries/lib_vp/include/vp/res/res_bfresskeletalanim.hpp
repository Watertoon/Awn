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

    enum class BfresBoneAnimRotationType : u32 {
        EulerXYZ = 0,
        Quaternion = 1,
    };
    enum class BfresBoneAnimCurveType : u16 {
        ScaleX       = (1 << 0),
        ScaleY       = (1 << 1),
        ScaleZ       = (1 << 2),
        RotationX    = (1 << 3),
        RotationY    = (1 << 4),
        RotationZ    = (1 << 5),
        RotationW    = (1 << 6),
        TranslationX = (1 << 7),
        TranslationY = (1 << 8),
        TranslationZ = (1 << 9),
    };
    VP_ENUM_FLAG_TRAITS(BfresBoneAnimCurveType);

    struct ResBfresBoneAnimResult {
        util::Float3 scale;
        util::Float4 rotation;
        util::Float3 translation;
    };
    static_assert(sizeof(ResBfresBoneAnimResult) == sizeof(float) * 10);

    struct BfresBoneAnimResult {
        union {
            u32 flags;
            struct {
                u32 reserve0 : 20;
                u32 rotation_type : 2;
                u32 reserve1 : 1;
                u32 local_transform_mode   : 5;
                u32 reserve2 : 4;
            };
        };
        util::Float3   scale;
        util::Float3   translation;
        u32            reserve3;
        util::Vector4f rotation;

        constexpr BfresBoneAnimRotationType GetRotationType() const {
            return static_cast<BfresBoneAnimRotationType>(this->rotation_type);
        }
        constexpr BfresBoneLocalTransformMode GetLocalTransformMode() const {
            return static_cast<BfresBoneLocalTransformMode>(this->local_transform_mode);
        }
    };
    static_assert(sizeof(BfresBoneAnimResult) == 0x30);

    struct ResBfresBoneAnim {
        const char                    *bone_name;
        ResBfresAnimCurve             *anim_curve_array;
        void                          *default_result;
        ResGfxUserData                *user_data_array;
        ResNintendoWareDictionary     *user_data_dictionary;
        union {
            u32 flags;
            struct {
                u32 reserve0               : 3;
                u32 is_use_scale           : 1;
                u32 is_use_rotation        : 1;
                u32 is_use_translation     : 1;
                u32 is_curve_scale_x       : 1;
                u32 is_curve_scale_y       : 1;
                u32 is_curve_scale_z       : 1;
                u32 is_curve_rotation_x    : 1;
                u32 is_curve_rotation_y    : 1;
                u32 is_curve_rotation_z    : 1;
                u32 is_curve_rotation_w    : 1;
                u32 is_curve_translation_x : 1;
                u32 is_curve_translation_y : 1;
                u32 is_curve_translation_z : 1;
                u32 reserve1               : 4;
                u32 rotation_type          : 2;
                u32 reserve2               : 1;
                u32 local_transform_mode   : 5;
                u32 reserve3               : 4;
            };
        };
        u8                             reserve4;
        u8                             reserve5;
        u8                             anim_curve_count;
        u8                             section_index;
        u32                            default_translation_index;
        u16                            user_data_count;
        u16                            reserve6;

        constexpr bool IsTranslation() const {
            return this->is_use_translation;
        }
        constexpr bool IsRotation() const {
            return this->is_use_rotation;
        }
        constexpr bool IsScale() const {
            return this->is_use_scale;
        }
        constexpr bool IsTranslationCurve() const {
            constexpr const u32 mask = static_cast<u32>(BfresBoneAnimCurveType::TranslationX | BfresBoneAnimCurveType::TranslationY | BfresBoneAnimCurveType::TranslationZ);
            return ((this->flags >> 6) & mask) != 0;
        }
        constexpr bool IsRotationCurve() const {
            constexpr const u32 mask = static_cast<u32>(BfresBoneAnimCurveType::RotationX | BfresBoneAnimCurveType::RotationY | BfresBoneAnimCurveType::RotationZ | BfresBoneAnimCurveType::RotationW);
            return ((this->flags >> 6) & mask) != 0;
        }
        constexpr bool IsScaleCurve() const {
            constexpr const u32 mask = static_cast<u32>(BfresBoneAnimCurveType::ScaleX | BfresBoneAnimCurveType::ScaleY | BfresBoneAnimCurveType::ScaleZ);
            return ((this->flags >> 6) & mask) != 0;
        }
        constexpr BfresBoneRotationType GetRotationType() const {
            return static_cast<BfresBoneRotationType>(this->rotation_type);
        }
        constexpr BfresBoneLocalTransformMode GetLocalTransformMode() const {
            return static_cast<BfresBoneLocalTransformMode>(this->local_transform_mode);
        }
        vp::util::Float3 GetScale() const {
            const vp::util::Float3 *result = reinterpret_cast<const vp::util::Float3*>(this->default_result);
            return *result;
        }
        vp::util::Float4 GetRotation() const {
            const u32 offset = (this->is_use_scale) ? sizeof(vp::util::Float3) : 0;
            const vp::util::Float4 *result = reinterpret_cast<const vp::util::Float4*>(reinterpret_cast<uintptr_t>(this->default_result) + offset);
            return *result;
        }
        vp::util::Float3 GetTranslation() const {
            const vp::util::Float3 *result = reinterpret_cast<const vp::util::Float3*>(reinterpret_cast<uintptr_t>(this->default_result) + sizeof(float) * this->default_translation_index);
            return *result;
        }

        void Initialize(BfresBoneAnimResult *out_result, const vp::res::ResBfresBone *bone) const {

            /* Set flags */
            out_result->flags = 0;
            out_result->local_transform_mode = this->local_transform_mode;
            
            /* Try get scale in order anim -> bone -> cZero */
            uintptr_t result_iter = reinterpret_cast<uintptr_t>(this->default_result);
            if (!this->is_use_scale) {
                const vp::util::Float3 *scale = (bone != nullptr) ? std::addressof(bone->scale) : std::addressof(vp::util::Float3::cZero);
                out_result->scale = *scale;
            } else {
                const vp::util::Float3 *scale = reinterpret_cast<const vp::util::Float3*>(result_iter);
                out_result->scale = *scale;
                result_iter += sizeof(vp::util::Float3);
            }
            /* Try get rotation in order anim -> bone -> cUnitW, rotation mode in anim -> bone -> anim */
            if (!this->is_use_rotation) {
                const vp::util::Float4 *rotation;
                if (bone != nullptr) {
                    rotation = std::addressof(bone->rotation);
                    if (bone->GetRotationType() == BfresBoneRotationType::EulerXYZ) {
                        out_result->rotation_type = static_cast<u32>(BfresBoneAnimRotationType::EulerXYZ);
                    } else if (bone->GetRotationType() == BfresBoneRotationType::Quaternion) {
                        out_result->rotation_type = static_cast<u32>(BfresBoneAnimRotationType::Quaternion);
                    }
                } else {
                    rotation = std::addressof(vp::util::Float4::cUnitW);
                    out_result->rotation_type = this->rotation_type;
                }
                out_result->rotation = vp::util::CastVector4f(*rotation);
            } else {
                const vp::util::Float4 *rotation = reinterpret_cast<const vp::util::Float4*>(result_iter);
                out_result->rotation = vp::util::CastVector4f(*rotation);
                out_result->rotation_type = this->rotation_type;
                result_iter += sizeof(vp::util::Float4);
            }
            /* Try get translation in order anim -> bone -> cZero */
            if (!this->is_use_scale) {
                const vp::util::Float3 *translation = (bone != nullptr) ? std::addressof(bone->translation) : std::addressof(vp::util::Float3::cZero);
                out_result->translation = *translation;
            } else {
                const vp::util::Float3 *translation = reinterpret_cast<const vp::util::Float3*>(result_iter);
                out_result->translation = *translation;
                result_iter += sizeof(vp::util::Float3);
            }
            return;
        }

        void Evaluate(BfresBoneAnimResult *out_result, float frame, BfresAnimFrameCache *cache_array) const {

            /* Nothing to evaluate if no curves */
            const u32 curve_count = this->anim_curve_count;
            if (curve_count < 1) { return; }

            /* Evaluate special quat curve */
            u32 i = 0;
            if (this->anim_curve_array[0].IsQuaternion()) {
                i = 1;
                BfresAnimFrameCache *cache = std::addressof(cache_array[0]);
                vp::util::Vector4f quat = this->anim_curve_array[0].EvaluateQuaternion(cache, frame);
                const u8 result_offset = this->anim_curve_array[0].base_result_offset;
                vp::util::Store4(std::addressof(reinterpret_cast<float*>(out_result)[result_offset]), std::addressof(quat.v));
            }

            /* Evaluate float curves */
            for (; i < curve_count; ++i) {
                BfresAnimFrameCache *cache = std::addressof(cache_array[i]);
                const u8 result_offset = this->anim_curve_array[i].base_result_offset;
                const float value = this->anim_curve_array[i].EvaluateFloat(cache, frame);
                reinterpret_cast<float*>(out_result)[result_offset] = value;
            }

            return;
        }

        void Evaluate(BfresBoneAnimResult *out_result, float frame) const {

            /* Nothing to evaluate if no curves */
            const u32 curve_count = this->anim_curve_count;
            if (curve_count < 1) { return; }

            /* Evaluate special quat curve */
            u32 i = 0;
            if (this->anim_curve_array[0].IsQuaternion()) {
                i = 1;
                BfresAnimFrameCache cache = {};
                vp::util::Vector4f quat = this->anim_curve_array[0].EvaluateQuaternion(std::addressof(cache), frame);
                const u8 result_offset = this->anim_curve_array[0].base_result_offset;
                vp::util::Store4(std::addressof(reinterpret_cast<float*>(out_result)[result_offset]), std::addressof(quat.v));
            }

            /* Evaluate float curves */
            for (; i < curve_count; ++i) {
                BfresAnimFrameCache cache = {};
                const u8 result_offset = this->anim_curve_array[i].base_result_offset;
                const float value = this->anim_curve_array[i].EvaluateFloat(std::addressof(cache), frame);
                reinterpret_cast<float*>(out_result)[result_offset] = value;
            }

            return;
        }

        constexpr ResGfxUserData *TryGetUserData(const char *user_data_name) {
            if (user_data_dictionary == nullptr) { return nullptr; }
            const u32 entry_id = user_data_dictionary->TryGetEntryIndexByKey(user_data_name);
            if (entry_id == ResNintendoWareDictionary::cInvalidEntryIndex) { return nullptr; }
            return std::addressof(user_data_array[entry_id]);
        }
    };
    static_assert(sizeof(ResBfresBoneAnim) == 0x38);

    struct ResBfresSkeletalAnim {
        u32                        magic;
        union {
            u32 flags;
            struct {
                u32 is_baked       : 1;
                u32 reserve0       : 1;
                u32 is_looping     : 1;
                u32 reserve1       : 5;
                u32 scale_mode     : 2;
                u32 reserve2       : 2;
                u32 rotation_type  : 1;
                u32 reserve3       : 19;
            };
        };
        const char                *name;
        const char                *reserve4;
        ResBfresSkeleton          *runtime_bound_skeleton;
        u16                       *fast_bind_table;
        ResBfresBoneAnim          *bone_anim_array;
        ResGfxUserData            *user_data_array;
        ResNintendoWareDictionary *user_data_dictionary;
        u32                        frame_count;
        u32                        total_anim_curves;
        u32                        baked_size;
        u16                        bone_anim_count;
        u16                        user_data_count;

        static constexpr u32 cMagic = util::TCharCode32("FSKA");

        constexpr ResGfxUserData *TryGetUserData(const char *user_data_name) {

            /* Ensure dictionary */
            if (user_data_dictionary == nullptr) { return nullptr; }

            /* Find index */
            const u32 entry_id = user_data_dictionary->TryGetEntryIndexByKey(user_data_name);
            if (entry_id == ResNintendoWareDictionary::cInvalidEntryIndex) { return nullptr; }

            /* Get data */
            return std::addressof(user_data_array[entry_id]);
        }
    };
    static_assert(sizeof(ResBfresSkeletalAnim) == 0x50);
}
