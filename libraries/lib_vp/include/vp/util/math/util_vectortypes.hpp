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

    typedef float  v8f __attribute__((vector_size(32)));
    typedef double v4d __attribute__((vector_size(32)));

    typedef float  v4f __attribute__((vector_size(16)));
    typedef double v2d __attribute__((vector_size(16)));

    struct v128f {
        union {
            v4f f;
            v2d d;
            __m128 mm;
        };

        constexpr ALWAYS_INLINE v128f() : mm() {}
        constexpr ALWAYS_INLINE v128f(const v128f& copy) : mm(copy.mm) {}

        constexpr ALWAYS_INLINE v128f(const v4f& copy) : f(copy) {}
        constexpr ALWAYS_INLINE v128f(const v2d& copy) : d(copy) {}

        constexpr ALWAYS_INLINE v128f(float x, float y, float z, float w) : f{x,y,z,w} {}
        constexpr ALWAYS_INLINE v128f(double x, double y) : d{x,y} {}
    };
    
    typedef long long          v4sll __attribute__((vector_size(32)));
    typedef unsigned long long v4ull __attribute__((vector_size(32)));
    typedef int                v8si  __attribute__((vector_size(32)));
    typedef unsigned int       v8ui  __attribute__((vector_size(32)));
    typedef short              v16ss __attribute__((vector_size(32)));
    typedef unsigned short     v16us __attribute__((vector_size(32)));
    typedef char               v32cc __attribute__((vector_size(32)));
    typedef signed char        v32sc __attribute__((vector_size(32)));
    typedef unsigned char      v32uc __attribute__((vector_size(32)));

    typedef long long          v2sll __attribute__((vector_size(16)));
    typedef unsigned long long v2ull __attribute__((vector_size(16)));
    typedef int                v4si  __attribute__((vector_size(16)));
    typedef unsigned int       v4ui  __attribute__((vector_size(16)));
    typedef short              v8ss  __attribute__((vector_size(16)));
    typedef unsigned short     v8us  __attribute__((vector_size(16)));
    typedef char               v16cc __attribute__((vector_size(16)));
    typedef signed char        v16sc __attribute__((vector_size(16)));
    typedef unsigned char      v16uc __attribute__((vector_size(16)));

    struct v128i {
        union {
            v4si  si;
            v4ui  ui;
            v2sll sll;
            v2ull ull;
            v8ss  ss;
            v8us  us;
            v16cc cc;
            v16sc sc;
            v16uc uc;
            __m128i mm;
        };

        constexpr ALWAYS_INLINE v128i() : mm() {/*...*/}
        constexpr ALWAYS_INLINE v128i(const v128i& rhs) : mm(rhs.mm) {/*...*/}

        constexpr ALWAYS_INLINE v128i(__m128i rhs) : mm(rhs) {/*Takes care of v2sll*/}
        constexpr ALWAYS_INLINE v128i(v4si rhs)  : si(rhs) {/*...*/}
        constexpr ALWAYS_INLINE v128i(v4ui rhs)  : ui(rhs) {/*...*/}
        constexpr ALWAYS_INLINE v128i(v2ull rhs) : ull(rhs) {/*...*/}
        constexpr ALWAYS_INLINE v128i(v8ss rhs)  : ss(rhs) {/*...*/}
        constexpr ALWAYS_INLINE v128i(v8us rhs)  : us(rhs) {/*...*/}
        constexpr ALWAYS_INLINE v128i(v16sc rhs) : sc(rhs) {/*...*/}
        constexpr ALWAYS_INLINE v128i(v16uc rhs) : uc(rhs) {/*...*/}

        constexpr ALWAYS_INLINE v128i(int x, int y = 0, int z = 0, int w = 0) : si{x,y,z,w} {/*...*/}
        constexpr ALWAYS_INLINE v128i(unsigned int x, unsigned int y = 0, unsigned int z = 0, unsigned int w = 0) : ui{x,y,z,w} {/*...*/}
        constexpr ALWAYS_INLINE v128i(long long x, long long y = 0) : sll{x,y} {/*...*/}
        constexpr ALWAYS_INLINE v128i(unsigned long long x, unsigned long long y = 0) : ull{x,y} {/*...*/}
        constexpr ALWAYS_INLINE v128i(short x, short y = 0, short z = 0, short w = 0, short x2 = 0, short y2 = 0, short z2 = 0, short w2 = 0) : ss{x,y,z,w,x2,y2,z2,w2} {/*...*/}
        constexpr ALWAYS_INLINE v128i(unsigned short x, unsigned short y = 0, unsigned short z = 0, unsigned short w = 0, unsigned short x2 = 0, unsigned short y2 = 0, unsigned short z2 = 0, unsigned short w2 = 0) : us{x,y,z,w,x2,y2,z2,w2} {/*...*/}
        constexpr ALWAYS_INLINE v128i(char x1, char y1 = 0, char z1 = 0, char w1 = 0, char x2 = 0, char y2 = 0, char z2 = 0, char w2 = 0, char x3 = 0, char y3 = 0, char z3 = 0, char w3 = 0, char x4 = 0, char y4 = 0, char z4 = 0, char w4 = 0) : cc{x1,y1,z1,w1,x2,y2,z2,w2,x3,y3,z3,w3,x4,y4,z4,w4} {/*...*/}
        constexpr ALWAYS_INLINE v128i(signed char x1, signed char y1 = 0, signed char z1 = 0, signed char w1 = 0, signed char x2 = 0, signed char y2 = 0, signed char z2 = 0, signed char w2 = 0, signed char x3 = 0, signed char y3 = 0, signed char z3 = 0, signed char w3 = 0, signed char x4 = 0, signed char y4 = 0, signed char z4 = 0, signed char w4 = 0) : sc{x1,y1,z1,w1,x2,y2,z2,w2,x3,y3,z3,w3,x4,y4,z4,w4} {/*...*/}
        constexpr ALWAYS_INLINE v128i(unsigned char x1, unsigned char y1 = 0, unsigned char z1 = 0, unsigned char w1 = 0, unsigned char x2 = 0, unsigned char y2 = 0, unsigned char z2 = 0, unsigned char w2 = 0, unsigned char x3 = 0, unsigned char y3 = 0, unsigned char z3 = 0, unsigned char w3 = 0, unsigned char x4 = 0, unsigned char y4 = 0, unsigned char z4 = 0, unsigned char w4 = 0) : uc{x1,y1,z1,w1,x2,y2,z2,w2,x3,y3,z3,w3,x4,y4,z4,w4} {/*...*/}
    };

    typedef v128i v2ll;
    typedef v128i v4i;
    typedef v128i v8s;
    typedef v128i v16c;
}
