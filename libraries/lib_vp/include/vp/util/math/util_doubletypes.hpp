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
        double x;
        double y;

        static const Double2 cUnitX;
        static const Double2 cUnitY;
        static const Double2 cZero;
        static const Double2 cOne;
    };
    static_assert(sizeof(Double2) == 0x10);
    static_assert(alignof(Double2) == 0x8);

    struct Double3 {
        double x;
        double y;
        double z;

        static const Double3 cUnitX;
        static const Double3 cUnitY;
        static const Double3 cUnitZ;
        static const Double3 cZero;
        static const Double3 cOne;
    };
    static_assert(sizeof(Double3) == 0x18);
    static_assert(alignof(Double3) == 0x8);

    struct Double4 {
        double x;
        double y;
        double z;
        double w;

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

    using Double2Vector  = double __attribute__((vector_size(sizeof(double) * 2)));
    using Double3Vector  = double __attribute__((vector_size(sizeof(double) * 4)));
    using Double4Vector  = double __attribute__((vector_size(sizeof(double) * 4)));
    using Matrix33dVector = double __attribute__((vector_size(sizeof(double) * 16)));
    using Matrix34dVector = double __attribute__((vector_size(sizeof(double) * 16)));
    using Matrix44dVector = double __attribute__((vector_size(sizeof(double) * 16)));

    struct Vector2d {
        union {
            struct {
                double x;
                double y;
            };
            Double2Vector vector;
        };


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
            Double3Vector vector;
        };

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
            Double4Vector vector;
        };

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
            Matrix34dVector vector;
            Double3         array_row[4];
            double          array_2d[4][3];
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
        return { in_vector.x, in_vector.y, };
    }
    constexpr Vector3d ConvertVector(const Double3 &in_vector) {
        return { in_vector.x, in_vector.y, in_vector.z, };
    }
    constexpr Vector4d ConvertVector(const Double4 &in_vector) {
        return { in_vector.x, in_vector.y, in_vector.z, in_vector.w, };
    }
    constexpr Vector4d ConvertVector(const Vector3d &in_vector) {
        return { in_vector.x, in_vector.y, in_vector.z, 0.0f, };
    }
    constexpr Vector4d ConvertVectorUnit(const Vector3d &in_vector) {
        return { in_vector.x, in_vector.y, in_vector.z, 1.0f, };
    }

    constexpr void ConvertMatrix(MatrixRowMajor4x3d *out_matrix, const DoubleRowMajor3x3 *in_matrix) {

        const Double3 vector0 = in_matrix->array_row[0];
        const Double3 vector1 = in_matrix->array_row[1];
        const Double3 vector2 = in_matrix->array_row[2];

        out_matrix->array_row[0] = vector0;
        out_matrix->array_row[1] = vector1;
        out_matrix->array_row[2] = vector2;
        out_matrix->array_row[3] = Double3::cZero;
    }

    constexpr void ConvertMatrix(MatrixRowMajor4x3d *out_matrix, const DoubleRowMajor4x3 *in_matrix) {

        const Double3 vector0 = in_matrix->array_row[0];
        const Double3 vector1 = in_matrix->array_row[1];
        const Double3 vector2 = in_matrix->array_row[2];
        const Double3 vector3 = in_matrix->array_row[3];

        out_matrix->array_row[0] = vector0;
        out_matrix->array_row[1] = vector1;
        out_matrix->array_row[2] = vector2;
        out_matrix->array_row[3] = vector3;
    }

    constexpr void ConvertMatrix(MatrixRowMajor4x4d *out_matrix, const DoubleRowMajor3x3 *in_matrix) {

        const Vector4d vector0 = ConvertVector(ConvertVector(in_matrix->array_row[0]));
        const Vector4d vector1 = ConvertVector(ConvertVector(in_matrix->array_row[1]));
        const Vector4d vector2 = ConvertVector(ConvertVector(in_matrix->array_row[2]));

        out_matrix->array_row[0] = vector0;
        out_matrix->array_row[1] = vector1;
        out_matrix->array_row[2] = vector2;
        out_matrix->array_row[3] = Vector4d::cUnitW;
    }

    constexpr void ConvertMatrix(MatrixRowMajor4x4d *out_matrix, const DoubleRowMajor4x3 *in_matrix) {

        const Vector4d vector0 = ConvertVector(ConvertVector(in_matrix->array_row[0]));
        const Vector4d vector1 = ConvertVector(ConvertVector(in_matrix->array_row[1]));
        const Vector4d vector2 = ConvertVector(ConvertVector(in_matrix->array_row[2]));
        const Vector4d vector3 = ConvertVectorUnit(ConvertVector(in_matrix->array_row[3]));

        out_matrix->array_row[0] = vector0;
        out_matrix->array_row[1] = vector1;
        out_matrix->array_row[2] = vector2;
        out_matrix->array_row[3] = vector3;
    }

    constexpr void ConvertMatrix(MatrixRowMajor4x4d *out_matrix, const DoubleRowMajor4x4 *in_matrix) {

        const Vector4d vector0 = ConvertVector(in_matrix->array_row[0]);
        const Vector4d vector1 = ConvertVector(in_matrix->array_row[1]);
        const Vector4d vector2 = ConvertVector(in_matrix->array_row[2]);
        const Vector4d vector3 = ConvertVector(in_matrix->array_row[3]);

        out_matrix->array_row[0] = vector0;
        out_matrix->array_row[1] = vector1;
        out_matrix->array_row[2] = vector2;
        out_matrix->array_row[3] = vector3;
    }

    constexpr void ConvertMatrix(MatrixRowMajor4x4d *out_matrix, const MatrixRowMajor4x3d *in_matrix) {

        const Vector4d vector0 = ConvertVector(ConvertVector(in_matrix->array_row[0]));
        const Vector4d vector1 = ConvertVector(ConvertVector(in_matrix->array_row[1]));
        const Vector4d vector2 = ConvertVector(ConvertVector(in_matrix->array_row[2]));
        const Vector4d vector3 = ConvertVectorUnit(ConvertVector(in_matrix->array_row[3]));

        out_matrix->array_row[0] = vector0;
        out_matrix->array_row[1] = vector1;
        out_matrix->array_row[2] = vector2;
        out_matrix->array_row[3] = vector3;
    }

    constexpr void ConvertMatrix(MatrixColumnMajor4x3d *out_matrix, const DoubleColumnMajor3x3 *in_matrix) {

        const Vector4d vector0 = ConvertVector(ConvertVector(in_matrix->array_col[0]));
        const Vector4d vector1 = ConvertVector(ConvertVector(in_matrix->array_col[1]));
        const Vector4d vector2 = ConvertVector(ConvertVector(in_matrix->array_col[2]));

        out_matrix->array_col[0] = vector0;
        out_matrix->array_col[1] = vector1;
        out_matrix->array_col[2] = vector2;
        out_matrix->array_col[3] = Vector4d::cZero;
    }

    constexpr void ConvertMatrix(MatrixColumnMajor4x3d *out_matrix, const DoubleColumnMajor4x3 *in_matrix) {

        const Vector4d vector0 = ConvertVector(in_matrix->array_col[0]);
        const Vector4d vector1 = ConvertVector(in_matrix->array_col[1]);
        const Vector4d vector2 = ConvertVector(in_matrix->array_col[2]);
        out_matrix->array_col[0] = vector0;
        out_matrix->array_col[1] = vector1;
        out_matrix->array_col[2] = vector2;
        out_matrix->array_col[3] = Vector4d::cZero;
    }

    constexpr void ConvertMatrix(MatrixColumnMajor4x4d *out_matrix, const DoubleColumnMajor3x3 *in_matrix) {

        const Vector4d vector0 = ConvertVector(ConvertVector(in_matrix->array_col[0]));
        const Vector4d vector1 = ConvertVector(ConvertVector(in_matrix->array_col[1]));
        const Vector4d vector2 = ConvertVector(ConvertVector(in_matrix->array_col[2]));

        out_matrix->array_col[0] = vector0;
        out_matrix->array_col[1] = vector1;
        out_matrix->array_col[2] = vector2;
        out_matrix->array_col[3] = Vector4d::cUnitW;
    }

    constexpr void ConvertMatrix(MatrixColumnMajor4x4d *out_matrix, const DoubleColumnMajor4x3 *in_matrix) {

        const Vector4d vector0 = ConvertVector(in_matrix->array_col[0]);
        const Vector4d vector1 = ConvertVector(in_matrix->array_col[1]);
        const Vector4d vector2 = ConvertVector(in_matrix->array_col[2]);

        out_matrix->array_col[0] = vector0;
        out_matrix->array_col[1] = vector1;
        out_matrix->array_col[2] = vector2;
        out_matrix->array_col[3] = Vector4d::cUnitW;
    }

    constexpr void ConvertMatrix(MatrixColumnMajor4x4d *out_matrix, const DoubleColumnMajor4x4 *in_matrix) {

        const Vector4d vector0 = ConvertVector(in_matrix->array_col[0]);
        const Vector4d vector1 = ConvertVector(in_matrix->array_col[1]);
        const Vector4d vector2 = ConvertVector(in_matrix->array_col[2]);
        const Vector4d vector3 = ConvertVector(in_matrix->array_col[3]);

        out_matrix->array_col[0] = vector0;
        out_matrix->array_col[1] = vector1;
        out_matrix->array_col[2] = vector2;
        out_matrix->array_col[3] = vector3;
    }

    constexpr void ConvertMatrix(MatrixColumnMajor4x4d *out_matrix, const MatrixColumnMajor4x3d *in_matrix) {

        const Vector4d vector0 = in_matrix->array_col[0];
        const Vector4d vector1 = in_matrix->array_col[1];
        const Vector4d vector2 = in_matrix->array_col[2];

        out_matrix->array_col[0] = vector0;
        out_matrix->array_col[1] = vector1;
        out_matrix->array_col[2] = vector2;
        out_matrix->array_col[3] = Vector4d::cUnitW;
    }

    constexpr Double2 ConvertDouble(const Vector2d &in_vector) {
        return { in_vector.x, in_vector.y };
    }
    constexpr Double3 ConvertDouble(const Vector3d &in_vector) {
        return { in_vector.x, in_vector.y, in_vector.z };
    }
    constexpr Double4 ConvertDouble(const Vector4d &in_vector) {
        return { in_vector.x, in_vector.y, in_vector.z, in_vector.w };
    }
}
