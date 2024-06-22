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

    constexpr ALWAYS_INLINE __m256 addps(const v256f& a, const v256f& b) {
        if (!std::is_constant_evaluated()) {
            return __builtin_ia32_addps256(a.f, b.f);
        }
        return a.f + b.f;
    }

    constexpr ALWAYS_INLINE __m256 subps(const v256f& a, const v256f& b) {
        if(!std::is_constant_evaluated()) {
            return __builtin_ia32_subps256(a.f, b.f);
        }
        return a.f - b.f;
    }

    constexpr ALWAYS_INLINE __m256 mulps(const v256f& a, const v256f& b) {
        if(!std::is_constant_evaluated()) {
            return __builtin_ia32_mulps256(a.f, b.f);
        }
        return a.f * b.f;
    }

    constexpr ALWAYS_INLINE __m256 fmaddps(const v256f& a, const v256f& b, const v256f& c) {
        if(!std::is_constant_evaluated()) {
            return __builtin_ia32_vfmaddps256(a.f, b.f, c.f);
        }
        v256f temp;
        for (u32 i = 0; i < 8; ++i) {
            temp.f[i] = (a.f[i] * b.f[i]) + c.f[i];
        }
        return temp.mmf;
    }

    constexpr ALWAYS_INLINE __m256 fnmaddps(const v256f& a, const v256f& b, const v256f& c) {
        if(!std::is_constant_evaluated()) {
            return __builtin_ia32_vfnmaddps256(a.f, b.f, c.f);
        }
        v256f temp;
        for (u32 i = 0; i < 8; ++i) {
            temp.f[i] = -(a.f[i] * b.f[i]) + c.f[i];
        }
        return temp.mmf;
    }

    constexpr ALWAYS_INLINE __m256 fmsubps(const v256f& a, const v256f& b, const v256f& c) {
        if(!std::is_constant_evaluated()) {
            return __builtin_ia32_vfmsubps256(a.f, b.f, c.f);
        }
        v256f temp;
        for (u32 i = 0; i < 8; ++i) {
            temp.f[i] = (a.f[i] * b.f[i]) - c.f[i];
        }
        return temp.mmf;
    }

    constexpr ALWAYS_INLINE __m256 fnmsubps(const v256f& a, const v256f& b, const v256f& c) {
        if(!std::is_constant_evaluated()) {
            return __builtin_ia32_vfnmsubps256(a.f, b.f, c.f);
        }
        v256f temp;
        for (u32 i = 0; i < 8; ++i) {
            temp.f[i] = -(a.f[i] * b.f[i]) - c.f[i];
        }
        return temp.mmf;
    }

    constexpr ALWAYS_INLINE __m256 fmaddsubps(const v256f& a, const v256f& b, const v256f& c) {
        if(!std::is_constant_evaluated()) {
            return __builtin_ia32_vfmaddsubps256(a.f, b.f, c.f);
        }
        v256f temp;
        for (u32 i = 0; i < 8; ++i) {
            if ((i & 1) == 0) {
                temp.f[i] = (a.f[i] * b.f[i]) - c.f[i];
            } else {
                temp.f[i] = (a.f[i] * b.f[i]) + c.f[i];
            }
        }
        return temp.mmf;
    }

    constexpr ALWAYS_INLINE __m256 fmsubaddps(const v256f& a, const v256f& b, const v256f& c) {
        if(!std::is_constant_evaluated()) {
            return _mm256_fmsubadd_ps(a.f, b.f, c.f);
        }
        v256f temp;
        for (u32 i = 0; i < 8; ++i) {
            if ((i & 1) == 0) {
                temp.f[i] = (a.f[i] * b.f[i]) + c.f[i];
            } else {
                temp.f[i] = (a.f[i] * b.f[i]) - c.f[i];
            }
        }
        return temp.mmf;
    }

    constexpr ALWAYS_INLINE __m256 vbroadcastss256(float *value) {
        if (!std::is_constant_evaluated()) {
            return __builtin_ia32_vbroadcastss256(value);
        }
        const float v = *value;
        return __m256{v,v,v,v,v,v,v,v};
    }

    constexpr ALWAYS_INLINE __m256 shufps(const v256f& a, const v256f& b, const int shuffle_mask) {
        if(!std::is_constant_evaluated()) {
            return __builtin_ia32_shufps256(a.f, b.f, shuffle_mask);
        }
        v8f temp;
        for(int i = 0; i < 8; ++i) {
            int index = (shuffle_mask >> (i + i)) & 3;
            if(i < 4) {
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

    consteval v256i GetLoadMask256(bool is_x0, bool is_y0, bool is_z0, bool is_w0, bool is_x1, bool is_y1, bool is_z1, bool is_w1) {
        return v256i{ .ui = {is_x0, is_y0, is_z0, is_w0, is_x1, is_y1, is_z1, is_w1 }, };
    }

    constexpr __m256 maskmovps256(float const *array, const v256i& mask) {
        if (!std::is_constant_evaluated()) {
            return _mm256_maskload_ps(array, mask.mm);
        }
        v256f temp = {};
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
        if (mask.ui[4]) {
            temp.f[4] = array[4];
        }
        if (mask.ui[5]) {
            temp.f[5] = array[5];
        }
        if (mask.ui[6]) {
            temp.f[6] = array[6];
        }
        if (mask.ui[7]) {
            temp.f[7] = array[7];
        }
        return temp.mmf;
    }

    constexpr __m256 movaps256(float const *array) {
        if (!std::is_constant_evaluated()) {
            return _mm256_load_ps(array);
        }
        return v8f{ array[0], array[1], array[2], array[3], array[4], array[5], array[6], array[7] }; 
    }
    constexpr __m256 movups256(float const *array) {
        if (!std::is_constant_evaluated()) {
            return __builtin_ia32_loadups256(array);
        }
        return v8f{ array[0], array[1], array[2], array[3], array[4], array[5], array[6], array[7] }; 
    }

    constexpr void movaps256(float *array, const v256f& a) {
        if (!std::is_constant_evaluated()) {
            _mm256_store_ps(array, a.f);
            return;
        }
        array[0] = a.f[0];
        array[1] = a.f[1];
        array[2] = a.f[2];
        array[3] = a.f[3];
        array[4] = a.f[4];
        array[5] = a.f[5];
        array[6] = a.f[6];
        array[7] = a.f[7];
    }
    constexpr void movups256(float *array, const v256f& a) {
        if (!std::is_constant_evaluated()) {
            __builtin_ia32_storeups256(array, a.f);
            return;
        }
        array[0] = a.f[0];
        array[1] = a.f[1];
        array[2] = a.f[2];
        array[3] = a.f[3];
        array[4] = a.f[4];
        array[5] = a.f[5];
        array[6] = a.f[6];
        array[7] = a.f[7];
    }

    constexpr __m256 insertf128ps(const v256f& a, const v128f& b, const int insertf128_mask) {
        if (!std::is_constant_evaluated()) {
            return __builtin_ia32_vinsertf128_ps256(a.f, b.f, static_cast<u8>(insertf128_mask));
        }
        const int select_mask = (insertf128_mask & 1);
        v256f temp0(a);
        switch(select_mask) {
            case 0: 
            {
                temp0.f[0] = b.f[0];
                temp0.f[1] = b.f[1];
                temp0.f[2] = b.f[2];
                temp0.f[3] = b.f[3];
                break;
            }
            case 1: 
            {
                temp0.f[4] = b.f[0];
                temp0.f[5] = b.f[1];
                temp0.f[6] = b.f[2];
                temp0.f[7] = b.f[3];
                break;
            }
        }
        return temp0.mmf;
    }

    constexpr __m128 extractf128ps(const v256f& a, int extract_mask) {
        if(!std::is_constant_evaluated()) {
            return __builtin_ia32_vextractf128_ps256(a.f, extract_mask);
        }
        v128f temp = {};
        switch (extract_mask & 1) {
            case 0:
            {
                temp.f[0] = a.f[0];
                temp.f[1] = a.f[1];
                temp.f[2] = a.f[2];
                temp.f[3] = a.f[3];
                break;
            }
            case 1:
            {
                temp.f[0] = a.f[4];
                temp.f[1] = a.f[5];
                temp.f[2] = a.f[6];
                temp.f[3] = a.f[7];
                break;
            }
        };
        return temp.mmf;
    }
}
