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

    template <typename T>
    concept IsChar8 = (std::is_same_v<std::add_cv_t<T>,std::add_cv_t<char>>) || (std::is_same_v<std::add_cv_t<T>,std::add_cv_t<char8_t>>) || (std::is_same_v<std::add_cv_t<T>,std::add_cv_t<u8>>);
    template <typename T>
    concept IsChar16 = (std::is_same_v<std::add_cv_t<T>,std::add_cv_t<char16_t>>) || (std::is_same_v<std::add_cv_t<T>,std::add_cv_t<wchar_t>>) || (std::is_same_v<std::add_cv_t<T>,std::add_cv_t<u16>>);
    template <typename T>
    concept IsChar32 = (std::is_same_v<std::add_cv_t<T>,std::add_cv_t<char32_t>>) || (std::is_same_v<std::add_cv_t<T>,std::add_cv_t<u32>>);
    template <typename T>
    concept IsChar = (IsChar8<T>) || (IsChar16<T>) || (IsChar32<T>);

    template <typename T>
    concept IsRawString8 = (std::is_same_v<std::remove_pointer_t<std::remove_extent_t<std::add_cv_t<T>>>,std::add_cv_t<char>>) || (std::is_same_v<std::remove_pointer_t<std::remove_extent_t<std::add_cv_t<T>>>,std::add_cv_t<char8_t>>) || (std::is_same_v<std::remove_pointer_t<std::remove_extent_t<std::add_cv_t<T>>>,std::add_cv_t<u8>>);
    template <typename T>
    concept IsRawString16 = (std::is_same_v<std::remove_pointer_t<std::remove_extent_t<std::add_cv_t<T>>>,std::add_cv_t<char16_t>>) || (std::is_same_v<std::remove_pointer_t<std::remove_extent_t<std::add_cv_t<T>>>,std::add_cv_t<wchar_t>>) || (std::is_same_v<std::remove_pointer_t<std::remove_extent_t<std::add_cv_t<T>>>,std::add_cv_t<u16>>);
    template <typename T>
    concept IsRawString32 = (std::is_same_v<std::remove_pointer_t<std::remove_extent_t<std::add_cv_t<T>>>,std::add_cv_t<char32_t>>) || (std::is_same_v<std::remove_pointer_t<std::remove_extent_t<std::add_cv_t<T>>>,std::add_cv_t<u32>>);
    template <typename T>
    concept IsRawString = (IsRawString8<T>) || (IsRawString16<T>) || (IsRawString32<T>);

    template <typename T>
    concept IsHeapString = (T::cIsHeapString8) || (T::cIsHeapString16) || (T::cIsHeapString32);
    template <typename T>
    concept IsFixedString = (T::cIsFixedString8) || (T::cIsFixedString16) || (T::cIsFixedString32);
    template <typename T>
    concept IsString = (IsHeapString<T>) || (IsFixedString<T>);
    template <typename T>
    concept IsString8 = (T::cIsHeapString8) || (T::cIsFixedString8);
    template <typename T>
    concept IsString16 = (T::cIsHeapString16) || (T::cIsFixedString16);
    template <typename T>
    concept IsString32 = (T::cIsHeapString32) || (T::cIsFixedString32);

    template <typename T>
    concept IsString8OrRaw = (IsString8<T>) || (IsRawString8<T>);
    template <typename T>
    concept IsString16OrRaw = (IsString16<T>) || (IsRawString16<T>);
    template <typename T>
    concept IsString32OrRaw = (IsString32<T>) || (IsRawString32<T>);
    template <typename T>
    concept IsStringOrRaw = (IsString8OrRaw<T>) || (IsString16OrRaw<T>) || (IsString32OrRaw<T>);

    namespace {

        template <typename T>
            requires (IsStringOrRaw<T>)
        struct UnderlyingCharTypeImpl {
            using Type = typename T::UnderlyingType;
        };
        template <>
        struct UnderlyingCharTypeImpl<char> {
            using Type = char;
        };
        template <>
        struct UnderlyingCharTypeImpl<char8_t> {
            using Type = char8_t;
        };
        template <>
        struct UnderlyingCharTypeImpl<u8> {
            using Type = u8;
        };
        template <>
        struct UnderlyingCharTypeImpl<wchar_t> {
            using Type = wchar_t;
        };
        template <>
        struct UnderlyingCharTypeImpl<char16_t> {
            using Type = char16_t;
        };
        template <>
        struct UnderlyingCharTypeImpl<u16> {
            using Type = u16;
        };
        template <>
        struct UnderlyingCharTypeImpl<char32_t> {
            using Type = char32_t;
        };
        template <>
        struct UnderlyingCharTypeImpl<u32> {
            using Type = u32;
        };
        template <>
        struct UnderlyingCharTypeImpl<const char> {
            using Type = const char;
        };
        template <>
        struct UnderlyingCharTypeImpl<const char8_t> {
            using Type = const char8_t;
        };
        template <>
        struct UnderlyingCharTypeImpl<const u8> {
            using Type = const u8;
        };
        template <>
        struct UnderlyingCharTypeImpl<const wchar_t> {
            using Type = const wchar_t;
        };
        template <>
        struct UnderlyingCharTypeImpl<const char16_t> {
            using Type = const char16_t;
        };
        template <>
        struct UnderlyingCharTypeImpl<const u16> {
            using Type = const u16;
        };
        template <>
        struct UnderlyingCharTypeImpl<const char32_t> {
            using Type = const char32_t;
        };
        template <>
        struct UnderlyingCharTypeImpl<const u32> {
            using Type = const u32;
        };
        template <size_t Len>
        struct UnderlyingCharTypeImpl<char[Len]> {
            using Type = char;
        };
        template <size_t Len>
        struct UnderlyingCharTypeImpl<char8_t[Len]> {
            using Type = char8_t;
        };
        template <size_t Len>
        struct UnderlyingCharTypeImpl<u8[Len]> {
            using Type = u8;
        };
        template <size_t Len>
        struct UnderlyingCharTypeImpl<wchar_t[Len]> {
            using Type = wchar_t;
        };
        template <size_t Len>
        struct UnderlyingCharTypeImpl<char16_t[Len]> {
            using Type = char16_t;
        };
        template <size_t Len>
        struct UnderlyingCharTypeImpl<u16[Len]> {
            using Type = u16;
        };
        template <size_t Len>
        struct UnderlyingCharTypeImpl<char32_t[Len]> {
            using Type = char32_t;
        };
        template <size_t Len>
        struct UnderlyingCharTypeImpl<u32[Len]> {
            using Type = u32;
        };
        template <size_t Len>
        struct UnderlyingCharTypeImpl<const char[Len]> {
            using Type = const char;
        };
        template <size_t Len>
        struct UnderlyingCharTypeImpl<const char8_t[Len]> {
            using Type = const char8_t;
        };
        template <size_t Len>
        struct UnderlyingCharTypeImpl<const u8[Len]> {
            using Type = const u8;
        };
        template <size_t Len>
        struct UnderlyingCharTypeImpl<const wchar_t[Len]> {
            using Type = const wchar_t;
        };
        template <size_t Len>
        struct UnderlyingCharTypeImpl<const char16_t[Len]> {
            using Type = const char16_t;
        };
        template <size_t Len>
        struct UnderlyingCharTypeImpl<const u16[Len]> {
            using Type = const u16;
        };
        template <size_t Len>
        struct UnderlyingCharTypeImpl<const char32_t[Len]> {
            using Type = const char32_t;
        };
        template <size_t Len>
        struct UnderlyingCharTypeImpl<const u32[Len]> {
            using Type = const u32;
        };
    }

    template <typename T>
        requires (IsStringOrRaw<T>)
    using UnderlyingCharType = typename UnderlyingCharTypeImpl<T>::Type;

