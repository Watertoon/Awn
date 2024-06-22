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

namespace vp::util::avx2 {

    constexpr ALWAYS_INLINE __m128 addps(const v128f& a, const v128f& b) {
        if (!std::is_constant_evaluated()) {
            return __builtin_ia32_addps(a.f, b.f);
        }
        return a.f + b.f;
    }

    constexpr ALWAYS_INLINE __m128 subps(const v128f& a, const v128f& b) {
        if(!std::is_constant_evaluated()) {
            return __builtin_ia32_subps(a.f, b.f);
        }
        return a.f - b.f;
    }

    constexpr ALWAYS_INLINE __m128 mulps(const v128f& a, const v128f& b) {
        if(!std::is_constant_evaluated()) {
            return __builtin_ia32_mulps(a.f, b.f);
        }
        return a.f * b.f;
    }

    constexpr ALWAYS_INLINE __m128 fmaddps(const v128f& a, const v128f& b, const v128f& c) {
        if(!std::is_constant_evaluated()) {
            return __builtin_ia32_vfmaddps(a.f, b.f, c.f);
        }
        v128f temp;
        for (u32 i = 0; i < 4; ++i) {
            temp.f[i] = (a.f[i] * b.f[i]) + c.f[i];
        }
        return temp.mmf;
    }

    constexpr ALWAYS_INLINE __m128 fnmaddps(const v128f& a, const v128f& b, const v128f& c) {
        if(!std::is_constant_evaluated()) {
            return __builtin_ia32_vfnmaddps(a.f, b.f, c.f);
        }
        v128f temp;
        for (u32 i = 0; i < 4; ++i) {
            temp.f[i] = -(a.f[i] * b.f[i]) + c.f[i];
        }
        return temp.mmf;
    }

    constexpr ALWAYS_INLINE __m128 fmsubps(const v128f& a, const v128f& b, const v128f& c) {
        if(!std::is_constant_evaluated()) {
            return __builtin_ia32_vfmsubps(a.f, b.f, c.f);
        }
        v128f temp;
        for (u32 i = 0; i < 4; ++i) {
            temp.f[i] = (a.f[i] * b.f[i]) - c.f[i];
        }
        return temp.mmf;
    }

    constexpr ALWAYS_INLINE __m128 fnmsubps(const v128f& a, const v128f& b, const v128f& c) {
        if(!std::is_constant_evaluated()) {
            return __builtin_ia32_vfnmsubps(a.f, b.f, c.f);
        }
        v128f temp;
        for (u32 i = 0; i < 4; ++i) {
            temp.f[i] = -(a.f[i] * b.f[i]) - c.f[i];
        }
        return temp.mmf;
    }

    constexpr ALWAYS_INLINE __m128 fmaddsubps(const v128f& a, const v128f& b, const v128f& c) {
        if(!std::is_constant_evaluated()) {
            return __builtin_ia32_vfmaddsubps(a.f, b.f, c.f);
        }
        v128f temp;
        for (u32 i = 0; i < 4; ++i) {
            if ((i & 1) == 0) {
                temp.f[i] = (a.f[i] * b.f[i]) - c.f[i];
            } else {
                temp.f[i] = (a.f[i] * b.f[i]) + c.f[i];
            }
        }
        return temp.mmf;
    }

    constexpr ALWAYS_INLINE __m128 fmsubaddps(const v128f& a, const v128f& b, const v128f& c) {
        if(!std::is_constant_evaluated()) {
            return _mm_fmsubadd_ps(a.f, b.f, c.f);
        }
        v128f temp;
        for (u32 i = 0; i < 4; ++i) {
            if ((i & 1) == 0) {
                temp.f[i] = (a.f[i] * b.f[i]) + c.f[i];
            } else {
                temp.f[i] = (a.f[i] * b.f[i]) - c.f[i];
            }
        }
        return temp.mmf;
    }

    consteval int GetSelectionMask(const int x, const int y = false, const int z = false, const int w = false) {
        const int mask = x | (y << 1) | (z << 2) | (w << 3);
        return mask;
    }

    consteval int GetDotProductMask(const int selection_mask, const int result_selection_mask) {
        int result_mask = Clamp(result_selection_mask, 0, 15);
        int select_mask = Clamp(selection_mask, 0, 15);
        return (result_mask) | (select_mask << 4);
    }

