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

    constexpr float ConvertUnorm8ToFloat(u8 value) {
        return static_cast<float>(value) / 255.0f;
    }

    constexpr u8 ConvertFloatToUnorm8(float value) {
        const float clamp0 = Min(value, 1.0f);
        const float clamp1 = Max(clamp0, 0.0f);
        return static_cast<u8>((clamp1 * 255.0f) + 0.5f);
    }

    constexpr s16 ConvertFloatToSnorm16(float value) {
        const s32 base = static_cast<s32>(value * 32767.0f);
        const s32 clamp0 = Max(base, -32768);
        const s32 clamp1 = Min(clamp0, 32767);
        return static_cast<s16>(clamp1);
    }

    constexpr s16 ConvertSnorm16ToFloat(s16 value) {
        const float base = (value < 0) ? static_cast<float>(value) / 32768.0f : static_cast<float>(value) / 32767.0f;
        const float clamp0 = Min(base, 1.0f);
        const float clamp1 = Max(clamp0, -1.0f);
        return static_cast<s16>(clamp1);
    }

    constexpr float ConvertFloat16ToFloat(s16 value) {
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

    constexpr s16 ConvertFloatToFloat16(float value) {
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

    struct Color4f;

    struct Color4u8 {
        union {
            struct {
                u8 r;
                u8 g;
                u8 b;
                u8 a;
            };
            u32 packed_color;
        };

        constexpr ALWAYS_INLINE Color4u8 &operator=(const Color4f &rhs);
    };

    struct Color4f {
        float r;
        float g;
        float b;
        float a;

        constexpr ALWAYS_INLINE Color4f &operator=(const Color4u8 &rhs) {
            r = ConvertUnorm8ToFloat(rhs.r);
            g = ConvertUnorm8ToFloat(rhs.g);
            b = ConvertUnorm8ToFloat(rhs.b);
            a = ConvertUnorm8ToFloat(rhs.a);
            return *this;
        }
    };

    constexpr ALWAYS_INLINE Color4u8 &Color4u8::operator=(const Color4f &rhs) {
        r = ConvertFloatToUnorm8(rhs.r);
        g = ConvertFloatToUnorm8(rhs.g);
        b = ConvertFloatToUnorm8(rhs.b);
        a = ConvertFloatToUnorm8(rhs.a);
        return *this;
    }

    constexpr inline Color4u8 cBlack4u8 = {0x00, 0x00, 0x00, 0xff};
    constexpr inline Color4u8 cWhite4u8 = {0xff, 0xff, 0xff, 0xff};

    constexpr inline Color4f cBlack4f   = {0.0f, 0.0f, 0.0f, 1.0f};
    constexpr inline Color4f cWhite4f   = {1.0f, 1.0f, 1.0f, 1.0f};

    constexpr ALWAYS_INLINE Color4f operator-(const Color4f &lhs, const Color4f &rhs) {
        const Color4f new_color = {
            lhs.r - rhs.r,
            lhs.g - rhs.g,
            lhs.b - rhs.b,
            lhs.a - rhs.a,
        };
        return new_color;
    }

    constexpr ALWAYS_INLINE Color4u8 operator*(const Color4u8 &lhs, const Color4u8 &rhs) {
        const Color4u8 new_color = {
            static_cast<u8>(static_cast<u32>(lhs.r) * static_cast<u32>(rhs.r) / 0xff),
            static_cast<u8>(static_cast<u32>(lhs.g) * static_cast<u32>(rhs.g) / 0xff),
            static_cast<u8>(static_cast<u32>(lhs.b) * static_cast<u32>(rhs.b) / 0xff),
            static_cast<u8>(static_cast<u32>(lhs.a) * static_cast<u32>(rhs.a) / 0xff),
        };
        return new_color;
    }
}
