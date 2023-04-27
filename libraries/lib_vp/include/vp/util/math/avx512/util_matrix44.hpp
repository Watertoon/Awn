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

    typedef float  __attribute__((vector_size(sizeof(float) * 16)))  m44f;
    typedef double __attribute__((vector_size(sizeof(double) * 16))) m44d;
    typedef s32    __attribute__((vector_size(sizeof(s32) * 16)))    m44si;
    typedef s64    __attribute__((vector_size(sizeof(s64) * 16)))    m44sll;
    typedef u32    __attribute__((vector_size(sizeof(u32) * 16)))    m44ui;
    typedef u64    __attribute__((vector_size(sizeof(u64) * 16)))    m44ull;

    template<typename T> requires std::is_floating_point<T>::value
    class Matrix44RowMajorType {
        public:
            typedef T __attribute__((vector_size(sizeof(T) * 16))) m44t;
            union {
                struct{
                    Vector4Type<T> m_row1; /* a b c d */
                    Vector4Type<T> m_row2; /* e f g h */
                    Vector4Type<T> m_row3; /* i j k l*/
                    Vector4Type<T> m_row4; /* m n o p */
                };
                T m_arr[16];
                T m_arr2d[4][4];
            };
        public:
            constexpr Matrix44RowMajorType() : m_arr{} {/*...*/}
            constexpr Matrix44RowMajorType(const T m11, const T m12 = 0, const T m13 = 0, const T m14 = 0,  const T m21 = 0, const T m22 = 0, const T m23 = 0, const T m24 = 0, const T m31 = 0,  const T m32 = 0,  const T m33 = 0,  const T m34 = 0,  const T m41 = 0, const T m42 = 0, const T m43 = 0, const T m44 = 0) : m_arr{m11,m12,m13,m14,m21,m22,m23,m24,m31,m32,m33,m34,m41,m42,m43,m44} {/*...*/}
            constexpr Matrix44RowMajorType(const Vector4Type<T>& r1, const Vector4Type<T>& r2 = 0, const Vector4Type<T>& r3 = 0, const Vector4Type<T>& r4 = 0) : m_row1(r1), m_row2(r2), m_row3(r3), m_row4(r4) {/*...*/}

            constexpr Matrix44RowMajorType(const Matrix44RowMajorType& rhs) : m_row1(rhs.m_row1), m_row2(rhs.m_row2), m_row3(rhs.m_row3), m_row4(rhs.m_row4) {/*...*/}

            constexpr ALWAYS_INLINE Matrix44RowMajorType& operator=(const Matrix44RowMajorType& rhs) {
                m_row1 = rhs.m_row1;
                m_row2 = rhs.m_row2;
                m_row3 = rhs.m_row3;
                m_row4 = rhs.m_row4;
                return *this;
            }

            constexpr ALWAYS_INLINE Matrix44RowMajorType operator+(const Matrix44RowMajorType& rhs) const {
                const Matrix44RowMajorType out = { m_row1 + rhs.m_row1, m_row2 + rhs.m_row2, m_row3 + rhs.m_row3, m_row4 + rhs.r4 };
                return out;
            }

            constexpr ALWAYS_INLINE Matrix44RowMajorType operator-(const Matrix44RowMajorType& rhs) const {
                const Matrix44RowMajorType out = { m_row1 - rhs.m_row1, m_row2 - rhs.m_row2, m_row3 - rhs.m_row3, m_row4 + rhs.r4 };
                return out;
            }

            constexpr ALWAYS_INLINE Matrix44RowMajorType operator*(const Matrix44RowMajorType& rhs) const {
                const Vector4Type<T> t_aeim(rhs.m_arr2d[0][0], rhs.m_arr2d[1][0], rhs.m_arr2d[2][0], rhs.m_arr2d[3][0]);
                const Vector4Type<T> t_bfjn(rhs.m_arr2d[0][1], rhs.m_arr2d[1][1], rhs.m_arr2d[2][1], rhs.m_arr2d[3][1]);
                const Vector4Type<T> t_cgko(rhs.m_arr2d[0][2], rhs.m_arr2d[1][2], rhs.m_arr2d[2][2], rhs.m_arr2d[3][2]);
                const Vector4Type<T> t_dhlp(rhs.m_arr2d[0][3], rhs.m_arr2d[1][3], rhs.m_arr2d[2][3], rhs.m_arr2d[3][3]);
                const Matrix44RowMajorType out = { m_row1 * t_aeim, m_row2 * t_bfjn, m_row3 * t_cgko, m_row4 * t_dhlp};
                return out;
            }
    };

    using Matrix44f = Matrix44RowMajorType<float>;
}
