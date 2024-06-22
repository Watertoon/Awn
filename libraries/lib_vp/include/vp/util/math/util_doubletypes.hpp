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

    struct Double2 {
        union {
            struct {
                double x;
                double y;
            };
            double array[2];
        };

        using VectorType                    = double;
        static constexpr bool cIsVectorType = true;
        static constexpr bool cIsVectorized = false;
        static constexpr bool cVectorCount  = 2;

        static const Double2 cUnitX;
        static const Double2 cUnitY;
        static const Double2 cZero;
        static const Double2 cOne;
    };
    static_assert(sizeof(Double2) == 0x10);
    static_assert(alignof(Double2) == 0x8);

    struct Double3 {
        union {
            struct {
                double x;
                double y;
                double z;
            };
            double array[3];
        };

        using VectorType                    = double;
        static constexpr bool cIsVectorType = true;
        static constexpr bool cIsVectorized = false;
        static constexpr bool cVectorCount  = 3;

        static const Double3 cUnitX;
        static const Double3 cUnitY;
        static const Double3 cUnitZ;
        static const Double3 cZero;
        static const Double3 cOne;
    };
    static_assert(sizeof(Double3) == 0x18);
    static_assert(alignof(Double3) == 0x8);

    struct Double4 {
        union {
            struct {
                double x;
                double y;
                double z;
                double w;
            };
            double array[4];
        };

        using VectorType                    = double;
        static constexpr bool cIsVectorType = true;
        static constexpr bool cIsVectorized = false;
        static constexpr bool cVectorCount  = 4;

        static const Double4 cUnitX;
        static const Double4 cUnitY;
        static const Double4 cUnitZ;
        static const Double4 cUnitW;
        static const Double4 cZero;
        static const Double4 cOne;
    };
    static_assert(sizeof(Double4) == 0x20);
    static_assert(alignof(Double4) == 0x8);

    struct DoubleRowMajor3x3 {
        union {
            struct {
                double m00;
                double m01;
                double m02;
                double m10;
                double m11;
                double m12;
                double m20;
                double m21;
                double m22;
            };
            Double3 array_row[3];
            double  array_2d[3][3];
        };
    };
    static_assert(sizeof(DoubleRowMajor3x3) == 0x48);
    static_assert(alignof(DoubleRowMajor3x3) == 0x8);

    struct DoubleRowMajor4x3 {
        union {
            struct {
                double m00;
                double m01;
                double m02;
                double m10;
                double m11;
                double m12;
                double m20;
                double m21;
                double m22;
                double m30;
                double m31;
                double m32;
            };
            Double3 array_row[4];
            double  array_2d[4][3];
        };
    };
    static_assert(sizeof(DoubleRowMajor4x3) == 0x60);
    static_assert(alignof(DoubleRowMajor4x3) == 0x8);

    struct DoubleRowMajor4x4 {
        union {
            struct {
                double m00;
                double m01;
                double m02;
                double m03;
                double m10;
                double m11;
                double m12;
                double m13;
                double m20;
                double m21;
                double m22;
                double m23;
                double m30;
                double m31;
                double m32;
                double m33;
            };
            Double4 array_row[4];
            double  array_2d[4][4];
        };
    };
    static_assert(sizeof(DoubleRowMajor4x4) == 0x80);
    static_assert(alignof(DoubleRowMajor4x4) == 0x8);

    struct DoubleColumnMajor3x3 {
        union {
            struct {
                double m00;
                double m10;
                double m20;
                double m01;
                double m11;
                double m21;
                double m02;
                double m12;
                double m22;
            };
            Double3 array_col[3];
            double  array_2d[3][3];
        };
    };
    static_assert(sizeof(DoubleColumnMajor3x3) == 0x48);
    static_assert(alignof(DoubleColumnMajor3x3) == 0x8);

    struct DoubleColumnMajor4x3 {
        union {
            struct {
                double m00;
                double m10;
                double m20;
                double m30;
                double m01;
                double m11;
                double m21;
                double m31;
                double m02;
                double m12;
                double m22;
                double m32;
            };
            Double4 array_col[3];
            double  array_2d[3][4];
        };
    };
    static_assert(sizeof(DoubleColumnMajor4x3) == 0x60);
    static_assert(alignof(DoubleColumnMajor4x3) == 0x8);

    struct DoubleColumnMajor4x4 {
        union {
            struct {
                double m00;
                double m10;
                double m20;
                double m30;
                double m01;
                double m11;
                double m21;
                double m31;
                double m02;
                double m12;
                double m22;
                double m32;
                double m03;
                double m13;
                double m23;
                double m33;
            };
            Double4 array_col[4];
            double  array_2d[4][4];
        };
    };
    static_assert(sizeof(DoubleColumnMajor4x4) == 0x80);
    static_assert(alignof(DoubleColumnMajor4x4) == 0x8);

    using Double2Vector  = v128f;
    using Double3Vector  = v256f;
    using Double4Vector  = v256f;
    using Matrix33dVector = v256f[4];
    using Matrix34dVector = v256f[4];
    using Matrix44dVector = v256f[4];

    struct Vector2d {
        union {
            struct {
                double x;
                double y;
            };
            double array[2];
            Double2Vector vector;
        };

        using VectorType                    = double;
        static constexpr bool cIsVectorType = true;
        static constexpr bool cIsVectorized = true;
        static constexpr bool cVectorCount  = 2;

        static const Vector2d cUnitX;
        static const Vector2d cUnitY;
        static const Vector2d cZero;
        static const Vector2d cOne;
    };
    static_assert(sizeof(Vector2d) == 0x10);

    struct Vector3d {
        union {
            struct {
                double x;
                double y;
                double z;
                double w;
            };
            double array[4];
            Double3Vector vector;
        };

        using VectorType                    = double;
        static constexpr bool cIsVectorType = true;
        static constexpr bool cIsVectorized = true;
        static constexpr bool cVectorCount  = 3;


        static const Vector3d cUnitX;
        static const Vector3d cUnitY;
        static const Vector3d cUnitZ;
        static const Vector3d cZero;
        static const Vector3d cOne;
    };
    static_assert(sizeof(Vector3d) == 0x20);

    struct Vector4d {
        union {
            struct {
                double x;
                double y;
                double z;
                double w;
            };
            double array[4];
            Double4Vector vector;
        };

        using VectorType                    = double;
        static constexpr bool cIsVectorType = true;
        static constexpr bool cIsVectorized = true;
        static constexpr bool cVectorCount  = 4;


        static const Vector4d cUnitX;
        static const Vector4d cUnitY;
        static const Vector4d cUnitZ;
        static const Vector4d cUnitW;
        static const Vector4d cZero;
        static const Vector4d cOne;
    };
    static_assert(sizeof(Vector4d) == 0x20);

    struct MatrixRowMajor4x3d {
        union {
            struct {
                double m00;
                double m01;
                double m02;
                double reserve0;
                double m10;
                double m11;
                double m12;
                double reserve1;
                double m20;
                double m21;
                double m22;
                double reserve2;
                double m30;
                double m31;
                double m32;
                double reserve3;
            };
            Matrix34dVector vector;
            Vector4d        array_row[4];
            double          array_2d[4][4];
        };

        static const MatrixRowMajor4x3d cIdentity;
    };
    static_assert(sizeof(MatrixRowMajor4x3d) == 0x80);

    struct MatrixRowMajor4x4d {
        union {
            struct {
                double m00;
                double m01;
                double m02;
                double m03;
                double m10;
                double m11;
                double m12;
                double m13;
                double m20;
                double m21;
                double m22;
                double m23;
                double m30;
                double m31;
                double m32;
                double m33;
            };
            Matrix44dVector vector;
            Vector4d        array_row[4];
            double          array_2d[4][4];
        };

        static const MatrixRowMajor4x4d cIdentity;
    };
    static_assert(sizeof(MatrixRowMajor4x4d) == 0x80);

    struct MatrixColumnMajor4x3d {
        union {
            struct {
                double m00;
                double m10;
                double m20;
                double m30;
                double m01;
                double m11;
                double m21;
                double m31;
                double m02;
                double m12;
                double m22;
                double m32;
            };
            Matrix34dVector vector;
            Vector4d        array_col[3];
            double          array_2d[3][4];
        };

        static const MatrixColumnMajor4x3d cIdentity;
    };
    static_assert(sizeof(MatrixColumnMajor4x3d) == 0x80);

    struct MatrixColumnMajor4x4d {
        union {
            struct {
                double m00;
                double m10;
                double m20;
                double m30;
                double m01;
                double m11;
                double m21;
                double m31;
                double m02;
                double m12;
                double m22;
                double m32;
                double m03;
                double m13;
                double m23;
                double m33;
            };
            Matrix44dVector vector;
            Vector4d        array_col[4];
            double          array_2d[4][4];
        };

        static const MatrixColumnMajor4x4d cIdentity;
    };
    static_assert(sizeof(MatrixColumnMajor4x4d) == 0x80);

    constexpr const Double2 Double2::cUnitX = { .x = 1.0f };
    constexpr const Double2 Double2::cUnitY = { .y = 1.0f };
    constexpr const Double2 Double2::cZero  = {};
    constexpr const Double2 Double2::cOne   = { 1.0f, 1.0f };

    constexpr const Double3 Double3::cUnitX = { .x = 1.0f };
    constexpr const Double3 Double3::cUnitY = { .y = 1.0f };
    constexpr const Double3 Double3::cUnitZ = { .z = 1.0f };
    constexpr const Double3 Double3::cZero  = {};
    constexpr const Double3 Double3::cOne   = { 1.0f, 1.0f, 1.0f };

    constexpr const Double4 Double4::cUnitX = { .x = 1.0f };
    constexpr const Double4 Double4::cUnitY = { .y = 1.0f };
    constexpr const Double4 Double4::cUnitZ = { .z = 1.0f };
    constexpr const Double4 Double4::cUnitW = { .w = 1.0f };
    constexpr const Double4 Double4::cZero  = {};
    constexpr const Double4 Double4::cOne   = { 1.0f, 1.0f, 1.0f, 1.0f };

    constexpr const Vector2d Vector2d::cUnitX = { .x = 1.0f };
    constexpr const Vector2d Vector2d::cUnitY = { .y = 1.0f };
    constexpr const Vector2d Vector2d::cZero  = {};
    constexpr const Vector2d Vector2d::cOne   = { 1.0f, 1.0f };

    constexpr const Vector3d Vector3d::cUnitX = { .x = 1.0f };
    constexpr const Vector3d Vector3d::cUnitY = { .y = 1.0f };
    constexpr const Vector3d Vector3d::cUnitZ = { .z = 1.0f };
    constexpr const Vector3d Vector3d::cZero  = {};
    constexpr const Vector3d Vector3d::cOne   = { 1.0f, 1.0f, 1.0f };

    constexpr const Vector4d Vector4d::cUnitX = { .x = 1.0f };
    constexpr const Vector4d Vector4d::cUnitY = { .y = 1.0f };
    constexpr const Vector4d Vector4d::cUnitZ = { .z = 1.0f };
    constexpr const Vector4d Vector4d::cUnitW = { .w = 1.0f };
    constexpr const Vector4d Vector4d::cZero  = {};
    constexpr const Vector4d Vector4d::cOne   = { 1.0f, 1.0f, 1.0f, 1.0f };

    constexpr const MatrixRowMajor4x3d MatrixRowMajor4x3d::cIdentity = {
        .m00 = 1.0f,
        .m10 = 1.0f,
        .m20 = 1.0f,
        .m30 = 1.0f,
    };
    constexpr const MatrixRowMajor4x4d MatrixRowMajor4x4d::cIdentity = {
        .m00 = 1.0f,
        .m10 = 1.0f,
        .m20 = 1.0f,
        .m30 = 1.0f,
    };
    constexpr const MatrixColumnMajor4x3d MatrixColumnMajor4x3d::cIdentity = {
        .m00 = 1.0f,
        .m10 = 1.0f,
        .m20 = 1.0f,
        .m30 = 1.0f,
    };
    constexpr const MatrixColumnMajor4x4d MatrixColumnMajor4x4d::cIdentity = {
        .m00 = 1.0f,
        .m10 = 1.0f,
        .m20 = 1.0f,
        .m30 = 1.0f,
    };

    constexpr Vector2d ConvertVector(const Double2 &in_vector) {
        const double x = in_vector.x;
        const double y = in_vector.y;
        return { x, y, };
    }
    constexpr Vector3d ConvertVector(const Double3 &in_vector) {
        const double x = in_vector.x;
        const double y = in_vector.y;
        const double z = in_vector.z;
        return { x, y, z, };
    }
    constexpr Vector4d ConvertVector(const Double4 &in_vector) {
        const double x = in_vector.x;
        const double y = in_vector.y;
        const double z = in_vector.z;
        const double w = in_vector.w;
        return { x, y, z, w, };
    }
    constexpr Vector4d ConvertVector(const Vector3d &in_vector) {
        const double x = in_vector.x;
        const double y = in_vector.y;
        const double z = in_vector.z;
        const double w = 0.0f;
        return { x, y, z, w, };
    }
    constexpr Vector4d ConvertVectorUnit(const Vector3d &in_vector) {
        const double x = in_vector.x;
        const double y = in_vector.y;
        const double z = in_vector.z;
        const double w = 1.0f;
        return { x, y, z, w, };
    }

    constexpr void ConvertMatrix(MatrixRowMajor4x3d *out_matrix, const DoubleRowMajor3x3 *in_matrix) {

        const v256i mask = avx2::GetLoadMaskDouble256(true, true, true, false);
        const v256f a = { .mmd = avx2::maskmovpd256(std::addressof(in_matrix->array_2d[0][0]), mask), };
        const v256f b = { .mmd = avx2::maskmovpd256(std::addressof(in_matrix->array_2d[1][0]), mask), };
        const v256f c = { .mmd = avx2::maskmovpd256(std::addressof(in_matrix->array_2d[2][0]), mask), };

        out_matrix->array_row[0].vector.d = a.d;
        out_matrix->array_row[1].vector.d = b.d;
        out_matrix->array_row[2].vector.d = c.d;
        out_matrix->array_row[3]          = Vector4d::cZero;
    }

    constexpr void ConvertMatrix(MatrixRowMajor4x3d *out_matrix, const DoubleRowMajor4x3 *in_matrix) {

        const v256i mask = avx2::GetLoadMaskDouble256(true, true, true, false);
        const v256f a = { .mmd = avx2::maskmovpd256(std::addressof(in_matrix->array_2d[0][0]), mask), };
        const v256f b = { .mmd = avx2::maskmovpd256(std::addressof(in_matrix->array_2d[1][0]), mask), };
        const v256f c = { .mmd = avx2::maskmovpd256(std::addressof(in_matrix->array_2d[2][0]), mask), };
        const v256f d = { .mmd = avx2::maskmovpd256(std::addressof(in_matrix->array_2d[3][0]), mask), };

        out_matrix->array_row[0].vector.d = a.d;
        out_matrix->array_row[1].vector.d = b.d;
        out_matrix->array_row[2].vector.d = c.d;
        out_matrix->array_row[3].vector.d = d.d;
    }

    constexpr void ConvertMatrix(MatrixRowMajor4x4d *out_matrix, const DoubleRowMajor3x3 *in_matrix) {

        const v256i mask = avx2::GetLoadMaskDouble256(true, true, true, false);
        const v256f a = { .mmd = avx2::maskmovpd256(std::addressof(in_matrix->array_2d[0][0]), mask), };
        const v256f b = { .mmd = avx2::maskmovpd256(std::addressof(in_matrix->array_2d[1][0]), mask), };
        const v256f c = { .mmd = avx2::maskmovpd256(std::addressof(in_matrix->array_2d[2][0]), mask), };

        out_matrix->array_row[0].vector.d = a.d;
        out_matrix->array_row[1].vector.d = b.d;
        out_matrix->array_row[2].vector.d = c.d;
        out_matrix->array_row[3]          = Vector4d::cZero;
    }

    constexpr void ConvertMatrix(MatrixRowMajor4x4d *out_matrix, const DoubleRowMajor4x3 *in_matrix) {

        const v256i mask = avx2::GetLoadMaskDouble256(true, true, true, false);
        const v256f a = { .mmd = avx2::maskmovpd256(std::addressof(in_matrix->array_2d[0][0]), mask), };
        const v256f b = { .mmd = avx2::maskmovpd256(std::addressof(in_matrix->array_2d[1][0]), mask), };
        const v256f c = { .mmd = avx2::maskmovpd256(std::addressof(in_matrix->array_2d[2][0]), mask), };
        const v256f d = { .mmd = avx2::maskmovpd256(std::addressof(in_matrix->array_2d[3][0]), mask), };

        out_matrix->array_row[0].vector.d = a.d;
        out_matrix->array_row[1].vector.d = b.d;
        out_matrix->array_row[2].vector.d = c.d;
        out_matrix->array_row[3].vector.d = d.d;
    }

    constexpr void ConvertMatrix(MatrixRowMajor4x4d *out_matrix, const DoubleRowMajor4x4 *in_matrix) {
        out_matrix->array_row[0].vector.d = avx2::movupd256(std::addressof(in_matrix->array_2d[0][0]));
        out_matrix->array_row[1].vector.d = avx2::movupd256(std::addressof(in_matrix->array_2d[1][0]));
        out_matrix->array_row[2].vector.d = avx2::movupd256(std::addressof(in_matrix->array_2d[2][0]));
        out_matrix->array_row[3].vector.d = avx2::movupd256(std::addressof(in_matrix->array_2d[3][0]));
    }

    constexpr void ConvertMatrix(MatrixRowMajor4x4d *out_matrix, const MatrixRowMajor4x3d *in_matrix) {
        out_matrix->array_row[0].vector.d = avx2::movapd256(std::addressof(in_matrix->array_2d[0][0]));
        out_matrix->array_row[1].vector.d = avx2::movapd256(std::addressof(in_matrix->array_2d[1][0]));
        out_matrix->array_row[2].vector.d = avx2::movapd256(std::addressof(in_matrix->array_2d[2][0]));
        out_matrix->array_row[3].vector.d = avx2::movapd256(std::addressof(in_matrix->array_2d[3][0]));
    }

    constexpr void ConvertMatrix(MatrixColumnMajor4x3d *out_matrix, const DoubleColumnMajor3x3 *in_matrix) {

        const v256i mask = avx2::GetLoadMaskDouble256(true, true, true, false);
        const v256f a = { .mmd = avx2::maskmovpd256(std::addressof(in_matrix->array_2d[0][0]), mask), };
        const v256f b = { .mmd = avx2::maskmovpd256(std::addressof(in_matrix->array_2d[1][0]), mask), };
        const v256f c = { .mmd = avx2::maskmovpd256(std::addressof(in_matrix->array_2d[2][0]), mask), };

        out_matrix->array_col[0].vector.d = a.d;
        out_matrix->array_col[1].vector.d = b.d;
        out_matrix->array_col[2].vector.d = c.d;
        out_matrix->array_col[3]          = Vector4d::cZero;
    }

    constexpr void ConvertMatrix(MatrixColumnMajor4x3d *out_matrix, const DoubleColumnMajor4x3 *in_matrix) {

        const v256i mask = avx2::GetLoadMaskDouble256(true, true, true, false);
        const v256f a = { .mmd = avx2::maskmovpd256(std::addressof(in_matrix->array_2d[0][0]), mask), };
        const v256f b = { .mmd = avx2::maskmovpd256(std::addressof(in_matrix->array_2d[1][0]), mask), };
        const v256f c = { .mmd = avx2::maskmovpd256(std::addressof(in_matrix->array_2d[2][0]), mask), };
        const v256f d = { .mmd = avx2::maskmovpd256(std::addressof(in_matrix->array_2d[3][0]), mask), };

        out_matrix->array_col[0].vector.d = a.d;
        out_matrix->array_col[1].vector.d = b.d;
        out_matrix->array_col[2].vector.d = c.d;
        out_matrix->array_col[3].vector.d = d.d;
    }

    constexpr void ConvertMatrix(MatrixColumnMajor4x4d *out_matrix, const DoubleColumnMajor3x3 *in_matrix) {

        const v256i mask = avx2::GetLoadMaskDouble256(true, true, true, false);
        const v256f a = { .mmd = avx2::maskmovpd256(std::addressof(in_matrix->array_2d[0][0]), mask), };
        const v256f b = { .mmd = avx2::maskmovpd256(std::addressof(in_matrix->array_2d[1][0]), mask), };
        const v256f c = { .mmd = avx2::maskmovpd256(std::addressof(in_matrix->array_2d[2][0]), mask), };

        out_matrix->array_col[0].vector.d = a.d;
        out_matrix->array_col[1].vector.d = b.d;
        out_matrix->array_col[2].vector.d = c.d;
        out_matrix->array_col[3]          = Vector4d::cZero;
    }

    constexpr void ConvertMatrix(MatrixColumnMajor4x4d *out_matrix, const DoubleColumnMajor4x3 *in_matrix) {

        const v256i mask = avx2::GetLoadMaskDouble256(true, true, true, false);
        const v256f a = { .mmd = avx2::maskmovpd256(std::addressof(in_matrix->array_2d[0][0]), mask), };
        const v256f b = { .mmd = avx2::maskmovpd256(std::addressof(in_matrix->array_2d[1][0]), mask), };
        const v256f c = { .mmd = avx2::maskmovpd256(std::addressof(in_matrix->array_2d[2][0]), mask), };
        const v256f d = { .mmd = avx2::maskmovpd256(std::addressof(in_matrix->array_2d[3][0]), mask), };

        out_matrix->array_col[0].vector.d = a.d;
        out_matrix->array_col[1].vector.d = b.d;
        out_matrix->array_col[2].vector.d = c.d;
        out_matrix->array_col[3].vector.d = d.d;
    }

    constexpr void ConvertMatrix(MatrixColumnMajor4x4d *out_matrix, const DoubleColumnMajor4x4 *in_matrix) {
        out_matrix->array_col[0].vector.d = avx2::movupd256(std::addressof(in_matrix->array_2d[0][0]));
        out_matrix->array_col[1].vector.d = avx2::movupd256(std::addressof(in_matrix->array_2d[1][0]));
        out_matrix->array_col[2].vector.d = avx2::movupd256(std::addressof(in_matrix->array_2d[2][0]));
        out_matrix->array_col[3].vector.d = avx2::movupd256(std::addressof(in_matrix->array_2d[3][0]));
    }

    constexpr void ConvertMatrix(MatrixColumnMajor4x4d *out_matrix, const MatrixColumnMajor4x3d *in_matrix) {
        out_matrix->array_col[0].vector.d = avx2::movapd256(std::addressof(in_matrix->array_2d[0][0]));
        out_matrix->array_col[1].vector.d = avx2::movapd256(std::addressof(in_matrix->array_2d[1][0]));
        out_matrix->array_col[2].vector.d = avx2::movapd256(std::addressof(in_matrix->array_2d[2][0]));
        out_matrix->array_col[3].vector.d = avx2::movapd256(std::addressof(in_matrix->array_2d[3][0]));
    }

    constexpr Double2 ConvertDouble(const Vector2d &in_vector) {
        const double x = in_vector.x;
        const double y = in_vector.y;
        return { x, y, };
    }
    constexpr Double3 ConvertDouble(const Vector3d &in_vector) {
        const double x = in_vector.x;
        const double y = in_vector.y;
        const double z = in_vector.z;
        return { x, y, z, };
    }
    constexpr Double4 ConvertDouble(const Vector4d &in_vector) {
        const double x = in_vector.x;
        const double y = in_vector.y;
        const double z = in_vector.z;
        const double w = in_vector.w;
        return { x, y, z, w, };
    }
}
