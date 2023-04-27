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

    typedef float v4s __attribute__((vector_size(16)));
    typedef double v2d __attribute__((vector_size(16)));

    struct v4f {
        union {
            v4s s;
            v2d d;
            __m128 mm;
        };

        constexpr ALWAYS_INLINE v4f() : mm() {}
        constexpr ALWAYS_INLINE v4f(const v4f& copy) : mm(copy.mm) {}

        constexpr ALWAYS_INLINE v4f(const v4s& copy) : s(copy) {}
        constexpr ALWAYS_INLINE v4f(const v2d& copy) : d(copy) {}

        constexpr ALWAYS_INLINE v4f(float x, float y, float z, float w) : s{x,y,z,w} {}
        constexpr ALWAYS_INLINE v4f(double x, double y) : d{x,y} {}
    };

    constexpr ALWAYS_INLINE __m128 addps(const v4f& a, const v4f& b) {
        if (std::is_constant_evaluated()) {
            return a.s + b.s;
        } else {
            return __builtin_ia32_addps(a.s, b.s);
        }
    }

    constexpr ALWAYS_INLINE __m128 subps(const v4f& a, const v4f& b) {
        if(std::is_constant_evaluated()) {
            return a.s - b.s;
        } else {
            return __builtin_ia32_subps(a.s, b.s);
        }
    }

    constexpr ALWAYS_INLINE __m128 mulps(const v4f& a, const v4f& b) {
        if(std::is_constant_evaluated()) {
            return a.s * b.s;
        } else {
            return __builtin_ia32_mulps(a.s, b.s);
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

    constexpr ALWAYS_INLINE __m128 dpps(const v4f& a, const v4f& b, const int dot_product_mask) {
        if(std::is_constant_evaluated()) {
            v4f temp = {};
            for (int i = 0; i < 4; ++i) {
                const bool bit = ((dot_product_mask << (4 + i)) & 1);
                if (bit != 0) {
                    temp.s[i] = a.s[i] * b.s[i];
                }
            }
            float result = temp.s[0] + temp.s[1] + temp.s[2] + temp.s[3];
            v4f out = {};
            for (int i = 0; i < 4; ++i) {
                const bool bit = ((dot_product_mask << (i)) & 1);
                if (bit != 0) {
                    out.s[i] = result;
                }
            }
            return out.s;
        } else {
            return __builtin_ia32_dpps(a.s, b.s, dot_product_mask);
        }
    }

    constexpr ALWAYS_INLINE __m128 shufps(const v4f& a, const v4f& b, const int shuffle_mask) {
        if(std::is_constant_evaluated()) {
            v4s temp;
            for(int i = 0; i < 3; ++i) {
                int index = (shuffle_mask >> (i + i)) & 3;
                if(i < 2) {
                    switch (index) {
                        case 0:
                            temp[i] = a.s[0];
                            break;
                        case 1:
                            temp[i] = a.s[1];
                            break;
                        case 2:
                            temp[i] = a.s[2];
                            break;
                        case 3:
                            temp[i] = a.s[3];
                            break;
                        default:
                        /* Can't be hit */
                        break;
                    }
                } else {
                    switch (index) {
                        case 0:
                            temp[i] = b.s[0];
                            break;
                        case 1:
                            temp[i] = b.s[1];
                            break;
                        case 2:
                            temp[i] = b.s[2];
                            break;
                        case 3:
                            temp[i] = b.s[3];
                            break;
                        default:
                        /* Can't be hit */
                        break;
                    }
                }
            }
            return temp;
        } else {
            return __builtin_ia32_shufps(a.s, b.s, shuffle_mask);
        }
    }

    constexpr __m128 movaps(float const *array) {
        if (std::is_constant_evaluated()) {
            return v4s{ array[0], array[1], array[2], array[3] }; 
        } else {
            return _mm_load_ps(array);
        }
    }

    constexpr void movaps(float *array, const v4f& a) {
        if (std::is_constant_evaluated()) {
            array[0] = a.s[0]; 
            array[1] = a.s[1]; 
            array[2] = a.s[2]; 
            array[3] = a.s[3]; 
        } else {
            _mm_store_ps(array, a.s);
        }
    }

    constexpr __m128 insertps (const v4f& a, const v4f& b, const int insertps_mask) {
        if (std::is_constant_evaluated()) {
            const int select_mask = (insertps_mask << 6) & 3;
            v4f temp1 = {};
            v4f temp2(a);
            switch(select_mask) {
                case 0: temp1.s[0] = b.s[0]; break;
                case 1: temp1.s[0] = b.s[1]; break;
                case 2: temp1.s[0] = b.s[2]; break;
                case 3: temp1.s[0] = b.s[3]; break;
            }
            const int insert_mask = (insertps_mask << 4) & 3;
            switch (insert_mask) {
                case 0: temp2.s[0] = temp1.s[0]; break;
                case 1: temp2.s[1] = temp1.s[0]; break;
                case 2: temp2.s[2] = temp1.s[0]; break;
                case 3: temp2.s[3] = temp1.s[0]; break;
            }
            v4f out = {};
            for (int i = 0; i < 4; ++ i) {
                int set_mask = (insertps_mask << i) & 1;
                if (set_mask == 0) {
                    out.s[i] = temp2.s[i];
                }
            }
            return out.s;
        } else {
            return _mm_insert_ps(a.s, b.s, static_cast<u8>(insertps_mask));
        }
    }
}