    constexpr ALWAYS_INLINE __m128 dpps(const v128f& a, const v128f& b, const int dot_product_mask) {
        if(!std::is_constant_evaluated()) {
            return __builtin_ia32_dpps(a.f, b.f, dot_product_mask);
        }
        v128f temp = {};
        for (int i = 0; i < 4; ++i) {
            const bool bit = ((dot_product_mask << (4 + i)) & 1);
            if (bit != 0) {
                temp.f[i] = a.f[i] * b.f[i];
            }
        }
        float result = temp.f[0] + temp.f[1] + temp.f[2] + temp.f[3];
        v128f out = {};
        for (int i = 0; i < 4; ++i) {
            const bool bit = ((dot_product_mask << (i)) & 1);
            if (bit != 0) {
                out.f[i] = result;
            }
        }
        return out.f;
    }

    constexpr ALWAYS_INLINE __m128 shufps(const v128f& a, const v128f& b, const int shuffle_mask) {
        if(!std::is_constant_evaluated()) {
            return __builtin_ia32_shufps(a.f, b.f, shuffle_mask);
        }
        v4f temp;
        for(int i = 0; i < 4; ++i) {
            int index = (shuffle_mask >> (i * 2)) & 3;
            if(i < 2) {
                switch (index) {
                    case 0:
                        temp[i] = a.f[0];
                        break;
                    case 1:
                        temp[i] = a.f[1];
                        break;
                    case 2:
                        temp[i] = a.f[2];
                        break;
                    case 3:
                        temp[i] = a.f[3];
                        break;
                    default:
                    /* Can't be hit */
                    break;
                }
            } else {
                switch (index) {
                    case 0:
                        temp[i] = b.f[0];
                        break;
                    case 1:
                        temp[i] = b.f[1];
                        break;
                    case 2:
                        temp[i] = b.f[2];
                        break;
                    case 3:
                        temp[i] = b.f[3];
                        break;
                    default:
                    /* Can't be hit */
                    break;
                }
            }
        }
        return temp;
    }

    constexpr __m128 vbroadcastss128(float *value) {
        if (!std::is_constant_evaluated()) {
            return __builtin_ia32_vbroadcastss(value);
        }
        const float v = *value;
        return __m128{v,v,v,v};
    }

    consteval v128i GetLoadMaskFloat128(bool is_x, bool is_y, bool is_z, bool is_w) {
        return v128i{ .ui = {is_x, is_y, is_z, is_w}, };
    }

    constexpr __m128 maskmovps128(float const *array, const v128i& mask) {
        if (!std::is_constant_evaluated()) {
            return _mm_maskload_ps(array, mask.mm);
        }
        v128f temp = {};
        if (mask.ui[0]) {
            temp.f[0] = array[0];
        }
        if (mask.ui[1]) {
            temp.f[1] = array[1];
        }
        if (mask.ui[2]) {
            temp.f[2] = array[2];
        }
        if (mask.ui[3]) {
            temp.f[3] = array[3];
        }
        return temp.mmf;
    }

    constexpr __m128 movaps128(float const *array) {
        if (!std::is_constant_evaluated()) {
            return _mm_load_ps(array);
        }
        return v4f{ array[0], array[1], array[2], array[3] }; 
    }
    constexpr __m128 movups128(float const *array) {
        if (!std::is_constant_evaluated()) {
            return __builtin_ia32_loadups(array);
        }
        return v4f{ array[0], array[1], array[2], array[3] }; 
    }

    constexpr void movaps128(float *array, const v128f& a) {
        if (!std::is_constant_evaluated()) {
            __builtin_ia32_storeups(array, a.f);
        }
        array[0] = a.f[0]; 
        array[1] = a.f[1]; 
        array[2] = a.f[2]; 
        array[3] = a.f[3]; 
    }

    constexpr __m128 insertps(const v128f& a, const v128f& b, const int insertps_mask) {
        if (!std::is_constant_evaluated()) {
            return __builtin_ia32_insertps128(a.f, b.f, static_cast<u8>(insertps_mask));
        }
        const int select_mask = (insertps_mask << 6) & 3;
        v128f temp1 = {};
        v128f temp2(a);
        switch(select_mask) {
            case 0: temp1.f[0] = b.f[0]; break;
            case 1: temp1.f[0] = b.f[1]; break;
            case 2: temp1.f[0] = b.f[2]; break;
            case 3: temp1.f[0] = b.f[3]; break;
        }
        const int insert_mask = (insertps_mask << 4) & 3;
        switch (insert_mask) {
            case 0: temp2.f[0] = temp1.f[0]; break;
            case 1: temp2.f[1] = temp1.f[0]; break;
            case 2: temp2.f[2] = temp1.f[0]; break;
            case 3: temp2.f[3] = temp1.f[0]; break;
        }
        v128f out = {};
        for (int i = 0; i < 4; ++ i) {
            int set_mask = (insertps_mask << i) & 1;
            if (set_mask == 0) {
                out.f[i] = temp2.f[i];
            }
        }
        return out.f;
    }
}
