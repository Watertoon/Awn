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

#ifdef VP_TARGET_ARCHITECTURE_aarch64
    using __m128  = _Float128;
    using __m128i = __int128;
#endif

    typedef float  v8f __attribute__((vector_size(32)));
    typedef double v4d __attribute__((vector_size(32)));

    struct v256f {
        union {
            v8f     f;
            v4d     d;
            __m256  mmf;
            __m256d mmd;
        };
    };

    typedef float  v4f __attribute__((vector_size(16)));
    typedef double v2d __attribute__((vector_size(16)));

    struct v128f {
        union {
            v4f     f;
            v2d     d;
            __m128  mmf;
            __m128d mmd;
        };
    };

    typedef float v2f __attribute__((vector_size(8)));

    struct v64f {
        union {
            v2f f;
        };
    };

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
    };

    typedef v128i v2ll;
    typedef v128i v4i;
    typedef v128i v8s;
    typedef v128i v16c;

    typedef long long          v4sll __attribute__((vector_size(32)));
    typedef unsigned long long v4ull __attribute__((vector_size(32)));
    typedef int                v8si  __attribute__((vector_size(32)));
    typedef unsigned int       v8ui  __attribute__((vector_size(32)));
    typedef short              v16ss __attribute__((vector_size(32)));
    typedef unsigned short     v16us __attribute__((vector_size(32)));
    typedef char               v32cc __attribute__((vector_size(32)));
    typedef signed char        v32sc __attribute__((vector_size(32)));
    typedef unsigned char      v32uc __attribute__((vector_size(32)));

    struct v256i {
        union {
            v8si    si;
            v8ui    ui;
            v4sll   sll;
            v4ull   ull;
            v16ss   ss;
            v16us   us;
            v32cc   cc;
            v32sc   sc;
            v32uc   uc;

#ifdef VP_TARGET_ARCHITECTURE_x86
            __m256i mm;
#else
            v4sll mm;
#endif
        };
    };

    typedef v256i v4ll;
    typedef v256i v8i;
    typedef v256i v16s;
    typedef v256i v32c;
}
