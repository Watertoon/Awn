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

namespace vp::res {
    class ByamlIterator;
}

namespace vp::util {

    /* Vectorized vector naming follows Vector{count}{Type abbriviation} */
    /* Non-vectorized vector naming follows {Type}{count} */
    #define VP_VECTOR_TRAITS(type, count, is_vectorized) \
        using UnderlyingType = type ; \
        static constexpr bool cIsVectorType = true; \
        static constexpr bool cIsVectorized = is_vectorized ; \
        static constexpr u32 cVectorCount = count ; \
        bool ApplyByamlArray(vp::res::ByamlIterator iter); \
        bool ApplyByamlDictionary(vp::res::ByamlIterator iter); \
        bool ApplyByamlDictionaryUpperCase(vp::res::ByamlIterator iter); \
        bool ApplyByamlDictionaryLowerCase(vp::res::ByamlIterator iter); \
        template <class Self> \
        constexpr auto &&operator[](this Self&& self, u32 index) { \
            if constexpr (cIsVectorized && sizeof(UnderlyingType) == sizeof(double) && 2 < cVectorCount && cSimdRegisterSize == 128) { \
                return std::forward<Self>(self).v[index / 2][index & 1]; \
            } else { \
                return std::forward<Self>(self).v[index]; \
            } \
        }

    /* Vectorized matrix naming follows Matrix{row_count}{column_count}{Row or Column}Major{Type abbriviation} */
    /* Non-vectorized matrix naming follows {Type}{row_count}{column_count}{Row or Column}Major */
    /* Matrices are addressed by operator[] always use [row_index][column_index] */
    /* Logically: Row | Col -> */
    /*                v        */
    #define VP_MATRIX_TRAITS(type, row_count, column_count, is_row_major, is_vectorized) \
        using UnderlyingType = type ; \
        static constexpr bool cIsMatrixType = true; \
        static constexpr bool cIsVectorized = is_vectorized ; \
        static constexpr bool cIsRowMajor = is_row_major ; \
        static constexpr bool cIsColumnMajor = ! is_row_major ; \
        static constexpr u32 cRowCount = row_count ; \
        static constexpr u32 cColumnCount = column_count ; \
        template <class Self> \
            requires (cIsRowMajor) \
        constexpr auto &&operator[](this Self&& self, u32 row, u32 column) { \
            const u32 index = column + row * sizeof( type ); \
            return std::forward<Self>(self).m[index]; \
        } \
        template <class Self> \
            requires (cIsColumnMajor && !(cIsVectorized && cRowCount == 4 && cColumnCount == 3)) \
        constexpr auto &&operator[](this Self&& self, u32 row, u32 column) { \
            const u32 index = row + column * sizeof( type ); \
            return std::forward<Self>(self).m[index]; \
        } \
        template <class Self> \
            requires (cIsColumnMajor && (cIsVectorized && cRowCount == 4 && cColumnCount == 3)) \
        constexpr auto &&operator[](this Self&& self, u32 row, u32 column) { \
            return std::forward<Self>(self).m[column][row]; \
        }

    struct Float2 {
        union {
            struct {
                float x;
                float y;
            };
            float v[2];
        };
        static const Float2 cUnitX;
        static const Float2 cUnitY;
        static const Float2 cZero;
        static const Float2 cOne;

        VP_VECTOR_TRAITS(float, 2, false);
    };
    static_assert(sizeof(Float2) == 0x8);
    static_assert(alignof(Float2) == 0x4);

    struct Float3 {
        union {
            struct {
                float x;
                float y;
                float z;
            };
            float v[3];
        };

        static const Float3 cUnitX;
        static const Float3 cUnitY;
        static const Float3 cUnitZ;
        static const Float3 cZero;
        static const Float3 cOne;

        VP_VECTOR_TRAITS(float, 3, false);
    };
    static_assert(sizeof(Float3) == 0xc);
    static_assert(alignof(Float3) == 0x4);

