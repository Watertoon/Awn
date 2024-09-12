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
            s32 array[2];
        };

        using VectorType                    = s32;
        static constexpr bool cIsVectorType = true;
        static constexpr bool cIsVectorized = false;
        static constexpr u32 cVectorCount  = 2;

        static const Int2 cUnitX;
        static const Int2 cUnitY;
        static const Int2 cZero;
        static const Int2 cOne;
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
            s32 array[3];
        };

        using VectorType                    = s32;
        static constexpr bool cIsVectorType = true;
        static constexpr bool cIsVectorized = false;
        static constexpr u32 cVectorCount  = 3;

        static const Int3 cUnitX;
        static const Int3 cUnitY;
        static const Int3 cUnitZ;
        static const Int3 cZero;
        static const Int3 cOne;
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

        using VectorType                    = s32;
        static constexpr bool cIsVectorType = true;
        static constexpr bool cIsVectorized = false;
        static constexpr u32 cVectorCount  = 4;

        static const Int4 cUnitX;
        static const Int4 cUnitY;
        static const Int4 cUnitZ;
        static const Int4 cUnitW;
        static const Int4 cZero;
        static const Int4 cOne;
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
            Int3 array_row[3];
            s32  array_2d[3][3];
        };
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
            Int3 array_row[4];
            s32  array_2d[4][3];
        };
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
            Int4 array_row[4];
            s32  array_2d[4][4];
        };
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
            Int3 array_col[3];
            s32  array_2d[3][3];
        };
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
            Int4 array_col[3];
            s32  array_2d[3][4];
        };
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
            Int4 array_col[4];
            s32  array_2d[4][4];
        };
    };
    static_assert(sizeof(IntColumnMajor4x4) == 0x40);
    static_assert(alignof(IntColumnMajor4x4) == 0x4);
    using Int2Vector      = s32 __attribute__((vector_size(sizeof(s32) * 2)));
    using Int3Vector      = s32 __attribute__((vector_size(sizeof(s32) * 4)));
    using Int4Vector      = s32 __attribute__((vector_size(sizeof(s32) * 4)));
    using Matrix33iVector = s32 __attribute__((vector_size(sizeof(s32) * 16)));
    using Matrix34iVector = s32 __attribute__((vector_size(sizeof(s32) * 16)));
    using Matrix44iVector = s32 __attribute__((vector_size(sizeof(s32) * 16)));

    struct Vector2i {
        union {
            struct {
                s32 x;
                s32 y;
            };
            s32 array[2];
            Int2Vector vector;
        };


        using VectorType                    = s32;
        static constexpr bool cIsVectorType = true;
        static constexpr bool cIsVectorized = true;
        static constexpr u32 cVectorCount  = 2;

        static const Vector2i cUnitX;
        static const Vector2i cUnitY;
        static const Vector2i cZero;
        static const Vector2i cOne;
    };
    static_assert(sizeof(Vector2i) == 0x8);
    static_assert(alignof(Vector2i) == 0x8);

    struct Vector3i {
        union {
            struct {
                s32 x;
                s32 y;
                s32 z;
                s32 w;
            };
            s32 array[4];
            Int3Vector vector;
        };

        using VectorType                    = s32;
        static constexpr bool cIsVectorType = true;
        static constexpr bool cIsVectorized = true;
        static constexpr u32 cVectorCount  = 3;

        static const Vector3i cUnitX;
        static const Vector3i cUnitY;
        static const Vector3i cUnitZ;
        static const Vector3i cZero;
        static const Vector3i cOne;
    };
    static_assert(sizeof(Vector3i) == 0x10);
    static_assert(alignof(Vector3i) == 0x10);

    struct Vector4i {
        union {
            struct {
                s32 x;
                s32 y;
                s32 z;
                s32 w;
            };
            s32 array[4];
            Int4Vector vector;
        };

        using VectorType                    = s32;
        static constexpr bool cIsVectorType = true;
        static constexpr bool cIsVectorized = true;
        static constexpr u32 cVectorCount  = 4;

        static const Vector4i cUnitX;
        static const Vector4i cUnitY;
        static const Vector4i cUnitZ;
        static const Vector4i cUnitW;
        static const Vector4i cZero;
        static const Vector4i cOne;
    };
    static_assert(sizeof(Vector4i) == 0x10);
    static_assert(alignof(Vector4i) == 0x10);

    struct MatrixRowMajor4x3i {
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
            Matrix34iVector vector;
            Int3            array_row[4];
            s32             array_2d[4][3];
        };

        static const MatrixRowMajor4x3i cIdentity;
    };
    static_assert(sizeof(MatrixRowMajor4x3i) == 0x40);

    struct MatrixRowMajor4x4i {
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
            Matrix44iVector vector;
            Vector4i       array_row[4];
            s32          array_2d[4][4];
        };

        static const MatrixRowMajor4x4i cIdentity;
    };
    static_assert(sizeof(MatrixRowMajor4x4i) == 0x40);

    struct MatrixColumnMajor4x3i {
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
            Matrix34iVector vector;
            Vector4i       array_col[3];
            s32          array_2d[3][4];
        };

        static const MatrixColumnMajor4x3i cIdentity;
    };
    static_assert(sizeof(MatrixColumnMajor4x3i) == 0x40);

    struct MatrixColumnMajor4x4i {
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
            Matrix44iVector vector;
            Vector4i        array_col[4];
            s32             array_2d[4][4];
        };

        static const MatrixColumnMajor4x4i cIdentity;
    };
    static_assert(sizeof(MatrixColumnMajor4x4i) == 0x40);

    constexpr const Int2 Int2::cUnitX = { .x = 1 };
    constexpr const Int2 Int2::cUnitY = { .y = 1 };
    constexpr const Int2 Int2::cZero  = {};
    constexpr const Int2 Int2::cOne   = { 1, 1 };

    constexpr const Int3 Int3::cUnitX = { .x = 1 };
    constexpr const Int3 Int3::cUnitY = { .y = 1 };
    constexpr const Int3 Int3::cUnitZ = { .z = 1 };
    constexpr const Int3 Int3::cZero  = {};
    constexpr const Int3 Int3::cOne   = { 1, 1, 1 };

    constexpr const Int4 Int4::cUnitX = { .x = 1 };
    constexpr const Int4 Int4::cUnitY = { .y = 1 };
    constexpr const Int4 Int4::cUnitZ = { .z = 1 };
    constexpr const Int4 Int4::cUnitW = { .w = 1 };
    constexpr const Int4 Int4::cZero  = {};
    constexpr const Int4 Int4::cOne   = { 1, 1, 1, 1 };

    constexpr const Vector2i Vector2i::cUnitX = { .x = 1 };
    constexpr const Vector2i Vector2i::cUnitY = { .y = 1 };
    constexpr const Vector2i Vector2i::cZero  = {};
    constexpr const Vector2i Vector2i::cOne   = { 1, 1 };

    constexpr const Vector3i Vector3i::cUnitX = { .x = 1 };
    constexpr const Vector3i Vector3i::cUnitY = { .y = 1 };
    constexpr const Vector3i Vector3i::cUnitZ = { .z = 1 };
    constexpr const Vector3i Vector3i::cZero  = {};
    constexpr const Vector3i Vector3i::cOne   = { 1, 1, 1 };

    constexpr const Vector4i Vector4i::cUnitX = { .x = 1 };
    constexpr const Vector4i Vector4i::cUnitY = { .y = 1 };
    constexpr const Vector4i Vector4i::cUnitZ = { .z = 1 };
    constexpr const Vector4i Vector4i::cUnitW = { .w = 1 };
    constexpr const Vector4i Vector4i::cZero  = {};
    constexpr const Vector4i Vector4i::cOne   = { 1, 1, 1, 1 };

    constexpr const MatrixRowMajor4x3i MatrixRowMajor4x3i::cIdentity = {
        .m00 = 1,
        .m10 = 1,
        .m20 = 1,
        .m30 = 1,
    };
    constexpr const MatrixRowMajor4x4i MatrixRowMajor4x4i::cIdentity = {
        .m00 = 1,
        .m10 = 1,
        .m20 = 1,
        .m30 = 1,
    };
    constexpr const MatrixColumnMajor4x3i MatrixColumnMajor4x3i::cIdentity = {
        .m00 = 1,
        .m10 = 1,
        .m20 = 1,
        .m30 = 1,
    };
    constexpr const MatrixColumnMajor4x4i MatrixColumnMajor4x4i::cIdentity = {
        .m00 = 1,
        .m10 = 1,
        .m20 = 1,
        .m30 = 1,
    };

    constexpr Vector2i ConvertVector(const Int2 &in_vector) {
        return { in_vector.x, in_vector.y, };
    }
    constexpr Vector3i ConvertVector(const Int3 &in_vector) {
        return { in_vector.x, in_vector.y, in_vector.z, };
    }
    constexpr Vector4i ConvertVector(const Int4 &in_vector) {
        return { in_vector.x, in_vector.y, in_vector.z, in_vector.w, };
    }
    constexpr Vector4i ConvertVector(const Vector3i &in_vector) {
        return { in_vector.x, in_vector.y, in_vector.z, 0, };
    }
    constexpr Vector4i ConvertVectorUnit(const Vector3i &in_vector) {
        return { in_vector.x, in_vector.y, in_vector.z, 1, };
    }

    constexpr void ConvertMatrix(MatrixRowMajor4x3i *out_matrix, const IntRowMajor3x3 *in_matrix) {

        const Int3 vector0 = in_matrix->array_row[0];
        const Int3 vector1 = in_matrix->array_row[1];
        const Int3 vector2 = in_matrix->array_row[2];

        out_matrix->array_row[0] = vector0;
        out_matrix->array_row[1] = vector1;
        out_matrix->array_row[2] = vector2;
        out_matrix->array_row[3] = Int3::cZero;
    }

    constexpr void ConvertMatrix(MatrixRowMajor4x3i *out_matrix, const IntRowMajor4x3 *in_matrix) {

        const Int3 vector0 = in_matrix->array_row[0];
        const Int3 vector1 = in_matrix->array_row[1];
        const Int3 vector2 = in_matrix->array_row[2];
        const Int3 vector3 = in_matrix->array_row[3];

        out_matrix->array_row[0] = vector0;
        out_matrix->array_row[1] = vector1;
        out_matrix->array_row[2] = vector2;
        out_matrix->array_row[3] = vector3;
    }

    constexpr void ConvertMatrix(MatrixRowMajor4x4i *out_matrix, const IntRowMajor3x3 *in_matrix) {

        const Vector4i vector0 = ConvertVector(ConvertVector(in_matrix->array_row[0]));
        const Vector4i vector1 = ConvertVector(ConvertVector(in_matrix->array_row[1]));
        const Vector4i vector2 = ConvertVector(ConvertVector(in_matrix->array_row[2]));

        out_matrix->array_row[0] = vector0;
        out_matrix->array_row[1] = vector1;
        out_matrix->array_row[2] = vector2;
        out_matrix->array_row[3] = Vector4i::cUnitW;
    }

    constexpr void ConvertMatrix(MatrixRowMajor4x4i *out_matrix, const IntRowMajor4x3 *in_matrix) {

        const Vector4i vector0 = ConvertVector(ConvertVector(in_matrix->array_row[0]));
        const Vector4i vector1 = ConvertVector(ConvertVector(in_matrix->array_row[1]));
        const Vector4i vector2 = ConvertVector(ConvertVector(in_matrix->array_row[2]));
        const Vector4i vector3 = ConvertVectorUnit(ConvertVector(in_matrix->array_row[3]));

        out_matrix->array_row[0] = vector0;
        out_matrix->array_row[1] = vector1;
        out_matrix->array_row[2] = vector2;
        out_matrix->array_row[3] = vector3;
    }

    constexpr void ConvertMatrix(MatrixRowMajor4x4i *out_matrix, const IntRowMajor4x4 *in_matrix) {

        const Vector4i vector0 = ConvertVector(in_matrix->array_row[0]);
        const Vector4i vector1 = ConvertVector(in_matrix->array_row[1]);
        const Vector4i vector2 = ConvertVector(in_matrix->array_row[2]);
        const Vector4i vector3 = ConvertVector(in_matrix->array_row[3]);

        out_matrix->array_row[0] = vector0;
        out_matrix->array_row[1] = vector1;
        out_matrix->array_row[2] = vector2;
        out_matrix->array_row[3] = vector3;
    }

    constexpr void ConvertMatrix(MatrixRowMajor4x4i *out_matrix, const MatrixRowMajor4x3i *in_matrix) {

        const Vector4i vector0 = ConvertVector(ConvertVector(in_matrix->array_row[0]));
        const Vector4i vector1 = ConvertVector(ConvertVector(in_matrix->array_row[1]));
        const Vector4i vector2 = ConvertVector(ConvertVector(in_matrix->array_row[2]));
        const Vector4i vector3 = ConvertVectorUnit(ConvertVector(in_matrix->array_row[3]));

        out_matrix->array_row[0] = vector0;
        out_matrix->array_row[1] = vector1;
        out_matrix->array_row[2] = vector2;
        out_matrix->array_row[3] = vector3;
    }

    constexpr void ConvertMatrix(MatrixColumnMajor4x3i *out_matrix, const IntColumnMajor3x3 *in_matrix) {

        const Vector4i vector0 = ConvertVector(ConvertVector(in_matrix->array_col[0]));
        const Vector4i vector1 = ConvertVector(ConvertVector(in_matrix->array_col[1]));
        const Vector4i vector2 = ConvertVector(ConvertVector(in_matrix->array_col[2]));

        out_matrix->array_col[0] = vector0;
        out_matrix->array_col[1] = vector1;
        out_matrix->array_col[2] = vector2;
        out_matrix->array_col[3] = Vector4i::cZero;
    }

    constexpr void ConvertMatrix(MatrixColumnMajor4x3i *out_matrix, const IntColumnMajor4x3 *in_matrix) {

        const Vector4i vector0 = ConvertVector(in_matrix->array_col[0]);
        const Vector4i vector1 = ConvertVector(in_matrix->array_col[1]);
        const Vector4i vector2 = ConvertVector(in_matrix->array_col[2]);
        out_matrix->array_col[0] = vector0;
        out_matrix->array_col[1] = vector1;
        out_matrix->array_col[2] = vector2;
        out_matrix->array_col[3] = Vector4i::cZero;
    }

    constexpr void ConvertMatrix(MatrixColumnMajor4x4i *out_matrix, const IntColumnMajor3x3 *in_matrix) {

        const Vector4i vector0 = ConvertVector(ConvertVector(in_matrix->array_col[0]));
        const Vector4i vector1 = ConvertVector(ConvertVector(in_matrix->array_col[1]));
        const Vector4i vector2 = ConvertVector(ConvertVector(in_matrix->array_col[2]));

        out_matrix->array_col[0] = vector0;
        out_matrix->array_col[1] = vector1;
        out_matrix->array_col[2] = vector2;
        out_matrix->array_col[3] = Vector4i::cUnitW;
    }

    constexpr void ConvertMatrix(MatrixColumnMajor4x4i *out_matrix, const IntColumnMajor4x3 *in_matrix) {

        const Vector4i vector0 = ConvertVector(in_matrix->array_col[0]);
        const Vector4i vector1 = ConvertVector(in_matrix->array_col[1]);
        const Vector4i vector2 = ConvertVector(in_matrix->array_col[2]);

        out_matrix->array_col[0] = vector0;
        out_matrix->array_col[1] = vector1;
        out_matrix->array_col[2] = vector2;
        out_matrix->array_col[3] = Vector4i::cUnitW;
    }

    constexpr void ConvertMatrix(MatrixColumnMajor4x4i *out_matrix, const IntColumnMajor4x4 *in_matrix) {

        const Vector4i vector0 = ConvertVector(in_matrix->array_col[0]);
        const Vector4i vector1 = ConvertVector(in_matrix->array_col[1]);
        const Vector4i vector2 = ConvertVector(in_matrix->array_col[2]);
        const Vector4i vector3 = ConvertVector(in_matrix->array_col[3]);

        out_matrix->array_col[0] = vector0;
        out_matrix->array_col[1] = vector1;
        out_matrix->array_col[2] = vector2;
        out_matrix->array_col[3] = vector3;
    }

    constexpr void ConvertMatrix(MatrixColumnMajor4x4i *out_matrix, const MatrixColumnMajor4x3i *in_matrix) {

        const Vector4i vector0 = in_matrix->array_col[0];
        const Vector4i vector1 = in_matrix->array_col[1];
        const Vector4i vector2 = in_matrix->array_col[2];

        out_matrix->array_col[0] = vector0;
        out_matrix->array_col[1] = vector1;
        out_matrix->array_col[2] = vector2;
        out_matrix->array_col[3] = Vector4i::cUnitW;
    }

    constexpr Int2 ConvertInt(const Vector2i &in_vector) {
        return { in_vector.x, in_vector.y };
    }
    constexpr Int3 ConvertInt(const Vector3i &in_vector) {
        return { in_vector.x, in_vector.y, in_vector.z };
    }
    constexpr Int4 ConvertInt(const Vector4i &in_vector) {
        return { in_vector.x, in_vector.y, in_vector.z, in_vector.w };
    }
}
