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

#ifndef VP_MAX_VECTOR_REGISTER_SIZE
 
    /* We assume avx2 on x86 */
    /* We assume neon on aarch64 */
    #ifdef VP_TARGET_ARCHITECTURE_x86
        #define VP_MAX_VECTOR_REGISTER_SIZE 256
    #else
        #define VP_MAX_VECTOR_REGISTER_SIZE 128
    #endif

#endif
static_assert(VP_MAX_VECTOR_REGISTER_SIZE == 128 || VP_MAX_VECTOR_REGISTER_SIZE == 256, "Invalid VP_MAX_VECTOR_REGISTER_SIZE (must be 128 or 256)");

namespace vp::util {

    static constexpr u32 cSimdRegisterSize = VP_MAX_VECTOR_REGISTER_SIZE;

    #define VP_VECTORIZED_TYPE_TRAITS(underlying, count) \
        using UnderlyingType = underlying ; \
        static constexpr bool cIsVectorizedType = true; \
        static constexpr u32 cCount = count ; \
        static constexpr u32 cCountPerRegister = cSimdRegisterSize / BITSIZEOF(UnderlyingType); \
        static constexpr u32 cRegisterCount = cCount / cCountPerRegister + (cCount % cCountPerRegister != 0); \
        static_assert(0 < cRegisterCount); \
        template <class Self> \
            requires (cRegisterCount == 1) \
        constexpr auto &&operator[](this Self&& self, u32 index) { \
            return std::forward<Self>(self).v[index]; \
        } \
        template <class Self> \
        constexpr auto &&operator[](this Self&& self, u32 index) { \
            const u32 vector = index / cCountPerRegister; \
            const u32 base_index = index % cCountPerRegister; \
            return std::forward<Self>(self).v[vector][base_index]; \
        }

    struct v2f {
        float v __attribute__((vector_size(8)));

        VP_VECTORIZED_TYPE_TRAITS(float, 2);
    };

    struct v4f {
        float v __attribute__((vector_size(16)));

        VP_VECTORIZED_TYPE_TRAITS(float, 4);
    };

    struct v2d {
        double v __attribute__((vector_size(16)));

        VP_VECTORIZED_TYPE_TRAITS(double, 2);
    };

#if (VP_MAX_VECTOR_REGISTER_SIZE == 256)
    struct v8f {
        float v __attribute__((vector_size(32)));

        VP_VECTORIZED_TYPE_TRAITS(float, 8);
    };

    struct v4d {
        double v __attribute__((vector_size(32)));

        VP_VECTORIZED_TYPE_TRAITS(double, 4);
    };
#elif (VP_MAX_VECTOR_REGISTER_SIZE == 128)
    struct v8f {
        v4f v[2];

        VP_VECTORIZED_TYPE_TRAITS(float, 8);
    };

    struct v4d {
        v2d v[2];

        VP_VECTORIZED_TYPE_TRAITS(double, 2);
    };
#endif

#if (VP_MAX_VECTOR_REGISTER_SIZE == 256)
    struct v16f {
        v8f v[2];

        VP_VECTORIZED_TYPE_TRAITS(float, 16);
    };

    struct v8d {
        v4d v[2];

        VP_VECTORIZED_TYPE_TRAITS(double, 8);
    };
#elif (VP_MAX_VECTOR_REGISTER_SIZE == 128)
    struct v16f {
        v4f v[4];

        VP_VECTORIZED_TYPE_TRAITS(float, 16);
    };

    struct v8d {
        v2d v[4];

        VP_VECTORIZED_TYPE_TRAITS(double, 8);
    };
#endif

#if (VP_MAX_VECTOR_REGISTER_SIZE == 256)
    struct v32f {
        v8f v[4];

        VP_VECTORIZED_TYPE_TRAITS(float, 32);
    };

    struct v16d {
        v4d v[4];

        VP_VECTORIZED_TYPE_TRAITS(double, 16);
    };
#elif (VP_MAX_VECTOR_REGISTER_SIZE == 128)
    struct v32f {
        v4f v[8];

        VP_VECTORIZED_TYPE_TRAITS(float, 32);
    };

    struct v16d {
        v2d v[8];

        VP_VECTORIZED_TYPE_TRAITS(double, 16);
    };
#endif

    struct v2si {
        int v __attribute__((vector_size(8)));

        VP_VECTORIZED_TYPE_TRAITS(int, 2);
    };

    struct v2sll {
        long long v __attribute__((vector_size(16)));

        VP_VECTORIZED_TYPE_TRAITS(long long, 2);
    };

