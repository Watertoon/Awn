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
        requires std::is_pointer<T>::value
    ALWAYS_INLINE T AlignUp(T value, size_t align) {
        const size_t alignment = align - 1;
        return reinterpret_cast<T>((reinterpret_cast<uintptr_t>(value) + alignment) & ~alignment);
    }

    template<typename T>
        requires (std::is_pointer<T>::value == false)
    constexpr ALWAYS_INLINE T AlignUp(T value, size_t align) {
        const size_t alignment = align - 1;
        return static_cast<T>((value + alignment) & ~alignment);
    }

    template<typename T>
        requires std::is_pointer<T>::value
    ALWAYS_INLINE T AlignDown(T value, size_t align) {
        const size_t alignment = align - 1;
        return reinterpret_cast<T>(reinterpret_cast<uintptr_t>(value) & ~alignment);
    }

    template<typename T>
        requires (std::is_pointer<T>::value == false)
    constexpr ALWAYS_INLINE T AlignDown(T value, size_t align) {
        const size_t alignment = align - 1;
        return static_cast<T>(value & ~alignment);
    }
}
