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

    constexpr Vector2f operator+(const Vector2f &left, const Vector2f &right) {
        const Vector2f out = { .vector = left.vector.f + right.vector.f, };
        return out;
    }
    constexpr Vector2f operator-(const Vector2f &left, const Vector2f &right) {
        const Vector2f out = { .vector = left.vector.f - right.vector.f, };
        return out;
    }
    constexpr Vector2f operator-(const Vector2f &vec) {
        const Vector2f out = { .vector = -vec.vector.f, };
        return out;
    }
    constexpr Vector2f operator*(const Vector2f &vec, float scalar) {
        const Vector2f out = { .vector = vec.vector.f * scalar, };
        return out;
    }
    constexpr Vector2f operator/(const Vector2f &vec, float scalar) {
        const Vector2f out = { .vector = vec.vector.f / scalar, };
        return out;
    }

    constexpr Vector3f operator+(const Vector3f &left, const Vector3f &right) {
        const Vector3f out = { .vector = left.vector.f + right.vector.f, };
        return out;
    }
    constexpr Vector3f operator-(const Vector3f &left, const Vector3f &right) {
        const Vector3f out = { .vector = left.vector.f - right.vector.f, };
        return out;
    }
    constexpr Vector3f operator-(const Vector3f &vec) {
        const Vector3f out = { .vector = -vec.vector.f, };
        return out;
    }
    constexpr Vector3f operator*(const Vector3f &vec, float scalar) {
        const Vector3f out = { .vector = vec.vector.f * scalar, };
        return out;
    }
    constexpr Vector3f operator/(const Vector3f &vec, float scalar) {
        const Vector3f out = { .vector = vec.vector.f / scalar, };
        return out;
    }

    constexpr Vector4f operator+(const Vector4f &left, const Vector4f &right) {
        const Vector4f out = { .vector = left.vector.f + right.vector.f, };
        return out;
    }
    constexpr Vector4f operator-(const Vector4f &left, const Vector4f &right) {
        const Vector4f out = { .vector = left.vector.f - right.vector.f, };
        return out;
    }
    constexpr Vector4f operator-(const Vector4f &vec) {
        const Vector4f out = { .vector = -vec.vector.f, };
        return out;
    }
    constexpr Vector4f operator*(const Vector4f &vec, float scalar) {
        const Vector4f out = { .vector = vec.vector.f * scalar, };
        return out;
    }
    constexpr Vector4f operator/(const Vector4f &vec, float scalar) {
        const Vector4f out = { .vector = vec.vector.f / scalar, };
        return out;
    }

    constexpr Float2 operator+(const Float2 &left, const Float2 &right) {
        const Float2 out = { left.x + right.x, left.y + right.y, };
        return out;
    }
    constexpr Float2 operator-(const Float2 &left, const Float2 &right) {
        const Float2 out = { left.x - right.x, left.y - right.y, };
        return out;
    }
    constexpr Float2 operator-(const Float2 &vec) {
        const Float2 out = { -vec.x, -vec.y, };
        return out;
    }
    constexpr Float2 operator*(const Float2 &vec, float scalar) {
        const Float2 out = { vec.x * scalar, vec.y * scalar, };
        return out;
    }
    constexpr Float2 operator/(const Float2 &vec, float scalar) {
        const Float2 out = { vec.x / scalar, vec.y / scalar, };
        return out;
    }

    constexpr Float3 operator+(const Float3 &left, const Float3 &right) {
        const Float3 out = { left.x + right.x, left.y + right.y, left.z + right.z, };
        return out;
    }
    constexpr Float3 operator-(const Float3 &left, const Float3 &right) {
        const Float3 out = { left.x - right.x, left.y - right.y, left.z - right.z, };
        return out;
    }
    constexpr Float3 operator-(const Float3 &vec) {
        const Float3 out = { -vec.x, -vec.y, -vec.z, };
        return out;
    }
    constexpr Float3 operator*(const Float3 &vec, float scalar) {
        const Float3 out = { vec.x * scalar, vec.y * scalar, vec.z * scalar, };
        return out;
    }
    constexpr Float3 operator/(const Float3 &vec, float scalar) {
        const Float3 out = { vec.x / scalar, vec.y / scalar, vec.z / scalar, };
        return out;
    }

    constexpr Float4 operator+(const Float4 &left, const Float4 &right) {
        const Float4 out = { left.x + right.x, left.y + right.y, left.z + right.z, left.w + right.w, };
        return out;
    }
    constexpr Float4 operator-(const Float4 &left, const Float4 &right) {
        const Float4 out = { left.x - right.x, left.y - right.y, left.z - right.z, left.w - right.w, };
        return out;
    }
    constexpr Float4 operator-(const Float4 &vec) {
        const Float4 out = { -vec.x, -vec.y, -vec.z, -vec.w, };
        return out;
    }
    constexpr Float4 operator*(const Float4 &vec, float scalar) {
        const Float4 out = { vec.x * scalar, vec.y * scalar, vec.z * scalar, vec.w * scalar, };
        return out;
    }
    constexpr Float4 operator/(const Float4 &vec, float scalar) {
        const Float4 out = { vec.x / scalar, vec.y / scalar, vec.z / scalar, vec.w / scalar, };
        return out;
    }

	constexpr Vector2f CastVector(const Vector2d &vector) {
		const Vector2f out = { static_cast<float>(vector.x), static_cast<float>(vector.y) };
        return out;
	}
	constexpr Vector3f CastVector(const Vector3d &vector) {
		const Vector3f out = { static_cast<float>(vector.x), static_cast<float>(vector.y), static_cast<float>(vector.z), static_cast<float>(vector.array[3]) };
        return out;
	}
	constexpr Vector4f CastVector(const Vector4d &vector) {
		const Vector4f out = { static_cast<float>(vector.x), static_cast<float>(vector.y), static_cast<float>(vector.z), static_cast<float>(vector.w) };
        return out;
	}

    constexpr Vector2f ScalarMultiply(const Vector2f &vector, float scalar) {
        const Vector2f out = { .vector = vector.vector.f * scalar };
        return out;
    }

    constexpr Vector3f ScalarMultiply(const Vector3f &vector, float scalar) {
        const Vector3f out = { .vector = vector.vector.f * scalar };
        return out;
    }

    constexpr Vector4f ScalarMultiply(const Vector4f &vector, float scalar) {
        const Vector4f out = { .vector = vector.vector.f * scalar };
        return out;
    }

    constexpr float Magnitude(const Vector2f &vector) {
        const float squared_summation = vector.x * vector.x + vector.y * vector.y;
        const float out               = ::sqrtf(squared_summation);
        return out;
    }

    constexpr float Magnitude(const Vector3f &vector) {
        const float squared_summation = vector.x * vector.x + vector.y * vector.y + vector.z * vector.z;
        const float out               = ::sqrtf(squared_summation);
        return out;
    }

    constexpr float Magnitude(const Vector4f &vector) {
        const float squared_summation = vector.x * vector.x + vector.y * vector.y + vector.z * vector.z + vector.w * vector.w;
        const float out               = ::sqrtf(squared_summation);
        return out;
    }

    constexpr Vector2f Normalize(const Vector2f &vector) {
        
        Vector2f vec     = vector;  
        const float mag0 = Magnitude(vector);
        if (mag0 == cFloatInfinity) {
            vec              = ScalarMultiply(vec, 3.129822e-20f);
            const float mag1 = Magnitude(vec);
            vec              = ScalarMultiply(vec, mag1);
        } else if (cFloatEpsilon < mag0) {
            const float reciprocal_mag = 1.0f / mag0;
            vec                        = ScalarMultiply(vec, reciprocal_mag);
        } else {
            vec = {0.0f, 0.0f};
        }

        return vec;
    }

    constexpr Vector3f Normalize(const Vector3f &vector) {
        
        Vector3f vec     = vector;  
        const float mag0 = Magnitude(vector);
        if (mag0 == cFloatInfinity) {
            vec              = ScalarMultiply(vec, 3.129822e-20f);
            const float mag1 = Magnitude(vec);
            vec              = ScalarMultiply(vec, mag1);
        } else if (cFloatEpsilon < mag0) {
            const float reciprocal_mag = 1.0f / mag0;
            vec                        = ScalarMultiply(vec, reciprocal_mag);
        } else {
            vec = {0.0f, 0.0f, 0.0f, 0.0f};
        }

        return vec;
    }

    constexpr Vector4f Normalize(const Vector4f &vector) {
        
        Vector4f vec     = vector;  
        const float mag0 = Magnitude(vector);
        if (mag0 == cFloatInfinity) {
            vec              = ScalarMultiply(vec, 3.129822e-20f);
            const float mag1 = Magnitude(vec);
            vec              = ScalarMultiply(vec, mag1);
        } else if (cFloatEpsilon < mag0) {
            const float reciprocal_mag = 1.0f / mag0;
            vec                        = ScalarMultiply(vec, reciprocal_mag);
        } else {
            vec = {0.0f, 0.0f, 0.0f, 0.0f};
        }

        return vec;
    }

