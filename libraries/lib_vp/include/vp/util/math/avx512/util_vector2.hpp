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
            v2 m_vec;
        public:
            constexpr Vector2Type() : m_vec() {/*...*/}
            constexpr Vector2Type(T x, T y) : m_vec{x, y} {/*...*/}
            constexpr Vector2Type(v2& copy) : m_vec(copy) {/*...*/}
            constexpr Vector2Type(const v2& copy) : m_vec(copy) {/*...*/}

            constexpr Vector2Type(const Vector2Type& rhs) : m_vec(rhs.m_vec) {/*...*/}

            constexpr T operator[](int index) {
                return m_vec[index];
            }

            constexpr T operator[](int index) const {
                return m_vec[index];
            }

            constexpr Vector2Type& operator=(const Vector2Type& rhs) {
                m_vec = rhs.m_vec;
                return *this;
            }

            constexpr Vector2Type operator+(Vector2Type& rhs) {
                v2 a = m_vec + rhs.m_vec;
                return Vector2Type(a);
            }

            constexpr Vector2Type operator+(const Vector2Type& rhs) {
                v2 a = m_vec + rhs.m_vec;
                return Vector2Type(a);
            }

            constexpr Vector2Type operator-(Vector2Type& rhs) {
                v2 a = m_vec - rhs.m_vec;
                return Vector2Type(a);
            }

            constexpr Vector2Type operator-(const Vector2Type& rhs) {
                v2 a = m_vec - rhs.m_vec;
                return Vector2Type(a);
            }

            constexpr T operator*(Vector2Type& rhs) {
                v2 a = m_vec * rhs.m_vec;
                return a[0] + a[1];
            }

            constexpr T operator*(const Vector2Type& rhs) {
                v2 a = m_vec * rhs.m_vec;
                return a[0] + a[1];
            }

            constexpr Vector2Type operator/(Vector2Type& rhs) {
                v2 a = m_vec / rhs.m_vec;
                return Vector2Type(a);
            }

            constexpr Vector2Type operator/(const Vector2Type& rhs) {
                v2 a = m_vec / rhs.m_vec;
                return Vector2Type(a);
            }

            constexpr Vector2Type& operator+=(Vector2Type& rhs) {
                m_vec = m_vec + rhs.m_vec;
                return *this;
            }

            constexpr Vector2Type& operator+=(const Vector2Type& rhs) {
                m_vec = m_vec + rhs.m_vec;
                return *this;
            }

            constexpr Vector2Type& operator-=(Vector2Type& rhs) {
                m_vec = m_vec - rhs.m_vec;
                return *this;
            }

            constexpr Vector2Type& operator-=(const Vector2Type& rhs) {
                m_vec = m_vec - rhs.m_vec;
                return *this;
            }

            constexpr bool operator==(Vector2Type& rhs) {
                return (m_vec[0] == rhs.m_vec[0]) & (m_vec[1] == rhs.m_vec[1]);
            }

            constexpr bool operator==(const Vector2Type& rhs) {
                return (m_vec[0] == rhs.m_vec[0]) & (m_vec[1] == rhs.m_vec[1]);
            }

            constexpr bool operator!=(Vector2Type& rhs) {
                return !((m_vec[0] == rhs.m_vec[0]) & (m_vec[1] == rhs.m_vec[1]));
            }

            constexpr bool operator!=(const Vector2Type& rhs) {
                return !((m_vec[0] == rhs.m_vec[0]) & (m_vec[1] == rhs.m_vec[1]));
            }

            constexpr T Dot(const Vector2Type& rhs) {
                const v2 temp = m_vec * rhs.m_vec;
                return temp.m_vec[0] + temp.m_vec[1];
            }

            constexpr const T Dot(const Vector2Type& rhs) const {
                const v2 temp = m_vec * rhs.m_vec;
                return temp.m_vec[0] + temp.m_vec[1];
            }
    };

    using Vector2f = Vector2Type<float>;
    using Vector2u  = Vector2Type<unsigned int>;
    using Vector2i  = Vector2Type<int>;
    using Vector2d = Vector2Type<double>;

    static_assert((Vector2u(v2ui{2,2}) + Vector2u(v2ui{2,4})) != Vector2u(v2ui{2,2}));
    static_assert((Vector2f(v2f{1.0f,1.0f}) + Vector2f(v2f{1.0f,1.0f})) == Vector2f(v2f{2.0f,2.0f}));
}
