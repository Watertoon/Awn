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
        const int result0 = ((index0 & 1) << 0);
        const int result1 = ((index1 & 1) << 2);
        const int result2 = ((index2 & 1) << 4);
        const int result3 = ((index3 & 1) << 6);
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
}
