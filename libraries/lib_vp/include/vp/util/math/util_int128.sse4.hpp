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

namespace vp::util::sse4 {

    /* naming structure: v [number of elements] [{s}igned or {u}nsigned] [integer type] */
    typedef int v4si __attribute__((vector_size(16)));
    typedef unsigned int v4ui __attribute__((vector_size(16)));
    typedef long long v2sll __attribute__((vector_size(16)));
    typedef unsigned long long v2ull __attribute__((vector_size(16)));
    typedef short v8ss __attribute__((vector_size(16)));
    typedef unsigned short v8us __attribute__((vector_size(16)));
    typedef char v16cc __attribute__((vector_size(16)));
    typedef signed char v16sc __attribute__((vector_size(16)));
    typedef unsigned char v16uc __attribute__((vector_size(16)));

    /* 128 bit integer interop struct */
    struct v128 {
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

        constexpr ALWAYS_INLINE v128() : mm() {/*...*/}
        constexpr ALWAYS_INLINE v128(const v128& rhs) : mm(rhs.mm) {/*...*/}

        constexpr ALWAYS_INLINE v128(__m128i rhs) : mm(rhs) {/*Takes care of v2sll*/}
        constexpr ALWAYS_INLINE v128(v4si rhs)  : si(rhs) {/*...*/}
        constexpr ALWAYS_INLINE v128(v4ui rhs)  : ui(rhs) {/*...*/}
        constexpr ALWAYS_INLINE v128(v2ull rhs) : ull(rhs) {/*...*/}
        constexpr ALWAYS_INLINE v128(v8ss rhs)  : ss(rhs) {/*...*/}
        constexpr ALWAYS_INLINE v128(v8us rhs)  : us(rhs) {/*...*/}
        constexpr ALWAYS_INLINE v128(v16sc rhs) : sc(rhs) {/*...*/}
        constexpr ALWAYS_INLINE v128(v16uc rhs) : uc(rhs) {/*...*/}

        constexpr ALWAYS_INLINE v128(int x, int y = 0, int z = 0, int w = 0) : si{x,y,z,w} {/*...*/}
        constexpr ALWAYS_INLINE v128(unsigned int x, unsigned int y = 0, unsigned int z = 0, unsigned int w = 0) : ui{x,y,z,w} {/*...*/}
        constexpr ALWAYS_INLINE v128(long long x, long long y = 0) : sll{x,y} {/*...*/}
        constexpr ALWAYS_INLINE v128(unsigned long long x, unsigned long long y = 0) : ull{x,y} {/*...*/}
        constexpr ALWAYS_INLINE v128(short x, short y = 0, short z = 0, short w = 0, short x2 = 0, short y2 = 0, short z2 = 0, short w2 = 0) : ss{x,y,z,w,x2,y2,z2,w2} {/*...*/}
        constexpr ALWAYS_INLINE v128(unsigned short x, unsigned short y = 0, unsigned short z = 0, unsigned short w = 0, unsigned short x2 = 0, unsigned short y2 = 0, unsigned short z2 = 0, unsigned short w2 = 0) : us{x,y,z,w,x2,y2,z2,w2} {/*...*/}
        constexpr ALWAYS_INLINE v128(char x1, char y1 = 0, char z1 = 0, char w1 = 0, char x2 = 0, char y2 = 0, char z2 = 0, char w2 = 0, char x3 = 0, char y3 = 0, char z3 = 0, char w3 = 0, char x4 = 0, char y4 = 0, char z4 = 0, char w4 = 0) : cc{x1,y1,z1,w1,x2,y2,z2,w2,x3,y3,z3,w3,x4,y4,z4,w4} {/*...*/}
        constexpr ALWAYS_INLINE v128(signed char x1, signed char y1 = 0, signed char z1 = 0, signed char w1 = 0, signed char x2 = 0, signed char y2 = 0, signed char z2 = 0, signed char w2 = 0, signed char x3 = 0, signed char y3 = 0, signed char z3 = 0, signed char w3 = 0, signed char x4 = 0, signed char y4 = 0, signed char z4 = 0, signed char w4 = 0) : sc{x1,y1,z1,w1,x2,y2,z2,w2,x3,y3,z3,w3,x4,y4,z4,w4} {/*...*/}
        constexpr ALWAYS_INLINE v128(unsigned char x1, unsigned char y1 = 0, unsigned char z1 = 0, unsigned char w1 = 0, unsigned char x2 = 0, unsigned char y2 = 0, unsigned char z2 = 0, unsigned char w2 = 0, unsigned char x3 = 0, unsigned char y3 = 0, unsigned char z3 = 0, unsigned char w3 = 0, unsigned char x4 = 0, unsigned char y4 = 0, unsigned char z4 = 0, unsigned char w4 = 0) : uc{x1,y1,z1,w1,x2,y2,z2,w2,x3,y3,z3,w3,x4,y4,z4,w4} {/*...*/}
    };

    typedef v128 v2ll;
    typedef v128 v4i;
    typedef v128 v8s;
    typedef v128 v16c;

