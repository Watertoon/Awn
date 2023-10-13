#pragma once

namespace vp::util {

    ALWAYS_INLINE void MemoryBarrierReadWrite() {
        asm volatile(
            "dmb ish"
            :
            :
            : "memory"
        );
        return;
    }
}
