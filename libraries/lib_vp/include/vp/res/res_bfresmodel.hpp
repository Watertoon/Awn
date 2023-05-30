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

    struct ResBfresModel {
        u32                        magic;
        u32                        reserve0;
        const char                *model_name;
        const char                *reserve1;
        ResBfresSkeleton          *skeleton;
        ResBfresVertex            *vertex_array;
        ResBfresShape             *shape_array;
        ResNintendoWareDictionary *shape_dictionary;
        ResBfresMaterial          *material_array;
        ResNintendoWareDictionary *material_dictionary;
        ResBfresShaderReflection  *shader_reflection_array;
        ResGfxUserData            *user_data_array;
        ResNintendoWareDictionary *user_data_dictionary;
        void                      *user_pointer;
        u16                        vertex_count;
        u16                        shape_count;
        u16                        material_count;
        u16                        shader_reflection_count;
        u16                        user_data_count;
        u16                        reserve2;
        u32                        reserve3;

        static constexpr u32 cMagic = util::TCharCode32("FMDL");

        void BindTexture(GfxBindTextureCallback bind_callback, ResBntx *res_bntx) {
            const u32 mat_count = material_count;
            for (u32 i = 0; i < mat_count; ++i) {
                material_array[i].BindTexture(bind_callback, res_bntx);
            }
        }
    };
    static_assert(sizeof(ResBfresModel) == 0x78);
}
