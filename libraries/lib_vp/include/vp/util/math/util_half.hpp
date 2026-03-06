#pragma once

namespace vp::util {

    constexpr u16 ConvertFloatToHalf(float value) {
        const u32 cast = std::bit_cast<u32>(value);
        const u32 sign = (cast >> 0x1f) << 0xf;
        const u32 abs = cast & 0x7fff'ffff;
        const u32 mantissa = (cast >> 23) & 0xff;
        if (mantissa < 0x8f) {
            const u32 abs_mant = abs >> 23;
            const u32 bits = (cast & 0x7f'ffff) | 0x80'0000;
            const u32 bits1 = (0x32 < (abs >> 24)) ? (bits + ~(static_cast<u32>(-1) << static_cast<u32>((0x7d - abs_mant) & 0x1f)) + (bits >> ((((0x7e - abs_mant) & 0x1f)) & 1))) >> ((0x7e - abs_mant) & 0x1f) : 0;
            const u32 bits2 = (0x70 < abs_mant) ? (abs + ((cast >> 0xd) & 1) + 0xc800'0fff) >> 0xd : bits1;
            return static_cast<u16>(bits2 | sign);
        }
        const u32 bits = (0x7f80'0000 < abs) ? ((cast >> 0xd) & 0x1ff) | 0x7e00 : 0x7c00;
        return static_cast<u16>(bits | sign);
    }

#ifdef VP_TARGET_ARCHITECTURE_x86
    template <typename T>
        requires (T::cCount == 4 && std::is_same_v<typename T::UnderlyingType, float>)
    Vector8h ConvertFloat4ToHalf4(const T &vec) {
        const Vector8h ret = Vector8h{ .v = _mm_cvtps_ph(vec.v, 0), };
        return ret;
    }

    template <typename T>
        requires (T::cCount == 8 && std::is_same_v<typename T::UnderlyingType, float>)
    Vector8h ConvertFloat8ToHalf8(const T &vec) {
        const Vector8h ret = Vector8h{ .v = _mm256_cvtps_ph(vec.v, 0), };
        return ret;
    }
#endif

    constexpr float ConvertHalfToFloat(s16 value) {
        const s32 cast = static_cast<s32>(value);
        const s32 mantissa = cast & 0x7c00;
        const s32 sign = cast & 0x8000'0000;
        const u32 ucast = static_cast<u32>(value);
        if (mantissa == 0x7c00) {
            const float result = ((ucast & 0x3ff) != 0) ? std::bit_cast<float>(sign | 0x7fc0'0000 | ((ucast & 0x3ff) << 0xd)) : std::bit_cast<float>(sign | 0x7f80'0000);
            return result;
        }
        if (mantissa != 0) {
            const float result = std::bit_cast<float>(sign | (mantissa * 0x2000 + 0x3800'0000) | ((ucast & 0x3ff) << 0xd));
            return result;
        }
        if ((ucast & 0x3ff) == 0) {
            return std::bit_cast<float>(sign);
        }
        const u32 bits = vp::util::CountLeftZeroBits(ucast);
        const u32 r_mant = (sign | 0x4300'0000) + bits * 0xff80'0000;
        const u32 r_bits = (ucast << ((bits - 8) & 0x1f)) & 0x7f'ffff;
        return std::bit_cast<float>(r_mant | r_bits);
    }

    //v4f ConvertHalf4ToFloat4(const v8ss &vec) {
    //    const v8ss ret = v8ss{ .v = _mm_cvtph_ps(vec.v), };
    //    return ret;
    //}
    //
    //v8f ConvertHalf8ToFloat8(const v8ss &vec) {
    //    const v8ss ret = v16ss{ .v = _mm256_cvtph_ps(vec.v), };
    //    return ret;
    //}
}
