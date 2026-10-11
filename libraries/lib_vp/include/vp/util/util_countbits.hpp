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

    constexpr ALWAYS_INLINE u32 CountOneBits8(u8 value) {
        return std::popcount(value);
    }

    constexpr ALWAYS_INLINE u32 CountOneBits16(u16 value) {
        return std::popcount(value);
    }

    constexpr ALWAYS_INLINE u32 CountOneBits32(u32 value) {
        return std::popcount(value);
    }

    constexpr ALWAYS_INLINE u32 CountOneBits64(u64 value) {
        return std::popcount(value);
    }

    template <typename T>
        requires (std::is_integral_v<T> && std::is_unsigned_v<T>)
    constexpr ALWAYS_INLINE u32 CountOneBits(T value) {
        if constexpr (sizeof(u8) == sizeof(T)) {
            return CountOneBits8(value);
        } else if constexpr (sizeof(u16) == sizeof(T)) {
            return CountOneBits16(value);
        } else if constexpr (sizeof(u32) == sizeof(T)) {
            return CountOneBits32(value);
        }
        return CountOneBits64(value);
    }

    constexpr ALWAYS_INLINE u32 CountZeroBits8(u8 value) {
        return std::popcount(static_cast<u8>(~value));
    }

    constexpr ALWAYS_INLINE u32 CountZeroBits16(u16 value) {
        return std::popcount(static_cast<u16>(~value));
    }

    constexpr ALWAYS_INLINE u32 CountZeroBits32(u32 value) {
        return std::popcount(~value);
    }

    constexpr ALWAYS_INLINE u32 CountZeroBits64(u64 value) {
        return std::popcount(~value);
    }

    template <typename T>
        requires (std::is_integral_v<T> && std::is_unsigned_v<T>)
    constexpr ALWAYS_INLINE u32 CountZeroBits(T value) {
        if constexpr (sizeof(u8) == sizeof(T)) {
            return CountZeroBits8(value);
        } else if constexpr (sizeof(u16) == sizeof(T)) {
            return CountZeroBits16(value);
        } else if constexpr (sizeof(u32) == sizeof(T)) {
            return CountZeroBits32(value);
        }
        return CountZeroBits64(value);
    }

    constexpr ALWAYS_INLINE u8 CountRightOneBits8(u8 value) {
        return std::countr_one(value);
    }

    constexpr ALWAYS_INLINE u16 CountRightOneBits16(u16 value) {
        return std::countr_one(value);
    }

    constexpr ALWAYS_INLINE u32 CountRightOneBits32(u32 value) {
        return std::countr_one(value);
    }

    constexpr ALWAYS_INLINE u32 CountRightOneBits64(u64 value) {
        return std::countr_one(value);
    }

    template <typename T>
        requires (std::is_integral_v<T> && std::is_unsigned_v<T>)
    constexpr ALWAYS_INLINE u32 CountRightOneBits(T value) {
        if constexpr (sizeof(u8) == sizeof(T)) {
            return CountRightOneBits8(value);
        } else if constexpr (sizeof(u16) == sizeof(T)) {
            return CountRightOneBits16(value);
        } else if constexpr (sizeof(u32) == sizeof(T)) {
            return CountRightOneBits32(value);
        }
        return CountRightOneBits64(value);
    }

    constexpr ALWAYS_INLINE u32 CountLeftOneBits8(u8 value) {
        return std::countl_one(value);
    }

    constexpr ALWAYS_INLINE u32 CountLeftOneBits16(u16 value) {
        return std::countl_one(value);
    }

    constexpr ALWAYS_INLINE u32 CountLeftOneBits32(u32 value) {
        return std::countl_one(value);
    }

    constexpr ALWAYS_INLINE u32 CountLeftOneBits64(u64 value) {
        return std::countl_one(value);
    }

    template <typename T>
        requires (std::is_integral_v<T> && std::is_unsigned_v<T>)
    constexpr ALWAYS_INLINE u32 CountLeftOneBits(T value) {
        if constexpr (sizeof(u8) == sizeof(T)) {
            return CountLeftOneBits8(value);
        } else if constexpr (sizeof(u16) == sizeof(T)) {
            return CountLeftOneBits16(value);
        } else if constexpr (sizeof(u32) == sizeof(T)) {
            return CountLeftOneBits32(value);
        }
        return CountLeftOneBits64(value);
    }

    constexpr ALWAYS_INLINE u32 CountRightZeroBits8(u8 value) {
        return std::countr_zero(value);
    }

    constexpr ALWAYS_INLINE u32 CountRightZeroBits16(u16 value) {
        return std::countr_zero(value);
    }

    constexpr ALWAYS_INLINE u32 CountRightZeroBits32(u32 value) {
        return std::countr_zero(value);
    }

    constexpr ALWAYS_INLINE u32 CountRightZeroBits64(u64 value) {
        return std::countr_zero(value);
    }

    template <typename T>
        requires (std::is_integral_v<T> && std::is_unsigned_v<T>)
    constexpr ALWAYS_INLINE u32 CountRightZeroBits(T value) {
        if constexpr (sizeof(u8) == sizeof(T)) {
            return CountRightZeroBits8(value);
        } else if constexpr (sizeof(u16) == sizeof(T)) {
            return CountRightZeroBits16(value);
        } else if constexpr (sizeof(u32) == sizeof(T)) {
            return CountRightZeroBits32(value);
        }
        return CountRightZeroBits64(value);
    }

    constexpr ALWAYS_INLINE u32 CountLeftZeroBits8(u8 value) {
        return std::countl_zero(value);
    }

    constexpr ALWAYS_INLINE u32 CountLeftZeroBits16(u16 value) {
        return std::countl_zero(value);
    }

    constexpr ALWAYS_INLINE u32 CountLeftZeroBits32(u32 value) {
        return std::countl_zero(value);
    }

    constexpr ALWAYS_INLINE u32 CountLeftZeroBits64(u64 value) {
        return std::countl_zero(value);
    }

    template <typename T>
        requires (std::is_integral_v<T> && std::is_unsigned_v<T>)
    constexpr ALWAYS_INLINE u32 CountLeftZeroBits(T value) {
        if constexpr (sizeof(u8) == sizeof(T)) {
            return CountLeftZeroBits8(value);
        } else if constexpr (sizeof(u16) == sizeof(T)) {
            return CountLeftZeroBits16(value);
        } else if constexpr (sizeof(u32) == sizeof(T)) {
            return CountLeftZeroBits32(value);
        }
        return CountLeftZeroBits64(value);
    }
}
