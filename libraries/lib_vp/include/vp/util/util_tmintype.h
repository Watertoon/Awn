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

    static constexpr u8  cMinU8  = 0;
    static constexpr u16 cMinU16 = 0;
    static constexpr u32 cMinU32 = 0;
    static constexpr u64 cMinU64 = 0;

    static constexpr u8  cMaxU8  = 0xff;
    static constexpr u16 cMaxU16 = 0xffff;
    static constexpr u32 cMaxU32 = 0xffff'ffff;
    static constexpr u64 cMaxU64 = 0xffff'ffff'ffff'ffff;

    static constexpr s8  cMinS8  = 0x80;
    static constexpr s16 cMinS16 = 0x8000;
    static constexpr s32 cMinS32 = 0x8000'0000;
    static constexpr s64 cMinS64 = 0x8000'0000'0000'0000;

    static constexpr s8  cMaxS8  = 0x7f;
    static constexpr s16 cMaxS16 = 0x7fff;
    static constexpr s32 cMaxS32 = 0x7fff'ffff;
    static constexpr s64 cMaxS64 = 0x7fff'ffff'ffff'ffff;

	namespace {

        enum class MinUIntTypeEnum : u32 {
            U8  = 0,
            U16 = 1,
            U32 = 2,
            U64 = 3,
        };

        template <typename T>
            requires (std::is_unsigned<T>::value && std::is_integral<T>::value)
        consteval MinUIntTypeEnum GetMinUIntTypeByMaxValue(T value) {
            if (value > cMaxU32) {
                return MinUIntTypeEnum::U64;
            } else if (value > cMaxU16) {
                return MinUIntTypeEnum::U32;
            } else if (value > cMaxU8) {
                return MinUIntTypeEnum::U16;
            } else {
                return MinUIntTypeEnum::U8;
            }
        }

        template <MinUIntTypeEnum T>
        struct MinUIntTypeHelper {};

        template<>
        struct MinUIntTypeHelper<MinUIntTypeEnum::U8> {
            using Type = u8;
        };
        template<>
        struct MinUIntTypeHelper<MinUIntTypeEnum::U16> {
            using Type = u16;
        };
        template<>
        struct MinUIntTypeHelper<MinUIntTypeEnum::U32> {
            using Type = u32;
        };
        template<>
        struct MinUIntTypeHelper<MinUIntTypeEnum::U64> {
            using Type = u64;
        };
    }

	template <u64 T>
        requires (std::is_unsigned<decltype(T)>::value && std::is_integral<decltype(T)>::value)
    using MinUIntType = MinUIntTypeHelper<GetMinUIntTypeByMaxValue(T)>::Type;

    static_assert(std::is_same<MinUIntType<cMinU8>,                        u8>::value);
    static_assert(std::is_same<MinUIntType<cMaxU8>,                        u8>::value);
    static_assert(std::is_same<MinUIntType<static_cast<u16>(cMaxU8) + 1>,  u16>::value);
    static_assert(std::is_same<MinUIntType<cMaxU16>,                       u16>::value);
    static_assert(std::is_same<MinUIntType<static_cast<u32>(cMaxU16) + 1>, u32>::value);
    static_assert(std::is_same<MinUIntType<cMaxU32>,                       u32>::value);
    static_assert(std::is_same<MinUIntType<static_cast<u64>(cMaxU32) + 1>, u64>::value);
    static_assert(std::is_same<MinUIntType<cMaxU64>,                       u64>::value);

	namespace {

        enum class MinIntTypeEnum : u32 {
            S8  = 0,
            S16 = 1,
            S32 = 2,
            S64 = 3,
        };

        template <typename T>
            requires (std::is_signed<T>::value && std::is_integral<T>::value)
        consteval MinIntTypeEnum GetMinIntTypeByMaxValue(T value) {
            if (value < cMinS32 || cMaxS32 < value) {
                return MinIntTypeEnum::S64;
            } else if (value < cMinS16 || cMaxS16 < value) {
                return MinIntTypeEnum::S32;
            } else if (value < cMinS8 || cMaxS8 < value) {
                return MinIntTypeEnum::S16;
            } else {
                return MinIntTypeEnum::S8;
            }
        }

        template <MinIntTypeEnum T>
        struct MinIntTypeHelper {};

        template<>
        struct MinIntTypeHelper<MinIntTypeEnum::S8> {
            using Type = s8;
        };
        template<>
        struct MinIntTypeHelper<MinIntTypeEnum::S16> {
            using Type = s16;
        };
        template<>
        struct MinIntTypeHelper<MinIntTypeEnum::S32> {
            using Type = s32;
        };
        template<>
        struct MinIntTypeHelper<MinIntTypeEnum::S64> {
            using Type = s64;
        };
    }

	template <s64 T>
        requires (std::is_signed<decltype(T)>::value && std::is_integral<decltype(T)>::value)
    using MinIntType = MinIntTypeHelper<GetMinIntTypeByMaxValue(T)>::Type;

    static_assert(std::is_same<MinIntType<0>,                             s8>::value);
    static_assert(std::is_same<MinIntType<cMinS8>,                        s8>::value);
    static_assert(std::is_same<MinIntType<cMaxS8>,                        s8>::value);
    static_assert(std::is_same<MinIntType<static_cast<s16>(cMinS8) - 1>,  s16>::value);
    static_assert(std::is_same<MinIntType<static_cast<s16>(cMaxS8) + 1>,  s16>::value);
    static_assert(std::is_same<MinIntType<cMinS16>,                       s16>::value);
    static_assert(std::is_same<MinIntType<cMaxS16>,                       s16>::value);
    static_assert(std::is_same<MinIntType<static_cast<s32>(cMinS16) - 1>, s32>::value);
    static_assert(std::is_same<MinIntType<static_cast<s32>(cMaxS16) + 1>, s32>::value);
    static_assert(std::is_same<MinIntType<cMinS32>,                       s32>::value);
    static_assert(std::is_same<MinIntType<cMaxS32>,                       s32>::value);
    static_assert(std::is_same<MinIntType<static_cast<s64>(cMinS32) - 1>, s64>::value);
    static_assert(std::is_same<MinIntType<static_cast<s64>(cMaxS32) + 1>, s64>::value);
    static_assert(std::is_same<MinIntType<cMinS64>,                       s64>::value);
    static_assert(std::is_same<MinIntType<cMaxS64>,                       s64>::value);
}
