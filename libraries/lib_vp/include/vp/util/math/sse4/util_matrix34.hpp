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
    class Matrix34RowMajorType {
        public:
            typedef T __attribute__((vector_size(sizeof(T) * 16))) m34t;
            union {
                struct {
                    Vector4Type<T> m_row1;
                    Vector4Type<T> m_row2;
                    Vector4Type<T> m_row3;
                };
                Vector4Type<T> m_row_array[3];
                T              m_arr[12];
                T              m_arr2d[3][4];
            };
        public:
            constexpr Matrix34RowMajorType() : m_arr{} { /*...*/ }
            constexpr Matrix34RowMajorType(const m34t& vec) : m_row1(vec[0], vec[1], vec[2], vec[3]), m_row2(vec[4], vec[5], vec[6], vec[7]), m_row3(vec[8], vec[9], vec[10], vec[11]) {/*...*/}
            constexpr Matrix34RowMajorType(const Vector4Type<T>& row1, const Vector4Type<T>& row2, const Vector4Type<T>& row3) : m_row1(row1), m_row2(row2), m_row3(row3) {/*...*/}
            constexpr Matrix34RowMajorType(T m11, T m12, T m13, T m14, T m21, T m22, T m23, T m24, T m31, T m32, T m33, T m34) : m_row1(m11, m12, m13, m14), m_row2(m21, m22, m23, m24), m_row3(m31, m32, m33, m34) {/*...*/}

            constexpr Matrix34RowMajorType(const Matrix34RowMajorType& rhs) : m_row1(rhs.m_row1), m_row2(rhs.m_row2), m_row3(rhs.m_row3) {/*...*/}

            constexpr ALWAYS_INLINE Matrix34RowMajorType& operator=(const Matrix34RowMajorType& rhs) {
                m_row1 = rhs.m_row1;
                m_row2 = rhs.m_row2;
                m_row3 = rhs.m_row3;
                return *this;
            }

            constexpr void SetColumn(int index, const Vector3f& col_vec3) {
                const Vector3f::v3 vec = col_vec3.GetVectorType();
                m_arr2d[0][index] = vec[0];
                m_arr2d[1][index] = vec[1];
                m_arr2d[2][index] = vec[2];
            }       

            constexpr void SetRow(int index, const Vector4f& row_vec4) {
                m_row_array[index] = row_vec4;
            }
    };

    using Matrix34f = Matrix34RowMajorType<float>;

    static_assert(sizeof(Matrix34f) == sizeof(float) * 12);

    template<typename T>
    constexpr Matrix34RowMajorType<T> ZeroMatrix34 = {};

    template<typename T>
    constexpr Matrix34RowMajorType<T> IdentityMatrix34(Vector4Type<T>(1.0, 0.0, 0.0, 0.0), Vector4Type<T>(0.0, 1.0, 0.0, 0.0), Vector4Type<T>(0.0, 0.0, 1.0, 0.0));

    constexpr bool MakeVectorRotation(Matrix34f *out_rot_matrix, const Vector3f& align, const Vector3f& base) {
        const float dot = base.Dot(align) + 1.0f;

        if (dot <= cFloatUlp) {
            *out_rot_matrix = IdentityMatrix34<float>;
            return false;
        }

        const float sqrt2dot = ::sqrt(dot + dot);
        const float k = 1.0f / sqrt2dot;

        const Vector3f kcross = base.Cross(align) * k;

        const float kcross2x               = kcross.x + kcross.x;
        const float kcross2y_kcrossy       = kcross.y * (kcross.y + kcross.y);
        const float kcross2z_kcrossz       = kcross.z * (kcross.z + kcross.z);
        const float kcross2y_kcrossz       = kcross.z * (kcross.y + kcross.y);
        const float oproj_kcross2x_kcrossx = 1.0f - (kcross.x * kcross2x);

        const Vector4f row1( (1.0f - kcross2y_kcrossy) - kcross2y_kcrossz,  (kcross.y * kcross2x) - (kcross.z * sqrt2dot), (kcross.y * sqrt2dot) + (kcross.z * kcross2x), 0.0f );
        const Vector4f row2( (kcross.z * sqrt2dot) + (kcross.y * kcross2x),  oproj_kcross2x_kcrossx - kcross2z_kcrossz,     kcross2z_kcrossz - (kcross.x * sqrt2dot),     0.0f );
        const Vector4f row3( (kcross.z * kcross2x) - (kcross.y - sqrt2dot), (kcross.x * sqrt2dot) + kcross2y_kcrossz,       oproj_kcross2x_kcrossx - kcross2y_kcrossy,    0.0f );

        const Matrix34f out(row1, row2, row3);

        *out_rot_matrix = out;

        return true;
    }

    /*void MakeSRTIndex(Matrix34f *out_srt_matrix, const Vector3f& scale, const Vector3f& rotation, const Vector3f& translation) {
        const float sin_x = SampleSin(rotation.m_vec[0]);
        const float cos_x = SampleCos(rotation.m_vec[0]);
        const float sin_y = SampleSin(rotation.m_vec[1]);
        const float cos_y = SampleCos(rotation.m_vec[1]);
        const float sin_z = SampleSin(rotation.m_vec[2]);
        const float cos_z = SampleCos(rotation.m_vec[2]);
        const Vector4f row1(  translation.m_vec[0] );
        const Vector4f row2(  translation.m_vec[1] );
        const Vector4f row3(  translation.m_vec[2] );
    }*/
}
