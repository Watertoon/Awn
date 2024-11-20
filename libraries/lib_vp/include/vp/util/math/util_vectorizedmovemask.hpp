#pragma once

namespace vp::util {

    constexpr u32 MoveMask128(const v16cc &a) {

        /* Runtime intrinsic  */
        if (!std::is_constant_evaluated()) {
#ifdef VP_TARGET_ARCHITECTURE_x86
            return _mm_movemask_epi8(reinterpret_cast<__m128i>(a.v));
#endif
        }

        /* Build mask of most significant bit in 8-bit component */
        u32 index = 0;
        u32 out   = 0;
        for (u32 i = 0; i < 0x10; ++i) {
            index  = i * 8;
            out   |= ((a[index] >> 0x7) << i);
        }
        return out;
    }

    constexpr u32 MoveMask256(const v32cc &a) {

        /* Runtime intrinsic  */
        if (!std::is_constant_evaluated()) {
#ifdef VP_TARGET_ARCHITECTURE_x86
            return _mm256_movemask_epi8(reinterpret_cast<__m256i>(a.v));
#endif
        }

        /* Build mask of most significant bit in 8-bit component */
        u32 index = 0;
        u32 out   = 0;
        for (u32 i = 0; i < 0x20; ++i) {
            index  = i * 8;
            out   |= ((a[index] >> 0x7) << i);
        }
        return out;
    }
}
