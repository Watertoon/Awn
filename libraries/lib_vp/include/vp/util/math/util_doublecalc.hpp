#pragma once

namespace vp::util {

#ifdef VP_TARGET_ARCHITECTURE_x86
    //constexpr void Transpose(MatrixRowMajor4x4d *out_matrix, const MatrixRowMajor4x4d &in_matrix) {
    //    const v256f row0 = in_matrix.array_row[0].vector;
    //    const v256f row1 = in_matrix.array_row[1].vector;
    //    const v256f row2 = in_matrix.array_row[2].vector;
    //    const v256f row3 = in_matrix.array_row[3].vector;
    //
    //    /* _               _       _               _
    //    * | m00 m01 m02 m03 |    | m00 m10 m02 m12 |
    //    * | m10 m11 m12 m13 |    | m01 m11 m03 m13 |
    //    * | m20 m21 m22 m23 | -> | m20 m30 m22 m32 |
    //    * | m30 m31 m32 m33 |    | m21 m31 m23 m33 |
    //    *  _               _      _               _
    //    */
    //    const v256f shuf0 = avx2::shufpd(row0, row1, avx2::GetShuffleDouble256Mask(0,0,0,0));
    //    const v256f shuf1 = avx2::shufpd(row0, row1, avx2::GetShuffleDouble256Mask(1,1,1,1));
    //    const v256f shuf2 = avx2::shufpd(row2, row3, avx2::GetShuffleDouble256Mask(0,0,0,0));
    //    const v256f shuf3 = avx2::shufpd(row2, row3, avx2::GetShuffleDouble256Mask(1,1,1,1));
    //
    //    /* _               _       _               _
    //    * | m00 m10 m02 m12 |    | m00 m10 m20 m30 |
    //    * | m01 m11 m03 m13 |    | m01 m11 m21 m31 |
    //    * | m20 m30 m22 m32 | -> | m02 m12 m22 m32 |
    //    * | m21 m31 m23 m33 |    | m03 m13 m23 m33 |
    //    *  _               _      _               _
    //    */
    //    const v256f perm0 = avx2::perm2f128pd(shuf0, shuf2, avx2::GetPermute2f128Mask(0,2));
    //    const v256f perm1 = avx2::perm2f128pd(shuf1, shuf3, avx2::GetPermute2f128Mask(0,2));
    //    const v256f perm2 = avx2::perm2f128pd(shuf0, shuf2, avx2::GetPermute2f128Mask(1,3));
    //    const v256f perm3 = avx2::perm2f128pd(shuf1, shuf3, avx2::GetPermute2f128Mask(1,3));
    //
    //    out_matrix->array_row[0].vector.d = perm0.d;
    //    out_matrix->array_row[1].vector.d = perm1.d;
    //    out_matrix->array_row[2].vector.d = perm2.d;
    //    out_matrix->array_row[3].vector.d = perm3.d;
    //}
#endif
}