#ifdef VP_TARGET_ARCHITECTURE_x86

    template <typename T>
        requires (IsRawString8<T>)
    constexpr u32 StringLength8(T *string) {

        /* Constexpr impl */
        if (std::is_constant_evaluated()) {
            const char *start = string;
            while (*string != '\0') { ++string; }
            return static_cast<u32>(string - start);
        }

        /* Handle unaligned string */
        const u32 unaligned = 0x20 - (reinterpret_cast<uintptr_t>(string) & 0x1f);
        for (u32 i = 0; i < unaligned; ++i) {
            if (string[i] == '\0') { return i; }
        }
        string = string + unaligned;

        /* Handle aligned string */
        using v32cc = Vector32cc;
        u32 i = unaligned;
        u32 mask = 0;
        const v32cc zero = {};
        for (;;) {

            /* Load 32 characters, compare if equal to zero, and convert result to a mask */
            v32cc c;
            Load(std::addressof(c), string);
            const v32cc result = CompareEqual(zero, c);
            mask = impl::MoveMask_8bit(result);

            /* Check if the mask has a bit set, meaning a character is non-zero */
            if (mask != 0) {
                const u32 offset = vp::util::CountRightZeroBits32(mask);
                return i + offset;
            }

            /* Advance while mask is not empty */
            i = i + 0x20;
            string = string + 0x20;
        }
    }

    template <typename T0, typename T1>
        requires (IsRawString8<T0>) && (IsRawString8<T1>)
    constexpr bool StringCompare8(T0 *lhs, T1 *rhs) {

        /* Constexpr impl */
        if (std::is_constant_evaluated()) {
            while (*lhs != '\0' || *rhs != '\0') { 
                if (*lhs != *rhs) { return false; }
                lhs += 1;
                rhs += 1;
            }
            return true;
        }

        /* Handle unaligned string */
        u32 i = 0;
        const u32 iter = 0x20;
        const char *l_iter = lhs;
        const char *r_iter = rhs;
        using v32cc = Vector32cc;
        const v32cc zero = {};
        for (;;) {

            /* Load 32 characters, compare if equal to zero, and convert result to a mask */
            v32cc l,r;
            Load(std::addressof(l), l_iter);
            Load(std::addressof(r), r_iter);
            const v32cc result0 = CompareEqual(l, zero);
            const v32cc result1 = CompareEqual(r, zero);
            const v32cc result2 = CompareEqual(l, r);
            const u32 mask0 = impl::MoveMask_8bit(result0);
            const u32 mask1 = impl::MoveMask_8bit(result1);
            const u32 mask2 = impl::MoveMask_8bit(result2);

            /* Check if the mask has a bit set, meaning a character is non-zero */
            if (mask0 != 0 || mask1 != 0 || mask2 != 0) {
                const u32 offset0 = vp::util::CountRightZeroBits32(mask0) + i;
                const u32 offset1 = vp::util::CountRightZeroBits32(mask1) + i;
                const u32 offset2 = vp::util::CountRightOneBits32(mask2) + i;
                return offset0 == offset1 && offset0 <= offset2;
            }

            /* Advance while mask is not empty */
            i = i + iter;
            l_iter = std::addressof(lhs[i]);
            r_iter = std::addressof(rhs[i]);
        }

        return true;
    }