    /* Arithmitic intructions */

    constexpr ALWAYS_INLINE __m128i paddq(const v4i& a, const v4i& b) {
        if (std::is_constant_evaluated()) {
            return a.mm + b.mm;
        } else {
            return __builtin_ia32_paddq128(a.sll, b.sll);
        }
    }

    constexpr ALWAYS_INLINE v4si paddd(const v4i& a, const v4i& b) {
        if (std::is_constant_evaluated()) {
            return a.si + b.si;
        } else {
            return __builtin_ia32_paddd128(a.si, b.si);
        }
    }

    constexpr ALWAYS_INLINE __m128i psubq(const v4i& a, const v4i& b) {
        if (std::is_constant_evaluated()) {
            return a.mm - b.mm;
        } else {
            return __builtin_ia32_psubq128(a.mm, b.mm);
        }
    }

    constexpr ALWAYS_INLINE v4si psubd(const v4i& a, const v4i& b) { return a.si - b.si;
        if (std::is_constant_evaluated()) {
            return a.si + b.si;
        } else {
            return __builtin_ia32_psubd128(a.si, b.si);
        }
    }

    constexpr ALWAYS_INLINE v4si pmuld(const v4i& a, const v4i& b) { return a.si - b.si;
        if (std::is_constant_evaluated()) {
            return a.si * b.si;
        } else {
            return __builtin_ia32_pmulld128(a.si, b.si);
        }
    }

    /* Bitwise Instructions */

    constexpr ALWAYS_INLINE __m128i psllq(const v4i& a, const v4i& count) {
        if (std::is_constant_evaluated()) {
            return a.mm << count.mm;
        } else {
            return __builtin_ia32_psllq128(a.mm, count.mm);
        }
    }

    constexpr ALWAYS_INLINE v4si pslld(const v4i& a, const v4i& count) {
        if (std::is_constant_evaluated()) {
            return a.si << count.si;
        } else {
            return __builtin_ia32_pslld128(a.si, count.si);
        }
    }

    constexpr ALWAYS_INLINE v8ss psllw(const v4i& a, const v4i& count) {
        if (std::is_constant_evaluated()) {
            return a.ss << count.ss;
        } else {
            return __builtin_ia32_psllw128(a.ss, count.ss);
        }
    }

    constexpr ALWAYS_INLINE __m128i pand(const v4i& a, const v4i& b) {
        if (std::is_constant_evaluated()) {
            return (__m128i)(a.mm & b.mm);
        } else {
            return __builtin_ia32_pand128(a.mm, b.mm);
        }
    }

    constexpr ALWAYS_INLINE __m128i pandn(const v4i& a, const v4i& b) {
        if (std::is_constant_evaluated()) {
            return ((~a.mm) & b.mm);
        } else {
            return __builtin_ia32_pandn128(a.mm, b.mm);
        }
    }

    constexpr ALWAYS_INLINE __m128i por(const v4i& a, const v4i& b) {
        if (std::is_constant_evaluated()) {
            return a.sll | b.sll;
        } else {
            return __builtin_ia32_por128(a.mm, b.mm);
        }
    }

    constexpr ALWAYS_INLINE __m128i pxor(const v4i& a, const v4i& b) {
        if (std::is_constant_evaluated()) {
            return a.sll ^ b.sll;
        } else {
            return __builtin_ia32_pxor128(a.mm, b.mm);
        }
    }

    /* Comparison Instructions */

    constexpr ALWAYS_INLINE v4si pcmpeqd(const v4i& a, const v4i& b) { 
        if (std::is_constant_evaluated()) {
            return a.si == b.si; 
        } else {
            return __builtin_ia32_pcmpeqd128(a.si, b.si);
        }
    }

    constexpr ALWAYS_INLINE v8ss pcmpeqw(const v4i& a, const v4i& b) { 
        if (std::is_constant_evaluated()) {
            return a.ss == b.ss; 
        } else {
            return __builtin_ia32_pcmpeqw128(a.ss, b.ss);
        }
    }

    constexpr ALWAYS_INLINE v16cc pcmpeqb(const v4i& a, const v4i& b) { 
        if (std::is_constant_evaluated()) {
            return a.cc == b.cc; 
        } else {
            return __builtin_ia32_pcmpeqb128(a.cc, b.cc);
        }
    }

    constexpr ALWAYS_INLINE v4si pcmpgtd(const v4i& a, const v4i& b) { 
        if (std::is_constant_evaluated()) {
            return a.si > b.si; 
        } else {
            return __builtin_ia32_pcmpgtd128(a.si, b.si);
        }
    }

    constexpr ALWAYS_INLINE v8ss pcmpgtw(const v4i& a, const v4i& b) { 
        if (std::is_constant_evaluated()) {
            return a.ss > b.ss; 
        } else {
            return __builtin_ia32_pcmpgtw128(a.ss, b.ss);
        }
    }

    constexpr ALWAYS_INLINE v16cc pcmpgtb(const v4i& a, const v4i& b) { 
        if (std::is_constant_evaluated()) {
            return a.cc > b.cc; 
        } else {
            return __builtin_ia32_pcmpgtb128(a.cc, b.cc);
        }
    }

