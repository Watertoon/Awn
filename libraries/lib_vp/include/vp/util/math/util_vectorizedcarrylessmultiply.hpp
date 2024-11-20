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

    namespace {
        struct _BitwiseHelper {
            unsigned char bit : 1;
        };
    }

    consteval int CarryLessMultiply2SelectionMask(const bool a, const bool b) {
        return ((b << 4) | (a));
    }

    constexpr ALWAYS_INLINE v2sll CarryLessMultiply2(const v2sll &a, const v2sll &b, const int selection_mask) {

        if (!std::is_constant_evaluated()) {
#ifdef VP_TARGET_ARCHITECTURE_x86
            return v2sll{ .v = _mm_clmulepi64_si128(reinterpret_cast<__m128i>(a.v), reinterpret_cast<__m128i>(b.v), selection_mask), };
#endif
        }

        const u64 sel1 = ((selection_mask & 1) == 0) ? a[0] : a[1];
        const u64 sel2 = (((selection_mask >> 4) & 1) == 0) ? b[0] : b[1];

        _BitwiseHelper t1[64] = {};
        _BitwiseHelper t2[64] = {};

        for (int i = 0; i < 64; ++i) {
            t1[i].bit = (sel1 >> i) & 1;
            t2[i].bit = (sel2 >> i) & 1;
        }

        _BitwiseHelper temp[128] = {};

        u64 out1 = 0;
        u64 out2 = 0;
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

        return v2sll{ out1, out2, };
    }
    static_assert(CarryLessMultiply2(v2sll{5 , 8}, v2sll{5 , 7}, CarryLessMultiply2SelectionMask(false, false)).v[0] == 17ll);
}