#else

    template <typename T>
        requires (IsRawString8<T>)
    constexpr u32 StringLength8(T *string) {

        /* Handle unaligned string */
        u32 i = 0;
        while (string[i] != '\0') { ++i; }

        return i;
    }

    template <typename T0, typename T1>
        requires (IsRawString8<T0>) && (IsRawString8<T1>)
    constexpr bool StringCompare8(T0 *lhs, T1 *rhs) {
        while (*lhs != '\0' || *rhs != '\0') { 
            if (*lhs != *rhs) { return false; }
            lhs += 1;
            rhs += 1;
        }
        return true;
    }
#endif

    template <typename T>
        requires (IsRawString16<T>)
    constexpr u32 StringLength16(T *string) {

        /* Handle unaligned string */
        u32 i = 0;
        while (string[i] != u'\0') { ++i; }

        return i;
    }

    template <typename T0, typename T1>
        requires (IsRawString16<T0>) && (IsRawString16<T1>)
    constexpr bool StringCompare16(T0 *lhs, T1 *rhs) {
        while (*lhs != u'\0' || *rhs != u'\0') { 
            if (*lhs != *rhs) { return false; }
            lhs += 1;
            rhs += 1;
        }
        return true;
    }

    template <typename T>
        requires (IsRawString32<T>)
    constexpr u32 StringLength32(T *string) {

        /* Handle unaligned string */
        u32 i = 0;
        while (string[i] != 0u) { ++i; }

        return i;
    }

    template <typename T0, typename T1>
        requires (IsRawString32<T0>) && (IsRawString32<T1>)
    constexpr bool StringCompare32(T0 *lhs, T1 *rhs) {
        while (*lhs != 0u || *rhs != 0u) { 
            if (*lhs != *rhs) { return false; }
            lhs += 1;
            rhs += 1;
        }
        return true;
    }

    template <typename T>
        requires (IsStringOrRaw<T>)
    constexpr u32 StringLength(const T *string) {
        if constexpr (IsRawString8<T>) {
            return StringLength8(string);
        } else if constexpr (IsRawString16<T>) {
            return StringLength16(string);
        } else if constexpr (IsRawString32<T>) {
            return StringLength32(string);
        } else if constexpr (IsString<T>) {
            return string->GetLength();
        } else {
            _consteval_fail();
        }
    }

    template <typename T0, typename T1>
        requires (IsRawString<T0> && IsRawString<T1>) || (IsString<T0> && IsStringOrRaw<T1>) || (IsStringOrRaw<T0> && IsString<T1>) && (sizeof(UnderlyingCharType<T0>) == sizeof(UnderlyingCharType<T1>))
    constexpr bool StringCompare(const T0 *lhs, const T1 *rhs) {
        if constexpr (IsRawString8<T0> && IsRawString8<T1>) {
            return StringCompare8(lhs, rhs);
        } else if constexpr (IsRawString16<T0> && IsRawString16<T1>) {
            return StringCompare16(lhs, rhs);
        } else if constexpr (IsRawString32<T0> && IsRawString32<T1>) {
            return StringCompare32(lhs, rhs);
        } else if constexpr (IsString<T0> && IsString<T1>) {
            return *lhs == *rhs;
        }  else if constexpr (IsString<T0>) {
            return *lhs == rhs;
        } else if constexpr (IsString<T1>) {
            return *rhs == lhs;
        } else {
            _consteval_fail();
        }
    }

    template <typename T>
        requires (IsStringOrRaw<T>)
    constexpr inline void GetUnderlyingString(const UnderlyingCharType<T> **out_string, u32 *out_len, T *string) {
        const UnderlyingCharType<T> *path;
        if constexpr (IsString<T>) {
            path = string->GetString();
        } else if constexpr (std::is_array_v<std::remove_pointer_t<T>>) {
            path = *string;
        } else {
            path = string;
        }
        if (out_string != nullptr) {            
            *out_string = path;
        }
        if (out_len != nullptr) {            
            const u32 path_len = vp::util::StringLength(path);
            *out_len = path_len;
        }
        return;
    }

    
}
