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
        if (std::is_constant_evaluated()) {
            return a.f + b.f;
        } else {
            return __builtin_ia32_addps(a.f, b.f);
        }
    }

    constexpr ALWAYS_INLINE __m128 subps(const v128f& a, const v128f& b) {
        if(std::is_constant_evaluated()) {
            return a.f - b.f;
        } else {
            return __builtin_ia32_subps(a.f, b.f);
        }
    }

    constexpr ALWAYS_INLINE __m128 mulps(const v128f& a, const v128f& b) {
        if(std::is_constant_evaluated()) {
            return a.f * b.f;
        } else {
            return __builtin_ia32_mulps(a.f, b.f);
        }
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
        if(std::is_constant_evaluated()) {
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
        } else {
            return __builtin_ia32_dpps(a.f, b.f, dot_product_mask);
        }
    }

    constexpr ALWAYS_INLINE __m128 shufps(const v128f& a, const v128f& b, const int shuffle_mask) {
        if(std::is_constant_evaluated()) {
            v4f temp;
            for(int i = 0; i < 3; ++i) {
                int index = (shuffle_mask >> (i + i)) & 3;
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
        } else {
            return __builtin_ia32_shufps(a.f, b.f, shuffle_mask);
        }
    }

    constexpr __m128 movaps(float const *array) {
        if (std::is_constant_evaluated()) {
            return v4f{ array[0], array[1], array[2], array[3] }; 
        } else {
            return _mm_load_ps(array);
        }
    }

    constexpr void movaps(float *array, const v128f& a) {
        if (std::is_constant_evaluated()) {
            array[0] = a.f[0]; 
            array[1] = a.f[1]; 
            array[2] = a.f[2]; 
            array[3] = a.f[3]; 
        } else {
            _mm_store_ps(array, a.f);
        }
    }

    constexpr __m128 insertps (const v128f& a, const v128f& b, const int insertps_mask) {
        if (std::is_constant_evaluated()) {
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
        } else {
            return _mm_insert_ps(a.f, b.f, static_cast<u8>(insertps_mask));
        }
    }
}