    struct Float4 {
        union {
            struct {
                float x;
                float y;
                float z;
                float w;
            };
            float v[4];
        };

        static const Float4 cUnitX;
        static const Float4 cUnitY;
        static const Float4 cUnitZ;
        static const Float4 cUnitW;
        static const Float4 cZero;
        static const Float4 cOne;

        VP_VECTOR_TRAITS(float, 4, false);
    };
    static_assert(sizeof(Float4) == 0x10);
    static_assert(alignof(Float4) == 0x4);

    struct FloatRowMajor3x3 {
        union {
            struct {
                float m00;
                float m01;
                float m02;
                float m10;
                float m11;
                float m12;
                float m20;
                float m21;
                float m22;
            };
            float m[9];
            Float3 array_row[3];
        };

        static const FloatRowMajor3x3 cIdentity;

        VP_MATRIX_TRAITS(float, 3, 3, true, false);
    };
    static_assert(sizeof(FloatRowMajor3x3) == 0x24);
    static_assert(alignof(FloatRowMajor3x3) == 0x4);

    struct FloatRowMajor4x3 {
        union {
            struct {
                float m00;
                float m01;
                float m02;
                float m10;
                float m11;
                float m12;
                float m20;
                float m21;
                float m22;
                float m30;
                float m31;
                float m32;
            };
            float m[12];
            Float3 array_row[4];
        };

        static const FloatRowMajor4x3 cIdentity;

        VP_MATRIX_TRAITS(float, 4, 3, true, false);
    };
    static_assert(sizeof(FloatRowMajor4x3) == 0x30);
    static_assert(alignof(FloatRowMajor4x3) == 0x4);

    struct FloatRowMajor4x4 {
        union {
            struct {
                float m00;
                float m01;
                float m02;
                float m03;
                float m10;
                float m11;
                float m12;
                float m13;
                float m20;
                float m21;
                float m22;
                float m23;
                float m30;
                float m31;
                float m32;
                float m33;
            };
            float m[16];
            Float4 array_row[4];
        };

        static const FloatRowMajor4x4 cIdentity;

        VP_MATRIX_TRAITS(float, 4, 4, true, false);
    };
    static_assert(sizeof(FloatRowMajor4x4) == 0x40);
    static_assert(alignof(FloatRowMajor4x4) == 0x4);

    struct FloatColumnMajor3x3 {
        union {
            struct {
                float m00;
                float m10;
                float m20;
                float m01;
                float m11;
                float m21;
                float m02;
                float m12;
                float m22;
            };
            float m[9];
            Float3 array_col[3];
        };

        static const FloatColumnMajor3x3 cIdentity;

        VP_MATRIX_TRAITS(float, 3, 3, false, false);
    };
    static_assert(sizeof(FloatColumnMajor3x3) == 0x24);
    static_assert(alignof(FloatColumnMajor3x3) == 0x4);

    struct FloatColumnMajor4x3 {
        union {
            struct {
                float m00;
                float m10;
                float m20;
                float m30;
                float m01;
                float m11;
                float m21;
                float m31;
                float m02;
                float m12;
                float m22;
                float m32;
            };
            float m[12];
            Float4 array_col[3];
        };

        static const FloatColumnMajor4x3 cIdentity;

        VP_MATRIX_TRAITS(float, 4, 3, false, false);
    };
    static_assert(sizeof(FloatColumnMajor4x3) == 0x30);
    static_assert(alignof(FloatColumnMajor4x3) == 0x4);

    struct FloatColumnMajor4x4 {
        union {
            struct {
                float m00;
                float m10;
                float m20;
                float m30;
                float m01;
                float m11;
                float m21;
                float m31;
                float m02;
                float m12;
                float m22;
                float m32;
                float m03;
                float m13;
                float m23;
                float m33;
            };
            float m[16];
            Float4 array_col[4];
        };

        static const FloatColumnMajor4x4 cIdentity;

        VP_MATRIX_TRAITS(float, 4, 4, false, false);
    };
    static_assert(sizeof(FloatColumnMajor4x4) == 0x40);
    static_assert(alignof(FloatColumnMajor4x4) == 0x4);

