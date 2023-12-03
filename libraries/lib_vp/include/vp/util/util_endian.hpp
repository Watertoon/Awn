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
        requires (std::is_integral<T>::value) && (sizeof(T) == sizeof(u64) || sizeof(T) == sizeof(u32) || sizeof(T) == sizeof(u16) || sizeof(T) == sizeof(u8))
    constexpr ALWAYS_INLINE T SwapEndian(T value) {
        if constexpr (sizeof(T) == sizeof(u8)) {
            return value;
        } else if constexpr (sizeof(T) == sizeof(u16)) {
            const u32 swap0 = ((value & 0xff00) >> 0x8) | ((value & 0x00ff) << 0x8);
            return swap0;
        } else if constexpr (sizeof(T) == sizeof(u32)) {
            const u32 swap0 = ((value & 0xff00'ff00) >> 0x8) | ((value & 0x00ff'00ff) << 0x8);
            const u32 swap1 = (swap0 >> 0x10) | (swap0 << 0x10);
            return swap1;
        } else if constexpr (sizeof(T) == sizeof(u64)) {
            const u64 swap0 = ((value & 0xff00'ff00'ff00'ff00) >> 0x8) | ((value & 0x00ff'00ff'00ff'00ff) << 0x8);
            const u64 swap1 = ((swap0 & 0xffff'0000'ffff'0000) >> 0x10) | ((swap0 & 0x0000'ffff'0000'ffff) << 0x10);
            const u64 swap2 = (swap1 >> 0x20) | (swap1 << 0x20);
            return swap2;
        }
    }

    template <typename T>
        requires (std::is_floating_point<T>::value) && (sizeof(T) == sizeof(double) || sizeof(T) == sizeof(float))
    ALWAYS_INLINE T SwapEndian(T value) {
        if constexpr (sizeof(T) == sizeof(u32)) {
            u32 *value_data = reinterpret_cast<u32*>(std::addressof(value));
            const u32 swap0 = ((*value_data & 0xff00'ff00) >> 0x8) | ((*value_data & 0x00ff'00ff) << 0x8);
            const u32 swap1 = (swap0 >> 0x10) | (swap0 << 0x10);
            return swap1;
        } else if constexpr (sizeof(T) == sizeof(u64)) {
            u64 *value_data = reinterpret_cast<u64*>(std::addressof(value));
            const u64 swap0 = ((*value_data & 0xff00'ff00'ff00'ff00) >> 0x8) | ((*value_data & 0x00ff'00ff'00ff'00ff) << 0x8);
            const u64 swap1 = ((swap0 & 0xffff'0000'ffff'0000) >> 0x10) | ((swap0 & 0x0000'ffff'0000'ffff) << 0x10);
            const u64 swap2 = (swap1 >> 0x20) | (swap1 << 0x20);
            return swap2;
        }
    }

    constexpr ALWAYS_INLINE u32 SwapEndian24(u32 value) {
        const u32 swap0 = ((value & 0xff0000) >> 0x10) | ((value & 0x0000ff) << 0x10) | (value & 0x00ff00);
        return swap0;
    }
}
