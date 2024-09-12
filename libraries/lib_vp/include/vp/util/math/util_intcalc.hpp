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

    constexpr Vector2i operator+(const Vector2i &left, const Vector2i &right) {
        const Vector2i out = { .vector = left.vector + right.vector };
        return out;
    }
    constexpr Vector2i operator-(const Vector2i &left, const Vector2i &right) {
        const Vector2i out = { .vector = left.vector - right.vector };
        return out;
    }

    constexpr Vector3i operator+(const Vector3i &left, const Vector3i &right) {
        const Vector3i out = { .vector = left.vector + right.vector };
        return out;
    }
    constexpr Vector3i operator-(const Vector3i &left, const Vector3i &right) {
        const Vector3i out = { .vector = left.vector - right.vector };
        return out;
    }

    constexpr Vector4i operator+(const Vector4i &left, const Vector4i &right) {
        const Vector4i out = { .vector = left.vector + right.vector };
        return out;
    }
    constexpr Vector4i operator-(const Vector4i &left, const Vector4i &right) {
        const Vector4i out = { .vector = left.vector - right.vector };
        return out;
    }

    constexpr bool operator==(const Int2 &left, const Int2 &right) {
        const bool out = (left.x == right.x && left.y == right.y);
        return out;
    }
}