    struct Vector2f {
        v2f v;

        static const Vector2f cUnitX;
        static const Vector2f cUnitY;
        static const Vector2f cZero;
        static const Vector2f cOne;

        VP_VECTOR_TRAITS(float, 2, true);
    };
    static_assert(sizeof(Vector2f) == 0x8);
    static_assert(alignof(Vector2f) == 0x8);

    struct Vector3f {
        v4f v;

        static const Vector3f cUnitX;
        static const Vector3f cUnitY;
        static const Vector3f cUnitZ;
        static const Vector3f cZero;
        static const Vector3f cOne;

        VP_VECTOR_TRAITS(float, 3, true);
    };
    static_assert(sizeof(Vector3f) == 0x10);
    static_assert(alignof(Vector3f) == 0x10);

    struct Vector4f {
        v4f v;

        static const Vector4f cUnitX;
        static const Vector4f cUnitY;
        static const Vector4f cUnitZ;
        static const Vector4f cUnitW;
        static const Vector4f cZero;
        static const Vector4f cOne;

        VP_VECTOR_TRAITS(float, 4, true);
    };
    static_assert(sizeof(Vector4f) == 0x10);
    static_assert(alignof(Vector4f) == 0x10);

    using Quaternionf = Vector4f;

    struct MatrixRowMajor4x3f {
        v16f m;

        static const MatrixRowMajor4x3f cIdentity;

        VP_MATRIX_TRAITS(float, 4, 3, true, true);
    };
    static_assert(sizeof(MatrixRowMajor4x3f) == 0x40);

    struct MatrixRowMajor4x4f {
        v16f m;

        static const MatrixRowMajor4x4f cIdentity;

        VP_MATRIX_TRAITS(float, 4, 4, true, true);
    };
    static_assert(sizeof(MatrixRowMajor4x4f) == 0x40);

    struct MatrixColumnMajor4x3f {
        v4f m[3];

        static const MatrixColumnMajor4x3f cIdentity;

        VP_MATRIX_TRAITS(float, 4, 3, false, true);
    };
    static_assert(sizeof(MatrixColumnMajor4x3f) == 0x30);

    struct MatrixColumnMajor4x4f {
        v16f m;

        static const MatrixColumnMajor4x4f cIdentity;

        VP_MATRIX_TRAITS(float, 4, 4, false, true);
    };
    static_assert(sizeof(MatrixColumnMajor4x4f) == 0x40);

    constexpr const Float2 Float2::cUnitX = { .x = 1.0f };
    constexpr const Float2 Float2::cUnitY = { .y = 1.0f };
    constexpr const Float2 Float2::cZero  = {};
    constexpr const Float2 Float2::cOne   = { 1.0f, 1.0f };

    constexpr const Float3 Float3::cUnitX = { .x = 1.0f };
    constexpr const Float3 Float3::cUnitY = { .y = 1.0f };
    constexpr const Float3 Float3::cUnitZ = { .z = 1.0f };
    constexpr const Float3 Float3::cZero  = {};
    constexpr const Float3 Float3::cOne   = { 1.0f, 1.0f, 1.0f };

    constexpr const Float4 Float4::cUnitX = { .x = 1.0f };
    constexpr const Float4 Float4::cUnitY = { .y = 1.0f };
    constexpr const Float4 Float4::cUnitZ = { .z = 1.0f };
    constexpr const Float4 Float4::cUnitW = { .w = 1.0f };
    constexpr const Float4 Float4::cZero  = {};
    constexpr const Float4 Float4::cOne   = { 1.0f, 1.0f, 1.0f, 1.0f };

    constexpr const Vector2f Vector2f::cUnitX = { 1.0f, 0.0f, };
    constexpr const Vector2f Vector2f::cUnitY = { 0.0f, 1.0f, };
    constexpr const Vector2f Vector2f::cZero  = {};
    constexpr const Vector2f Vector2f::cOne   = { 1.0f, 1.0f };