    struct v2ull {
        unsigned long long v __attribute__((vector_size(16)));

        VP_VECTORIZED_TYPE_TRAITS(unsigned long long, 2);
    };

    struct v4si {
        int v __attribute__((vector_size(16)));

        VP_VECTORIZED_TYPE_TRAITS(int, 4);
    };

    struct v4ui {
        unsigned int v __attribute__((vector_size(16)));

        VP_VECTORIZED_TYPE_TRAITS(unsigned int, 4);
    };

    struct v8ss {
        short v __attribute__((vector_size(16)));

        VP_VECTORIZED_TYPE_TRAITS(short, 8);
    };

    struct v8us {
        unsigned short v __attribute__((vector_size(16)));

        VP_VECTORIZED_TYPE_TRAITS(unsigned short, 8);
    };

    struct v16cc {
        char v __attribute__((vector_size(16)));

        VP_VECTORIZED_TYPE_TRAITS(char, 16);
    };

    struct v16sc {
        signed char v __attribute__((vector_size(16)));

        VP_VECTORIZED_TYPE_TRAITS(signed char, 16);
    };

    struct v16uc {
        unsigned char v __attribute__((vector_size(16)));

        VP_VECTORIZED_TYPE_TRAITS(unsigned char, 16);
    };

#if (VP_MAX_VECTOR_REGISTER_SIZE == 256)
    struct v4sll {
        long long v __attribute__((vector_size(32)));

        VP_VECTORIZED_TYPE_TRAITS(long long, 4);
    };

    struct v4ull {
        unsigned long long v __attribute__((vector_size(32)));

        VP_VECTORIZED_TYPE_TRAITS(unsigned long long, 4);
    };

    struct v8si {
        int v __attribute__((vector_size(32)));

        VP_VECTORIZED_TYPE_TRAITS(int, 8);
    };

    struct v8ui {
        unsigned int v __attribute__((vector_size(32)));

        VP_VECTORIZED_TYPE_TRAITS(unsigned int, 8);
    };

    struct v16ss {
        short v __attribute__((vector_size(32)));

        VP_VECTORIZED_TYPE_TRAITS(short, 16);
    };

    struct v16us {
        unsigned short v __attribute__((vector_size(32)));

        VP_VECTORIZED_TYPE_TRAITS(unsigned short, 16);
    };

    struct v32cc {
        char v __attribute__((vector_size(32)));

        VP_VECTORIZED_TYPE_TRAITS(char, 32);
    };

    struct v32sc {
        signed char v __attribute__((vector_size(32)));

        VP_VECTORIZED_TYPE_TRAITS(signed char, 32);
    };

    struct v32uc {
        unsigned char v __attribute__((vector_size(32)));

        VP_VECTORIZED_TYPE_TRAITS(unsigned char, 32);
    };
#elif (VP_MAX_VECTOR_REGISTER_SIZE == 128)
    struct v4sll {
        v2sll v[2];

        VP_VECTORIZED_TYPE_TRAITS(long long, 4);
    };

    struct v4ull {
        v2ull v[2];

        VP_VECTORIZED_TYPE_TRAITS(unsigned long long, 4);
    };

    struct v8si {
        v4si v[2];

        VP_VECTORIZED_TYPE_TRAITS(int, 8);
    };

    struct v8ui {
        v4ui v[2];

        VP_VECTORIZED_TYPE_TRAITS(unsigned int, 8);
    };

    struct v16ss {
        v8ss v[2];

        VP_VECTORIZED_TYPE_TRAITS(short, 16);
    };

    struct v16us {
        v8us v[2];

        VP_VECTORIZED_TYPE_TRAITS(unsigned short, 16);
    };

    struct v32cc {
        v16cc v[2];

        VP_VECTORIZED_TYPE_TRAITS(char, 32);
    };

    struct v32sc {
        v16sc v[2];

        VP_VECTORIZED_TYPE_TRAITS(signed char, 32);
    };

    struct v32uc {
        v16uc v[2];

        VP_VECTORIZED_TYPE_TRAITS(unsigned char, 32);
    };
#endif

#if (VP_MAX_VECTOR_REGISTER_SIZE == 256)
    struct v8sll {
        v4sll v[2];

        VP_VECTORIZED_TYPE_TRAITS(long long, 8);
    };

    struct v8ull {
        v4ull v[2];

        VP_VECTORIZED_TYPE_TRAITS(unsigned long long, 8);
    };

    struct v16si {
        v8si v[2];

        VP_VECTORIZED_TYPE_TRAITS(int, 16);
    };

