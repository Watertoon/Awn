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

    consteval v4si GetLoad4Mask128(bool is_x, bool is_y, bool is_z, bool is_w) {
        return v4si{ is_x, is_y, is_z, is_w, };
    }

    consteval v4sll GetLoad4Mask256(bool is_x, bool is_y, bool is_z, bool is_w) {
        return v4sll{ is_x, is_y, is_z, is_w, };
    }

    consteval v8si GetLoad8Mask256(bool is_x0, bool is_y0, bool is_z0, bool is_w0, bool is_x1, bool is_y1, bool is_z1, bool is_w1) {
        return v8si{ is_x0, is_y0, is_z0, is_w0, is_x1, is_y1, is_z1, is_w1, };
    }

    template <typename T>
        requires (sizeof(TVectorType<T, 4>) == 16)
    constexpr TVectorType<T, 4> Load4Mask128(T const *address, const v4si &mask) {
        using v4t = TVectorType<T, 4>;
        using Type = v4t::UnderlyingType;
        if (!std::is_constant_evaluated()) {
#ifdef VP_TARGET_ARCHITECTURE_x86
            if constexpr (std::is_same_v<Type, float>) {
                return v4t{ .v = reinterpret_cast<float __attribute__((vector_size(16)))>(_mm_maskload_ps(address, reinterpret_cast<__m128i>(mask.v))), };
            } else if constexpr (std::is_integral_v<Type>) {
                return v4t{ .v = reinterpret_cast<int __attribute__((vector_size(16)))>(_mm_maskload_epi32(address, reinterpret_cast<__m128i>(mask.v))), };
            }
#endif
        }
        v4t ret = {};
        if (mask[0]) {
            ret[0] = address[0];
        }
        if (mask[1]) {
            ret[1] = address[1];
        }
        if (mask[2]) {
            ret[2] = address[2];
        }
        if (mask[3]) {
            ret[3] = address[3];
        }
        return ret;
    }

    template <typename T>
        requires (sizeof(TVectorType<T, 4>) == 32)
    constexpr TVectorType<T, 4> Load4Mask256(T const *address, const v4sll &mask) {
        using v4t = TVectorType<T, 4>;
        using Type = v4t::UnderlyingType;
        if (!std::is_constant_evaluated()) {
#ifdef VP_TARGET_ARCHITECTURE_x86
            if constexpr (std::is_same_v<Type, double>) {
                return v4t{ .v = _mm256_maskload_pd(address, reinterpret_cast<__m256i>(mask.v)), };
            } else if constexpr (std::is_integral_v<Type>) {
                return v4t{ .v = _mm256_maskload_epi64(address, reinterpret_cast<__m256i>(mask.v)), };
            }
#endif
        }
        v4t ret = {};
        if (mask[0]) {
            ret[0] = address[0];
        }
        if (mask[1]) {
            ret[1] = address[1];
        }
        if (mask[2]) {
            ret[2] = address[2];
        }
        if (mask[3]) {
            ret[3] = address[3];
        }
        return ret;
    }

    template <typename T>
        requires (sizeof(TVectorType<T, 8>) == 32)
    constexpr TVectorType<T, 8> Load8Mask256(T const *address, const v8si &mask) {
        using v8t = TVectorType<T, 8>;
        using Type = v8t::UnderlyingType;
        if (!std::is_constant_evaluated()) {
#ifdef VP_TARGET_ARCHITECTURE_x86
            if constexpr (std::is_same_v<Type, float>) {
                return v8t{ .v = _mm256_maskload_ps(address, reinterpret_cast<__m256i>(mask.v)), };
            } else if constexpr (std::is_integral_v<Type> && BITSIZEOF(v8t) == 256) {
                return v8t{ .v = _mm256_maskload_epi32(address), };
            }
#endif
        }
        v8t ret = {};
        if (mask[0]) {
            ret[0] = address[0];
        }
        if (mask[1]) {
            ret[1] = address[1];
        }
        if (mask[2]) {
            ret[2] = address[2];
        }
        if (mask[3]) {
            ret[3] = address[3];
        }
        if (mask[4]) {
            ret[4] = address[4];
        }
        if (mask[5]) {
            ret[5] = address[5];
        }
        if (mask[6]) {
            ret[6] = address[6];
        }
        if (mask[7]) {
            ret[7] = address[7];
        }
        return ret;
    }
}
