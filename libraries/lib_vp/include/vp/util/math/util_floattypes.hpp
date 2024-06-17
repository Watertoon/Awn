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

    struct Float2 {
        union {
            struct {
                float x;
                float y;
            };
            float array[2];
        };

        using VectorType                    = float;
        static constexpr bool cIsVectorType = true;
        static constexpr bool cIsVectorized = false;
        static constexpr bool cVectorCount  = 2;

        static const Float2 cUnitX;
        static const Float2 cUnitY;
        static const Float2 cZero;
        static const Float2 cOne;
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
            float array[3];
        };

        using VectorType                    = float;
        static constexpr bool cIsVectorType = true;
        static constexpr bool cIsVectorized = false;
        static constexpr bool cVectorCount  = 3;

        static const Float3 cUnitX;
        static const Float3 cUnitY;
        static const Float3 cUnitZ;
        static const Float3 cZero;
        static const Float3 cOne;
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
            float array[4];
        };

        using VectorType                    = float;
        static constexpr bool cIsVectorType = true;
        static constexpr bool cIsVectorized = false;
        static constexpr bool cVectorCount  = 4;

        static const Float4 cUnitX;
        static const Float4 cUnitY;
        static const Float4 cUnitZ;
        static const Float4 cUnitW;
        static const Float4 cZero;
        static const Float4 cOne;
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
            Float3 array_row[3];
            float  array_2d[3][3];
        };
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
            Float3 array_row[4];
            float  array_2d[4][3];
        };
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
            Float4 array_row[4];
            float  array_2d[4][4];
        };
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
            Float3 array_col[3];
            float  array_2d[3][3];
        };
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
            Float4 array_col[3];
            float  array_2d[3][4];
        };
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
            Float4 array_col[4];
            float  array_2d[4][4];
        };
    };
    static_assert(sizeof(FloatColumnMajor4x4) == 0x40);
    static_assert(alignof(FloatColumnMajor4x4) == 0x4);

    using Float2Vector    = float __attribute__((vector_size(sizeof(float) * 2)));
    using Float3Vector    = float __attribute__((vector_size(sizeof(float) * 4)));
    using Float4Vector    = float __attribute__((vector_size(sizeof(float) * 4)));
    using Matrix33fVector = float __attribute__((vector_size(sizeof(float) * 16)));
    using Matrix34fVector = float __attribute__((vector_size(sizeof(float) * 16)));
    using Matrix44fVector = float __attribute__((vector_size(sizeof(float) * 16)));

    struct Vector2f {
        union {
            struct {
                float x;
                float y;
            };
            float array[2];
            Float2Vector vector;
        };

        using VectorType                    = float;
        static constexpr bool cIsVectorType = true;
        static constexpr bool cIsVectorized = true;
        static constexpr bool cVectorCount  = 2;

        static const Vector2f cUnitX;
        static const Vector2f cUnitY;
        static const Vector2f cZero;
        static const Vector2f cOne;
    };
    static_assert(sizeof(Vector2f) == 0x8);
    static_assert(alignof(Vector2f) == 0x8);

    struct Vector3f {
        union {
            struct {
                float x;
                float y;
                float z;
                float w;
            };
            float array[4];
            Float3Vector vector;
        };

        using VectorType                    = float;
        static constexpr bool cIsVectorType = true;
        static constexpr bool cIsVectorized = true;
        static constexpr bool cVectorCount  = 3;


        static const Vector3f cUnitX;
        static const Vector3f cUnitY;
        static const Vector3f cUnitZ;
        static const Vector3f cZero;
        static const Vector3f cOne;
    };
    static_assert(sizeof(Vector3f) == 0x10);
    static_assert(alignof(Vector3f) == 0x10);

    struct Vector4f {
        union {
            struct {
                float x;
                float y;
                float z;
                float w;
            };
            float array[4];
            Float4Vector vector;
        };

        using VectorType                    = float;
        static constexpr bool cIsVectorType = true;
        static constexpr bool cIsVectorized = true;
        static constexpr bool cVectorCount  = 4;


        static const Vector4f cUnitX;
        static const Vector4f cUnitY;
        static const Vector4f cUnitZ;
        static const Vector4f cUnitW;
        static const Vector4f cZero;
        static const Vector4f cOne;
    };
    static_assert(sizeof(Vector4f) == 0x10);
    static_assert(alignof(Vector4f) == 0x10);

    struct MatrixRowMajor4x3f {
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
            Matrix34fVector vector;
            Float3         array_row[4];
            float          array_2d[4][3];
        };

        static const MatrixRowMajor4x3f cIdentity;
    };
    static_assert(sizeof(MatrixRowMajor4x3f) == 0x40);

    struct MatrixRowMajor4x4f {
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
            Matrix44fVector vector;
            Vector4f       array_row[4];
            float          array_2d[4][4];
        };

        static const MatrixRowMajor4x4f cIdentity;
    };
    static_assert(sizeof(MatrixRowMajor4x4f) == 0x40);

    struct MatrixColumnMajor4x3f {
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
            Matrix34fVector vector;
            Vector4f       array_col[3];
            float          array_2d[3][4];
        };

        static const MatrixColumnMajor4x3f cIdentity;
    };
    static_assert(sizeof(MatrixColumnMajor4x3f) == 0x40);

    struct MatrixColumnMajor4x4f {
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
            Matrix44fVector vector;
            Vector4f       array_col[4];
            float          array_2d[4][4];
        };

        static const MatrixColumnMajor4x4f cIdentity;
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

    constexpr const Vector2f Vector2f::cUnitX = { .x = 1.0f };
    constexpr const Vector2f Vector2f::cUnitY = { .y = 1.0f };
    constexpr const Vector2f Vector2f::cZero  = {};
    constexpr const Vector2f Vector2f::cOne   = { 1.0f, 1.0f };

    constexpr const Vector3f Vector3f::cUnitX = { .x = 1.0f };
    constexpr const Vector3f Vector3f::cUnitY = { .y = 1.0f };
    constexpr const Vector3f Vector3f::cUnitZ = { .z = 1.0f };
    constexpr const Vector3f Vector3f::cZero  = {};
    constexpr const Vector3f Vector3f::cOne   = { 1.0f, 1.0f, 1.0f };

    constexpr const Vector4f Vector4f::cUnitX = { .x = 1.0f };
    constexpr const Vector4f Vector4f::cUnitY = { .y = 1.0f };
    constexpr const Vector4f Vector4f::cUnitZ = { .z = 1.0f };
    constexpr const Vector4f Vector4f::cUnitW = { .w = 1.0f };
    constexpr const Vector4f Vector4f::cZero  = {};
    constexpr const Vector4f Vector4f::cOne   = { 1.0f, 1.0f, 1.0f, 1.0f };

    constexpr const MatrixRowMajor4x3f MatrixRowMajor4x3f::cIdentity = {
        .m00 = 1.0f,
        .m10 = 1.0f,
        .m20 = 1.0f,
        .m30 = 1.0f,
    };
    constexpr const MatrixRowMajor4x4f MatrixRowMajor4x4f::cIdentity = {
        .m00 = 1.0f,
        .m10 = 1.0f,
        .m20 = 1.0f,
        .m30 = 1.0f,
    };
    constexpr const MatrixColumnMajor4x3f MatrixColumnMajor4x3f::cIdentity = {
        .m00 = 1.0f,
        .m10 = 1.0f,
        .m20 = 1.0f,
        .m30 = 1.0f,
    };
    constexpr const MatrixColumnMajor4x4f MatrixColumnMajor4x4f::cIdentity = {
        .m00 = 1.0f,
        .m10 = 1.0f,
        .m20 = 1.0f,
        .m30 = 1.0f,
    };

    constexpr Vector2f ConvertVector(const Float2 &in_vector) {
        return { in_vector.x, in_vector.y, };
    }
    constexpr Vector3f ConvertVector(const Float3 &in_vector) {
        return { in_vector.x, in_vector.y, in_vector.z, };
    }
    constexpr Vector4f ConvertVector(const Float4 &in_vector) {
        return { in_vector.x, in_vector.y, in_vector.z, in_vector.w, };
    }
    constexpr Vector4f ConvertVector(const Vector3f &in_vector) {
        return { in_vector.x, in_vector.y, in_vector.z, 0.0f, };
    }
    constexpr Vector4f ConvertVectorUnit(const Vector3f &in_vector) {
        return { in_vector.x, in_vector.y, in_vector.z, 1.0f, };
    }

    constexpr void ConvertMatrix(MatrixRowMajor4x3f *out_matrix, const FloatRowMajor3x3 *in_matrix) {

        const Float3 vector0 = in_matrix->array_row[0];
        const Float3 vector1 = in_matrix->array_row[1];
        const Float3 vector2 = in_matrix->array_row[2];

        out_matrix->array_row[0] = vector0;
        out_matrix->array_row[1] = vector1;
        out_matrix->array_row[2] = vector2;
        out_matrix->array_row[3] = Float3::cZero;
    }

    constexpr void ConvertMatrix(MatrixRowMajor4x3f *out_matrix, const FloatRowMajor4x3 *in_matrix) {

        const Float3 vector0 = in_matrix->array_row[0];
        const Float3 vector1 = in_matrix->array_row[1];
        const Float3 vector2 = in_matrix->array_row[2];
        const Float3 vector3 = in_matrix->array_row[3];

        out_matrix->array_row[0] = vector0;
        out_matrix->array_row[1] = vector1;
        out_matrix->array_row[2] = vector2;
        out_matrix->array_row[3] = vector3;
    }

    constexpr void ConvertMatrix(MatrixRowMajor4x4f *out_matrix, const FloatRowMajor3x3 *in_matrix) {

        const Vector4f vector0 = ConvertVector(ConvertVector(in_matrix->array_row[0]));
        const Vector4f vector1 = ConvertVector(ConvertVector(in_matrix->array_row[1]));
        const Vector4f vector2 = ConvertVector(ConvertVector(in_matrix->array_row[2]));

        out_matrix->array_row[0] = vector0;
        out_matrix->array_row[1] = vector1;
        out_matrix->array_row[2] = vector2;
        out_matrix->array_row[3] = Vector4f::cUnitW;
    }

    constexpr void ConvertMatrix(MatrixRowMajor4x4f *out_matrix, const FloatRowMajor4x3 *in_matrix) {

        const Vector4f vector0 = ConvertVector(ConvertVector(in_matrix->array_row[0]));
        const Vector4f vector1 = ConvertVector(ConvertVector(in_matrix->array_row[1]));
        const Vector4f vector2 = ConvertVector(ConvertVector(in_matrix->array_row[2]));
        const Vector4f vector3 = ConvertVectorUnit(ConvertVector(in_matrix->array_row[3]));

        out_matrix->array_row[0] = vector0;
        out_matrix->array_row[1] = vector1;
        out_matrix->array_row[2] = vector2;
        out_matrix->array_row[3] = vector3;
    }

    constexpr void ConvertMatrix(MatrixRowMajor4x4f *out_matrix, const FloatRowMajor4x4 *in_matrix) {

        const Vector4f vector0 = ConvertVector(in_matrix->array_row[0]);
        const Vector4f vector1 = ConvertVector(in_matrix->array_row[1]);
        const Vector4f vector2 = ConvertVector(in_matrix->array_row[2]);
        const Vector4f vector3 = ConvertVector(in_matrix->array_row[3]);

        out_matrix->array_row[0] = vector0;
        out_matrix->array_row[1] = vector1;
        out_matrix->array_row[2] = vector2;
        out_matrix->array_row[3] = vector3;
    }

    constexpr void ConvertMatrix(MatrixRowMajor4x4f *out_matrix, const MatrixRowMajor4x3f *in_matrix) {

        const Vector4f vector0 = ConvertVector(ConvertVector(in_matrix->array_row[0]));
        const Vector4f vector1 = ConvertVector(ConvertVector(in_matrix->array_row[1]));
        const Vector4f vector2 = ConvertVector(ConvertVector(in_matrix->array_row[2]));
        const Vector4f vector3 = ConvertVectorUnit(ConvertVector(in_matrix->array_row[3]));

        out_matrix->array_row[0] = vector0;
        out_matrix->array_row[1] = vector1;
        out_matrix->array_row[2] = vector2;
        out_matrix->array_row[3] = vector3;
    }

    constexpr void ConvertMatrix(MatrixColumnMajor4x3f *out_matrix, const FloatColumnMajor3x3 *in_matrix) {

        const Vector4f vector0 = ConvertVector(ConvertVector(in_matrix->array_col[0]));
        const Vector4f vector1 = ConvertVector(ConvertVector(in_matrix->array_col[1]));
        const Vector4f vector2 = ConvertVector(ConvertVector(in_matrix->array_col[2]));

        out_matrix->array_col[0] = vector0;
        out_matrix->array_col[1] = vector1;
        out_matrix->array_col[2] = vector2;
        out_matrix->array_col[3] = Vector4f::cZero;
    }

    constexpr void ConvertMatrix(MatrixColumnMajor4x3f *out_matrix, const FloatColumnMajor4x3 *in_matrix) {

        const Vector4f vector0 = ConvertVector(in_matrix->array_col[0]);
        const Vector4f vector1 = ConvertVector(in_matrix->array_col[1]);
        const Vector4f vector2 = ConvertVector(in_matrix->array_col[2]);
        out_matrix->array_col[0] = vector0;
        out_matrix->array_col[1] = vector1;
        out_matrix->array_col[2] = vector2;
        out_matrix->array_col[3] = Vector4f::cZero;
    }

    constexpr void ConvertMatrix(MatrixColumnMajor4x4f *out_matrix, const FloatColumnMajor3x3 *in_matrix) {

        const Vector4f vector0 = ConvertVector(ConvertVector(in_matrix->array_col[0]));
        const Vector4f vector1 = ConvertVector(ConvertVector(in_matrix->array_col[1]));
        const Vector4f vector2 = ConvertVector(ConvertVector(in_matrix->array_col[2]));

        out_matrix->array_col[0] = vector0;
        out_matrix->array_col[1] = vector1;
        out_matrix->array_col[2] = vector2;
        out_matrix->array_col[3] = Vector4f::cUnitW;
    }

    constexpr void ConvertMatrix(MatrixColumnMajor4x4f *out_matrix, const FloatColumnMajor4x3 *in_matrix) {

        const Vector4f vector0 = ConvertVector(in_matrix->array_col[0]);
        const Vector4f vector1 = ConvertVector(in_matrix->array_col[1]);
        const Vector4f vector2 = ConvertVector(in_matrix->array_col[2]);

        out_matrix->array_col[0] = vector0;
        out_matrix->array_col[1] = vector1;
        out_matrix->array_col[2] = vector2;
        out_matrix->array_col[3] = Vector4f::cUnitW;
    }

    constexpr void ConvertMatrix(MatrixColumnMajor4x4f *out_matrix, const FloatColumnMajor4x4 *in_matrix) {

        const Vector4f vector0 = ConvertVector(in_matrix->array_col[0]);
        const Vector4f vector1 = ConvertVector(in_matrix->array_col[1]);
        const Vector4f vector2 = ConvertVector(in_matrix->array_col[2]);
        const Vector4f vector3 = ConvertVector(in_matrix->array_col[3]);

        out_matrix->array_col[0] = vector0;
        out_matrix->array_col[1] = vector1;
        out_matrix->array_col[2] = vector2;
        out_matrix->array_col[3] = vector3;
    }

    constexpr void ConvertMatrix(MatrixColumnMajor4x4f *out_matrix, const MatrixColumnMajor4x3f *in_matrix) {

        const Vector4f vector0 = in_matrix->array_col[0];
        const Vector4f vector1 = in_matrix->array_col[1];
        const Vector4f vector2 = in_matrix->array_col[2];

        out_matrix->array_col[0] = vector0;
        out_matrix->array_col[1] = vector1;
        out_matrix->array_col[2] = vector2;
        out_matrix->array_col[3] = Vector4f::cUnitW;
    }

    constexpr Float2 ConvertFloat(const Vector2f &in_vector) {
        return { in_vector.x, in_vector.y };
    }
    constexpr Float3 ConvertFloat(const Vector3f &in_vector) {
        return { in_vector.x, in_vector.y, in_vector.z };
    }
    constexpr Float4 ConvertFloat(const Vector4f &in_vector) {
        return { in_vector.x, in_vector.y, in_vector.z, in_vector.w };
    }
}
