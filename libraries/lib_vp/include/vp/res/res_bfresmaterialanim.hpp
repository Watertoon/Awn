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
        const char *shader_param_name;
        u16         base_curve;
        u16         float_curve_count;
        u16         int_curve_count;
        u16         base_constant;
        u16         constant_count;
        u16         material_sub_shader_param_index;
        u32         reserve0;
    };
    static_assert(sizeof(ResBfresShaderParamAnim) == 0x18);

    struct ResBfresTexturePatternAnim {
        const char *texture_name;
        u16         base_curve;
        u16         base_constant;
        u8          material_sub_sampler_index;
        u8          reserve0;
        u16         reserve1;
    };
    static_assert(sizeof(ResBfresTexturePatternAnim) == 0x10);

    struct ResBfresMaterialAnimData {
        const char                 *model_name;
        ResBfresShaderParamAnim    *shader_param_anim_array;
        ResBfresTexturePatternAnim *texture_pattern_anim_array;
        ResBfresAnimCurve          *anim_curve_array;
        void                       *constant_array;
        u16                         base_shader_param_curve_index;
        u16                         base_texture_pattern_curve_index;
        u16                         base_visibility_curve_index;
        u16                         visibility_curve_index;
        u16                         visibility_constant_index;
        u16                         shader_param_anim_count;
        u16                         texture_pattern_anim_count;
        u16                         constant_count;
        u16                         anim_curve_count;
        u16                         reserve0;
        u32                         reserve1;
    };
    static_assert(sizeof(ResBfresMaterialAnimData) == 0x40);

    struct ResBfresMaterialAnim  {
        u32                         magic;
        u32                         is_baked        : 1;
        u32                         reserve0        : 1;
        u32                         is_looping      : 1;
        u32                         reserve1        : 29;
        const char                 *animation_name;
        const char                 *reserve2;
        ResBfresModel              *user_bound_model;
        u16                        *bind_table;
        ResBfresMaterialAnimData   *material_anim_data_array;
        void                      **runtime_texture_view_array;
        const char                **texture_name_array;
        ResGfxUserData             *user_data_array;
        ResNintendoWareDictionary  *user_data_dictionary;
        u64                        *runtime_texture_descriptor_slot_array;
        u32                         frame_count;
        u32                         bake_size;
        u16                         user_data_count;
        u16                         per_material_anim_count;
        u16                         total_anim_curves;
        u16                         shader_param_anim_count;
        u16                         texture_pattern_anim_count;
        u16                         material_visibility_anim_count;
        u16                         texture_count;
        u16                         reserve3;

        static constexpr u32    cMagic = util::TCharCode32("FMAA");
        static constexpr size_t cInvalidDescriptorSlot = 0xffff'ffff'ffff'ffff;

        void BindTextures(GfxBindTextureCallback bind_callback, ResBntx *res_bntx) {
            for (u32 i = 0; i < texture_count; ++i) {
                if (runtime_texture_view_array[i] != nullptr && runtime_texture_descriptor_slot_array[i] != 0xffff'ffff'ffff'ffff) { continue; }
        
                GfxBindTextureView bind_texture_view  = (bind_callback)(res_bntx, texture_name_array[i] + 2);
                runtime_texture_view_array[i]            = bind_texture_view.texture_view;
                runtime_texture_descriptor_slot_array[i] = bind_texture_view.texture_view_decriptor_slot;
            }
        }

        void ReleaseTextures() {
            for (u32 i = 0; i < texture_count; ++i) {
                runtime_texture_descriptor_slot_array[i] = cInvalidDescriptorSlot;
                runtime_texture_view_array[i]            = nullptr;
            }
        }

        bool TrySetTextureByName(const char *name, GfxBindTextureView *bind_texture_view) {

            bool had_success = false;
            for (u32 i = 0; i < texture_count; ++i) {
                if (::strcmp(texture_name_array[i], name) != 0) { continue; }

                runtime_texture_descriptor_slot_array[i] = bind_texture_view->texture_view_decriptor_slot;
                runtime_texture_view_array[i]            = bind_texture_view->texture_view;

                had_success = true;
            }

            return had_success;
        }

        ResGfxUserData *TryGetUserData(const char *user_data_name) {
            if (user_data_dictionary == nullptr) { return nullptr; }
            const u32 entry_id = user_data_dictionary->TryGetEntryIndexByKey(user_data_name);
            if (entry_id == ResNintendoWareDictionary::cInvalidEntryIndex) { return nullptr; }
            return std::addressof(user_data_array[entry_id]);
        }
    };
    static_assert(sizeof(ResBfresMaterialAnim) == 0x70);
}
