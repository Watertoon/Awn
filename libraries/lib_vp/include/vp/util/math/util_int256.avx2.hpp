#pragma once

namespace vp::util::avx2 {

    /* Load, Store, Set */
    constexpr u32 vpmovmskb(const v32c &a) {

        /* Runtime intrinsic  */
        if (std::is_constant_evaluated() == false) {
            return __builtin_ia32_pmovmskb256(a.cc);
        }

        /* Build mask of most significant bit in 8-bit component */
        u32 index = 0;
        u32 out   = 0;
        for (u32 i = 0; i < 0x20; ++i) {
            index  = i * 8;
            out   |= ((a.uc[index] >> 0x7) << i);
        }
        return out;
    }

    constexpr v32c vmovdqa(const char *a) {

        /* Runtime intrinsic  */
        if (std::is_constant_evaluated() == false) {
            return v32c(__builtin_ia32_loaddqu256(a));
        }

        /* Read 32 bytes */
        const v32cc out = {
            a[0x0],
            a[0x1],
            a[0x2],
            a[0x3],
            a[0x4],
            a[0x5],
            a[0x6],
            a[0x7],
            a[0x8],
            a[0x9],
            a[0xa],
            a[0xb],
            a[0xc],
            a[0xd],
            a[0xe],
            a[0xf],
            a[0x10],
            a[0x11],
            a[0x12],
            a[0x13],
            a[0x14],
            a[0x15],
            a[0x16],
            a[0x17],
            a[0x18],
            a[0x19],
            a[0x1a],
            a[0x1b],
            a[0x1c],
            a[0x1d],
            a[0x1e],
            a[0x1f],
        };
        return out;
    }

    constexpr v32c vcmpeqb(const v32c &a, const v32c &b) {

        /* Runtime intrinsic  */
        if (std::is_constant_evaluated() == false) {
            return v32c(__builtin_ia32_pcmpeqb256(a.cc, b.cc));
        }

        /* Compare 32 bytes, setting each output byte to 0xff if same, else 0 */
        v32c out = {};
        for (u32 i = 0; i < 0x20; ++i) {
            out.uc[i] = (a.uc[i] == b.uc[i]) ? 0xff : 0;
        }

        return out;
    }
}