    constexpr const Vector3f Vector3f::cUnitX = { 1.0f, 0.0f, 0.0f, };
    constexpr const Vector3f Vector3f::cUnitY = { 0.0f, 1.0f, 0.0f, };
    constexpr const Vector3f Vector3f::cUnitZ = { 0.0f, 0.0f, 1.0f, };
    constexpr const Vector3f Vector3f::cZero  = {};
    constexpr const Vector3f Vector3f::cOne   = { 1.0f, 1.0f, 1.0f };

    constexpr const Vector4f Vector4f::cUnitX = { 1.0f, 0.0f, 0.0f, 0.0f };
    constexpr const Vector4f Vector4f::cUnitY = { 0.0f, 1.0f, 0.0f, 0.0f };
    constexpr const Vector4f Vector4f::cUnitZ = { 0.0f, 0.0f, 1.0f, 0.0f };
    constexpr const Vector4f Vector4f::cUnitW = { 0.0f, 0.0f, 0.0f, 1.0f };
    constexpr const Vector4f Vector4f::cZero  = {};
    constexpr const Vector4f Vector4f::cOne   = { 1.0f, 1.0f, 1.0f, 1.0f };

    constexpr const FloatRowMajor3x3 FloatRowMajor3x3::cIdentity = {
        .m00 = 1.0f,
        .m11 = 1.0f,
        .m22 = 1.0f,
    };
    constexpr const FloatRowMajor4x3 FloatRowMajor4x3::cIdentity = {
        .m00 = 1.0f,
        .m11 = 1.0f,
        .m22 = 1.0f,
    };
    constexpr const FloatRowMajor4x4 FloatRowMajor4x4::cIdentity = {
        .m00 = 1.0f,
        .m11 = 1.0f,
        .m22 = 1.0f,
        .m33 = 1.0f,
    };
    constexpr const FloatColumnMajor3x3 FloatColumnMajor3x3::cIdentity = {
        .m00 = 1.0f,
        .m11 = 1.0f,
        .m22 = 1.0f,
    };
    constexpr const FloatColumnMajor4x3 FloatColumnMajor4x3::cIdentity = {
        .m00 = 1.0f,
        .m11 = 1.0f,
        .m22 = 1.0f,
    };
    constexpr const FloatColumnMajor4x4 FloatColumnMajor4x4::cIdentity = {
        .m00 = 1.0f,
        .m11 = 1.0f,
        .m22 = 1.0f,
        .m33 = 1.0f,
    };

    constexpr const MatrixRowMajor4x3f MatrixRowMajor4x3f::cIdentity = {
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f,
    };
    constexpr const MatrixRowMajor4x4f MatrixRowMajor4x4f::cIdentity = {
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f,
    };
    constexpr const MatrixColumnMajor4x3f MatrixColumnMajor4x3f::cIdentity = {
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
    };
    constexpr const MatrixColumnMajor4x4f MatrixColumnMajor4x4f::cIdentity = {
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f,
    };

    constexpr Vector2f ConvertToVector2(const Float2 &in_vector) {
        const float x = in_vector.x;
        const float y = in_vector.y;
        return Vector2f{ x, y, };
    }

    constexpr Vector3f ConvertToVector3(const Float3 &in_vector) {
        const float x = in_vector.x;
        const float y = in_vector.y;
        const float z = in_vector.z;
        return Vector3f{ x, y, z, };
    }

    constexpr Vector4f ConvertToVector4(const Float4 &in_vector) {
        const float x = in_vector.x;
        const float y = in_vector.y;
        const float z = in_vector.z;
        const float w = in_vector.w;
        return Vector4f{ x, y, z, w, };
    }

    constexpr Vector4f PromoteToVector4(const Vector3f &in_vector) {
        return Vector4f{ .v = in_vector.v, };
    }

