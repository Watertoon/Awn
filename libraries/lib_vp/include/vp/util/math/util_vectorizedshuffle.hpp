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

namespace vp::util {

    consteval int GetShuffle4Order(int comp0, int comp1, int comp2, int comp3) {
        const int result0 = ((comp0 & 1) << 0);
        const int result1 = ((comp1 & 1) << 1);
        const int result2 = ((comp2 & 1) << 2);
        const int result3 = ((comp3 & 1) << 3);
        return result3 | result2 | result1 | result0;
    }

    consteval int GetShuffle8Order(int comp0, int comp1, int comp2, int comp3,int comp4, int comp5, int comp6, int comp7) {
        const int result0 = ((comp0 & 3) << 0);
        const int result1 = ((comp1 & 3) << 2);
        const int result2 = ((comp2 & 3) << 4);
        const int result3 = ((comp3 & 3) << 6);
        const int result4 = ((comp4 & 3) << 8);
        const int result5 = ((comp5 & 3) << 10);
        const int result6 = ((comp6 & 3) << 12);
        const int result7 = ((comp7 & 3) << 14);
        return result7 | result6 | result5 | result4 | result3 | result2 | result1 | result0;
    }

    template <typename T>
        requires (T::cIsVectorizedType && T::cCount == 4)
    constexpr T Shuffle4(const T &a, const T &b, const int shuffle_mask) {
        using v4t = T;
        using Type = T::UnderlyingType;
        if(!std::is_constant_evaluated()) {
#ifdef VP_TARGET_ARCHITECTURE_x86
            if constexpr (std::is_same_v<Type, float>) {
                return v4t{ _mm_shuffle_ps(a.v, b.v, shuffle_mask) };
            } else if constexpr (std::is_same_v<Type, double>) {
                return v4t{ _mm256_shuffle_pd(a.v, b.v, shuffle_mask) };
            } else if constexpr (std::is_same_v<Type, int>) {
                return v4t{ _mm_shuffle_epi32(a.v, b.v, shuffle_mask) };
            }
#endif
        }
        v4t temp;
        temp[0] = (((shuffle_mask >> 0) & 1) == 0) ? a[0] : a[1];
        temp[1] = (((shuffle_mask >> 1) & 1) == 0) ? b[0] : b[1];
        temp[2] = (((shuffle_mask >> 2) & 1) == 0) ? a[2] : a[3];
        temp[3] = (((shuffle_mask >> 3) & 1) == 0) ? b[2] : b[3];
        return temp;
    }

    template <typename T>
        requires (T::cIsVectorizedType && T::cCount == 8)
    constexpr ALWAYS_INLINE T Shuffle8(const T& a, const T& b, const int shuffle_mask) {
        using v8t = T;
        using Type = T::UnderlyingType;
        if(!std::is_constant_evaluated()) {
#ifdef VP_TARGET_ARCHITECTURE_x86
            if constexpr (std::is_same_v<Type, float>) {
                return v8t{ _mm256_shuffle_ps(a.v, b.v, shuffle_mask) };
            } else if constexpr (std::is_same_v<Type, int>) {
                return v8t{ _mm256_shuffle_epi32(a.v, b.v, shuffle_mask) };
            }
#endif
        }
        v8t temp;
        for(int i = 0; i < 8; ++i) {
            int index = (shuffle_mask >> (i + i)) & 3;
            if ((i >> 1) & 1) {
                switch (index) {
                    case 0:
                        temp[i] = a[0];
                        break;
                    case 1:
                        temp[i] = a[1];
                        break;
                    case 2:
                        temp[i] = a[2];
                        break;
                    case 3:
                        temp[i] = a[3];
                        break;
                    default:
                    /* Can't be hit */
                    break;
                }
            } else {
                switch (index) {
                    case 0:
                        temp[i] = b[0];
                        break;
                    case 1:
                        temp[i] = b[1];
                        break;
                    case 2:
                        temp[i] = b[2];
                        break;
                    case 3:
                        temp[i] = b[3];
                        break;
                    default:
                    /* Can't be hit */
                    break;
                }
            }
        }
        return temp;
    }

    consteval int GetSwizzle4Order(int index0, int index1, int index2, int index3) {
        const int result0 = ((index0 & 3) << 0);
        const int result1 = ((index1 & 3) << 2);
        const int result2 = ((index2 & 3) << 4);
        const int result3 = ((index3 & 3) << 6);
        return result3 | result2 | result1 | result0;
    }

    consteval v8si GetSwizzle8Order256(int index0, int index1, int index2, int index3,int index4, int index5, int index6, int index7) {
        return v8si{ index0 & 7, index1 & 7, index2 & 7, index3 & 7, index4 & 7, index5 & 7, index6 & 7, index7 & 7, };
    }

