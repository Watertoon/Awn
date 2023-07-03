#pragma once

namespace vp::util {

    template <typename T>
        requires (std::is_integral<T>::value) && (sizeof(T) == sizeof(u64) || sizeof(T) == sizeof(u32) || sizeof(T) == sizeof(u16) || sizeof(T) == sizeof(u8))
    constexpr ALWAYS_INLINE T SwapEndian(T value) {
        if constexpr (sizeof(T) == sizeof(u8)) {
            return value;
        } else if constexpr (sizeof(T) == sizeof(u16)) {
            const u32 swap0 = ((value & 0xff00) >> 0x8) | ((value & 0x00ff) << 0x8);
            return swap0;
        } else if constexpr (sizeof(T) == sizeof(u32)) {
            const u32 swap0 = ((value & 0xff00'ff00) >> 0x8) | ((value & 0x00ff'00ff) << 0x8);
            const u32 swap1 = (swap0 >> 0x10) | (swap0 << 0x10);
            return swap1;
        } else if constexpr (sizeof(T) == sizeof(u64)) {
            const u64 swap0 = ((value & 0xff00'ff00'ff00'ff00) >> 0x8) | ((value & 0x00ff'00ff'00ff'00ff) << 0x8);
            const u64 swap1 = ((swap0 & 0xffff'0000'ffff'0000) >> 0x10) | ((swap0 & 0x0000'ffff'0000'ffff) << 0x10);
            const u64 swap2 = (swap1 >> 0x20) | (swap1 << 0x20);
            return swap2;
        }
    }
}
