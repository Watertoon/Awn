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

    enum class BfresAnimCurveFrameDataType : u8 {
        Float = 0,
        F16   = 1,
        U8    = 2,
    };
    enum class BfresAnimCurveValueDataType : u8 {
        Float = 0,
        S32   = 0,
        S16   = 1,
        S8    = 2,
    };
    enum class BfresAnimCurveCurveType : u8 {
        CubicFloat   = 0,
        LinearFloat  = 1,
        BakedFloat   = 2,
        StepInteger  = 3,
        BakedInteger = 4,
        StepBoolean  = 5,
        BakedBoolean = 6,
    };
    enum class BfresAnimCurveWrapMode : u8 {
        Clamp          = 0,
        Repeat         = 1,
        Mirror         = 2,
        RelativeRepeat = 3,
    };

    struct ResBfresAnimCurve {
        union {
            float *frame_array_f32;
            s16   *frame_array_s16;
            u8    *frame_array_u8;
        };
        union {
            float *value_array_f32;
            s32   *value_array_s32;
            s16   *value_array_s16;
            s8    *value_array_s8;
        };
        u16   frame_data_type : 2;
        u16   value_data_type : 2;
        u16   curve_type      : 3;
        u16   reserve0        : 1;
        u16   pre_wrap_mode   : 2;
        u16   reserve1        : 2;
        u16   post_wrap_mode  : 2;
        u16   reserve2        : 2;
        u16   frame_count;
        u32   base_result_offset;
        float start_key;
        float end_key;
        float frame_data_scale;
        float frame_data_add;
        float frame_delta;
        u32   reserve3;
    };
    static_assert(sizeof(ResBfresAnimCurve) == 0x30);
}
