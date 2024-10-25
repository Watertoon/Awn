#pragma once

namespace vp::util {

    constexpr Vector3d operator+(const Vector3d &left, const Vector3d &right) {
        const Vector3d out = { .vector = { .d = left.vector.d + right.vector.d }, };
        return out;
    }
    constexpr Vector3d operator-(const Vector3d &left, const Vector3d &right) {
        const Vector3d out = { .vector = { .d = left.vector.d - right.vector.d }, };
        return out;
    }

	constexpr Vector2d CastVector(const Vector2f &vector) {
		const Vector2d out = { static_cast<double>(vector.x), static_cast<double>(vector.y) };
        return out;
	}
	constexpr Vector3d CastVector(const Vector3f &vector) {
		const Vector3d out = { static_cast<double>(vector.x), static_cast<double>(vector.y), static_cast<double>(vector.z), static_cast<double>(vector.array[3]) };
        return out;
	}
	constexpr Vector4d CastVector(const Vector4f &vector) {
		const Vector4d out = { static_cast<double>(vector.x), static_cast<double>(vector.y), static_cast<double>(vector.z), static_cast<double>(vector.w) };
        return out;
	}

#ifdef VP_TARGET_ARCHITECTURE_x86
    constexpr void MatrixMultiply(MatrixRowMajor4x4d *out_matrix, const MatrixRowMajor4x4d *lhs_matrix, const MatrixRowMajor4x4d *rhs_matrix) {
        const v256f r_row0 = { .mmd = rhs_matrix->array_row[0].vector.d };
        const v256f r_row1 = { .mmd = rhs_matrix->array_row[1].vector.d };
        const v256f r_row2 = { .mmd = rhs_matrix->array_row[2].vector.d };
        const v256f r_row3 = { .mmd = rhs_matrix->array_row[3].vector.d };

        const auto row_lambda = [=] ALWAYS_INLINE_LAMBDA (const v256f &row) {
            const v256f row0_brod0 = { .mmd = avx2::vbroadcastsd256(std::addressof(row.d[0])), };
            const v256f row0_mul0  = { .mmd = avx2::mulpd(row0_brod0, r_row0), };
            const v256f row0_brod1 = { .mmd = avx2::vbroadcastsd256(std::addressof(row.d[1])), };
            const v256f row0_fmul1 = { .mmd = avx2::fmaddpd(row0_brod1, r_row1, row0_mul0), };
            const v256f row0_brod2 = { .mmd = avx2::vbroadcastsd256(std::addressof(row.d[2])), };
            const v256f row0_fmul2 = { .mmd = avx2::fmaddpd(row0_brod2, r_row2, row0_fmul1), };
            const v256f row0_brod3 = { .mmd = avx2::vbroadcastsd256(std::addressof(row.d[3])), };
            const v256f row0_fmul3 = { .mmd = avx2::fmaddpd(row0_brod3, r_row3, row0_fmul2), };
            return row0_fmul3;
        };
        /* | dot({l_00,l_01,l_02,l_03}, {r_00,r_10,r_20,r_30}) ... dot({l_00,l_01,l_02,l_03}, {r_03,r_13,r_23,r_33}) | */
        const v256f row0_fmul3 = row_lambda(lhs_matrix->vector[0]);

        /* | dot({l_10,l_11,l_12,l_13}, {r_00,r_10,r_20,r_30}) ... dot({l_10,l_11,l_12,l_13}, {r_03,r_13,r_23,r_33}) | */
        const v256f row1_fmul3 = row_lambda(lhs_matrix->vector[1]);

        /* | dot({l_20,l_21,l_22,l_23}, {r_00,r_10,r_20,r_30}) ... dot({l_20,l_21,l_22,l_23}, {r_03,r_13,r_23,r_33}) | */
        const v256f row2_fmul3 = row_lambda(lhs_matrix->vector[2]);

        /* | dot({l_30,l_31,l_32,l_33}, {r_00,r_10,r_20,r_30}) ... dot({l_30,l_31,l_32,l_33}, {r_03,r_13,r_23,r_33}) | */
        const v256f row3_fmul3 = row_lambda(lhs_matrix->vector[3]);

        out_matrix->array_row[0].vector.d = row0_fmul3.d;
        out_matrix->array_row[1].vector.d = row1_fmul3.d;
        out_matrix->array_row[2].vector.d = row2_fmul3.d;
        out_matrix->array_row[3].vector.d = row3_fmul3.d;
    }

    constexpr void Transpose(MatrixRowMajor4x4d *out_matrix, const MatrixRowMajor4x4d &in_matrix) {
        const v256f row0 = { .mmd = in_matrix.array_row[0].vector.d };
        const v256f row1 = { .mmd = in_matrix.array_row[1].vector.d };
        const v256f row2 = { .mmd = in_matrix.array_row[2].vector.d };
        const v256f row3 = { .mmd = in_matrix.array_row[3].vector.d };

        /* _               _       _               _
        * | m00 m01 m02 m03 |    | m00 m10 m02 m12 |
        * | m10 m11 m12 m13 |    | m01 m11 m03 m13 |
        * | m20 m21 m22 m23 | -> | m20 m30 m22 m32 |
        * | m30 m31 m32 m33 |    | m21 m31 m23 m33 |
        *  _               _      _               _
        */
        const v256f shuf0 = { .mmd = avx2::shufpd(row0, row1, avx2::GetShuffleDouble256Mask(0,0,0,0)), };
        const v256f shuf1 = { .mmd = avx2::shufpd(row0, row1, avx2::GetShuffleDouble256Mask(1,1,1,1)), };
        const v256f shuf2 = { .mmd = avx2::shufpd(row2, row3, avx2::GetShuffleDouble256Mask(0,0,0,0)), };
        const v256f shuf3 = { .mmd = avx2::shufpd(row2, row3, avx2::GetShuffleDouble256Mask(1,1,1,1)), };

        /* _               _       _               _
        * | m00 m10 m02 m12 |    | m00 m10 m20 m30 |
        * | m01 m11 m03 m13 |    | m01 m11 m21 m31 |
        * | m20 m30 m22 m32 | -> | m02 m12 m22 m32 |
        * | m21 m31 m23 m33 |    | m03 m13 m23 m33 |
        *  _               _      _               _
        */
        const v256f perm0 = { .mmd = avx2::perm2f128pd(shuf0, shuf2, avx2::GetPermute2f128Mask(0,2)), };
        const v256f perm1 = { .mmd = avx2::perm2f128pd(shuf1, shuf3, avx2::GetPermute2f128Mask(0,2)), };
        const v256f perm2 = { .mmd = avx2::perm2f128pd(shuf0, shuf2, avx2::GetPermute2f128Mask(1,3)), };
        const v256f perm3 = { .mmd = avx2::perm2f128pd(shuf1, shuf3, avx2::GetPermute2f128Mask(1,3)), };

        out_matrix->array_row[0].vector.d = perm0.d;
        out_matrix->array_row[1].vector.d = perm1.d;
        out_matrix->array_row[2].vector.d = perm2.d;
        out_matrix->array_row[3].vector.d = perm3.d;
    }
#endif
}