    template <typename T>
        requires (T::cIsVectorizedType && T::cCount == 4)
    constexpr T Swizzle4(const T &a, const int swizzle_mask) {
        using v4t = T;
        using Type = T::UnderlyingType;
        if(!std::is_constant_evaluated()) {
#ifdef VP_TARGET_ARCHITECTURE_x86
            if constexpr (std::is_same_v<Type, float>) {
                return v4t{ _mm_permute_ps(a.v, swizzle_mask) };
            } else if constexpr (std::is_same_v<Type, double>) {
                return v4t{ _mm256_permute4x64_pd(a.v, swizzle_mask) };
            } else if constexpr (std::is_same_v<Type, long long>) {
                return v4t{ _mm256_permute4x64_epi64(a.v, swizzle_mask) };
            }
#endif
        }
        v4t temp;
        const int index0 = ((swizzle_mask >> 0) & 0x3);
        const int index1 = ((swizzle_mask >> 2) & 0x3);
        const int index2 = ((swizzle_mask >> 4) & 0x3);
        const int index3 = ((swizzle_mask >> 6) & 0x3);
        temp[0] = a[index0];
        temp[1] = a[index1];
        temp[2] = a[index2];
        temp[3] = a[index3];
        return temp;
    }

    template <typename T>
        requires (T::cIsVectorizedType && T::cCount == 4)
    constexpr ALWAYS_INLINE T Swizzle8(const T &a, const v8si swizzle_mask) {
        using v8t = T;
        using Type = T::UnderlyingType;
        if(!std::is_constant_evaluated()) {
#ifdef VP_TARGET_ARCHITECTURE_x86
            if constexpr (std::is_same_v<Type, float>) {
                return v8t{ _mm256_permutevar8x32_ps(a.v, swizzle_mask) };
            } else if constexpr (std::is_same_v<Type, int>) {
                return v8t{ _mm256_permutevar8x32_epi32(a.v, swizzle_mask) };
            }
#endif
        }
        v8t temp;
        temp[0] = a[swizzle_mask[0]];
        temp[1] = a[swizzle_mask[1]];
        temp[2] = a[swizzle_mask[2]];
        temp[3] = a[swizzle_mask[3]];
        temp[4] = a[swizzle_mask[4]];
        temp[5] = a[swizzle_mask[5]];
        temp[6] = a[swizzle_mask[6]];
        temp[7] = a[swizzle_mask[7]];
        return temp;
    }

    template <typename T>
        requires (T::cIsVectorizedType && T::cCount == 16)
    constexpr ALWAYS_INLINE T InterleaveLow16(const T &a, const T &b) {
        using v16t = T;
        using Type = T::UnderlyingType;
        if(!std::is_constant_evaluated()) {
#ifdef VP_TARGET_ARCHITECTURE_x86
            if constexpr (std::is_same_v<Type, u16> || std::is_same_v<Type, s16>) {
                return v16t{ __builtin_ia32_punpcklwd256(a.v, b.v), };
            }
#endif
        }
        v16t temp;
        temp[0] = a[0];
        temp[1] = b[0];
        temp[2] = a[1];
        temp[3] = b[1];
        temp[4] = a[2];
        temp[5] = b[2];
        temp[6] = a[3];
        temp[7] = b[3];
        temp[8] = a[8];
        temp[9] = b[8];
        temp[10] = a[9];
        temp[11] = b[9];
        temp[12] = a[10];
        temp[13] = b[10];
        temp[14] = a[11];
        temp[15] = b[11];
        return temp;
    }

    template <typename T>
        requires (T::cIsVectorizedType && T::cCount == 16)
    constexpr ALWAYS_INLINE T InterleaveHigh16(const T &a, const T &b) {
        using v16t = T;
        using Type = T::UnderlyingType;
        if(!std::is_constant_evaluated()) {
#ifdef VP_TARGET_ARCHITECTURE_x86
            if constexpr (std::is_same_v<Type, u16> || std::is_same_v<Type, s16>) {
                return v16t{ __builtin_ia32_punpckhwd256(a.v, b.v), };
            }
#endif
        }
        v16t temp;
        temp[0] = a[4];
        temp[1] = b[4];
        temp[2] = a[5];
        temp[3] = b[5];
        temp[4] = a[6];
        temp[5] = b[6];
        temp[6] = a[7];
        temp[7] = b[7];
        temp[8] = a[12];
        temp[9] = b[12];
        temp[10] = a[13];
        temp[11] = b[13];
        temp[12] = a[14];
        temp[13] = b[14];
        temp[14] = a[15];
        temp[15] = b[15];
        return temp;
    }

