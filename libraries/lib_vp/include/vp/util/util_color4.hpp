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

    constexpr float Unorm8ToFloat(u8 value) {
        return static_cast<float>(value) / 255.0f;
    }

    constexpr u8 FloatToUnorm8(float value) {
        const float clamp0 = Min(value, 1.0f);
        const float clamp1 = Max(clamp0, 0.0f);
        return static_cast<u8>(clamp1 * 255.0f);
    }

    constexpr u8 FloatToUnorm16(float value) {
        const float clamp0 = Min(value, 1.0f);
        const float clamp1 = Max(clamp0, 0.0f);
        return static_cast<u16>(clamp1 * 65535.0f);
    }

    constexpr u32 FloatToUnorm32(float value) {
        const float clamp0 = Min(value, 1.0f);
        const float clamp1 = Max(clamp0, 0.0f);
        return static_cast<u32>(static_cast<double>(clamp1) * 4'294'967'295.0);
    }

    constexpr s16 FloatToSnorm16(float value) {
        const s32 base = static_cast<s32>(value * 32767.0f);
        const s32 clamp0 = Max(base, -32768);
        const s32 clamp1 = Min(clamp0, 32767);
        return static_cast<s16>(clamp1);
    }

    constexpr s16 Snorm16ToFloat(s16 value) {
        const float base = (value < 0) ? static_cast<float>(value) / 32768.0f : static_cast<float>(value) / 32767.0f;
        const float clamp0 = Min(base, 1.0f);
        const float clamp1 = Max(clamp0, -1.0f);
        return static_cast<s16>(clamp1);
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
        union {
            struct {                
                float r;
                float g;
                float b;
                float a;
            };
            Float4 v;
        };

        constexpr ALWAYS_INLINE Color4f &operator=(const Color4u8 &rhs) {
            r = Unorm8ToFloat(rhs.r);
            g = Unorm8ToFloat(rhs.g);
            b = Unorm8ToFloat(rhs.b);
            a = Unorm8ToFloat(rhs.a);
            return *this;
        }
    };

    constexpr ALWAYS_INLINE Color4u8 &Color4u8::operator=(const Color4f &rhs) {
        r = FloatToUnorm8(rhs.r);
        g = FloatToUnorm8(rhs.g);
        b = FloatToUnorm8(rhs.b);
        a = FloatToUnorm8(rhs.a);
        return *this;
    }
    constexpr ALWAYS_INLINE Color4f Color4u8ToColor4f(const Color4u8 &color) {
        Color4f ret;
        ret.r = Unorm8ToFloat(color.r);
        ret.g = Unorm8ToFloat(color.g);
        ret.b = Unorm8ToFloat(color.b);
        ret.a = Unorm8ToFloat(color.a);
        return ret;
    }
    constexpr ALWAYS_INLINE Color4u8 Color4fToColor4u8(const Color4f &color) {
        Color4u8 ret;
        ret.r = FloatToUnorm8(color.r);
        ret.g = FloatToUnorm8(color.g);
        ret.b = FloatToUnorm8(color.b);
        ret.a = FloatToUnorm8(color.a);
        return ret;
    }

    static constexpr inline Color4u8 cBlack4u8   = {0x00, 0x00, 0x00, 0xff,};
    static constexpr inline Color4u8 cWhite4u8   = {0xff, 0xff, 0xff, 0xff,};
    static constexpr inline Color4u8 cRed4u8     = {0xff, 0x00, 0x00, 0xff,};
    static constexpr inline Color4u8 cGreen4u8   = {0x00, 0xff, 0x00, 0xff,};
    static constexpr inline Color4u8 cBlue4u8    = {0x00, 0x00, 0xff, 0xff,};
    static constexpr inline Color4u8 cCyan4u8    = {0x00, 0xff, 0xff, 0xff,};
    static constexpr inline Color4u8 cMagenta4u8 = {0xff, 0x00, 0xff, 0xff,};
    static constexpr inline Color4u8 cYellow4u8  = {0xff, 0xff, 0x00, 0xff,};
    static constexpr inline Color4u8 cGray4u8    = {0x80, 0x80, 0x80, 0xff,};

    static constexpr inline Color4f cBlack4f   = {0.0f, 0.0f, 0.0f, 1.0f,};
    static constexpr inline Color4f cWhite4f   = {1.0f, 1.0f, 1.0f, 1.0f,};
    static constexpr inline Color4f cRed4f     = {1.0f, 0.0f, 0.0f, 1.0f,};
    static constexpr inline Color4f cGreen4f   = {0.0f, 1.0f, 0.0f, 1.0f,};
    static constexpr inline Color4f cBlue4f    = {0.0f, 0.0f, 1.0f, 1.0f,};
    static constexpr inline Color4f cCyan4f    = {0.0f, 1.0f, 1.0f, 1.0f,};
    static constexpr inline Color4f cMagenta4f = {1.0f, 0.0f, 1.0f, 1.0f,};
    static constexpr inline Color4f cYellow4f  = {1.0f, 1.0f, 0.0f, 1.0f,};
    static constexpr inline Color4f cGray4f    = {0.5f, 0.5f, 0.5f, 1.0f,};

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
            static_cast<u8>(static_cast<u32>(lhs.r) * static_cast<u32>(rhs.r) / 0xffu),
            static_cast<u8>(static_cast<u32>(lhs.g) * static_cast<u32>(rhs.g) / 0xffu),
            static_cast<u8>(static_cast<u32>(lhs.b) * static_cast<u32>(rhs.b) / 0xffu),
            static_cast<u8>(static_cast<u32>(lhs.a) * static_cast<u32>(rhs.a) / 0xffu),
        };
        return new_color;
    }
}
