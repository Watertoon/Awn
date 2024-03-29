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
        requires std::is_integral<T>::value || std::is_floating_point<T>::value
    class Vector3Type {
        public:
            static constexpr u32 cVectorLength = 3;
        public:
            typedef T __attribute__((vector_size(sizeof(T) * 4))) v3;
        public:
            T x;
            T y;
            T z;
        public:
            constexpr ALWAYS_INLINE Vector3Type() : x(), y(), z() {/*...*/}
            constexpr ALWAYS_INLINE Vector3Type(T x, T y = 0, T z = 0) : x(x), y(y), z(z) {/*...*/}

            constexpr ALWAYS_INLINE Vector3Type(v3& copy) : x(copy[0]), y(copy[1]), z(copy[2]) {/*...*/}
            constexpr ALWAYS_INLINE Vector3Type(const v3& copy) : x(copy[0]), y(copy[1]), z(copy[2]) {/*...*/}
            constexpr ALWAYS_INLINE Vector3Type(const Vector3Type& rhs) : x(rhs.x), y(rhs.y), z(rhs.z) {/*...*/}

            template<typename Y>
            constexpr ALWAYS_INLINE Vector3Type(const Vector3Type<Y>& rhs) : x(rhs.x), y(rhs.y), z(rhs.z) {/*...*/}

            constexpr ~Vector3Type() {/*...*/}

            constexpr v3 GetVectorType() {
                return v3{x,y,z,0};
            }

            constexpr const v3 GetVectorType() const {
                return v3{x,y,z,0};
            }

            constexpr Vector3Type& operator=(const Vector3Type& rhs) {
                x = rhs.x;
                y = rhs.y;
                z = rhs.z;
                return *this;
            }

            constexpr ALWAYS_INLINE Vector3Type operator+(Vector3Type& rhs) {
                return Vector3Type(this->GetVectorType() + rhs.GetVectorType());
            }

            constexpr ALWAYS_INLINE Vector3Type operator+(const Vector3Type& rhs) const {
                return Vector3Type(this->GetVectorType() + rhs.GetVectorType());
            }

            constexpr ALWAYS_INLINE Vector3Type operator-() {
                return Vector3Type(-this->x, -this->y, -this->z);
            }

            constexpr ALWAYS_INLINE Vector3Type operator-(Vector3Type& rhs) {
                return Vector3Type(this->GetVectorType() - rhs.GetVectorType());
            }

            constexpr ALWAYS_INLINE Vector3Type operator-(const Vector3Type& rhs) const {
                return Vector3Type(this->GetVectorType() - rhs.GetVectorType());
            }

            constexpr ALWAYS_INLINE Vector3Type operator*(T scalar) {
                return Vector3Type(this->x * scalar, this->y * scalar, this->z * scalar);
            }

            constexpr ALWAYS_INLINE Vector3Type operator*(const T scalar) const {
                return Vector3Type(this->x * scalar, this->y * scalar, this->z * scalar);
            }

            constexpr ALWAYS_INLINE Vector3Type operator/(T scalar) {
                return Vector3Type(this->x / scalar, this->y / scalar, this->z / scalar);
            }

            constexpr ALWAYS_INLINE Vector3Type operator/(const T scalar) const {
                return Vector3Type(this->x / scalar, this->y / scalar, this->z / scalar);
            }

            constexpr ALWAYS_INLINE Vector3Type& operator+=(Vector3Type& rhs) {
                const v3 v = this->GetVectorType() + rhs.GetVectorType();
                x = v[0];
                y = v[1];
                z = v[2];
                return *this;
            }

            constexpr ALWAYS_INLINE Vector3Type& operator+=(const Vector3Type& rhs) {
                const v3 v = this->GetVectorType() + rhs.GetVectorType();
                x = v[0];
                y = v[1];
                z = v[2];
                return *this;
            }

            constexpr ALWAYS_INLINE Vector3Type& operator-=(Vector3Type& rhs) {
                const v3 v = this->GetVectorType() - rhs.GetVectorType();
                x = v[0];
                y = v[1];
                z = v[2];
                return *this;
            }

            constexpr ALWAYS_INLINE Vector3Type& operator-=(const Vector3Type& rhs) {
                const v3 v = this->GetVectorType() - rhs.GetVectorType();
                x = v[0];
                y = v[1];
                z = v[2];
                return *this;
            }

            constexpr ALWAYS_INLINE bool operator==(Vector3Type& rhs) {
                return (x == rhs.x) & (y == rhs.y) & (z == rhs.z);
            }

            constexpr ALWAYS_INLINE bool operator==(const Vector3Type& rhs) {
                return (x == rhs.x) & (y == rhs.y) & (z == rhs.z);
            }

            constexpr ALWAYS_INLINE bool operator!=(Vector3Type& rhs) {
                return !(*this == rhs);
            }

            constexpr ALWAYS_INLINE bool operator!=(const Vector3Type& rhs) {
                return !(*this == rhs);
            }

#ifdef VP_TARGET_ARCHITECTURE_x86
            template<typename A = T> 
                requires std::is_floating_point<A>::value && (sizeof(Vector3Type<A>) == sizeof(float) * 3)
            constexpr ALWAYS_INLINE Vector3Type Cross(const Vector3Type& rhs) {
                const v3 a = avx2::shufps(this->GetVectorType(), this->GetVectorType(), avx2::ShuffleToOrder(1,2,0,3));
                const v3 b = avx2::shufps(rhs.GetVectorType(), rhs.GetVectorType(), avx2::ShuffleToOrder(2,0,1,3));
                const v3 c = avx2::mulps(a, rhs.GetVectorType());
                const v3 d = avx2::mulps(a, b);
                const v3 e = avx2::shufps(c, c, avx2::ShuffleToOrder(1, 2, 0, 3));
                return Vector3Type(avx2::subps(d, e));
            }

            template<typename A = T> 
                requires std::is_floating_point<A>::value && (sizeof(Vector3Type<A>) == sizeof(float) * 3)
            constexpr ALWAYS_INLINE Vector3Type Cross(const Vector3Type& rhs) const {
                const v3 a = avx2::shufps(this->GetVectorType(), this->GetVectorType(), avx2::ShuffleToOrder(1,2,0,3));
                const v3 b = avx2::shufps(rhs.GetVectorType(), rhs.GetVectorType(), avx2::ShuffleToOrder(2,0,1,3));
                const v3 c = avx2::mulps(a, rhs.GetVectorType());
                const v3 d = avx2::mulps(a, b);
                const v3 e = avx2::shufps(c, c, avx2::ShuffleToOrder(1, 2, 0, 3));
                return Vector3Type(avx2::subps(d, e));
            }

            template<typename A = T> requires std::is_integral<A>::value && (sizeof(Vector3Type<A>) == sizeof(s32) * 3)
            constexpr ALWAYS_INLINE Vector3Type Cross(const Vector3Type& rhs) {
                const v3 a = avx2::pshufd(this->GetVectorType(), avx2::ShuffleToOrder(1,2,0,3));
                const v3 b = avx2::pshufd(rhs.GetVectorType(), avx2::ShuffleToOrder(2,0,1,3));
                const v3 c = avx2::pmuld(a, rhs.GetVectorType());
                const v3 d = avx2::pmuld(a, b);
                const v3 e = avx2::pshufd(c, avx2::ShuffleToOrder(1, 2, 0, 3));
                return Vector3Type(avx2::psubd(d, e));
            }

            template<typename A = T> requires std::is_integral<A>::value && (sizeof(Vector3Type<A>) == sizeof(s32) * 3)
            constexpr ALWAYS_INLINE Vector3Type Cross(const Vector3Type& rhs) const {
                const v3 a = avx2::pshufd(this->GetVectorType(), avx2::ShuffleToOrder(1,2,0,3));
                const v3 b = avx2::pshufd(rhs.GetVectorType(), avx2::ShuffleToOrder(2,0,1,3));
                const v3 c = avx2::pmuld(a, rhs.GetVectorType());
                const v3 d = avx2::pmuld(a, b);
                const v3 e = avx2::pshufd(c, avx2::ShuffleToOrder(1, 2, 0, 3));
                return Vector3Type(avx2::psubd(d, e));
            }
#else
            /* TODO; generic impl */
            constexpr ALWAYS_INLINE Vector3Type Cross([[maybe_unused]] const Vector3Type& rhs) {
                return Vector3Type();
            }

            constexpr ALWAYS_INLINE Vector3Type Cross([[maybe_unused]] const Vector3Type& rhs) const {
                return Vector3Type();
            }
#endif

            constexpr ALWAYS_INLINE T Dot(const Vector3Type& rhs) {
                const v3 temp = this->GetVectorType() * rhs.GetVectorType();
                return temp[0] + temp[1] + temp[2];
            }

            constexpr ALWAYS_INLINE const T Dot(const Vector3Type& rhs) const {
                const v3 temp = this->GetVectorType() * rhs.GetVectorType();
                return temp[0] + temp[1] + temp[2];
            }

            template<typename A = T> 
                requires std::is_floating_point<A>::value && (sizeof(Vector3Type<A>) == sizeof(float) * 3)
            constexpr ALWAYS_INLINE T Magnitude() {
                return ::sqrtf(x * x + y * y + z * z);
            }

            template<typename A = T> 
                requires std::is_floating_point<A>::value && (sizeof(Vector3Type<A>) == sizeof(float) * 3)
            constexpr ALWAYS_INLINE const T Magnitude() const {
                return ::sqrtf(x * x + y * y + z * z);
            }

            template<typename A = T> 
                requires std::is_floating_point<A>::value && (sizeof(Vector3Type<A>) == sizeof(float) * 3)
            constexpr ALWAYS_INLINE Vector3Type<A> Normalize() {
                Vector3Type<A> vec = *this;
                const float mag0 = this->Magnitude();
                if (mag0 == cInfinity) {
                    vec              = vec * 3.129822e-20;
                    const float mag1 = vec.Magnitude();
                    vec              = vec * mag1;
                } else if (cEpsilon < mag0) {
                    const float reciprocal_mag = 1.0 / mag0;
                    vec = vec * reciprocal_mag;
                } else {
                    vec = v3{0.0, 0.0, 0.0, 0.0};
                }
                return vec;
            }

            template<typename A = T> 
                requires std::is_floating_point<A>::value && (sizeof(Vector3Type<A>) == sizeof(float) * 3)
            constexpr ALWAYS_INLINE Vector3Type<A> Normalize() const {
                Vector3Type<A> vec = *this;
                const float mag0 = this->Magnitude();
                if (mag0 == cInfinity) {
                    vec              = vec * 3.129822e-20;
                    const float mag1 = vec.Magnitude();
                    vec              = vec * mag1;
                } else if (cEpsilon < mag0) {
                    const float reciprocal_mag = 1.0 / mag0;
                    vec = vec * reciprocal_mag;
                } else {
                    vec = v3{0.0, 0.0, 0.0, 0.0};
                }
                return vec;
            }
    };

    using Vector3f = Vector3Type<float>;
    using Vector3u = Vector3Type<unsigned int>;
    using Vector3i = Vector3Type<int>;
    using Vector3d = Vector3Type<double>;
    static_assert(sizeof(Vector3f) == 0xc);
    static_assert(sizeof(Vector3u) == 0xc);
    static_assert(sizeof(Vector3i) == 0xc);
    static_assert(sizeof(Vector3d) == 0x18);
    static_assert(alignof(Vector3f) == 0x4);
    static_assert(alignof(Vector3u) == 0x4);
    static_assert(alignof(Vector3i) == 0x4);
    static_assert(alignof(Vector3d) == 0x8);

    template<typename T>
    constexpr Vector3Type<T> cZeroVector3 = {};

    template<typename T>
    constexpr Vector3Type<T> cEXVector3(1, 0, 0);

    template<typename T>
    constexpr Vector3Type<T> cEYVector3(0, 1, 0);

    template<typename T>
    constexpr Vector3Type<T> cEZVector3(0, 0, 1);

#ifdef VP_TARGET_ARCHITECTURE_x86
    static_assert((Vector3u(v4ui{2,2,2}) + Vector3u(v4ui{2,4,2})) != Vector3u(v4ui{2,2,2}));
    static_assert((Vector3f(v4f{1.0f,1.0f,1.0f}) + Vector3f(v4f{1.0f,1.0f,1.0f})) == Vector3f(v4f{2.0f,2.0f,2.0f}));
    static_assert(Vector3f(v4f{1.0f,0.0f,1.0f}).Dot(Vector3f(v4f{1.0f,1.0f,0.0f})) == 1.0f);
#endif
}
