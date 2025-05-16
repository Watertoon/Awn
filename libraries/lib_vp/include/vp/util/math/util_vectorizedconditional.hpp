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

    template <typename T>
        requires (T::cIsVectorizedType && T::cCount == 4)
    constexpr T CompareEqual4(const T &a, const T&b) {
        using Type = typename T::UnderlyingType;
        if (!std::is_constant_evaluated()) {
#ifdef VP_TARGET_ARCHITECTURE_x86
            if constexpr (std::is_same_v<Type, float>) {
                return T{ .v = _mm_cmpeq_ps(a.v, b.v) };
            } else if constexpr (std::is_same_v<Type, s32>) {
                return T{ .v = _mm_cmpeq_epi32(a.v, b.v) };
            }
#endif
        }
        using MaskType = MinUIntType<1ull << (BITSIZEOF(Type) - 1)>;
        T out = {};
        for (u32 i = 0; i < T::cCount; ++i) {
            out[i] = (a[i] == b[i]) ? std::bit_cast<Type>(static_cast<MaskType>(0xffff'ffff'ffff'ffff)) : 0;
        }
        return out;
    }

    template <typename T>
        requires (T::cIsVectorizedType && T::cCount == 4)
    constexpr T CompareGreaterThan4(const T &a, const T&b) {
        using Type = typename T::UnderlyingType;
        if (!std::is_constant_evaluated()) {
#ifdef VP_TARGET_ARCHITECTURE_x86
            if constexpr (std::is_same_v<Type, float>) {
                return T{ .v = _mm_cmpgt_ps(a.v, b.v) };
            } else if constexpr (std::is_same_v<Type, s32>) {
                return T{ .v = _mm_cmpgt_epi32(a.v, b.v) };
            }
#endif
        }
        using MaskType = MinUIntType<1ull << (BITSIZEOF(Type) - 1)>;
        T out = {};
        for (u32 i = 0; i < T::cCount; ++i) {
            out[i] = (a[i] > b[i]) ? std::bit_cast<Type>(static_cast<MaskType>(0xffff'ffff'ffff'ffff)) : 0;
        }
        return out;
    }

    template <typename T>
        requires (T::cIsVectorizedType && T::cCount == 4)
    constexpr T CompareGreaterThanEqual4(const T &a, const T&b) {
        using Type = typename T::UnderlyingType;
        if (!std::is_constant_evaluated()) {
#ifdef VP_TARGET_ARCHITECTURE_x86
            if constexpr (std::is_same_v<Type, float>) {
                return T{ .v = _mm_cmpge_ps(a.v, b.v) };
            } else if constexpr (std::is_same_v<Type, s32>) {
                return T{ .v = _mm_cmpge_epi32(a.v, b.v) };
            }
#endif
        }
        using MaskType = MinUIntType<1ull << (BITSIZEOF(Type) - 1)>;
        T out = {};
        for (u32 i = 0; i < T::cCount; ++i) {
            out[i] = (a[i] >= b[i]) ? std::bit_cast<Type>(static_cast<MaskType>(0xffff'ffff'ffff'ffff)) : 0;
        }
        return out;
    }

    constexpr v32cc CompareEqual32(const v32cc &a, const v32cc &b) {

        /* Runtime intrinsic  */
        if (!std::is_constant_evaluated()) {
#ifdef VP_TARGET_ARCHITECTURE_x86
            return v32cc{ .v = reinterpret_cast<char __attribute__((vector_size(32)))>(_mm256_cmpeq_epi8(reinterpret_cast<__m256i>(a.v), reinterpret_cast<__m256i>(b.v))), };
#endif
        }

        /* Compare 32 bytes, setting each output byte to 0xff if same, else 0 */
        v32cc out = {};
        for (u32 i = 0; i < 0x20; ++i) {
            out[i] = (a[i] == b[i]) ? 0xff : 0;
        }

        return out;
    }

    template <typename T>
        requires (T::cIsVectorizedType)
    constexpr T InsertTrue4(const T &base, const T &insert, const T &mask) {
        /* Equivalent to aarch64 bit  */
        const T result = base ^ ((base ^ insert) & mask);
        return result;
    }

    template <typename T>
        requires (T::cIsVectorizedType)
    constexpr T InsertFalse4(const T &base, const T &insert, const T &mask) {
        /* Equivalent to aarch64 bif  */
        const T result = base ^ AndNot((base ^ insert), mask);
        return result;
    }

    template <typename T>
        requires (T::cIsVectorizedType)
    constexpr T Select4(const T &base, const T &insert, const T &mask) {
        /* Equivalent to aarch64 bsl  */
        const T result = mask ^ ((mask ^ insert) & base);
        return result;
    }

    template <typename T>
        requires (T::cIsVectorizedType)
    constexpr T Absolute(const T &base) {
        using Type = typename T::UnderlyingType;
        if (!std::is_constant_evaluated()) {
#ifdef VP_TARGET_ARCHITECTURE_x86
            if constexpr (T::cCount == 4 && std::is_same_v<Type, s32>) {
                return T { .v = _mm_abs_epi32(base.v), };
            } else if constexpr (T::cCount == 8 && std::is_same_v<Type, s32>) {
                return T { .v = _mm256_abs_epi32(base.v), };
            } else if constexpr (T::cCount == 8 && std::is_same_v<Type, s16>) {
                return T { .v = _mm_abs_epi16(base.v), };
            } else if constexpr (T::cCount == 16 && std::is_same_v<Type, s16>) {
                return T { .v = _mm256_abs_epi16(base.v), };
            } else if constexpr (T::cCount == 16 && std::is_same_v<Type, s8>) {
                return T { .v = _mm_abs_epi8(base.v), };
            } else if constexpr (T::cCount == 32 && std::is_same_v<Type, s8>) {
                return T { .v = _mm256_abs_epi8(base.v), };
            }
#endif
        }
        T ret;
        for (u32 i = 0; i < T::cCount; ++i) {
            ret[i] = vp::util::Absolute(base[i]);
        }
        return ret;
    }
}
