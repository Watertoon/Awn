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

    typedef float v4f __attribute__((vector_size(16)));
    typedef int v4si __attribute__((vector_size(16)));
    typedef unsigned int v4ui __attribute__((vector_size(16)));
    typedef double v4d __attribute__((vector_size(32)));

    template<typename T>
        requires std::is_integral<T>::value || std::is_floating_point<T>::value
    class Vector4Type {
        public:
            typedef T __attribute__((vector_size(sizeof(T) * 4))) v4;
            v4 m_vec;
        public:
            constexpr Vector4Type() : m_vec() { /*...*/ }
            constexpr Vector4Type(v4& copy) : m_vec(copy) { /*...*/ }
            constexpr Vector4Type(const v4& copy) : m_vec(copy) { /*...*/ }
            constexpr Vector4Type(T x, T y = 0, T z = 0, T w = 0) : m_vec{x, y, z, w} { /*...*/ }

            constexpr Vector4Type(const Vector4Type& rhs) : m_vec(rhs.m_vec) { /*...*/ }

            constexpr Vector4Type& operator=(Vector4Type& rhs) {
                m_vec = rhs.m_vec;
                return *this;
            }

            constexpr Vector4Type& operator=(const Vector4Type& rhs) {
                m_vec = rhs.m_vec;
                return *this;
            }

            constexpr Vector4Type operator+(Vector4Type& rhs) {
                v4 a = m_vec + rhs.m_vec;
                return Vector4Type(a);
            }

            constexpr Vector4Type operator+(const Vector4Type& rhs) {
                v4 a = m_vec + rhs.m_vec;
                return Vector4Type(a);
            }

            constexpr Vector4Type operator-(Vector4Type& rhs) {
                v4 a = m_vec - rhs.m_vec;
                return Vector4Type(a);
            }

            constexpr Vector4Type operator-(const Vector4Type& rhs) {
                v4 a = m_vec - rhs.m_vec;
                return Vector4Type(a);
            }

            constexpr T operator*(Vector4Type& rhs) {
                v4 a = m_vec * rhs.m_vec;
                return a[0] + a[1] + a[2] + a[3];
            }

            constexpr T operator*(const Vector4Type& rhs) {
                v4 a = m_vec * rhs.m_vec;
                return a[0] + a[1] + a[2] + a[3];
            }

            constexpr Vector4Type operator/(Vector4Type& rhs) {
                v4 a = m_vec / rhs.m_vec;
                return Vector4Type(a);
            }

            constexpr Vector4Type operator/(const Vector4Type& rhs) {
                v4 a = m_vec / rhs.m_vec;
                return Vector4Type(a);
            }

            constexpr Vector4Type& operator+=(Vector4Type& rhs) {
                m_vec = m_vec + rhs.m_vec;
                return *this;
            }

            constexpr Vector4Type& operator+=(const Vector4Type& rhs) {
                m_vec = m_vec + rhs.m_vec;
                return *this;
            }

            constexpr Vector4Type& operator-=(Vector4Type& rhs) {
                m_vec = m_vec - rhs.m_vec;
                return *this;
            }

            constexpr Vector4Type& operator-=(const Vector4Type& rhs) {
                m_vec = m_vec - rhs.m_vec;
                return *this;
            }

            constexpr bool operator==(Vector4Type& rhs) {
                return m_vec[0] == rhs.m_vec[0] && m_vec[1] == rhs.m_vec[1] && m_vec[2] == rhs.m_vec[2] && m_vec[3] == rhs.m_vec[3];
            }

            constexpr bool operator==(const Vector4Type& rhs) {
                return m_vec[0] == rhs.m_vec[0] && m_vec[1] == rhs.m_vec[1] && m_vec[2] == rhs.m_vec[2] && m_vec[3] == rhs.m_vec[3];
            }

            constexpr bool operator!=(Vector4Type& rhs) {
                return !(m_vec[0] == rhs.m_vec[0] && m_vec[1] == rhs.m_vec[1] && m_vec[2] == rhs.m_vec[2] && m_vec[3] == rhs.m_vec[3]);
            }

            constexpr bool operator!=(const Vector4Type& rhs) {
                return !(m_vec[0] == rhs.m_vec[0] && m_vec[1] == rhs.m_vec[1] && m_vec[2] == rhs.m_vec[2] && m_vec[3] == rhs.m_vec[3]);
            }

            template<typename A = T> requires std::is_floating_point<A>::value && (sizeof(Vector3Type<A>) == sizeof(float) * 3)
            constexpr Vector4Type Cross(const Vector4Type& rhs) {
                const v4 a = sse4::shufps(this->GetVectorType(), this->GetVectorType(), sse4::ShuffleToOrder(1,2,0,3));
                const v4 b = sse4::shufps(rhs.GetVectorType(), rhs.GetVectorType(), sse4::ShuffleToOrder(2,0,1,3));
                const v4 c = sse4::mulps(a, rhs.GetVectorType());
                const v4 d = sse4::mulps(a, b);
                const v4 e = sse4::shufps(c, c, sse4::ShuffleToOrder(1, 2, 0, 3));
                return Vector4Type(sse4::subps(d, e));
            }

            template<typename A = T> requires std::is_floating_point<A>::value && (sizeof(Vector3Type<A>) == sizeof(float) * 3)
            constexpr Vector4Type Cross(const Vector4Type& rhs) const {
                const v4 a = sse4::shufps(this->GetVectorType(), this->GetVectorType(), sse4::ShuffleToOrder(1,2,0,3));
                const v4 b = sse4::shufps(rhs.GetVectorType(), rhs.GetVectorType(), sse4::ShuffleToOrder(2,0,1,3));
                const v4 c = sse4::mulps(a, rhs.GetVectorType());
                const v4 d = sse4::mulps(a, b);
                const v4 e = sse4::shufps(c, c, sse4::ShuffleToOrder(1, 2, 0, 3));
                return Vector4Type(sse4::subps(d, e));
            }

            template<typename A = T> requires std::is_integral<A>::value && (sizeof(Vector3Type<A>) == sizeof(s32) * 3)
            constexpr Vector4Type Cross(const Vector4Type& rhs) {
                const v4 a = sse4::pshufd(this->GetVectorType(), sse4::ShuffleToOrder(1,2,0,3));
                const v4 b = sse4::pshufd(rhs.GetVectorType(), sse4::ShuffleToOrder(2,0,1,3));
                const v4 c = sse4::pmuld(a, rhs.GetVectorType());
                const v4 d = sse4::pmuld(a, b);
                const v4 e = sse4::pshufd(c, sse4::ShuffleToOrder(1, 2, 0, 3));
                return Vector4Type(sse4::psubd(d, e));
            }

            template<typename A = T> requires std::is_integral<A>::value && (sizeof(Vector3Type<A>) == sizeof(s32) * 3)
            constexpr ALWAYS_INLINE Vector4Type Cross(const Vector4Type& rhs) const {
                const v4 a = sse4::pshufd(this->GetVectorType(), sse4::ShuffleToOrder(1,2,0,3));
                const v4 b = sse4::pshufd(rhs.GetVectorType(), sse4::ShuffleToOrder(2,0,1,3));
                const v4 c = sse4::pmuld(a, rhs.GetVectorType());
                const v4 d = sse4::pmuld(a, b);
                const v4 e = sse4::pshufd(c, sse4::ShuffleToOrder(1, 2, 0, 3));
                return Vector4Type(sse4::psubd(d, e));
            }

            constexpr T Dot(const Vector4Type& rhs) {
                const v4 temp = m_vec * rhs.m_vec;
                return temp.m_vec[0] + temp.m_vec[1] + temp.m_vec[2] + temp.m_vec[3];
            }

            constexpr const T Dot(const Vector4Type& rhs) const {
                const v4 temp = m_vec * rhs.m_vec;
                return temp.m_vec[0] + temp.m_vec[1] + temp.m_vec[2] + temp.m_vec[3];
            }
    };

    using Vector4f = Vector4Type<float>;
    using Vector4u  = Vector4Type<unsigned int>;
    using Vector4i  = Vector4Type<int>;
    using Vector4d = Vector4Type<double>;

    static_assert((Vector4u(v4ui{2,2,2,2}) + Vector4u(v4ui{2,4,2,2})) != Vector4u(v4ui{2,2,2,2}));
    static_assert((Vector4f(v4f{1.0f,1.0f,1.0f,1.0f}) + Vector4f(v4f{1.0f,1.0f,1.0f,1.0f})) == Vector4f(v4f{2.0f,2.0f,2.0f,2.0f}));
}
