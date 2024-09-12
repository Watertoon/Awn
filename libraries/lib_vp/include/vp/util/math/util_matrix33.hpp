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

    template<typename T> requires std::is_floating_point<T>::value
    class Matrix33RowMajorType {
        public:
            typedef T __attribute__((vector_size(sizeof(T) * 16))) m33t;
            union {
                struct{
                    Vector3Type<T> m_r1; /* a b c */
                    Vector3Type<T> m_r2; /* d e f */
                    Vector3Type<T> m_r3; /* h i j */
                };
                T m_arr[9];
                T m_arr2d[3][3];
            };

        public:
            constexpr Matrix33RowMajorType() : m_arr{} {/*...*/}
            constexpr Matrix33RowMajorType(const T m11, const T m21 = 0, const T m31 = 0, const T m12 = 0, const T m22 = 0, const T m32 = 0, const T m13 = 0, const T m23 = 0, const T m33 = 0) : m_arr{ m11, m21, m31, m12, m22, m32, m13, m23, m33 } {/*...*/}
            constexpr Matrix33RowMajorType(const Vector3Type<T>& r1, const Vector3Type<T>& r2 = 0, const Vector3Type<T>& r3 = 0) : m_r1(r1), m_r2(r2), m_r3(r3) {/*...*/}

            constexpr Matrix33RowMajorType(const Matrix33RowMajorType& rhs) : m_arr{rhs.m_arr[0],rhs.m_arr[1],rhs.m_arr[2],rhs.m_arr[3],rhs.m_arr[4],rhs.m_arr[5],rhs.m_arr[6],rhs.m_arr[7],rhs.m_arr[8]} {/*...*/}

            constexpr ~Matrix33RowMajorType() {/*...*/}

            constexpr ALWAYS_INLINE Matrix33RowMajorType operator+(const Matrix33RowMajorType& rhs) const {
                return { m_r1 + rhs.m_r1, m_r2 + rhs.m_r2, m_r3 + rhs.m_r3 };
            }

            constexpr ALWAYS_INLINE Matrix33RowMajorType operator-(const Matrix33RowMajorType& rhs) const {
                return { m_r1 - rhs.m_r1, m_r2 - rhs.m_r2, m_r3 - rhs.m_r3 };
            }

            constexpr ALWAYS_INLINE Matrix33RowMajorType operator*(const Matrix33RowMajorType& rhs) const {
                const Vector3Type<T> t_adg(rhs.m_arr2d[0][0], rhs.m_arr2d[1][0], rhs.m_arr2d[2][0]); 
                const Vector3Type<T> t_beh(rhs.m_arr2d[0][1], rhs.m_arr2d[1][1], rhs.m_arr2d[2][1]); 
                const Vector3Type<T> t_cfi(rhs.m_arr2d[0][2], rhs.m_arr2d[1][2], rhs.m_arr2d[2][2]); 
                return { m_r1 * t_adg, m_r2 * t_beh, m_r3 * t_cfi};
            }

            constexpr ALWAYS_INLINE T Determinent() const {
                const T d_eifh = (m_arr2d[1][1] * m_arr2d[2][2]) - (m_arr2d[1][2] * m_arr2d[2][1]);
                const T d_fgid = (m_arr2d[1][2] * m_arr2d[2][0]) - (m_arr2d[1][0] * m_arr2d[2][2]);
                const T d_dheg = (m_arr2d[1][0] * m_arr2d[2][1]) - (m_arr2d[1][1] * m_arr2d[0][2]);
                const T determinent = (m_arr2d[0][0] * d_eifh) + (m_arr2d[0][1] * d_fgid) + (m_arr2d[0][2] * d_dheg);
                return determinent;
            }
    };

    using Matrix33f = Matrix33RowMajorType<float>;

    //constexpr ALWAYS_INLINE bool Inverse(Matrix33f *out_inverse, Matrix33f *in_matrix) {
    //    const float d_eifh = (in_matrix->m_arr2d[1][1] * in_matrix->m_arr2d[2][2]) - (in_matrix->m_arr2d[1][2] * in_matrix->m_arr2d[2][1]);
    //    const float d_fgid = (in_matrix->m_arr2d[1][2] * in_matrix->m_arr2d[2][0]) - (in_matrix->m_arr2d[1][0] * in_matrix->m_arr2d[2][2]);
    //    const float d_dheg = (in_matrix->m_arr2d[1][0] * in_matrix->m_arr2d[2][1]) - (in_matrix->m_arr2d[1][1] * in_matrix->m_arr2d[0][2]);
    //    const float determinent = (in_matrix->m_arr2d[0][0] * d_eifh) + (in_matrix->m_arr2d[0][1] * d_fgid) + (in_matrix->m_arr2d[0][2] * d_dheg);
    //    if (determinent == 0.0) {
    //        return false;
    //    }
    //    
    //    return true;
    //}
}