    //constexpr MatrixRowMajor4x3f ConvertToMatrix43(const FloatRowMajor3x3 *in_matrix) {
    //
    //    const v4si mask = GetLoad4Mask128(true, true, true, false);
    //    const v4f a = Load4Mask128(std::addressof((*in_matrix)[0,0]), mask);
    //    const v4f b = Load4Mask128(std::addressof((*in_matrix)[1,0]), mask);
    //    const v4f c = Load4Mask128(std::addressof((*in_matrix)[2,0]), mask);
    //
    //    const MatrixRowMajor4x3f matrix = {
    //        a[0], a[1], a[2], a[3],
    //        b[0], b[1], b[2], b[3],
    //        c[0], c[1], c[2], c[3],
    //    };
    //    return matrix;
    //}
    //
    //constexpr MatrixRowMajor4x3f ConvertToMatrix43(const FloatRowMajor4x3 *in_matrix) {
    //
    //    const v4si mask = GetLoad4Mask128(true, true, true, false);
    //    const v4f a = Load4Mask128(std::addressof((*in_matrix)[0,0]), mask);
    //    const v4f b = Load4Mask128(std::addressof((*in_matrix)[1,0]), mask);
    //    const v4f c = Load4Mask128(std::addressof((*in_matrix)[2,0]), mask);
    //    const v4f d = Load4Mask128(std::addressof((*in_matrix)[3,0]), mask);
    //
    //    const MatrixRowMajor4x3f matrix = {
    //        a[0], a[1], a[2], a[3],
    //        b[0], b[1], b[2], b[3],
    //        c[0], c[1], c[2], c[3],
    //        d[0], d[1], d[2], d[3],
    //    };
    //    return matrix;
    //}
    //
    //constexpr MatrixRowMajor4x4f ConvertToMatrix44(const FloatRowMajor3x3 *in_matrix) {
    //
    //    const v4si mask = GetLoad4Mask128(true, true, true, false);
    //    const v4f a = Load4Mask128(std::addressof((*in_matrix)[0,0]), mask);
    //    const v4f b = Load4Mask128(std::addressof((*in_matrix)[1,0]), mask);
    //    const v4f c = Load4Mask128(std::addressof((*in_matrix)[2,0]), mask);
    //
    //    const MatrixRowMajor4x4f matrix = {
    //        a[0], a[1], a[2], a[3],
    //        b[0], b[1], b[2], b[3],
    //        c[0], c[1], c[2], c[3],
    //        0, 0, 0, 1,
    //    };
    //    return matrix;
    //}
    //
    //constexpr MatrixRowMajor4x4f ConvertToMatrix44(const FloatRowMajor4x3 *in_matrix) {
    //
    //    const v4si mask = GetLoad4Mask128(true, true, true, false);
    //    const v4f a = Load4Mask128(std::addressof((*in_matrix)[0,0]), mask);
    //    const v4f b = Load4Mask128(std::addressof((*in_matrix)[1,0]), mask);
    //    const v4f c = Load4Mask128(std::addressof((*in_matrix)[2,0]), mask);
    //    const v4f d = Load4Mask128(std::addressof((*in_matrix)[3,0]), mask);
    //
    //    const MatrixRowMajor4x4f matrix = {
    //        a[0], a[1], a[2], a[3],
    //        b[0], b[1], b[2], b[3],
    //        c[0], c[1], c[2], c[3],
    //        d[0], d[1], d[2], 1,
    //    };
    //    return matrix;
    //}
    //
    //constexpr MatrixRowMajor4x4f ConvertToMatrix44(const FloatRowMajor4x4 *in_matrix) {
    //    const v8f a = Load8(std::addressof((*in_matrix)[0, 0]));
    //    const v8f b = Load8(std::addressof((*in_matrix)[2, 0]));
    //    return MatrixRowMajor4x4f { a, b, };
    //}
    //
    //constexpr MatrixRowMajor4x4f PromoteMatrix44(const MatrixRowMajor4x3f *in_matrix) {
    //    return MatrixRowMajor4x4f { .m = in_matrix->m, };
    //}
    //
    //constexpr MatrixColumnMajor4x3f ConvertToMatrix43(const FloatColumnMajor3x3 *in_matrix) {
    //
    //    const v4si mask = GetLoad4Mask128(true, true, true, false);
    //    const v4f a = Load4Mask128(std::addressof((*in_matrix)[0,0]), mask);
    //    const v4f b = Load4Mask128(std::addressof((*in_matrix)[1,0]), mask);
    //    const v4f c = Load4Mask128(std::addressof((*in_matrix)[2,0]), mask);
    //
    //    return MatrixColumnMajor4x3f { a, b, c, };
    //}
    //
    //constexpr MatrixColumnMajor4x3f ConvertToMatrix43(const FloatColumnMajor4x3 *in_matrix) {
    //
    //    const v4f a = Load4(std::addressof((*in_matrix)[0,0]));
    //    const v4f b = Load4(std::addressof((*in_matrix)[1,0]));
    //    const v4f c = Load4(std::addressof((*in_matrix)[2,0]));
    //
    //    return MatrixColumnMajor4x3f { a, b, c, };
    //}
    //
    //constexpr MatrixColumnMajor4x4f ConvertToMatrix44(const FloatColumnMajor3x3 *in_matrix) {
    //
    //    const v4si mask = GetLoad4Mask128(true, true, true, false);
    //    const v4f a = Load4Mask128(std::addressof((*in_matrix)[0,0]), mask);
    //    const v4f b = Load4Mask128(std::addressof((*in_matrix)[1,0]), mask);
    //    const v4f c = Load4Mask128(std::addressof((*in_matrix)[2,0]), mask);
    //
    //    const MatrixColumnMajor4x4f matrix = {
    //        a[0], a[1], a[2], a[3],
    //        b[0], b[1], b[2], b[3],
    //        c[0], c[1], c[2], c[3],
    //        0,0,0,1,
    //    };
    //    return matrix;
    //}
    //
    //constexpr MatrixColumnMajor4x4f ConvertToMatrix44(const FloatColumnMajor4x3 *in_matrix) {
    //
    //    const v4f a = Load4(std::addressof((*in_matrix)[0,0]));
    //    const v4f b = Load4(std::addressof((*in_matrix)[1,0]));
    //    const v4f c = Load4(std::addressof((*in_matrix)[2,0]));
    //
    //    const MatrixColumnMajor4x4f matrix = {
    //        a[0], a[1], a[2], a[3],
    //        b[0], b[1], b[2], b[3],
    //        c[0], c[1], c[2], c[3],
    //        0,0,0,1,
    //    };
    //    return matrix;
    //}
    //
    //constexpr MatrixColumnMajor4x4f ConvertToMatrix44(const FloatColumnMajor4x4 *in_matrix) {
    //    const v8f a = Load8(std::addressof((*in_matrix)[0,0]));
    //    const v8f b = Load8(std::addressof((*in_matrix)[2,0]));
    //    return MatrixColumnMajor4x4f { a, b, };
    //}
    //
    //constexpr MatrixColumnMajor4x4f PromoteToMatrix44(const MatrixColumnMajor4x3f *in_matrix) {
    //    const v8f a = Load8Aligned(std::addressof((*in_matrix)[0,0]));
    //    const v8f b = Load8Aligned(std::addressof((*in_matrix)[2,0]));
    //    return MatrixColumnMajor4x4f {  a, b, };
    //}

    constexpr Float2 ConvertToFloat2(const Vector2f &in_vector) {
        const float x = in_vector[0];
        const float y = in_vector[1];
        return { x, y, };
    }

    constexpr Float3 ConvertToFloat3(const Vector3f &in_vector) {
        const float x = in_vector[0];
        const float y = in_vector[1];
        const float z = in_vector[2];
        return { x, y, z, };
    }

    constexpr Float4 ConvertToFloat4(const Vector4f &in_vector) {
        const float x = in_vector[0];
        const float y = in_vector[1];
        const float z = in_vector[2];
        const float w = in_vector[3];
        return { x, y, z, w, };
    }
}