    struct v16ui {
        v8ui v[2];

        VP_VECTORIZED_TYPE_TRAITS(unsigned int, 16);
    };

    struct v32ss {
        v16ss v[2];

        VP_VECTORIZED_TYPE_TRAITS(short, 32);
    };

    struct v32us {
        v16us v[2];

        VP_VECTORIZED_TYPE_TRAITS(unsigned short, 32);
    };

    struct v64cc {
        v32cc v[2];

        VP_VECTORIZED_TYPE_TRAITS(char, 64);
    };

    struct v64sc {
        v32sc v[2];

        VP_VECTORIZED_TYPE_TRAITS(signed char, 64);
    };

    struct v64uc {
        v32uc v[2];

        VP_VECTORIZED_TYPE_TRAITS(unsigned char, 64);
    };
#elif (VP_MAX_VECTOR_REGISTER_SIZE == 128)
    struct v8sll {
        v2sll v[4];

        VP_VECTORIZED_TYPE_TRAITS(long long, 8);
    };

    struct v8ull {
        v2ull v[4];

        VP_VECTORIZED_TYPE_TRAITS(unsigned long long, 8);
    };

    struct v16si {
        v4si v[4];

        VP_VECTORIZED_TYPE_TRAITS(int, 16);
    };

    struct v16ui {
        v4ui v[4];

        VP_VECTORIZED_TYPE_TRAITS(unsigned int, 16);
    };

    struct v32ss {
        v8ss v[4];

        VP_VECTORIZED_TYPE_TRAITS(short, 32);
    };

    struct v32us {
        v8us v[4];

        VP_VECTORIZED_TYPE_TRAITS(unsigned short, 32);
    };

    struct v64cc {
        v16cc v[4];

        VP_VECTORIZED_TYPE_TRAITS(char, 64);
    };

    struct v64sc {
        v16sc v[4];

        VP_VECTORIZED_TYPE_TRAITS(signed char, 64);
    };

    struct v64uc {
        v16uc v[4];

        VP_VECTORIZED_TYPE_TRAITS(unsigned char, 64);
    };
#endif

#if (VP_MAX_VECTOR_REGISTER_SIZE == 256)
    struct v16sll {
        v4sll v[4];

        VP_VECTORIZED_TYPE_TRAITS(long long, 16);
    };

    struct v16ull {
        v4ull v[4];

        VP_VECTORIZED_TYPE_TRAITS(unsigned long long, 16);
    };

    struct v32si {
        v8si v[4];

        VP_VECTORIZED_TYPE_TRAITS(int, 32);
    };

    struct v32ui {
        v8ui v[4];

        VP_VECTORIZED_TYPE_TRAITS(unsigned int, 32);
    };

    struct v64ss {
        v16ss v[4];

        VP_VECTORIZED_TYPE_TRAITS(short, 64);
    };

    struct v64us {
        v16us v[4];

        VP_VECTORIZED_TYPE_TRAITS(unsigned short, 64);
    };

    struct v128cc {
        v32cc v[4];

        VP_VECTORIZED_TYPE_TRAITS(char, 128);
    };

    struct v128sc {
        v32sc v[4];

        VP_VECTORIZED_TYPE_TRAITS(signed char, 128);
    };

    struct v128uc {
        v32uc v[4];

        VP_VECTORIZED_TYPE_TRAITS(unsigned char, 128);
    };
#elif (VP_MAX_VECTOR_REGISTER_SIZE == 128)
    struct v16sll {
        v2sll v[8];

        VP_VECTORIZED_TYPE_TRAITS(long long, 16);
    };

    struct v16ull {
        v2ull v[8];

        VP_VECTORIZED_TYPE_TRAITS(unsigned long long, 16);
    };

    struct v32si {
        v4si v[8];

        VP_VECTORIZED_TYPE_TRAITS(int, 32);
    };

    struct v32ui {
        v4ui v[8];

        VP_VECTORIZED_TYPE_TRAITS(unsigned int, 32);
    };

    struct v64ss {
        v8ss v[8];

        VP_VECTORIZED_TYPE_TRAITS(short, 64);
    };

    struct v64us {
        v8us v[8];

        VP_VECTORIZED_TYPE_TRAITS(unsigned short, 64);
    };

    struct v128cc {
        v16cc v[8];

        VP_VECTORIZED_TYPE_TRAITS(char, 128);
    };

    struct v128sc {
        v16sc v[8];

        VP_VECTORIZED_TYPE_TRAITS(signed char, 128);
    };

