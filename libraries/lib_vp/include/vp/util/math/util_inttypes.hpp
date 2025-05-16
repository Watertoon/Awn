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

    struct Int2 {
        union {
            struct {
                s32 x;
                s32 y;
            };
            s32 v[2];
        };

        static const Int2 cUnitX;
        static const Int2 cUnitY;
        static const Int2 cZero;
        static const Int2 cOne;

        VP_VECTOR_TRAITS(s32, 2, false);
    };
    static_assert(sizeof(Int2) == 0x8);
    static_assert(alignof(Int2) == 0x4);

    struct Int3 {
        union {
            struct {
                s32 x;
                s32 y;
                s32 z;
            };
            s32 v[3];
        };

        static const Int3 cUnitX;
        static const Int3 cUnitY;
        static const Int3 cUnitZ;
        static const Int3 cZero;
        static const Int3 cOne;

        VP_VECTOR_TRAITS(s32, 3, false);
    };
    static_assert(sizeof(Int3) == 0xc);
    static_assert(alignof(Int3) == 0x4);

    struct Int4 {
        union {
            struct {
                s32 x;
                s32 y;
                s32 z;
                s32 w;
            };
            s32 array[4];
        };

        static const Int4 cUnitX;
        static const Int4 cUnitY;
        static const Int4 cUnitZ;
        static const Int4 cUnitW;
        static const Int4 cZero;
        static const Int4 cOne;

        VP_VECTOR_TRAITS(s32, 4, false);
    };
    static_assert(sizeof(Int4) == 0x10);
    static_assert(alignof(Int4) == 0x4);

    struct IntRowMajor3x3 {
        union {
            struct {
                s32 m00;
                s32 m01;
                s32 m02;
                s32 m10;
                s32 m11;
                s32 m12;
                s32 m20;
                s32 m21;
                s32 m22;
            };
            s32 m[9];
            Int3 array_row[3];
        };

        VP_MATRIX_TRAITS(s32, 3, 3, true, false);
    };
    static_assert(sizeof(IntRowMajor3x3) == 0x24);
    static_assert(alignof(IntRowMajor3x3) == 0x4);

    struct IntRowMajor4x3 {
        union {
            struct {
                s32 m00;
                s32 m01;
                s32 m02;
                s32 m10;
                s32 m11;
                s32 m12;
                s32 m20;
                s32 m21;
                s32 m22;
                s32 m30;
                s32 m31;
                s32 m32;
            };
            s32 m[12];
            Int3 array_row[4];
        };

        VP_MATRIX_TRAITS(s32, 4, 3, true, false);
    };
    static_assert(sizeof(IntRowMajor4x3) == 0x30);
    static_assert(alignof(IntRowMajor4x3) == 0x4);

    struct IntRowMajor4x4 {
        union {
            struct {
                s32 m00;
                s32 m01;
                s32 m02;
                s32 m03;
                s32 m10;
                s32 m11;
                s32 m12;
                s32 m13;
                s32 m20;
                s32 m21;
                s32 m22;
                s32 m23;
                s32 m30;
                s32 m31;
                s32 m32;
                s32 m33;
            };
            s32 m[16];
            Int4 array_row[4];
        };

        VP_MATRIX_TRAITS(s32, 4, 4, true, false);
    };
    static_assert(sizeof(IntRowMajor4x4) == 0x40);
    static_assert(alignof(IntRowMajor4x4) == 0x4);

    struct IntColumnMajor3x3 {
        union {
            struct {
                s32 m00;
                s32 m10;
                s32 m20;
                s32 m01;
                s32 m11;
                s32 m21;
                s32 m02;
                s32 m12;
                s32 m22;
            };
            s32 m[9];
            Int3 array_col[3];
        };

        VP_MATRIX_TRAITS(s32, 3, 3, false, false);
    };
    static_assert(sizeof(IntColumnMajor3x3) == 0x24);
    static_assert(alignof(IntColumnMajor3x3) == 0x4);

    struct IntColumnMajor4x3 {
        union {
            struct {
                s32 m00;
                s32 m10;
                s32 m20;
                s32 m30;
                s32 m01;
                s32 m11;
                s32 m21;
                s32 m31;
                s32 m02;
                s32 m12;
                s32 m22;
                s32 m32;
            };
            s32 m[12];
            Int4 array_col[3];
        };

        VP_MATRIX_TRAITS(s32, 4, 3, false, false);
    };
    static_assert(sizeof(IntColumnMajor4x3) == 0x30);
    static_assert(alignof(IntColumnMajor4x3) == 0x4);

    struct IntColumnMajor4x4 {
        union {
            struct {
                s32 m00;
                s32 m10;
                s32 m20;
                s32 m30;
                s32 m01;
                s32 m11;
                s32 m21;
                s32 m31;
                s32 m02;
                s32 m12;
                s32 m22;
                s32 m32;
                s32 m03;
                s32 m13;
                s32 m23;
                s32 m33;
            };
            s32 m[16];
            Int4 array_col[4];
        };

        VP_MATRIX_TRAITS(s32, 4, 4, false, false);
    };
    static_assert(sizeof(IntColumnMajor4x4) == 0x40);
    static_assert(alignof(IntColumnMajor4x4) == 0x4);

    struct Vector2i {
         v2si v;

        static const Vector2i cUnitX;
        static const Vector2i cUnitY;
        static const Vector2i cZero;
        static const Vector2i cOne;

        VP_VECTOR_TRAITS(s32, 2, true);
    };
    static_assert(sizeof(Vector2i) == 0x8);
    static_assert(alignof(Vector2i) == 0x8);

    struct Vector3i {
         v4si v;

        static const Vector3i cUnitX;
        static const Vector3i cUnitY;
        static const Vector3i cUnitZ;
        static const Vector3i cZero;
        static const Vector3i cOne;

        VP_VECTOR_TRAITS(s32, 3, true);
    };
    static_assert(sizeof(Vector3i) == 0x10);
    static_assert(alignof(Vector3i) == 0x10);

    struct Vector4i {
         v4si v;

        static const Vector4i cUnitX;
        static const Vector4i cUnitY;
        static const Vector4i cUnitZ;
        static const Vector4i cUnitW;
        static const Vector4i cZero;
        static const Vector4i cOne;

        VP_VECTOR_TRAITS(s32, 4, true);
    };
    static_assert(sizeof(Vector4i) == 0x10);
    static_assert(alignof(Vector4i) == 0x10);

    struct MatrixRowMajor4x3i {
         v16si m;

        static const MatrixRowMajor4x3i cIdentity;

        VP_MATRIX_TRAITS(s32, 4, 3, true, true);
    };
    static_assert(sizeof(MatrixRowMajor4x3i) == 0x40);

    struct MatrixRowMajor4x4i {
         v16si m;

        static const MatrixRowMajor4x4i cIdentity;

        VP_MATRIX_TRAITS(s32, 4, 4, true, true);
    };
    static_assert(sizeof(MatrixRowMajor4x4i) == 0x40);

    struct MatrixColumnMajor4x3i {
         v4si m[3];

        static const MatrixColumnMajor4x3i cIdentity;

        VP_MATRIX_TRAITS(s32, 4, 3, false, true);
    };
    static_assert(sizeof(MatrixColumnMajor4x3i) == 0x30);

    struct MatrixColumnMajor4x4i {
         v16si m;

        static const MatrixColumnMajor4x4i cIdentity;

        VP_MATRIX_TRAITS(s32, 4, 4, false, true);
    };
    static_assert(sizeof(MatrixColumnMajor4x4i) == 0x40);

    constexpr const Int2 Int2::cUnitX = { .x = 1, };
    constexpr const Int2 Int2::cUnitY = { .y = 1, };
    constexpr const Int2 Int2::cZero  = {};
    constexpr const Int2 Int2::cOne   = { 1, 1, };

    constexpr const Int3 Int3::cUnitX = { .x = 1, };
    constexpr const Int3 Int3::cUnitY = { .y = 1, };
    constexpr const Int3 Int3::cUnitZ = { .z = 1, };
    constexpr const Int3 Int3::cZero  = {};
    constexpr const Int3 Int3::cOne   = { 1, 1, 1, };

    constexpr const Int4 Int4::cUnitX = { .x = 1, };
    constexpr const Int4 Int4::cUnitY = { .y = 1, };
    constexpr const Int4 Int4::cUnitZ = { .z = 1, };
    constexpr const Int4 Int4::cUnitW = { .w = 1, };
    constexpr const Int4 Int4::cZero  = {};
    constexpr const Int4 Int4::cOne   = { 1, 1, 1, 1, };

    constexpr const Vector2i Vector2i::cUnitX = { 1, 0, };
    constexpr const Vector2i Vector2i::cUnitY = { 0, 1, };
    constexpr const Vector2i Vector2i::cZero  = {};
    constexpr const Vector2i Vector2i::cOne   = { 1, 1, };

    constexpr const Vector3i Vector3i::cUnitX = { 1, 0, 0, };
    constexpr const Vector3i Vector3i::cUnitY = { 0, 1, 0, };
    constexpr const Vector3i Vector3i::cUnitZ = { 0, 0, 1, };
    constexpr const Vector3i Vector3i::cZero  = {};
    constexpr const Vector3i Vector3i::cOne   = { 1, 1, 1, };

    constexpr const Vector4i Vector4i::cUnitX = { 1, 0, 0, 0, };
    constexpr const Vector4i Vector4i::cUnitY = { 0, 1, 0, 0, };
    constexpr const Vector4i Vector4i::cUnitZ = { 0, 0, 1, 0, };
    constexpr const Vector4i Vector4i::cUnitW = { 0, 0, 0, 1, };
    constexpr const Vector4i Vector4i::cZero  = {};
    constexpr const Vector4i Vector4i::cOne   = { 1, 1, 1, 1, };

    constexpr const MatrixRowMajor4x3i MatrixRowMajor4x3i::cIdentity = {
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1,
    };
    constexpr const MatrixRowMajor4x4i MatrixRowMajor4x4i::cIdentity = {
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1,
    };
    constexpr const MatrixColumnMajor4x3i MatrixColumnMajor4x3i::cIdentity = {
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
    };
    constexpr const MatrixColumnMajor4x4i MatrixColumnMajor4x4i::cIdentity = {
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1,
    };

    constexpr Vector2i ConvertToVector2(const Int2 &in_vector) {
        return Vector2i{ in_vector.x, in_vector.y, };
    }

    constexpr Vector3i ConvertToVector3(const Int3 &in_vector) {
        return Vector3i{ in_vector.x, in_vector.y, in_vector.z, };
    }

    constexpr Vector4i ConvertToVector4(const Int4 &in_vector) {
        return Vector4i{ in_vector.x, in_vector.y, in_vector.z, in_vector.w, };
    }

    constexpr Vector4i PromoteToVector4(const Vector3i &in_vector) {
        return Vector4i{ .v = in_vector.v, };
    }

    //constexpr MatrixRowMajor4x3i ConvertToMatrix43(const IntRowMajor3x3 *in_matrix) {
    //
    //    const v4si mask = GetLoad4Mask128(true, true, true, false);
    //    const v4si a = Load4Mask128(std::addressof((*in_matrix)[0,0]), mask);
    //    const v4si b = Load4Mask128(std::addressof((*in_matrix)[1,0]), mask);
    //    const v4si c = Load4Mask128(std::addressof((*in_matrix)[2,0]), mask);
    //
    //    const MatrixRowMajor4x3i matrix = {
    //        a[0], a[1], a[2], a[3],
    //        b[0], b[1], b[2], b[3],
    //        c[0], c[1], c[2], c[3],
    //    };
    //    return matrix;
    //}
    //
    //constexpr MatrixRowMajor4x3i ConvertToMatrix43(const IntRowMajor4x3 *in_matrix) {
    //
    //    const v4si mask = GetLoad4Mask128(true, true, true, false);
    //    const v4si a = Load4Mask128(std::addressof((*in_matrix)[0,0]), mask);
    //    const v4si b = Load4Mask128(std::addressof((*in_matrix)[1,0]), mask);
    //    const v4si c = Load4Mask128(std::addressof((*in_matrix)[2,0]), mask);
    //    const v4si d = Load4Mask128(std::addressof((*in_matrix)[3,0]), mask);
    //
    //    const MatrixRowMajor4x3i matrix = {
    //        a[0], a[1], a[2], a[3],
    //        b[0], b[1], b[2], b[3],
    //        c[0], c[1], c[2], c[3],
    //        d[0], d[1], d[2], d[3],
    //    };
    //    return matrix;
    //}
    //
    //constexpr MatrixRowMajor4x4i ConvertToMatrix44(const IntRowMajor3x3 *in_matrix) {
    //
    //    const v4si mask = GetLoad4Mask128(true, true, true, false);
    //    const v4si a = Load4Mask128(std::addressof((*in_matrix)[0,0]), mask);
    //    const v4si b = Load4Mask128(std::addressof((*in_matrix)[1,0]), mask);
    //    const v4si c = Load4Mask128(std::addressof((*in_matrix)[2,0]), mask);
    //
    //    const MatrixRowMajor4x4i matrix = {
    //        a[0], a[1], a[2], a[3],
    //        b[0], b[1], b[2], b[3],
    //        c[0], c[1], c[2], c[3],
    //        0, 0, 0, 1,
    //    };
    //    return matrix;
    //}
    //
    //constexpr MatrixRowMajor4x4i ConvertToMatrix44(const IntRowMajor4x3 *in_matrix) {
    //
    //    const v4si mask = GetLoad4Mask128(true, true, true, false);
    //    const v4si a = Load4Mask128(std::addressof((*in_matrix)[0,0]), mask);
    //    const v4si b = Load4Mask128(std::addressof((*in_matrix)[1,0]), mask);
    //    const v4si c = Load4Mask128(std::addressof((*in_matrix)[2,0]), mask);
    //    const v4si d = Load4Mask128(std::addressof((*in_matrix)[3,0]), mask);
    //
    //    const MatrixRowMajor4x4i matrix = {
    //        a[0], a[1], a[2], a[3],
    //        b[0], b[1], b[2], b[3],
    //        c[0], c[1], c[2], c[3],
    //        d[0], d[1], d[2], 1,
    //    };
    //    return matrix;
    //}
    //
    //constexpr MatrixRowMajor4x4i ConvertToMatrix44(const IntRowMajor4x4 *in_matrix) {
    //    const v8si a = Load8(std::addressof((*in_matrix)[0, 0]));
    //    const v8si b = Load8(std::addressof((*in_matrix)[2, 0]));
    //    const MatrixRowMajor4x4i ret = { a, b, };
    //    return ret;
    //}
    //
    //constexpr MatrixRowMajor4x4i PromoteToMatrix44(const MatrixRowMajor4x3i *in_matrix) {
    //    return MatrixRowMajor4x4i { .m = in_matrix->m };
    //}
    //
    //constexpr MatrixColumnMajor4x3i ConvertToMatrix43(const IntColumnMajor3x3 *in_matrix) {
    //
    //    const v4si mask = GetLoad4Mask128(true, true, true, false);
    //    const v4si a = Load4Mask128(std::addressof((*in_matrix)[0,0]), mask);
    //    const v4si b = Load4Mask128(std::addressof((*in_matrix)[1,0]), mask);
    //    const v4si c = Load4Mask128(std::addressof((*in_matrix)[2,0]), mask);
    //
    //    return MatrixColumnMajor4x3i { a, b, c, };
    //}
    //
    //constexpr MatrixColumnMajor4x3i ConvertToMatrix43(const IntColumnMajor4x3 *in_matrix) {
    //
    //    const v4si a = Load4(std::addressof((*in_matrix)[0,0]));
    //    const v4si b = Load4(std::addressof((*in_matrix)[1,0]));
    //    const v4si c = Load4(std::addressof((*in_matrix)[2,0]));
    //
    //    return MatrixColumnMajor4x3i { a, b, c, };
    //}
    //
    //constexpr MatrixColumnMajor4x4i ConvertToMatrix44(const IntColumnMajor3x3 *in_matrix) {
    //
    //    const v4si mask = GetLoad4Mask128(true, true, true, false);
    //    const v4si a = Load4Mask128(std::addressof((*in_matrix)[0,0]), mask);
    //    const v4si b = Load4Mask128(std::addressof((*in_matrix)[1,0]), mask);
    //    const v4si c = Load4Mask128(std::addressof((*in_matrix)[2,0]), mask);
    //
    //    const MatrixColumnMajor4x4i matrix = {
    //        a[0], a[1], a[2], a[3],
    //        b[0], b[1], b[2], b[3],
    //        c[0], c[1], c[2], c[3],
    //        0,0,0,1,
    //    };
    //    return matrix;
    //}
    //
    //constexpr MatrixColumnMajor4x4i ConvertToMatrix44(const IntColumnMajor4x3 *in_matrix) {
    //
    //    const v4si a = Load4(std::addressof((*in_matrix)[0,0]));
    //    const v4si b = Load4(std::addressof((*in_matrix)[1,0]));
    //    const v4si c = Load4(std::addressof((*in_matrix)[2,0]));
    //
    //    const MatrixColumnMajor4x4i matrix = {
    //        a[0], a[1], a[2], a[3],
    //        b[0], b[1], b[2], b[3],
    //        c[0], c[1], c[2], c[3],
    //        0,0,0,1,
    //    };
    //    return matrix;
    //}
    //
    //constexpr MatrixColumnMajor4x4i ConvertToMatrix44(const IntColumnMajor4x4 *in_matrix) {
    //    const v8si a = Load8Aligned(std::addressof((*in_matrix)[0,0]));
    //    const v8si b = Load8Aligned(std::addressof((*in_matrix)[2,0]));
    //    const MatrixColumnMajor4x4i ret = { a, b, };
    //    return ret;
    //}
    //
    //constexpr MatrixColumnMajor4x4i PromoteToMatrix44(const MatrixColumnMajor4x3i *in_matrix) {
    //    const v8si a = Load8Aligned(std::addressof((*in_matrix)[0,0]));
    //    const v8si b = Load8Aligned(std::addressof((*in_matrix)[2,0]));
    //    return MatrixColumnMajor4x4i { a, b, };
    //}

    constexpr Int2 ConvertToInt2(const Vector2i &in_vector) {
        return { in_vector[0], in_vector[1], };
    }
    constexpr Int3 ConvertToInt3(const Vector3i &in_vector) {
        return { in_vector[0], in_vector[1], in_vector[2], };
    }
    constexpr Int4 ConvertToInt4(const Vector4i &in_vector) {
        return { in_vector[0], in_vector[1], in_vector[2], in_vector[3], };
    }
}
