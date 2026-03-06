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

    template<typename T>
        requires (std::is_pointer_v<T>)
    ALWAYS_INLINE T AlignUp(T value, size_t alignment) {
        const size_t align = alignment - 1;
        return reinterpret_cast<T>((reinterpret_cast<uintptr_t>(value) + align) & (~align));
    }

    template<typename T>
        requires (!std::is_pointer_v<T>)
    constexpr ALWAYS_INLINE T AlignUp(T value, size_t alignment) {
        const size_t align = alignment - 1;
        return static_cast<T>((value + align) & ~align);
    }

    template<typename T>
        requires (std::is_pointer_v<T>)
    ALWAYS_INLINE T AlignDown(T value, size_t alignment) {
        const size_t align = alignment - 1;
        return reinterpret_cast<T>(reinterpret_cast<uintptr_t>(value) & (~align));
    }

    template<typename T>
        requires (!std::is_pointer_v<T>)
    constexpr ALWAYS_INLINE T AlignDown(T value, size_t alignment) {
        const size_t align = alignment - 1;
        return static_cast<T>(value & (~align));
    }

    template<typename T>
        requires (std::is_pointer_v<T>)
    ALWAYS_INLINE bool IsAligned(T value, size_t alignment) {
        const size_t align = alignment - 1;
        return (reinterpret_cast<uintptr_t>(value) & align) == 0;
    }

    template<typename T>
        requires (!std::is_pointer_v<T>)
    constexpr ALWAYS_INLINE bool IsAligned(T value, size_t alignment) {
        const size_t align = alignment - 1;
        return (value & align) == 0;
    }

    template<typename T>
        requires (std::is_unsigned_v<T>)
    constexpr ALWAYS_INLINE T AlignPow2(T value) {
        const T high_bit = static_cast<T>(1u) << (BITSIZEOF(T) - 1u);
        VP_ASSERT(high_bit > value);
        const T align = high_bit >> (CountLeftZeroBits(value - 1u) - 1u);
        return align;
    }

    template<typename T>
        requires (std::is_unsigned_v<T>)
    constexpr ALWAYS_INLINE bool IsAlignedPow2(T value) {
        const bool is_one_bit = (CountOneBits(value) == 1);
        return is_one_bit;
    }
}