#ifdef VP_TARGET_ARCHITECTURE_x86
    constexpr Vector3f Cross(const Vector3f &left, const Vector3f &right) {
        const v128f a { .mmf = avx2::shufps(left.vector, left.vector, avx2::ShuffleToOrder(1,2,0,3)) };
        const v128f b { .mmf = avx2::shufps(right.vector, right.vector, avx2::ShuffleToOrder(2,0,1,3)) };
        const v128f c { .mmf = avx2::mulps(a, right.vector) };
        const v128f d { .mmf = avx2::mulps(a, b) };
        const v128f e { .mmf = avx2::shufps(c, c, avx2::ShuffleToOrder(1, 2, 0, 3)) };
        return Vector3f{ .vector = avx2::subps(d, e) };
    }
#else
    constexpr Vector3f Cross(const Vector3f &left, const Vector3f &right) {
        const float out_x = (left.y * right.z) - (left.z * right.y);
        const float out_y = -((left.x * right.z) - (left.z * right.x));
        const float out_z = (left.x * right.y) - (left.y * right.x);
        return Vector3f{ out_x, out_y, out_z };
    }
#endif
    constexpr float Dot(const Vector3f &left, const Vector3f &right) {
        const Vector3f temp = { .vector = left.vector.f * right.vector.f };
        return temp.x + temp.y + temp.z;
    }

    constexpr void RotateVectorAxisX(Vector3f *out_vector, const Vector3f &rot_vector, float angle) {
        const float index = angle * (static_cast<float>(cAngleIndexHalfRound) / cFloatPi);
        const float sin = SampleSin(index);
        const float cos = SampleCos(index);
        /*  | x                       |
         *  | y*sin(ang) - z*cos(ang) |
         *  | y*cos(ang) + z*sin(ang) |
         */ 
        out_vector->x = rot_vector.x;
        out_vector->y = (rot_vector.y * sin) - (cos * rot_vector.x);
        out_vector->x = (rot_vector.y * cos) + (sin * rot_vector.x);
    }

    constexpr void RotateVectorAxisY(Vector3f *out_vector, const Vector3f &rot_vector, float angle) {
        const float index = angle * (static_cast<float>(cAngleIndexHalfRound) / cFloatPi);
        const float sin = SampleSin(index);
        const float cos = SampleCos(index);
        /*  | z*cos(ang) + x*sin(ang) |
         *  | y                       |
         *  | z*sin(ang) - x*cos(ang) |
         */ 
        out_vector->x = (rot_vector.x * cos) + (sin * rot_vector.x);
        out_vector->y = rot_vector.y;
        out_vector->x = (rot_vector.x * sin) - (cos * rot_vector.x);
    }

    constexpr void RotateVectorAxisZ(Vector3f *out_vector, const Vector3f &rot_vector, float angle) {
        const float index = angle * (static_cast<float>(cAngleIndexHalfRound) / cFloatPi);
        const float sin = SampleSin(index);
        const float cos = SampleCos(index);
        /*  | x*sin(ang) - y*cos(ang) |
         *  | x*cos(ang) + y*sin(ang) |
         *  | z                       |
         */ 
        out_vector->x = (rot_vector.x * sin) + (cos * rot_vector.x);
        out_vector->y = rot_vector.y;
        out_vector->x = (rot_vector.x * cos) - (sin * rot_vector.x);
    }

    constexpr void Parallelize(Vector3f *out_vector, const Vector3f &base, const Vector3f &to_parallelize) {
        const float dot = Dot(base, to_parallelize);
        out_vector->x = to_parallelize.x * dot;
        out_vector->y = to_parallelize.y * dot;
        out_vector->x = to_parallelize.x * dot;
    }