    consteval int GetBlend16Order(int comp0, int comp1, int comp2, int comp3,int comp4, int comp5, int comp6, int comp7) {
        const int result0 = ((comp0 & 1) << 0);
        const int result1 = ((comp1 & 1) << 1);
        const int result2 = ((comp2 & 1) << 2);
        const int result3 = ((comp3 & 1) << 3);
        const int result4 = ((comp4 & 1) << 4);
        const int result5 = ((comp5 & 1) << 5);
        const int result6 = ((comp6 & 1) << 6);
        const int result7 = ((comp7 & 1) << 7);
        return result7 | result6 | result5 | result4 | result3 | result2 | result1 | result0;
    }

    template <typename T>
        requires (T::cIsVectorizedType && T::cCount == 16)
    constexpr ALWAYS_INLINE T Blend16(const T &a, const T &b, int imm) {
        using v16t = T;
        using Type = T::UnderlyingType;
        if(!std::is_constant_evaluated()) {
#ifdef VP_TARGET_ARCHITECTURE_x86
            if constexpr (std::is_same_v<Type, u16> || std::is_same_v<Type, s16>) {
                return v16t{ __builtin_ia32_pblendw256(a.v, b.v, imm), };
            }
#endif
        }
        v16t temp;
        for (u32 i = 0; i < 16; ++i) {
            temp[i] = ((imm >> (i % 8)) & 1) ? b[i] : a[i];
        }
        return temp;
    }

    template <typename T>
        requires (T::cIsVectorizedType && BITSIZEOF(T) == 256)
    constexpr ALWAYS_INLINE T Insert128(const T &a, const T &b, int imm) {
        using Type = typename T::UnderlyingType;
        if(!std::is_constant_evaluated()) {
#ifdef VP_TARGET_ARCHITECTURE_x86
            if constexpr (std::is_integral_v<Type>) {
                return T{ __builtin_ia32_insert128i256(a.v, b.v, imm), };
            }
#endif
        }
        T temp = a;
        if (imm & 1) {
            for (u32 i = 0; i < T::cCount / 2; ++i) {
                temp[i] = b[i];
            }
        } else {
            for (u32 i = T::cCount / 2; i < T::cCount; ++i) {
                temp[i] = b[i - (T::cCount / 2)];
            }
        }
        return temp;
    }

    consteval int GetPermute128Order(int comp0, int comp1) {
        return (comp0 & 0xf) | ((comp1 & 0xf) << 4);
    }

    template <typename T>
        requires (T::cIsVectorizedType && BITSIZEOF(T) == 256)
    constexpr ALWAYS_INLINE T Permute128(const T &a, const T &b, int imm) {
        using Type = typename T::UnderlyingType;
        if(!std::is_constant_evaluated()) {
#ifdef VP_TARGET_ARCHITECTURE_x86
            if constexpr (std::is_integral_v<Type>) {
                return T{ .v = reinterpret_cast<short __attribute__((vector_size(32)))>(__builtin_ia32_vperm2f128_si256(reinterpret_cast<const int __attribute__((vector_size(32)))>(a.v), reinterpret_cast<const int __attribute__((vector_size(32)))>(b.v), imm)), };
            }
#endif
        }
        T temp;
        const u32 m0 = imm & 0xf;
        const u32 m1 = (imm >> 4) & 0xf;
        if ((m0 >> 3) & 1) {
            for (u32 i = 0; i < T::cCount / 2; ++i) {
                temp[i] = 0;
            }
        } else if ((m0 & 0x3) == 0) {
            for (u32 i = 0; i < T::cCount / 2; ++i) {
                temp[i] = a[i];
            }
        } else if ((m0 & 0x3) == 1) {
            for (u32 i = 0; i < T::cCount / 2; ++i) {
                temp[i] = a[i + (T::cCount / 2)];
            }
        } else if ((m0 & 0x3) == 2) {
            for (u32 i = 0; i < T::cCount / 2; ++i) {
                temp[i] = b[i];
            }
        } else if ((m0 & 0x3) == 3) {
            for (u32 i = 0; i < T::cCount / 2; ++i) {
                temp[i] = b[i + (T::cCount / 2)];
            }
        }
        if ((m1 >> 3) & 1) {
            for (u32 i = 0; i < T::cCount / 2; ++i) {
                temp[i + (T::cCount / 2)] = 0;
            }
        } else if ((m1 & 0x3) == 0) {
            for (u32 i = 0; i < T::cCount / 2; ++i) {
                temp[i + (T::cCount / 2)] = a[i];
            }
        } else if ((m1 & 0x3) == 1) {
            for (u32 i = 0; i < T::cCount / 2; ++i) {
                temp[i + (T::cCount / 2)] = a[i + (T::cCount / 2)];
            }
        } else if ((m1 & 0x3) == 2) {
            for (u32 i = 0; i < T::cCount / 2; ++i) {
                temp[i + (T::cCount / 2)] = b[i];
            }
        } else if ((m1 & 0x3) == 3) {
            for (u32 i = 0; i < T::cCount / 2; ++i) {
                temp[i + (T::cCount / 2)] = b[i + (T::cCount / 2)];
            }
        }
        return temp;
    }
}
