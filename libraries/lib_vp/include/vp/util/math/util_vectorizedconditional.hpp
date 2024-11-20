#pragma once

namespace vp::util {

    constexpr v32cc CompareEqual32(const v32cc &a, const v32cc &b) {

        /* Runtime intrinsic  */
        if (!std::is_constant_evaluated()) {
#ifdef VP_TARGET_ARCHITECTURE_x86
            return v32cc{ .v = reinterpret_cast<char __attribute__((vector_size(32)))>(_mm256_cmpeq_epi8(reinterpret_cast<__m256i>(a.v), reinterpret_cast<__m256i>(b.v))), };
#endif
        }

        /* Compare 32 bytes, setting each output byte to 0xff if same, else 0 */
        v32cc out = {};
        for (u32 i = 0; i < 0x20; ++i) {
            out[i] = (a[i] == b[i]) ? 0xff : 0;
        }

        return out;
    }
}
