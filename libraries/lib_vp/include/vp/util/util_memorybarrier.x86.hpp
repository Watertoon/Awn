#pragma once

namespace vp::util {

    ALWAYS_INLINE void MemoryBarrierReadWrite() {
        __builtin_ia32_mfence();
        return;
    }
}
