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
            static constexpr u32 cVectorLength = 4;
        public:
            typedef T __attribute__((vector_size(sizeof(T) * 4))) v4;

            T x;
            T y;
            T z;
            T w;
        public:
            constexpr Vector4Type() : x(), y(), z(), w() { /*...*/ }
            constexpr Vector4Type(v4& copy) : x(copy[0]), y(copy[1]), z(copy[2]), w(copy[3]) { /*...*/ }
            constexpr Vector4Type(const v4& copy) : x(copy[0]), y(copy[1]), z(copy[2]), w(copy[3]) { /*...*/ }
            constexpr Vector4Type(T x, T y = 0, T z = 0, T w = 0) :  x(x), y(y), z(z), w(w) { /*...*/ }

            constexpr Vector4Type(const Vector4Type& rhs) : x(rhs.x), y(rhs.y), z(rhs.z), w(rhs.w) { /*...*/ }

            constexpr ~Vector4Type() {/*...*/}

            constexpr ALWAYS_INLINE v4 GetVectorType() const {
                return v4{x, y, z, w};
            }

            constexpr Vector4Type& operator=(Vector4Type& rhs) {
                const v4 temp{rhs.x, rhs.y, rhs.z, rhs.w};
                x = temp[0];
                y = temp[1];
                z = temp[2];
                w = temp[3];
                return *this;
            }

            constexpr Vector4Type& operator=(const Vector4Type& rhs) {
                const v4 temp{rhs.x, rhs.y, rhs.z, rhs.w};
                x = temp[0];
                y = temp[1];
                z = temp[2];
                w = temp[3];
                return *this;
            }

            constexpr Vector4Type operator+(Vector4Type& rhs) {
                const v4 lhsv = this->GetVectorType();
                const v4 rhsv = rhs.GetVectorType();
                v4 a = lhsv + rhsv;
                return Vector4Type(a);
            }

            constexpr Vector4Type operator+(const Vector4Type& rhs) {
                const v4 lhsv = this->GetVectorType();
                const v4 rhsv = rhs.GetVectorType();
                v4 a = lhsv + rhsv;
                return Vector4Type(a);
            }

            constexpr Vector4Type operator-(Vector4Type& rhs) {
                const v4 lhsv = this->GetVectorType();
                const v4 rhsv = rhs.GetVectorType();
                v4 a = lhsv - rhsv;
                return Vector4Type(a);
            }

            constexpr Vector4Type operator-(const Vector4Type& rhs) {
                const v4 lhsv = this->GetVectorType();
                const v4 rhsv = rhs.GetVectorType();
                v4 a = lhsv - rhsv;
                return Vector4Type(a);
            }

            constexpr Vector4Type operator*(Vector4Type& rhs) {
                const v4 lhsv = this->GetVectorType();
                const v4 rhsv = rhs.GetVectorType();
                const v4 a = lhsv * rhsv;
                return Vector4Type(a);
            }

            constexpr Vector4Type operator*(const Vector4Type& rhs) {
                const v4 lhsv = this->GetVectorType();
                const v4 rhsv = rhs.GetVectorType();
                const v4 a = lhsv * rhsv;
                return Vector4Type(a);
            }

            constexpr Vector4Type operator/(Vector4Type& rhs) {
                const v4 lhsv = this->GetVectorType();
                const v4 rhsv = rhs.GetVectorType();
                const v4 a = lhsv / rhsv;
                return Vector4Type(a);
            }

            constexpr Vector4Type operator/(const Vector4Type& rhs) {
                const v4 lhsv = this->GetVectorType();
                const v4 rhsv = rhs.GetVectorType();
                const v4 a = lhsv / rhsv;
                return Vector4Type(a);
            }

            constexpr Vector4Type &operator+=(Vector4Type& rhs) {
                const v4 lhsv = this->GetVectorType();
                const v4 rhsv = rhs.GetVectorType();
                std::construct_at(this, lhsv + rhsv);
                return *this;
            }

            constexpr Vector4Type &operator+=(const Vector4Type& rhs) {
                const v4 lhsv = this->GetVectorType();
                const v4 rhsv = rhs.GetVectorType();
                std::construct_at(this, lhsv + rhsv);
                return *this;
            }

            constexpr Vector4Type &operator-=(Vector4Type& rhs) {
                const v4 lhsv = this->GetVectorType();
                const v4 rhsv = rhs.GetVectorType();
                std::construct_at(this, lhsv - rhsv);
                return *this;
            }

            constexpr Vector4Type &operator-=(const Vector4Type& rhs) {
                const v4 lhsv = this->GetVectorType();
                const v4 rhsv = rhs.GetVectorType();
                std::construct_at(this, lhsv - rhsv);
                return *this;
            }

            constexpr bool operator==(Vector4Type& rhs) {
                const v4 lhsv = this->GetVectorType();
                const v4 rhsv = rhs.GetVectorType();
                return (lhsv[0] == rhsv[0]) & (lhsv[1] == rhsv[1]) & (lhsv[2] == rhsv[2]) & (lhsv[3] == rhsv[3]);
            }

            constexpr bool operator==(const Vector4Type& rhs) {
                const v4 lhsv = this->GetVectorType();
                const v4 rhsv = rhs.GetVectorType();
                return (lhsv[0] == rhsv[0]) & (lhsv[1] == rhsv[1]) & (lhsv[2] == rhsv[2]) & (lhsv[3] == rhsv[3]);
            }

            constexpr bool operator!=(Vector4Type& rhs) {
                const v4 lhsv = this->GetVectorType();
                const v4 rhsv = rhs.GetVectorType();
                return !((lhsv[0] == rhsv[0]) & (lhsv[1] == rhsv[1]) & (lhsv[2] == rhsv[2]) & (lhsv[3] == rhsv[3]));
            }

            constexpr bool operator!=(const Vector4Type& rhs) {
                const v4 lhsv = this->GetVectorType();
                const v4 rhsv = rhs.GetVectorType();
                return !((lhsv[0] == rhsv[0]) & (lhsv[1] == rhsv[1]) & (lhsv[2] == rhsv[2]) & (lhsv[3] == rhsv[3]));
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

            constexpr T Dot(Vector4Type& rhs) {
                const v4 lhsv = this->GetVectorType();
                const v4 rhsv = rhs.GetVectorType();
                const v4 a = lhsv * rhsv;
                return a[0] + a[1] + a[2] + a[3];
            }

            constexpr T Dot(const Vector4Type& rhs) {
                const v4 lhsv = this->GetVectorType();
                const v4 rhsv = rhs.GetVectorType();
                const v4 a = lhsv * rhsv;
                return a[0] + a[1] + a[2] + a[3];
            }
    };

    using Vector4f = Vector4Type<float>;
    using Vector4u  = Vector4Type<unsigned int>;
    using Vector4i  = Vector4Type<int>;
    using Vector4d = Vector4Type<double>;
    static_assert(sizeof(Vector4f) == 0x10);
    static_assert(sizeof(Vector4u) == 0x10);
    static_assert(sizeof(Vector4i) == 0x10);
    static_assert(sizeof(Vector4d) == 0x20);
    static_assert(alignof(Vector4f) == 0x4);
    static_assert(alignof(Vector4u) == 0x4);
    static_assert(alignof(Vector4i) == 0x4);
    static_assert(alignof(Vector4d) == 0x8);

    static_assert((Vector4u(v4ui{2,2,2,2}) + Vector4u(v4ui{2,4,2,2})) != Vector4u(v4ui{2,2,2,2}));
    static_assert((Vector4f(v4f{1.0f,1.0f,1.0f,1.0f}) + Vector4f(v4f{1.0f,1.0f,1.0f,1.0f})) == Vector4f(v4f{2.0f,2.0f,2.0f,2.0f}));
}
