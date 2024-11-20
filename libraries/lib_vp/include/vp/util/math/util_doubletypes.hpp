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
            double v[2];
        };

        static const Double2 cUnitX;
        static const Double2 cUnitY;
        static const Double2 cZero;
        static const Double2 cOne;

        VP_VECTOR_TRAITS(double, 2, false);
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
            double v[3];
        };

        static const Double3 cUnitX;
        static const Double3 cUnitY;
        static const Double3 cUnitZ;
        static const Double3 cZero;
        static const Double3 cOne;

        VP_VECTOR_TRAITS(double, 3, false);
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
            double v[4];
        };

        static const Double4 cUnitX;
        static const Double4 cUnitY;
        static const Double4 cUnitZ;
        static const Double4 cUnitW;
        static const Double4 cZero;
        static const Double4 cOne;

        VP_VECTOR_TRAITS(double, 4, false);
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
            double m[9];
            Double3 array_row[3];
        };

        VP_MATRIX_TRAITS(double, 3, 3, true, false);
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
            double m[12];
            Double3 array_row[4];
        };

        VP_MATRIX_TRAITS(double, 4, 3, true, false);
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
            double m[16];
            Double4 array_row[4];
        };

        VP_MATRIX_TRAITS(double, 4, 4, true, false);
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
            double m[9];
            Double3 array_col[3];
        };

        VP_MATRIX_TRAITS(double, 3, 3, false, false);
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
            double m[12];
            Double4 array_col[3];
        };

        VP_MATRIX_TRAITS(double, 4, 3, false, false);
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
            double m[16];
            Double4 array_col[4];
        };

        VP_MATRIX_TRAITS(double, 4, 4, false, false);
    };
    static_assert(sizeof(DoubleColumnMajor4x4) == 0x80);
    static_assert(alignof(DoubleColumnMajor4x4) == 0x8);

    struct Vector2d {
        v2d v;

        static const Vector2d cUnitX;
        static const Vector2d cUnitY;
        static const Vector2d cZero;
        static const Vector2d cOne;

        VP_VECTOR_TRAITS(double, 2, true);
    };
    static_assert(sizeof(Vector2d) == 0x10);

    struct Vector3d {
        v4d v;

        static const Vector3d cUnitX;
        static const Vector3d cUnitY;
        static const Vector3d cUnitZ;
        static const Vector3d cZero;
        static const Vector3d cOne;

        VP_VECTOR_TRAITS(double, 3, true);
    };
    static_assert(sizeof(Vector3d) == 0x20);

    struct Vector4d {
        v4d v;

        static const Vector4d cUnitX;
        static const Vector4d cUnitY;
        static const Vector4d cUnitZ;
        static const Vector4d cUnitW;
        static const Vector4d cZero;
        static const Vector4d cOne;

        VP_VECTOR_TRAITS(double, 4, true);
    };
    static_assert(sizeof(Vector4d) == 0x20);

    struct MatrixRowMajor4x3d {
        v16d m;

        static const MatrixRowMajor4x3d cIdentity;

        VP_MATRIX_TRAITS(double, 4, 3, true, true);
    };
    static_assert(sizeof(MatrixRowMajor4x3d) == 0x80);

    struct MatrixRowMajor4x4d {
        v16d m;

        static const MatrixRowMajor4x4d cIdentity;

        VP_MATRIX_TRAITS(double, 4, 4, true, true);
    };
    static_assert(sizeof(MatrixRowMajor4x4d) == 0x80);

    struct MatrixColumnMajor4x3d {
        v4d m[3];

        static const MatrixColumnMajor4x3d cIdentity;

        VP_MATRIX_TRAITS(double, 4, 3, false, true);
    };
    static_assert(sizeof(MatrixColumnMajor4x3d) == 0x60);

    struct MatrixColumnMajor4x4d {
        v16d m;

        static const MatrixColumnMajor4x4d cIdentity;

        VP_MATRIX_TRAITS(double, 4, 4, false, true);
    };
    static_assert(sizeof(MatrixColumnMajor4x4d) == 0x80);

    constexpr const Double2 Double2::cUnitX = { .x = 1.0 };
    constexpr const Double2 Double2::cUnitY = { .y = 1.0 };
    constexpr const Double2 Double2::cZero  = {};
    constexpr const Double2 Double2::cOne   = { 1.0, 1.0 };

    constexpr const Double3 Double3::cUnitX = { .x = 1.0 };
    constexpr const Double3 Double3::cUnitY = { .y = 1.0 };
    constexpr const Double3 Double3::cUnitZ = { .z = 1.0 };
    constexpr const Double3 Double3::cZero  = {};
    constexpr const Double3 Double3::cOne   = { 1.0, 1.0, 1.0 };

    constexpr const Double4 Double4::cUnitX = { .x = 1.0 };
    constexpr const Double4 Double4::cUnitY = { .y = 1.0 };
    constexpr const Double4 Double4::cUnitZ = { .z = 1.0 };
    constexpr const Double4 Double4::cUnitW = { .w = 1.0 };
    constexpr const Double4 Double4::cZero  = {};
    constexpr const Double4 Double4::cOne   = { 1.0, 1.0, 1.0, 1.0 };

    constexpr const Vector2d Vector2d::cUnitX = { 1.0, 0.0, };
    constexpr const Vector2d Vector2d::cUnitY = { 0.0, 1.0, };
    constexpr const Vector2d Vector2d::cZero  = {};
    constexpr const Vector2d Vector2d::cOne   = { 1.0, 1.0 };

    constexpr const Vector3d Vector3d::cUnitX = { 1.0, 0.0, 0.0, };
    constexpr const Vector3d Vector3d::cUnitY = { 0.0, 1.0, 0.0, };
    constexpr const Vector3d Vector3d::cUnitZ = { 0.0, 0.0, 1.0, };
    constexpr const Vector3d Vector3d::cZero  = {};
    constexpr const Vector3d Vector3d::cOne   = { 1.0, 1.0, 1.0 };

    constexpr const Vector4d Vector4d::cUnitX = { 1.0, 0.0, 0.0, 0.0, };
    constexpr const Vector4d Vector4d::cUnitY = { 0.0, 1.0, 0.0, 0.0, };
    constexpr const Vector4d Vector4d::cUnitZ = { 0.0, 0.0, 1.0, 0.0, };
    constexpr const Vector4d Vector4d::cUnitW = { 0.0, 0.0, 0.0, 1.0, };
    constexpr const Vector4d Vector4d::cZero  = {};
    constexpr const Vector4d Vector4d::cOne   = { 1.0, 1.0, 1.0, 1.0 };

    constexpr const MatrixRowMajor4x3d MatrixRowMajor4x3d::cIdentity = {
        1.0, 0.0, 0.0, 0.0,
        0.0, 1.0, 0.0, 0.0,
        0.0, 0.0, 1.0, 0.0,
        0.0, 0.0, 0.0, 1.0,
    };
    constexpr const MatrixRowMajor4x4d MatrixRowMajor4x4d::cIdentity = {
        1.0, 0.0, 0.0, 0.0,
        0.0, 1.0, 0.0, 0.0,
        0.0, 0.0, 1.0, 0.0,
        0.0, 0.0, 0.0, 1.0,
    };
    constexpr const MatrixColumnMajor4x3d MatrixColumnMajor4x3d::cIdentity = {
        1.0, 0.0, 0.0, 0.0,
        0.0, 1.0, 0.0, 0.0,
        0.0, 0.0, 1.0, 0.0,
    };
    constexpr const MatrixColumnMajor4x4d MatrixColumnMajor4x4d::cIdentity = {
        1.0, 0.0, 0.0, 0.0,
        0.0, 1.0, 0.0, 0.0,
        0.0, 0.0, 1.0, 0.0,
        0.0, 0.0, 0.0, 1.0,
    };

    constexpr Vector2d ConvertVector2(const Double2 &in_vector) {
        const double x = in_vector.x;
        const double y = in_vector.y;
        return Vector2d{ x, y, };
    }

    constexpr Vector3d ConvertVector3(const Double3 &in_vector) {
        const double x = in_vector.x;
        const double y = in_vector.y;
        const double z = in_vector.z;
        return Vector3d{ x, y, z, };
    }

    constexpr Vector4d ConvertVector4(const Double4 &in_vector) {
        const double x = in_vector.x;
        const double y = in_vector.y;
        const double z = in_vector.z;
        const double w = in_vector.w;
        return Vector4d{ x, y, z, w, };
    }

    constexpr Vector4d PromoteVector4(const Vector3d &in_vector) {
        return Vector4d{ .v = in_vector.v, };
    }

    constexpr MatrixRowMajor4x3d ConvertToMatrix43(MatrixRowMajor4x3d *out_matrix, const DoubleRowMajor3x3 *in_matrix) {

        const v4sll mask = GetLoad4Mask256(true, true, true, false);
        const v4d a = Load4Mask256(std::addressof((*in_matrix)[0,0]), mask);
        const v4d b = Load4Mask256(std::addressof((*in_matrix)[1,0]), mask);
        const v4d c = Load4Mask256(std::addressof((*in_matrix)[2,0]), mask);

        return MatrixRowMajor4x3d { a, b, c, Vector4d::cZero.v, };
    }

    constexpr MatrixRowMajor4x3d ConvertToMatrix43(MatrixRowMajor4x3d *out_matrix, const DoubleRowMajor4x3 *in_matrix) {

        const v4sll mask = GetLoad4Mask256(true, true, true, false);
        const v4d a = Load4Mask256(std::addressof((*in_matrix)[0,0]), mask);
        const v4d b = Load4Mask256(std::addressof((*in_matrix)[1,0]), mask);
        const v4d c = Load4Mask256(std::addressof((*in_matrix)[2,0]), mask);
        const v4d d = Load4Mask256(std::addressof((*in_matrix)[3,0]), mask);

        return MatrixRowMajor4x3d { a, b, c, d, };
    }

    constexpr MatrixRowMajor4x4d ConvertToMatrix44(MatrixRowMajor4x4d *out_matrix, const DoubleRowMajor3x3 *in_matrix) {

        const v4sll mask = GetLoad4Mask256(true, true, true, false);
        const v4d a = Load4Mask256(std::addressof((*in_matrix)[0,0]), mask);
        const v4d b = Load4Mask256(std::addressof((*in_matrix)[1,0]), mask);
        const v4d c = Load4Mask256(std::addressof((*in_matrix)[2,0]), mask);

        return MatrixRowMajor4x4d { a, b, c, Vector4d::cZero.v, };
    }

    constexpr MatrixRowMajor4x4d ConvertToMatrix44(MatrixRowMajor4x4d *out_matrix, const DoubleRowMajor4x3 *in_matrix) {

        const v4sll mask = GetLoad4Mask256(true, true, true, false);
        const v4d a = Load4Mask256(std::addressof((*in_matrix)[0,0]), mask);
        const v4d b = Load4Mask256(std::addressof((*in_matrix)[1,0]), mask);
        const v4d c = Load4Mask256(std::addressof((*in_matrix)[2,0]), mask);
        const v4d d = Load4Mask256(std::addressof((*in_matrix)[3,0]), mask);

        return MatrixRowMajor4x4d { a, b, c, d, };
    }

    constexpr MatrixRowMajor4x4d ConvertToMatrix44(const DoubleRowMajor4x4 *in_matrix) {
        const v4d a = Load4(std::addressof((*in_matrix)[0,0]));
        const v4d b = Load4(std::addressof((*in_matrix)[1,0]));
        const v4d c = Load4(std::addressof((*in_matrix)[2,0]));
        const v4d d = Load4(std::addressof((*in_matrix)[3,0]));
        return MatrixRowMajor4x4d { a, b, c, d, };
    }

    constexpr MatrixRowMajor4x4d PromoteMatrix44(const MatrixRowMajor4x3d *in_matrix) {
        const v4d a = Load4Aligned(std::addressof((*in_matrix)[0,0]));
        const v4d b = Load4Aligned(std::addressof((*in_matrix)[1,0]));
        const v4d c = Load4Aligned(std::addressof((*in_matrix)[2,0]));
        const v4d d = Load4Aligned(std::addressof((*in_matrix)[3,0]));
        return MatrixRowMajor4x4d { a, b, c, d, };
    }

    constexpr MatrixColumnMajor4x3d ConvertToMatrix43(const DoubleColumnMajor3x3 *in_matrix) {

        const v4sll mask = GetLoad4Mask256(true, true, true, false);
        const v4d a = Load4Mask256(std::addressof((*in_matrix)[0,0]), mask);
        const v4d b = Load4Mask256(std::addressof((*in_matrix)[1,0]), mask);
        const v4d c = Load4Mask256(std::addressof((*in_matrix)[2,0]), mask);

        return MatrixColumnMajor4x3d { a, b, c, };
    }

    constexpr MatrixColumnMajor4x3d ConvertToMatrix43(const DoubleColumnMajor4x3 *in_matrix) {

        const v4sll mask = GetLoad4Mask256(true, true, true, false);
        const v4d a = Load4Mask256(std::addressof((*in_matrix)[0,0]), mask);
        const v4d b = Load4Mask256(std::addressof((*in_matrix)[1,0]), mask);
        const v4d c = Load4Mask256(std::addressof((*in_matrix)[2,0]), mask);

        return MatrixColumnMajor4x3d { a, b, c, };
    }

    constexpr MatrixColumnMajor4x4d ConvertToMatrix44(const DoubleColumnMajor3x3 *in_matrix) {

        const v4sll mask = GetLoad4Mask256(true, true, true, false);
        const v4d a = Load4Mask256(std::addressof((*in_matrix)[0,0]), mask);
        const v4d b = Load4Mask256(std::addressof((*in_matrix)[1,0]), mask);
        const v4d c = Load4Mask256(std::addressof((*in_matrix)[2,0]), mask);

        return MatrixColumnMajor4x4d { a, b, c, Vector4d::cZero.v, };
    }

    constexpr MatrixColumnMajor4x4d ConvertToMatrix44(const DoubleColumnMajor4x3 *in_matrix) {

        const v4sll mask = GetLoad4Mask256(true, true, true, false);
        const v4d a = Load4Mask256(std::addressof((*in_matrix)[0,0]), mask);
        const v4d b = Load4Mask256(std::addressof((*in_matrix)[1,0]), mask);
        const v4d c = Load4Mask256(std::addressof((*in_matrix)[2,0]), mask);
        const v4d d = Load4Mask256(std::addressof((*in_matrix)[3,0]), mask);

        return MatrixColumnMajor4x4d { a, b, c, d, };
    }

    constexpr MatrixColumnMajor4x4d ConvertToMatrix44(const DoubleColumnMajor4x4 *in_matrix) {
        const v4d a = Load4(std::addressof((*in_matrix)[0,0]));
        const v4d b = Load4(std::addressof((*in_matrix)[1,0]));
        const v4d c = Load4(std::addressof((*in_matrix)[2,0]));
        const v4d d = Load4(std::addressof((*in_matrix)[3,0]));
        return MatrixColumnMajor4x4d { a, b, c, d, };
    }

    constexpr MatrixColumnMajor4x4d PromoteMatrix44(const MatrixColumnMajor4x3d *in_matrix) {
        const v4d a = Load4Aligned(std::addressof((*in_matrix)[0,0]));
        const v4d b = Load4Aligned(std::addressof((*in_matrix)[1,0]));
        const v4d c = Load4Aligned(std::addressof((*in_matrix)[2,0]));
        const v4d d = Load4Aligned(std::addressof((*in_matrix)[3,0]));
        return MatrixColumnMajor4x4d { a, b, c, d, };
    }

    constexpr Double2 ConvertDouble2(const Vector2d &in_vector) {
        const double x = in_vector[0];
        const double y = in_vector[1];
        return { x, y, };
    }
    constexpr Double3 ConvertDouble3(const Vector3d &in_vector) {
        const double x = in_vector[0];
        const double y = in_vector[1];
        const double z = in_vector[2];
        return { x, y, z, };
    }
    constexpr Double4 ConvertDouble4(const Vector4d &in_vector) {
        const double x = in_vector[0];
        const double y = in_vector[1];
        const double z = in_vector[2];
        const double w = in_vector[3];
        return { x, y, z, w, };
    }
}
