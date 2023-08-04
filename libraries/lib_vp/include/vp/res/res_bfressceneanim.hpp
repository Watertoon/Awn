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

    struct ResBfresCameraAnimResultDefault {
        float          near;
        float          far;
        float          aspect;
        float          fovy;
        util::Vector3f position;
        util::Vector3f rotation;
        float          twist;
    };
    static_assert(sizeof(ResBfresCameraAnimResultDefault) == 0x2c);

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
        u32                              is_baked        : 1;
        u32                              reserve0        : 1;
        u32                              is_looping      : 1;
        u32                              reserve1        : 1;
        u32                              rotation_mode   : 1;
        u32                              reserve2        : 1;
        u32                              projection_mode : 1;
        u32                              reserve3        : 25;
        const char                      *animation_name;
        ResBfresAnimCurve               *anim_curve_array;
        ResBfresCameraAnimResultDefault *camera_anim_result_default;
        ResGfxUserData                  *user_data_array;
        ResNintendoWareDictionary       *user_data_dictionary;
        u32                              frame_count;
        u32                              baked_size;
        u16                              user_data_count;
        u8                               anim_curve_count;
        u8                               reserve4;
        u32                              reserve5;

        static constexpr u32 cMagic = util::TCharCode32("FCAM");

        ResGfxUserData *TryGetUserData(const char *user_data_name) {
            if (user_data_dictionary == nullptr) { return nullptr; }
            const u32 entry_id = user_data_dictionary->TryGetEntryIndexByKey(user_data_name);
            if (entry_id == ResNintendoWareDictionary::cInvalidEntryIndex) { return nullptr; }
            return std::addressof(user_data_array[entry_id]);
        }
    };
    static_assert(sizeof(ResBfresCameraAnim) == 0x40);

    struct ResBfresLightAnimResultDefault {
        s32            enable;
        util::Vector3f position;
        util::Vector3f rotation;
        util::Vector2f distance_attenuation;
        util::Vector2f angle_attenuation;
        util::Vector3f color0;
        util::Vector3f color1;
    };
    static_assert(sizeof(ResBfresLightAnimResultDefault) == 0x44);

    struct ResBfresLightAnim {
        u32                             magic;
        u32                             is_baked                    : 1;
        u32                             reserve0                    : 1;
        u32                             is_looping                  : 1;
        u32                             reserve1                    : 5;
        u32                             is_use_curves               : 1;
        u32                             is_use_enable               : 1;
        u32                             is_use_position             : 1;
        u32                             is_use_rotation             : 1;
        u32                             is_use_distance_attenuation : 1;
        u32                             is_use_angle_attenuation    : 1;
        u32                             is_use_color0               : 1;
        u32                             is_use_color1               : 1;
        u32                             reserve2                    : 16;
        const char                     *animation_name;
        ResBfresAnimCurve              *anim_curve_array;
        ResBfresLightAnimResultDefault *light_anim_result_default;
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

        ResGfxUserData *TryGetUserData(const char *user_data_name) {
            if (user_data_dictionary == nullptr) { return nullptr; }
            const u32 entry_id = user_data_dictionary->TryGetEntryIndexByKey(user_data_name);
            if (entry_id == ResNintendoWareDictionary::cInvalidEntryIndex) { return nullptr; }
            return std::addressof(user_data_array[entry_id]);
        }
    };
    static_assert(sizeof(ResBfresLightAnim) == 0x58);

    struct ResBfresFogAnimResultDefault {
        util::Vector2f distance_attenuation;
        util::Vector3f color;
    };
    static_assert(sizeof(ResBfresFogAnimResultDefault) == 0x14);

    struct ResBfresFogAnim {
        u32                           magic;
        u32                           is_baked                    : 1;
        u32                           reserve0                    : 1;
        u32                           is_looping                  : 1;
        u32                           reserve1                    : 29;
        const char                   *animation_name;
        ResBfresAnimCurve            *anim_curve_array;
        ResBfresFogAnimResultDefault *fog_anim_result_default;
        ResGfxUserData               *user_data_array;
        ResNintendoWareDictionary    *user_data_dictionary;
        const char                   *distance_attenuation_type_name;
        u32                           frame_count;
        u32                           baked_size;
        u16                           user_data_count;
        u8                            anim_curve_count;
        s8                            distance_attenuation_type;
        u16                           reserve2;

        static constexpr u32 cMagic = util::TCharCode32("FFOG");

        ResGfxUserData *TryGetUserData(const char *user_data_name) {
            if (user_data_dictionary == nullptr) { return nullptr; }
            const u32 entry_id = user_data_dictionary->TryGetEntryIndexByKey(user_data_name);
            if (entry_id == ResNintendoWareDictionary::cInvalidEntryIndex) { return nullptr; }
            return std::addressof(user_data_array[entry_id]);
        }
    };
    static_assert(sizeof(ResBfresFogAnim) == 0x48);

    struct ResBfresSceneAnim {
        u32                        magic;
        u32                        reserve0;
        const char                *animation_name;
        const char                *reserve1;
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

        ResBfresCameraAnim *TryGetCameraAnim(const char *camera_anim_name) {
            if (camera_anim_dictionary == nullptr) { return nullptr; }
            const u32 entry_id = camera_anim_dictionary->TryGetEntryIndexByKey(camera_anim_name);
            if (entry_id == ResNintendoWareDictionary::cInvalidEntryIndex) { return nullptr; }
            return std::addressof(camera_anim_array[entry_id]);
        }
        ResBfresLightAnim *TryGetLightAnim(const char *light_anim_name) {
            if (light_anim_dictionary == nullptr) { return nullptr; }
            const u32 entry_id = light_anim_dictionary->TryGetEntryIndexByKey(light_anim_name);
            if (entry_id == ResNintendoWareDictionary::cInvalidEntryIndex) { return nullptr; }
            return std::addressof(light_anim_array[entry_id]);
        }
        ResBfresFogAnim *TryGetFogAnim(const char *fog_anim_name) {
            if (fog_anim_dictionary == nullptr) { return nullptr; }
            const u32 entry_id = fog_anim_dictionary->TryGetEntryIndexByKey(fog_anim_name);
            if (entry_id == ResNintendoWareDictionary::cInvalidEntryIndex) { return nullptr; }
            return std::addressof(fog_anim_array[entry_id]);
        }
        ResGfxUserData *TryGetUserData(const char *user_data_name) {
            if (user_data_dictionary == nullptr) { return nullptr; }
            const u32 entry_id = user_data_dictionary->TryGetEntryIndexByKey(user_data_name);
            if (entry_id == ResNintendoWareDictionary::cInvalidEntryIndex) { return nullptr; }
            return std::addressof(user_data_array[entry_id]);
        }
    };
    static_assert(sizeof(ResBfresSceneAnim) == 0x60);
}