#ifdef VP_TARGET_ARCHITECTURE_x86

    constexpr void MatrixMultiply(MatrixRowMajor4x4f *out_matrix, const MatrixRowMajor4x4f *lhs_matrix, const MatrixRowMajor4x3f *rhs_matrix) {
        const v128f r_row0 = { .mmf = rhs_matrix->array_row[0].vector.f };
        const v128f r_row1 = { .mmf = rhs_matrix->array_row[1].vector.f };
        const v128f r_row2 = { .mmf = rhs_matrix->array_row[2].vector.f };
        const v128f clear = { .mmf = {0.0f, 0.0f, 0.0f, 0.0f,} };
    
        const auto row_lambda = [=](const v128f &row) {
            const v128f row0_brod0 = { .mmf = avx2::vbroadcastss128(std::addressof(row.f[0])), };
            const v128f row0_mul0  = { .mmf = avx2::mulps(row0_brod0, r_row0), };
            const v128f row0_brod1 = { .mmf = avx2::vbroadcastss128(std::addressof(row.f[1])), };
            const v128f row0_fmul1 = { .mmf = avx2::fmaddps(row0_brod1, r_row1, row0_mul0), };
            const v128f row0_brod2 = { .mmf = avx2::vbroadcastss128(std::addressof(row.f[2])), };
            const v128f row0_fmul2 = { .mmf = avx2::fmaddps(row0_brod2, r_row2, row0_fmul1), };
            const v128f row0_brod3 = { .mmf = avx2::vbroadcastss128(std::addressof(row.f[3])), };
            const v128f row0_fmul3 = { .mmf = avx2::fmaddps(row0_brod3, clear, row0_fmul2), };
            return row0_fmul3;
        };
    
        /* | dot({l_00,l_01,l_02,l_03}, {r_00,r_10,r_20,r_30}) ... dot({l_00,l_01,l_02,l_03}, {r_03,r_13,r_23,r_33}) | */
        const v128f row0_fmul3 = row_lambda(lhs_matrix->vector[0].down_vec[0]);
    
        /* | dot({l_10,l_11,l_12,l_13}, {r_00,r_10,r_20,r_30}) ... dot({l_10,l_11,l_12,l_13}, {r_03,r_13,r_23,r_33}) | */
        const v128f row1_fmul3 = row_lambda(lhs_matrix->vector[0].down_vec[1]);
    
        /* | dot({l_20,l_21,l_22,l_23}, {r_00,r_10,r_20,r_30}) ... dot({l_20,l_21,l_22,l_23}, {r_03,r_13,r_23,r_33}) | */
        const v128f row2_fmul3 = row_lambda(lhs_matrix->vector[1].down_vec[0]);
    
        /* | dot({l_30,l_31,l_32,l_33}, {r_00,r_10,r_20,r_30}) ... dot({l_30,l_31,l_32,l_33}, {r_03,r_13,r_23,r_33}) | */
        const v128f row3_fmul3 = row_lambda(lhs_matrix->vector[1].down_vec[1]);
    
        out_matrix->array_row[0].vector.f = row0_fmul3.f;
        out_matrix->array_row[1].vector.f = row1_fmul3.f;
        out_matrix->array_row[2].vector.f = row2_fmul3.f;
        out_matrix->array_row[3].vector.f = row3_fmul3.f;
    }

    constexpr void MatrixMultiply(MatrixRowMajor4x4f *out_matrix, const MatrixRowMajor4x4f *lhs_matrix, const MatrixRowMajor4x4f *rhs_matrix) {
        const v128f r_row0 = { .mmf = rhs_matrix->array_row[0].vector.f };
        const v128f r_row1 = { .mmf = rhs_matrix->array_row[1].vector.f };
        const v128f r_row2 = { .mmf = rhs_matrix->array_row[2].vector.f };
        const v128f r_row3 = { .mmf = rhs_matrix->array_row[3].vector.f };
    
        const auto row_lambda = [=](const v128f &row) {
            const v128f row0_brod0 = { .mmf = avx2::vbroadcastss128(std::addressof(row.f[0])), };
            const v128f row0_mul0  = { .mmf = avx2::mulps(row0_brod0, r_row0), };
            const v128f row0_brod1 = { .mmf = avx2::vbroadcastss128(std::addressof(row.f[1])), };
            const v128f row0_fmul1 = { .mmf = avx2::fmaddps(row0_brod1, r_row1, row0_mul0), };
            const v128f row0_brod2 = { .mmf = avx2::vbroadcastss128(std::addressof(row.f[2])), };
            const v128f row0_fmul2 = { .mmf = avx2::fmaddps(row0_brod2, r_row2, row0_fmul1), };
            const v128f row0_brod3 = { .mmf = avx2::vbroadcastss128(std::addressof(row.f[3])), };
            const v128f row0_fmul3 = { .mmf = avx2::fmaddps(row0_brod3, r_row3, row0_fmul2), };
            return row0_fmul3;
        };
    
        /* | dot({l_00,l_01,l_02,l_03}, {r_00,r_10,r_20,r_30}) ... dot({l_00,l_01,l_02,l_03}, {r_03,r_13,r_23,r_33}) | */
        const v128f row0_fmul3 = row_lambda(lhs_matrix->vector[0].down_vec[0]);
    
        /* | dot({l_10,l_11,l_12,l_13}, {r_00,r_10,r_20,r_30}) ... dot({l_10,l_11,l_12,l_13}, {r_03,r_13,r_23,r_33}) | */
        const v128f row1_fmul3 = row_lambda(lhs_matrix->vector[0].down_vec[1]);
    
        /* | dot({l_20,l_21,l_22,l_23}, {r_00,r_10,r_20,r_30}) ... dot({l_20,l_21,l_22,l_23}, {r_03,r_13,r_23,r_33}) | */
        const v128f row2_fmul3 = row_lambda(lhs_matrix->vector[1].down_vec[0]);
    
        /* | dot({l_30,l_31,l_32,l_33}, {r_00,r_10,r_20,r_30}) ... dot({l_30,l_31,l_32,l_33}, {r_03,r_13,r_23,r_33}) | */
        const v128f row3_fmul3 = row_lambda(lhs_matrix->vector[1].down_vec[1]);
    
        out_matrix->array_row[0].vector.f = row0_fmul3.f;
        out_matrix->array_row[1].vector.f = row1_fmul3.f;
        out_matrix->array_row[2].vector.f = row2_fmul3.f;
        out_matrix->array_row[3].vector.f = row3_fmul3.f;
    }