    constexpr ALWAYS_INLINE v4si pcmpltd(const v4i& a, const v4i& b) { 
        if (std::is_constant_evaluated()) {
            return a.si < b.si; 
        } else {
            return __builtin_ia32_pcmpgtd128(b.si, a.si);
        }
    }

    constexpr ALWAYS_INLINE v8ss pcmpltw(const v4i& a, const v4i& b) { 
        if (std::is_constant_evaluated()) {
            return a.ss < b.ss; 
        } else {
            return __builtin_ia32_pcmpgtw128(b.ss, a.ss);
        }
    }

    constexpr ALWAYS_INLINE v16cc pcmpltb(const v4i& a, const v4i& b) { 
        if (std::is_constant_evaluated()) {
            return a.cc < b.cc; 
        } else {
            return __builtin_ia32_pcmpgtb128(b.cc, a.cc);
        }
    }

    /* Shuffle\Swizzle */

    constexpr ALWAYS_INLINE int Clamp(int val,  int min, int max) {
        if(max < val) {
            val = max;
        }
        if(min > val) {
            val = min;
        }
        return val;
    }

    consteval ALWAYS_INLINE int ShuffleToOrder(int x,  int y,  int z,  int w) {
        Clamp(x, 0, 3);
        Clamp(y, 0, 3);
        Clamp(z, 0, 3);
        Clamp(w, 0, 3);
        return ((x) | (y << 2) | (z << 4) | (w << 6)) & 255;
    }

    constexpr ALWAYS_INLINE v4si pshufd(const v4i& a, const int shuffle_mask) {
        if(std::is_constant_evaluated()) {
            v4si temp;
            for(int i = 0; i < 4; ++i) {
                int index = (shuffle_mask >> (i + i)) & 3;
                switch (index) {
                    case 0:
                        temp[i] = a.si[0];
                        break;
                    case 1:
                        temp[i] = a.si[1];
                        break;
                    case 2:
                        temp[i] = a.si[2];
                        break;
                    case 3:
                        temp[i] = a.si[3];
                        break;
                    default:
                    /* Can't be hit */
                    break;
                }
            }
            return temp;
        } else {
            return __builtin_ia32_pshufd(a.si, shuffle_mask);
        }
    }

    /* Load, Store, Set */

    constexpr v4si lddqu(int const *address) {
        if (std::is_constant_evaluated()) {
            v128 out = {};
            out.si[0] = address[0];
            out.si[1] = address[1];
            out.si[2] = address[2];
            out.si[3] = address[3];
            return out.si;
        } else {
            return v128(_mm_lddqu_si128(reinterpret_cast<__m128i const*>(address))).si;
        }
    }

    constexpr v4si pinsrd(const v128& a, int i, int index) {
        if (std::is_constant_evaluated()) {
            v128 out(a);
            out.si[index] = i;
            return out.si;
        } else {
            return v128(_mm_insert_epi32(a.mm, i, index)).si;
        }
    }

    namespace {
        struct _BitwiseHelper {
            unsigned char bit : 1;
        };
    }

    consteval int PclmulSelectionMask(const bool a, const bool b) {
        return ((b << 4) | (a));
    }

    constexpr ALWAYS_INLINE v2ull pclmulqdq(const v2ull& a, const v2ull& b, const int selection_mask) {
        if (std::is_constant_evaluated() == true) {
            const size_t sel1 = ((selection_mask & 1) == 0) ? a[0] : a[1];
            const size_t sel2 = (((selection_mask >> 4) & 1) == 0) ? b[0] : b[1];

            _BitwiseHelper t1[64] = {};
            _BitwiseHelper t2[64] = {};

            for (int i = 0; i < 64; ++i) {
                t1[i].bit = (sel1 >> i) & 1;
                t2[i].bit = (sel2 >> i) & 1;
            }

            _BitwiseHelper temp[128] = {};

            size_t out1 = 0;
            size_t out2 = 0;
            for (int i = 0; i < 64; ++i) {
                temp[i].bit = t1[0].bit & t2[i].bit;
                for (int j = 1; j <= i; ++j) {
                    temp[i].bit ^= (t1[j].bit & t2[i - j].bit);
                }
                out1 |= static_cast<size_t>(temp[i].bit) << (i);
            }

            for (int i = 64; i < 128; ++i) {
                temp[i].bit = 0;
                for (int j = (i - 63); j < 64; ++j) {
                    temp[i].bit ^= (t1[j].bit & t2[i - j].bit);
                }
                out2 |= (static_cast<size_t>(temp[i].bit) << (i - 64));
            }
            out2 &= ~(1ull << 63);

            return v2ull{out1, out2};
        } else {
            return v128{__builtin_ia32_pclmulqdq128(v128{a}.sll, v128{b}.sll, selection_mask)}.ull;
        }
    }

    static_assert(pclmulqdq(v2ull{5 , 8}, v2ull{5 , 7}, PclmulSelectionMask(false, false))[0] == 17ll);

}
