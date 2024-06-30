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

    constexpr ALWAYS_INLINE u32 CountOneBits32(u32 value) {
        if (std::is_constant_evaluated() == true) {
            return std::popcount(value);
        }
        return __builtin_popcount(value);
    }
    constexpr ALWAYS_INLINE u32 CountOneBits64(u64 value) {
        if (std::is_constant_evaluated() == true) {
            return std::popcount(value);
        }
        return __builtin_popcountll(value);
    }

    template <typename T>
        requires (std::is_integral<T>::value && sizeof(u32) <= sizeof(T))
    constexpr ALWAYS_INLINE u32 CountOneBits(T value) {
        if constexpr (sizeof(u32) == sizeof(T)) {
            return CountOneBits32(value);
        }
        return CountOneBits64(value);
    }

    constexpr ALWAYS_INLINE u32 CountRightOneBits32(u32 value) {
        if (std::is_constant_evaluated() == true) {
            return std::countr_one(value);
        }
        return __builtin_ctz(~value);
    }
    constexpr ALWAYS_INLINE u32 CountRightOneBits64(u64 value) {
        if (std::is_constant_evaluated() == true) {
            return std::countr_one(value);
        }
        return __builtin_ctzll(~value);
    }

    template <typename T>
        requires (std::is_integral<T>::value && sizeof(u32) <= sizeof(T))
    constexpr ALWAYS_INLINE u32 CountRightOneBits(T value) {
        if constexpr (sizeof(u32) == sizeof(T)) {
            return CountRightOneBits32(value);
        }
        return CountRightOneBits64(value);
    }

    constexpr ALWAYS_INLINE u32 CountLeftOneBits32(u32 value) {
        if (std::is_constant_evaluated() == true) {
            return std::countl_one(value);
        }
        return __builtin_clz(~value);
    }
    constexpr ALWAYS_INLINE u32 CountLeftOneBits64(u64 value) {
        if (std::is_constant_evaluated() == true) {
            return std::countl_one(value);
        }
        return __builtin_clzll(~value);
    }

    template <typename T>
        requires (std::is_integral<T>::value && sizeof(u32) <= sizeof(T))
    constexpr ALWAYS_INLINE u32 CountLeftOneBits(T value) {
        if constexpr (sizeof(u32) == sizeof(T)) {
            return CountLeftOneBits32(value);
        }
        return CountLeftOneBits64(value);
    }

    constexpr ALWAYS_INLINE u32 CountRightZeroBits32(u32 value) {
        if (std::is_constant_evaluated() == true) {
            return std::countr_zero(value);
        }
        return __builtin_ctz(value);
    }
    constexpr ALWAYS_INLINE u32 CountRightZeroBits64(u64 value) {
        if (std::is_constant_evaluated() == true) {
            return std::countr_zero(value);
        }
        return __builtin_ctzll(value);
    }

    template <typename T>
        requires (std::is_integral<T>::value && sizeof(u32) <= sizeof(T))
    constexpr ALWAYS_INLINE u32 CountRightZeroBits(T value) {
        if constexpr (sizeof(u32) == sizeof(T)) {
            return CountRightZeroBits32(value);
        }
        return CountRightZeroBits64(value);
    }

    constexpr ALWAYS_INLINE u32 CountLeftZeroBits32(u32 value) {
        if (std::is_constant_evaluated() == true) {
            return std::countl_zero(value);
        }
        return __builtin_ctz(value);
    }
    constexpr ALWAYS_INLINE u32 CountLeftZeroBits64(u64 value) {
        if (std::is_constant_evaluated() == true) {
            return std::countl_zero(value);
        }
        return __builtin_ctzll(value);
    }

    template <typename T>
        requires (std::is_integral<T>::value && sizeof(u32) <= sizeof(T))
    constexpr ALWAYS_INLINE u32 CountLeftZeroBits(T value) {
        if constexpr (sizeof(u32) == sizeof(T)) {
            return CountLeftZeroBits32(value);
        }
        return CountLeftZeroBits64(value);
    }
}