#endif

    constexpr void ConvertQuaternionToMatrix(MatrixRowMajor4x3f *out_matrix, const Quaternionf &quaternion) {
        out_matrix->array_row[3].vector = v128f{ .f = { 0.0f, 0.0f, 0.0f, 0.0f, }, };
        const float x = quaternion.x;
        const float y = quaternion.y;
        const float z = quaternion.z;
        const float w = quaternion.w;
        const float mag = 2.0f / (x * x + y * y + z * z + w * w);
        const float m_z = mag * z;
        const float m_y = mag * y;
        const float m_xw = mag * x * w;
        const float m_xx = mag * x * x;
        /* | 1 - 2y^2 - 2z^2, 2xy - 2zw,       2xz + 2yw,       0 |
         * | 2xy + 2zw,       1 - 2x^2 - 2z^2, 2yz - 2xw,       0 |
         * | 2xz - 2yw,       2yz - 2xw,       1 - 2x^2 - 2y^2, 0 |
         * | 0,               0,               0,               0 |
         */
        const Vector4f row0 = { 1.0f - (m_y * y + m_z * z), m_y * x + m_z * w,       m_z * x - m_y * w,       0.0f,};
        const Vector4f row1 = { m_y * x - m_z * w,          1.0f - (m_xx + m_z * z), m_z * y + m_xw,          0.0f,};
        const Vector4f row2 = { m_z * x + m_y * w,          m_z * y - m_xw,          1.0f - (m_xx + m_y * y), 0.0f,};
        out_matrix->array_row[0].vector = row0.vector;
        out_matrix->array_row[1].vector = row1.vector;
        out_matrix->array_row[2].vector = row2.vector;
        return;
    }

    //constexpr void ConvertMatrixToQuaternion(Quaternionf *out_quat, const MatrixRowMajor4x3f *matrix) {
    //    return;
    //}

    constexpr void RotateLocalX(MatrixRowMajor4x3f *out_rot_matrix, float theta) {
        const float index = theta * (static_cast<float>(cAngleIndexHalfRound) / cFloatPi);
        const float sin = SampleSin(index);
        const float cos = SampleCos(index);
        const float m10 = out_rot_matrix->array_2d[1][0];
        const float m11 = out_rot_matrix->array_2d[1][1];
        const float m12 = out_rot_matrix->array_2d[1][2];
        out_rot_matrix->array_2d[1][0] = (m10 * cos)                           + (out_rot_matrix->array_2d[2][0] * sin);
        out_rot_matrix->array_2d[2][0] = (out_rot_matrix->array_2d[2][0] * cos) - (m10 * sin);
        out_rot_matrix->array_2d[1][1] = (m11 * cos)                           + (out_rot_matrix->array_2d[2][1] * sin);
        out_rot_matrix->array_2d[2][1] = (out_rot_matrix->array_2d[2][1] * cos) - (m11 * sin);
        out_rot_matrix->array_2d[1][2] = (m12 * cos)                           + (out_rot_matrix->array_2d[2][2] * sin);
        out_rot_matrix->array_2d[2][2] = (out_rot_matrix->array_2d[2][2] * cos) - (m12 * sin);
    }

    constexpr void RotateLocalY(MatrixRowMajor4x3f *out_rot_matrix, float theta) {
        const float index = theta * (static_cast<float>(cAngleIndexHalfRound) / cFloatPi);
        const float sin = SampleSin(index);
        const float cos = SampleCos(index);
        const float m00 = out_rot_matrix->array_2d[0][0];
        const float m01 = out_rot_matrix->array_2d[0][1];
        const float m02 = out_rot_matrix->array_2d[0][2];
        out_rot_matrix->array_2d[0][0] = (m00 * cos)                            - (out_rot_matrix->array_2d[2][0] * sin);
        out_rot_matrix->array_2d[2][0] = (out_rot_matrix->array_2d[2][0] * cos) + (m00 * sin);
        out_rot_matrix->array_2d[0][1] = (m01 * cos)                            - (out_rot_matrix->array_2d[2][1] * sin);
        out_rot_matrix->array_2d[2][1] = (out_rot_matrix->array_2d[2][1] * cos) + (m01 * sin);
        out_rot_matrix->array_2d[0][2] = (m02 * cos)                            - (out_rot_matrix->array_2d[2][2] * sin);
        out_rot_matrix->array_2d[2][2] = (out_rot_matrix->array_2d[2][2] * cos) + (m02 * sin);
    }

    constexpr void RotateLocalZ(MatrixRowMajor4x3f *out_rot_matrix, float theta) {
        const float index = theta * (static_cast<float>(cAngleIndexHalfRound) / cFloatPi);
        const float sin = SampleSin(index);
        const float cos = SampleCos(index);
        const float m00 = out_rot_matrix->array_2d[0][0];
        const float m01 = out_rot_matrix->array_2d[0][1];
        const float m02 = out_rot_matrix->array_2d[0][2];
        out_rot_matrix->array_2d[0][0] = (m00 * cos)                           + (out_rot_matrix->array_2d[1][0] * sin);
        out_rot_matrix->array_2d[1][0] = (out_rot_matrix->array_2d[1][0] * cos) - (m00 * sin);
        out_rot_matrix->array_2d[0][1] = (m01 * cos)                            + (out_rot_matrix->array_2d[1][1] * sin);
        out_rot_matrix->array_2d[1][1] = (out_rot_matrix->array_2d[1][1] * cos) - (m01 * sin);
        out_rot_matrix->array_2d[0][2] = (m02 * cos)                            + (out_rot_matrix->array_2d[1][2] * sin);
        out_rot_matrix->array_2d[1][2] = (out_rot_matrix->array_2d[1][2] * cos) - (m02 * sin);
    }
}
