#pragma once

#define VP_ENABLE_FARMHASH64
#ifdef VP_ENABLE_FARMHASH64

#include <farmhash64.hpp>

namespace vp::util {

    constexpr u64 FarmHash64(const char *string, u32 length) {
        return farmhashna::Hash64(string, length);
    }
    constexpr u64 FarmHash64(const char *string) {
        const u32 len = StringLength(string);
        return FarmHash64(string, len);
    }
}

#endif
