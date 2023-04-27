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
        if constexpr (std::is_constant_evaluated() == true) {
            return std::popcount(value);
        }
        return __builtin_popcount(value);
    }
    constexpr ALWAYS_INLINE u32 CountOneBits64(u64 value) {
        if constexpr (std::is_constant_evaluated() == true) {
            return std::popcount(value);
        }
        return __builtin_popcountll(value);
    }
}
