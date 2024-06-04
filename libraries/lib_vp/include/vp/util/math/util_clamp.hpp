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
    constexpr T Min(T value0, T value1) {
        return std::min(value0, value1);
    }

    template <typename T>
    constexpr T Max(T value0, T value1) {
        return std::max(value0, value1);
    }

    template <typename T>
    constexpr T Clamp(T value, T min, T max) {
        const T clamp_min = Max(value, min);
        const T clamp_max = Min(clamp_min, max);
        return clamp_max;
    }
    
    template <typename T>
    constexpr T Abs(T value) {
        return std::abs(value);
    }
}
