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

    typedef float v2f __attribute__((vector_size(8)));
    typedef int v2si __attribute__((vector_size(8)));
    typedef unsigned int v2ui __attribute__((vector_size(8)));
    typedef double v2d __attribute__((vector_size(16)));

    template<typename T>
        requires std::is_integral<T>::value || std::is_floating_point<T>::value
    class Vector2Type {
        private:
            typedef T __attribute__((vector_size(sizeof(T) * 2))) v2;
            
            T x;
            T y;
        public:
            constexpr Vector2Type() : x(), y() {/*...*/}
            constexpr Vector2Type(T x, T y) : x(x), y(y) {/*...*/}
            constexpr Vector2Type(v2& copy) : x(copy[0]), y(copy[1]) {/*...*/}
            constexpr Vector2Type(const v2& copy) : x(copy[0]), y(copy[1]) {/*...*/}

            constexpr Vector2Type(const Vector2Type& rhs) : x(rhs.x), y(rhs.y) {/*...*/}

            constexpr T operator[](int index) {
                return std::addressof(x)[index];
            }

            constexpr T operator[](int index) const {
                return std::addressof(x)[index];
            }

            constexpr Vector2Type& operator=(const Vector2Type& rhs) {
                x = rhs.x;
                y = rhs.y;
                return *this;
            }

            constexpr Vector2Type operator+(Vector2Type& rhs) {
                return Vector2Type(x + rhs.x, y + rhs.y);
            }

            constexpr Vector2Type operator+(const Vector2Type& rhs) {
                return Vector2Type(x + rhs.x, y + rhs.y);
            }

            constexpr Vector2Type operator-(Vector2Type& rhs) {
                return Vector2Type(x - rhs.x, y - rhs.y);
            }

            constexpr Vector2Type operator-(const Vector2Type& rhs) {
                return Vector2Type(x - rhs.x, y - rhs.y);
            }

            constexpr T Dot(Vector2Type& rhs) {
                return x * rhs.x + y * rhs.y;
            }

            constexpr T Dot(const Vector2Type& rhs) {
                return x * rhs.x + y * rhs.y;
            }

            constexpr Vector2Type operator/(Vector2Type& rhs) {
                return Vector2Type(x / rhs.x, y / rhs.y);
            }

            constexpr Vector2Type operator/(const Vector2Type& rhs) {
                return Vector2Type(x / rhs.x, y / rhs.y);
            }

            constexpr Vector2Type& operator+=(Vector2Type& rhs) {
                x += rhs.x;
                y += rhs.y;
                return *this;
            }

            constexpr Vector2Type& operator+=(const Vector2Type& rhs) {
                x += rhs.x;
                y += rhs.y;
                return *this;
            }

            constexpr Vector2Type& operator-=(Vector2Type& rhs) {
                x -= rhs.x;
                y -= rhs.y;
                return *this;
            }

            constexpr Vector2Type& operator-=(const Vector2Type& rhs) {
                x -= rhs.x;
                y -= rhs.y;
                return *this;
            }

            constexpr bool operator==(Vector2Type& rhs) {
                return (x == rhs.x) & (y == rhs.y);
            }

            constexpr bool operator==(const Vector2Type& rhs) {
                return (x == rhs.x) & (y == rhs.y);
            }

            constexpr bool operator!=(Vector2Type& rhs) {
                return !((x == rhs.x) & (y == rhs.y));
            }

            constexpr bool operator!=(const Vector2Type& rhs) {
                return !((x == rhs.x) & (y == rhs.y));
            }
    };

    using Vector2f = Vector2Type<float>;
    using Vector2u  = Vector2Type<unsigned int>;
    using Vector2i  = Vector2Type<int>;
    using Vector2d = Vector2Type<double>;
    static_assert(sizeof(Vector2f) == 0x8);
    static_assert(sizeof(Vector2u) == 0x8);
    static_assert(sizeof(Vector2i) == 0x8);
    static_assert(sizeof(Vector2d) == 0x10);
    static_assert(alignof(Vector2f) == 0x4);
    static_assert(alignof(Vector2u) == 0x4);
    static_assert(alignof(Vector2i) == 0x4);
    static_assert(alignof(Vector2d) == 0x8);

    static_assert((Vector2u(v2ui{2,2}) + Vector2u(v2ui{2,4})) != Vector2u(v2ui{2,2}));
    static_assert((Vector2f(v2f{1.0f,1.0f}) + Vector2f(v2f{1.0f,1.0f})) == Vector2f(v2f{2.0f,2.0f}));
}