    struct v128uc {
        v16uc v[8];

        VP_VECTORIZED_TYPE_TRAITS(unsigned char, 128);
    };
#endif


    template <typename T>
        requires (T::cIsVectorizedType && T::cRegisterCount == 1)
    constexpr T operator+(const T &left, const T &right) {
        const T out = { .v = left.v + right.v, };
        return out;
    }

    template <typename T>
        requires (T::cIsVectorizedType && T::cRegisterCount == 1)
    constexpr T operator-(const T &left, const T &right) {
        const T out = { .v = left.v - right.v, };
        return out;
    }

    template <typename T>
        requires (T::cIsVectorizedType && T::cRegisterCount == 1)
    constexpr T operator-(const T &vec) {
        const T out = { .v = -vec.v, };
        return out;
    }

    template <typename T>
        requires (T::cIsVectorizedType && T::cRegisterCount == 1)
    constexpr T operator*(const T &left, const T &right) {
        const T out = { .v = left.v * right.v, };
        return out;
    }

    template <typename T>
        requires (T::cIsVectorizedType && T::cRegisterCount == 1)
    constexpr T operator*(const T &left, const typename T::UnderlyingType &right) {
        const T out = { .v = left.v * right, };
        return out;
    }

    template <typename T>
        requires (T::cIsVectorizedType && T::cRegisterCount == 1)
    constexpr T operator/(const T &left, typename T::UnderlyingType scalar) {
        const T out = { .v = left.v / scalar, };
        return out;
    }

    template <typename T>
        requires (T::cIsVectorizedType && T::cRegisterCount == 1)
    constexpr T MultiplyAdd(const T& a, const T& b, const T& c) {
        if(!std::is_constant_evaluated() && T::cCount == 4 && std::is_same_v<float, typename T::UnderlyingType>) {
            return T{ .v = __builtin_ia32_vfmaddps(a.v, b.v, c.v) };
        }
        T temp;
        for (u32 i = 0; i < T::cCount; ++i) {
            temp[i] = (a[i] * b[i]) + c[i];
        }
        return temp;
    }

    template <typename T>
        requires (T::cIsVectorizedType && T::cRegisterCount == 1)
    constexpr T NegateMultiplyAdd(const T& a, const T& b, const T& c) {
        if(!std::is_constant_evaluated() && T::cCount == 4 && std::is_same_v<float, typename T::UnderlyingType>) {
            return T{ .v = __builtin_ia32_vfnmaddps(a.v, b.v, c.v), };
        }
        T temp;
        for (u32 i = 0; i < T::cCount; ++i) {
            temp[i] = -(a[i] * b[i]) + c[i];
        }
        return temp;
    }

    template <typename T>
        requires (T::cIsVectorizedType && T::cRegisterCount == 1)
    constexpr T MultiplySub(const T& a, const T& b, const T& c) {
        if(!std::is_constant_evaluated() && T::cCount == 4 && std::is_same_v<float, typename T::UnderlyingType>) {
            return T{ .v = __builtin_ia32_vfmsubps(a.v, b.v, c.v), };
        }
        T temp;
        for (u32 i = 0; i < T::cCount; ++i) {
            temp[i] = (a[i] * b[i]) - c[i];
        }
        return temp;
    }

    template <typename T>
        requires (T::cIsVectorizedType && T::cRegisterCount == 1)
    constexpr T NegateMultiplySub(const T& a, const T& b, const T& c) {
        if(!std::is_constant_evaluated() && T::cCount == 4 && std::is_same_v<float, typename T::UnderlyingType>) {
            return T{ .v = __builtin_ia32_vfnmsubps(a.v, b.v, c.v), };
        }
        T temp;
        for (u32 i = 0; i < T::cCount; ++i) {
            temp[i] = -(a[i] * b[i]) - c[i];
        }
        return temp;
    }

    template <typename T>
        requires (T::cIsVectorizedType && T::cRegisterCount == 1)
    constexpr T MultiplySubOrAddInterleaved(const T& a, const T& b, const T& c) {
        if(!std::is_constant_evaluated() && T::cCount == 4 && std::is_same_v<float, typename T::UnderlyingType>) {
            return T{ .v = __builtin_ia32_vfmaddsubps(a.v, b.v, c.v), };
        }
        T temp;
        for (u32 i = 0; i < T::cCount; ++i) {
            if ((i & 1) == 0) {
                temp[i] = (a[i] * b[i]) - c[i];
            } else {
                temp[i] = (a[i] * b[i]) + c[i];
            }
        }
        return temp;
    }
}
