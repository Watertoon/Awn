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

    struct ResBfresCameraAnimResult {
        float        near;
        float        far;
        float        aspect;
        float        fovy;
        util::Float3 position;
        util::Float3 rotation;
        float        twist;
    };
    static_assert(sizeof(ResBfresCameraAnimResult) == 0x2c);

    using BfresCameraAnimResult = vp::res::ResBfresCameraAnimResult;

    enum class BfresCameraAnimRotationMode   : u8 {
        At       = 0,
        EulerZXY = 1,
    };

    enum class BfresCameraAnimProjectionMode : u8 {
        Orthographic = 0,
        Perspective  = 1,
    };

    struct ResBfresCameraAnim {
        u32                              magic;
        union {
            u32 flags;
            struct {
                u32 is_baked        : 1;
                u32 reserve0        : 1;
                u32 is_looping      : 1;
                u32 reserve1        : 1;
                u32 rotation_mode   : 1;
                u32 reserve2        : 1;
                u32 projection_mode : 1;
                u32 reserve3        : 25;
            };
        };
        const char                      *name;
        ResBfresAnimCurve               *anim_curve_array;
        ResBfresCameraAnimResult        *default_result;
        ResGfxUserData                  *user_data_array;
        ResNintendoWareDictionary       *user_data_dictionary;
        u32                              frame_count;
        u32                              baked_size;
        u16                              user_data_count;
        u8                               anim_curve_count;
        u8                               reserve4;
        u32                              reserve5;

        static constexpr u32 cMagic = util::TCharCode32("FCAM");

        constexpr void Initialize(BfresCameraAnimResult *out_result) {
            *out_result = *this->default_result;
            return;
        }

        void Evaluate(BfresCameraAnimResult *out_result, float frame, BfresAnimFrameCache *cache_array) {
            const u32 count = this->anim_curve_count;
            for (u32 i = 0; i < count; ++i) {
                BfresAnimFrameCache *cache = std::addressof(cache_array[i]);
                const float result = this->anim_curve_array[i].EvaluateFloat(cache, frame);
                *reinterpret_cast<float*>(reinterpret_cast<uintptr_t>(out_result) + this->anim_curve_array[i].base_result_offset) = result;
            }
            return;
        }

        void Evaluate(BfresCameraAnimResult *out_result, float frame) {
            const u32 count = this->anim_curve_count;
            BfresAnimFrameCache cache;
            for (u32 i = 0; i < count; ++i) {
                cache.SetDefaults();
                const float result = this->anim_curve_array[i].EvaluateFloat(std::addressof(cache), frame);
                *reinterpret_cast<float*>(reinterpret_cast<uintptr_t>(out_result) + this->anim_curve_array[i].base_result_offset) = result;
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
    static_assert(sizeof(ResBfresCameraAnim) == 0x40);

    struct ResBfresLightAnimResult {
        s32          is_enable;
        util::Float3 position;
        util::Float3 rotation;
        util::Float2 distance_attenuation;
        util::Float2 angle_attenuation;
        util::Float3 color0;
        util::Float3 color1;
    };
    static_assert(sizeof(ResBfresLightAnimResult) == 0x44);

    using BfresLightAnimResult = ResBfresLightAnimResult;

    struct ResBfresLightAnim {
        u32                             magic;
        union {
            u32 flags;
            struct {
                u32 is_baked                    : 1;
                u32 reserve0                    : 1;
                u32 is_looping                  : 1;
                u32 reserve1                    : 5;
                u32 is_anim_curve               : 1;
                u32 is_use_enable               : 1;
                u32 is_use_position             : 1;
                u32 is_use_rotation             : 1;
                u32 is_use_distance_attenuation : 1;
                u32 is_use_angle_attenuation    : 1;
                u32 is_use_color0               : 1;
                u32 is_use_color1               : 1;
                u32 reserve2                    : 16;
            };
        };
        const char                     *name;
        ResBfresAnimCurve              *anim_curve_array;
        void                           *default_result;
        ResGfxUserData                 *user_data_array;
        ResNintendoWareDictionary      *user_data_dictionary;
        const char                     *light_type_name;
        const char                     *distance_attenuation_type_name;
        const char                     *angle_attenuation_type_name;
        u32                             frame_count;
        u32                             baked_size;
        u16                             user_data_count;
        u8                              anim_curve_count;
        s8                              light_type;
        s8                              distance_attenuation_type;
        s8                              angle_attenuation_type;
        u16                             reserve3;

        static constexpr u32 cMagic = util::TCharCode32("FLIT");

        void Initialize(BfresLightAnimResult *out_result) {
            /* Decompose default result */
            uintptr_t result_iter = reinterpret_cast<uintptr_t>(this->default_result);
            if (this->is_use_enable) {
                s32 *enable = reinterpret_cast<s32*>(result_iter);
                out_result->is_enable = *enable;
                result_iter += sizeof(s32);
            }
            if (this->is_use_position) {
                vp::util::Float3 *position = reinterpret_cast<vp::util::Float3*>(result_iter);
                out_result->position = *position;
                result_iter += sizeof(vp::util::Float3);
            }
            if (this->is_use_rotation) {
                vp::util::Float3 *rotation = reinterpret_cast<vp::util::Float3*>(result_iter);
                out_result->rotation = *rotation;
                result_iter += sizeof(vp::util::Float3);
            }
            if (this->is_use_distance_attenuation) {
                vp::util::Float2 *distance_attenuation = reinterpret_cast<vp::util::Float2*>(result_iter);
                out_result->distance_attenuation = *distance_attenuation;
                result_iter += sizeof(vp::util::Float2);
            }
            if (this->is_use_angle_attenuation) {
                vp::util::Float2 *angle_attenuation = reinterpret_cast<vp::util::Float2*>(result_iter);
                out_result->angle_attenuation = *angle_attenuation;
                result_iter += sizeof(vp::util::Float2);
            }
            if (this->is_use_color0) {
                vp::util::Float3 *color0 = reinterpret_cast<vp::util::Float3*>(result_iter);
                out_result->color0 = *color0;
                result_iter += sizeof(vp::util::Float3);
            }
            if (this->is_use_color1) {
                vp::util::Float3 *color1 = reinterpret_cast<vp::util::Float3*>(result_iter);
                out_result->color1 = *color1;
                result_iter += sizeof(vp::util::Float3);
            }
            return;
        }

        void Evaluate(BfresLightAnimResult *out_result, float frame, BfresAnimFrameCache *cache_array) {

            /* Special enable int curve */
            const u32 count = this->anim_curve_count;
            if (count == 0) { return; }
            u32 i;
            if (this->is_use_enable) {
                const s32 enable = this->anim_curve_array[0].EvaluateInt(std::addressof(cache_array[0]), frame);
                *reinterpret_cast<s32*>(out_result) = enable;
                i = 1;
            } else {
                i = 0;
            }

            /* Float curves */
            for (; i < count; ++i) {
                BfresAnimFrameCache *cache = std::addressof(cache_array[i]);
                const float result = this->anim_curve_array[i].EvaluateFloat(cache, frame);
                *reinterpret_cast<float*>(reinterpret_cast<uintptr_t>(out_result) + this->anim_curve_array[i].base_result_offset) = result;
            }

            return;
        }

        void Evaluate(BfresLightAnimResult *out_result, float frame) {

            /* Special enable int curve */
            const u32 count = this->anim_curve_count;
            u32 i;
            BfresAnimFrameCache cache;
            if (this->is_use_enable) {
                cache.SetDefaults();
                const s32 enable = this->anim_curve_array[0].EvaluateInt(std::addressof(cache), frame);
                *reinterpret_cast<s32*>(out_result) = enable;
                i = 1;
            } else {
                i = 0;
            }

            /* Float curves */
            for (; i < count; ++i) {
                cache.SetDefaults();
                const float result = this->anim_curve_array[i].EvaluateFloat(std::addressof(cache), frame);
                *reinterpret_cast<float*>(reinterpret_cast<uintptr_t>(out_result) + this->anim_curve_array[i].base_result_offset) = result;
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
    static_assert(sizeof(ResBfresLightAnim) == 0x58);

    struct ResBfresFogAnimResult {
        util::Float2 distance_attenuation;
        util::Float3 color;
    };
    static_assert(sizeof(ResBfresFogAnimResult) == 0x14);
    
    using BfresFogAnimResult = ResBfresFogAnimResult;

    struct ResBfresFogAnim {
        u32                           magic;
        union {
            u32 flags;
            struct {
                u32 is_baked   : 1;
                u32 reserve0   : 1;
                u32 is_looping : 1;
                u32 reserve1   : 29;
            };
        };
        const char                   *name;
        ResBfresAnimCurve            *anim_curve_array;
        ResBfresFogAnimResult        *default_result;
        ResGfxUserData               *user_data_array;
        ResNintendoWareDictionary    *user_data_dictionary;
        const char                   *distance_attenuation_type_name;
        u32                           frame_count;
        u32                           baked_size;
        u16                           user_data_count;
        u8                            anim_curve_count;
        s8                            distance_attenuation_type;
        u32                           reserve2;

        static constexpr u32 cMagic = util::TCharCode32("FFOG");

        constexpr void Initialize(BfresFogAnimResult *out_result) {
            *out_result = *this->default_result;
            return;
        }

        void Evaluate(BfresFogAnimResult *out_result, float frame, BfresAnimFrameCache *cache_array) {
            const u32 count = this->anim_curve_count;
            for (u32 i = 0; i < count; ++i) {
                BfresAnimFrameCache *cache = std::addressof(cache_array[i]);
                const float result = this->anim_curve_array[i].EvaluateFloat(cache, frame);
                *reinterpret_cast<float*>(reinterpret_cast<uintptr_t>(out_result) + this->anim_curve_array[i].base_result_offset) = result;
            }
            return;
        }

        void Evaluate(BfresFogAnimResult *out_result, float frame) {
            const u32 count = this->anim_curve_count;
            BfresAnimFrameCache cache;
            for (u32 i = 0; i < count; ++i) {
                cache.SetDefaults();
                const float result = this->anim_curve_array[i].EvaluateFloat(std::addressof(cache), frame);
                *reinterpret_cast<float*>(reinterpret_cast<uintptr_t>(out_result) + this->anim_curve_array[i].base_result_offset) = result;
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
    static_assert(sizeof(ResBfresFogAnim) == 0x48);

    struct ResBfresSceneAnim {
        u32                        magic;
        u32                        flags;
        const char                *name;
        const char                *reserve0;
        ResBfresCameraAnim        *camera_anim_array;
        ResNintendoWareDictionary *camera_anim_dictionary;
        ResBfresLightAnim         *light_anim_array;
        ResNintendoWareDictionary *light_anim_dictionary;
        ResBfresFogAnim           *fog_anim_array;
        ResNintendoWareDictionary *fog_anim_dictionary;
        ResGfxUserData            *user_data_array;
        ResNintendoWareDictionary *user_data_dictionary;
        u16                        user_data_count;
        u16                        camera_anim_count;
        u16                        light_anim_count;
        u16                        fog_anim_count;

        static constexpr u32 cMagic = util::TCharCode32("FSCN");

        constexpr ResBfresCameraAnim *TryGetCameraAnim(const char *camera_anim_name) {
            if (camera_anim_dictionary == nullptr) { return nullptr; }
            const u32 entry_id = camera_anim_dictionary->TryGetEntryIndexByKey(camera_anim_name);
            if (entry_id == ResNintendoWareDictionary::cInvalidEntryIndex) { return nullptr; }
            return std::addressof(camera_anim_array[entry_id]);
        }
        constexpr ResBfresLightAnim *TryGetLightAnim(const char *light_anim_name) {
            if (light_anim_dictionary == nullptr) { return nullptr; }
            const u32 entry_id = light_anim_dictionary->TryGetEntryIndexByKey(light_anim_name);
            if (entry_id == ResNintendoWareDictionary::cInvalidEntryIndex) { return nullptr; }
            return std::addressof(light_anim_array[entry_id]);
        }
        constexpr ResBfresFogAnim *TryGetFogAnim(const char *fog_anim_name) {
            if (fog_anim_dictionary == nullptr) { return nullptr; }
            const u32 entry_id = fog_anim_dictionary->TryGetEntryIndexByKey(fog_anim_name);
            if (entry_id == ResNintendoWareDictionary::cInvalidEntryIndex) { return nullptr; }
            return std::addressof(fog_anim_array[entry_id]);
        }
        constexpr ResGfxUserData *TryGetUserData(const char *user_data_name) {
            if (user_data_dictionary == nullptr) { return nullptr; }
            const u32 entry_id = user_data_dictionary->TryGetEntryIndexByKey(user_data_name);
            if (entry_id == ResNintendoWareDictionary::cInvalidEntryIndex) { return nullptr; }
            return std::addressof(user_data_array[entry_id]);
        }
    };
    static_assert(sizeof(ResBfresSceneAnim) == 0x60);
}
