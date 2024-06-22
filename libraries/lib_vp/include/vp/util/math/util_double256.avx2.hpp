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

    constexpr ALWAYS_INLINE __m256d addpd(const v256f& a, const v256f& b) {
        if (!std::is_constant_evaluated()) {
            return __builtin_ia32_addpd256(a.d, b.d);
        }
        return a.d + b.d;
    }

    constexpr ALWAYS_INLINE __m256d subpd(const v256f& a, const v256f& b) {
        if(!std::is_constant_evaluated()) {
            return __builtin_ia32_subpd256(a.d, b.d);
        }
        return a.d - b.d;
    }

    constexpr ALWAYS_INLINE __m256d mulpd(const v256f& a, const v256f& b) {
        if(!std::is_constant_evaluated()) {
            return __builtin_ia32_mulpd256(a.d, b.d);
        }
        return a.d * b.d;
    }

    constexpr ALWAYS_INLINE __m256d fmaddpd(const v256f& a, const v256f& b, const v256f& c) {
        if(!std::is_constant_evaluated()) {
            return __builtin_ia32_vfmaddpd256(a.d, b.d, c.d);
        }
        v256f temp;
        for (u32 i = 0; i < 4; ++i) {
            temp.d[i] = (a.d[i] * b.d[i]) + c.d[i];
        }
        return temp.mmd;
    }

    constexpr ALWAYS_INLINE __m256d fnmaddpd(const v256f& a, const v256f& b, const v256f& c) {
        if(!std::is_constant_evaluated()) {
            return __builtin_ia32_vfnmaddpd256(a.d, b.d, c.d);
        }
        v256f temp;
        for (u32 i = 0; i < 4; ++i) {
            temp.d[i] = -(a.d[i] * b.d[i]) + c.d[i];
        }
        return temp.mmd;
    }

    constexpr ALWAYS_INLINE __m256d fmsubpd(const v256f& a, const v256f& b, const v256f& c) {
        if(!std::is_constant_evaluated()) {
            return __builtin_ia32_vfmsubpd256(a.d, b.d, c.d);
        }
        v256f temp;
        for (u32 i = 0; i < 4; ++i) {
            temp.d[i] = (a.d[i] * b.d[i]) - c.d[i];
        }
        return temp.mmd;
    }

    constexpr ALWAYS_INLINE __m256d fnmsubpd(const v256f& a, const v256f& b, const v256f& c) {
        if(!std::is_constant_evaluated()) {
            return __builtin_ia32_vfnmsubpd256(a.d, b.d, c.d);
        }
        v256f temp;
        for (u32 i = 0; i < 4; ++i) {
            temp.d[i] = -(a.d[i] * b.d[i]) - c.d[i];
        }
        return temp.mmd;
    }

    constexpr ALWAYS_INLINE __m256d fmaddsubpd(const v256f& a, const v256f& b, const v256f& c) {
        if(!std::is_constant_evaluated()) {
            return __builtin_ia32_vfmaddsubpd256(a.d, b.d, c.d);
        }
        v256f temp;
        for (u32 i = 0; i < 4; ++i) {
            if ((i & 1) == 0) {
                temp.d[i] = (a.d[i] * b.d[i]) - c.d[i];
            } else {
                temp.d[i] = (a.d[i] * b.d[i]) + c.d[i];
            }
        }
        return temp.mmd;
    }

    constexpr ALWAYS_INLINE __m256d fmsubaddpd(const v256f& a, const v256f& b, const v256f& c) {
        if(!std::is_constant_evaluated()) {
            return _mm256_fmsubadd_pd(a.d, b.d, c.d);
        }
        v256f temp;
        for (u32 i = 0; i < 4; ++i) {
            if ((i & 1) == 0) {
                temp.d[i] = (a.d[i] * b.d[i]) + c.d[i];
            } else {
                temp.d[i] = (a.d[i] * b.d[i]) - c.d[i];
            }
        }
        return temp.mmd;
    }

    consteval int GetPermute2f128Mask(int component0, int component1) {
        return (component1 << 4) | component0;
    }

    constexpr __m256d perm2f128pd(const v256f& a, const v256f& b, const int permute2f128_mask) {
        if(!std::is_constant_evaluated()) {
            return __builtin_ia32_vperm2f128_pd256(a.d, b.d, permute2f128_mask);
        }
        v256f temp;
        for (int i = 0; i < 2; ++i) {
            const int control = (permute2f128_mask >> (i * 4));
            switch(control & 4) {
                case 0:
                {
                    temp.d[(i * 2) + 0] = a.d[0];
                    temp.d[(i * 2) + 1] = a.d[1];
                    break;
                }
                case 1:
                {
                    temp.d[(i * 2) + 0] = a.d[2];
                    temp.d[(i * 2) + 1] = a.d[3];
                    break;
                }
                case 2:
                {
                    temp.d[(i * 2) + 0] = b.d[0];
                    temp.d[(i * 2) + 1] = b.d[1];
                    break;
                }
                case 3:
                {
                    temp.d[(i * 2) + 0] = b.d[2];
                    temp.d[(i * 2) + 1] = b.d[3];
                    break;
                }
            };
            if (((control >> 3) & 1) == 1) {
                temp.d[0] = 0;
                temp.d[1] = 0;
            }
        }
        return temp.mmd;
    }

    constexpr __m256d permilpd(const v256f& a, const int permute_mask) {
        if(!std::is_constant_evaluated()) {
            return __builtin_ia32_vpermilpd256(a.d, permute_mask);
        }
        v256f temp;
        temp.d[0] = (((permute_mask >> 0) & 1) == 0) ? a.d[0] : a.d[1];
        temp.d[1] = (((permute_mask >> 1) & 1) == 0) ? a.d[0] : a.d[1];
        temp.d[2] = (((permute_mask >> 2) & 1) == 0) ? a.d[2] : a.d[3];
        temp.d[3] = (((permute_mask >> 3) & 1) == 0) ? a.d[2] : a.d[3];
        return temp.mmd;
    }

    consteval int GetShuffleDouble256Mask(int comp0, int comp1, int comp2, int comp3) {
        return (comp3 << 3) | (comp2 << 2) | (comp1 << 1) | comp0;
    }

    constexpr __m256d shufpd(const v256f& a, const v256f& b, const int shuffle_mask) {
        if(!std::is_constant_evaluated()) {
            return __builtin_ia32_shufpd256(a.d, b.d, shuffle_mask);
        }
        v256f temp;
        temp.d[0] = (((shuffle_mask >> 0) & 1) == 0) ? a.d[0] : a.d[1];
        temp.d[1] = (((shuffle_mask >> 1) & 1) == 0) ? b.d[0] : b.d[1];
        temp.d[2] = (((shuffle_mask >> 2) & 1) == 0) ? a.d[2] : a.d[3];
        temp.d[3] = (((shuffle_mask >> 3) & 1) == 0) ? b.d[2] : b.d[3];
        return temp.mmd;
    }

    consteval v256i GetLoadMaskDouble256(bool is_x, bool is_y, bool is_z, bool is_w) {
        return v256i{ .ull = {is_x, is_y, is_z, is_w}, };
    }

    constexpr __m256d maskmovpd256(double const *array, const v256i& mask) {
        if (!std::is_constant_evaluated()) {
            return _mm256_maskload_pd(array, mask.mm);
        }
        v256f temp = {};
        if (mask.mm[0]) {
            temp.d[0] = array[0];
        }
        if (mask.mm[1]) {
            temp.d[1] = array[1];
        }
        if (mask.mm[2]) {
            temp.d[2] = array[2];
        }
        if (mask.mm[3]) {
            temp.d[3] = array[3];
        }
        return temp.mmd;
    }

    constexpr __m256d vbroadcastsd256(double const *value) {
        if (!std::is_constant_evaluated()) {
            return __builtin_ia32_vbroadcastsd256(value);
        }
        const double v = *value;
        return __m256d{v,v,v,v};
    }

    constexpr __m256d movapd256(double const *array) {
        if (!std::is_constant_evaluated()) {
            return _mm256_load_pd(array);
        }
        return v4d{ array[0], array[1], array[2], array[3], }; 
    }
    constexpr __m256d movupd256(double const *array) {
        if (!std::is_constant_evaluated()) {
            return __builtin_ia32_loadupd256(array);
        }
        return v4d{ array[0], array[1], array[2], array[3], }; 
    }

    constexpr void movapd256(double *array, const v256f& a) {
        if (!std::is_constant_evaluated()) {
            _mm256_store_pd(array, a.d);
        }
        array[0] = a.d[0]; 
        array[1] = a.d[1]; 
        array[2] = a.d[2]; 
        array[3] = a.d[3]; 
    }
    constexpr void movupd256(double *array, const v256f& a) {
        if (!std::is_constant_evaluated()) {
            __builtin_ia32_storeupd256(array, a.d);
        }
        array[0] = a.d[0]; 
        array[1] = a.d[1]; 
        array[2] = a.d[2]; 
        array[3] = a.d[3]; 
    }

    constexpr __m256d insertf128pd(const v256f& a, const v128f& b, const int insertf128_mask) {
        if (!std::is_constant_evaluated()) {
            return __builtin_ia32_vinsertf128_pd256(a.d, b.d, insertf128_mask);
        }
        const int select_mask = (insertf128_mask & 1);
        v256f temp0(a);
        switch(select_mask) {
            case 0: 
            {
                temp0.d[0] = b.d[0];
                temp0.d[1] = b.d[1];
                break;
            }
            case 1: 
            {
                temp0.d[2] = b.d[0];
                temp0.d[3] = b.d[1];
                break;
            }
        }
        return temp0.mmd;
    }

    constexpr __m128d extractf128pd(const v256f& a, int extractf128_mask) {
        if(!std::is_constant_evaluated()) {
            return __builtin_ia32_vextractf128_pd256(a.d, extractf128_mask);
        }
        v128f temp = {};
        switch (extractf128_mask) {
            case 0:
            {
                temp.d[0] = a.d[0];
                temp.d[1] = a.d[1];
                break;
            }
            case 1:
            {
                temp.d[0] = a.d[2];
                temp.d[1] = a.d[3];
                break;
            }
        };
        return temp.mmd;
    }
}
