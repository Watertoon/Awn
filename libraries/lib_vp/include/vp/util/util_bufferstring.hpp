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

#ifdef VP_TARGET_ARCHITECTURE_x86

    constexpr u32 StringLength(const char *string) {

        /* Handle unaligned string */
        const u32 unaligned = 0x20 - reinterpret_cast<uintptr_t>(string) & 0x1f;
        for (u32 i = 0; i < unaligned; ++i) {
            if (string[i] == '\0') { return i; }
        }
        string = string + unaligned;

        /* Handle aligned string */
        u32 i = unaligned;
        u32 mask = 0;
        const v32c zero = {};
        for (;;) {

            /* Load 32 characters, compare with zero, and convert result to a mask */
            const v32c characters = avx2::vmovdqa(string + i);
            const v32c result     = avx2::vcmpeqb(zero, characters);
            mask                  = avx2::vpmovmskb(result);

            /* Check if the mask has a bit set, meaning a character is non-zero */
            if (mask != 0) {
                const u32 offset = vp::util::CountRightZeroBits32(mask);
                return i + offset;
            }

            /* Advance while mask is not empty */
            i = i + 0x20;
        }
    }
#else
    constexpr u32 StringLength(const char *string) {

        /* Handle unaligned string */
        u32 i = 0;
        while (string[i] != '\0') { ++i; }

        return i;
    }
#endif

    template <size_t Size>
        requires (Size < 0x40'0000) && (0 < Size)
    class FixedString {
        public:
            static constexpr u32 cInvalidIndex = 0xffff'ffff;
        public:
            char m_string_array[Size];
            u32  m_length;
        public:
            explicit constexpr ALWAYS_INLINE FixedString() : m_string_array{'\0'}, m_length(0) {/*...*/}
            explicit constexpr ALWAYS_INLINE FixedString(const char *string) : m_string_array{'\0'}, m_length(0) { 
                if (string == nullptr) { return; }

                if (std::is_constant_evaluated() == true) {
                    m_length = TStringCopy(m_string_array, string, TStringLength(string, Size - 1));
                } else {
                    const size_t len = ::strlen(string);
                    ::memcpy(m_string_array, string, len);
                    m_string_array[len + 1] = '\0';
                    m_length = len; 
                }

                this->AssureTermination();
            }
            explicit constexpr FixedString(const FixedString<Size> &rhs) : m_string_array{'\0'} { 
                if (std::is_constant_evaluated() == true) {
                    m_length = TStringCopy(m_string_array, rhs.m_string_array, TStringLength(rhs.m_string_array, Size - 1));
                } else {
                    const size_t len = (rhs.m_length < Size - 1) ? rhs.m_length : Size - 1;
                    ::memcpy(m_string_array, rhs.m_string_array, len);
                    m_string_array[len + 1] = '\0';
                    m_length = len;
                }
                this->AssureTermination();
            }

            constexpr ALWAYS_INLINE ~FixedString() {/*...*/}

            constexpr ALWAYS_INLINE bool operator==(const FixedString& rhs) {
                if (std::is_constant_evaluated() == true) {
                    return TStringCompare(m_string_array, rhs.m_string_array, TStringLength(rhs.m_string_array, Size - 1));
                }

                if (m_length != rhs.m_length) { return false; }
                return ::memcmp(m_string_array, rhs.m_string_array, m_length) == 0;
            }
            constexpr ALWAYS_INLINE bool operator==(const FixedString& rhs) const {
                if (std::is_constant_evaluated() == true) {
                    return TStringCompare(m_string_array, rhs.m_string_array, TStringLength(rhs.m_string_array, Size - 1));
                }

                if (m_length != rhs.m_length) { return false; }
                return ::memcmp(m_string_array, rhs.m_string_array, m_length) == 0;
            }
            constexpr ALWAYS_INLINE bool operator==(const char *rhs) {
                if (std::is_constant_evaluated() == true) {
                    return TStringCompare(m_string_array, rhs, TStringLength(rhs, Size - 1));
                }

                const u32 length =  StringLength(rhs);
                if (m_length != length) { return false; }
                return ::memcmp(m_string_array, rhs, length) == 0;
            }
            constexpr ALWAYS_INLINE bool operator==(const char *rhs) const {
                if (std::is_constant_evaluated() == true) {
                    return TStringCompare(m_string_array, rhs, TStringLength(rhs, Size - 1));
                }

                const u32 length =  StringLength(rhs);
                if (m_length != length) { return false; }
                return ::memcmp(m_string_array, rhs, length) == 0;
            }
            constexpr ALWAYS_INLINE bool operator!=(const FixedString& rhs) {
                if (std::is_constant_evaluated() == true) {
                    return (TStringCompare(m_string_array, rhs.m_string_array, TStringLength(rhs.m_string_array, Size - 1)) != 0);
                }

                return (m_length != rhs.m_length) || ::memcmp(m_string_array, rhs.m_string_array, m_length) == 0;
            }
            constexpr ALWAYS_INLINE bool operator!=(const FixedString& rhs) const {
                if (std::is_constant_evaluated() == true) {
                    return (TStringCompare(m_string_array, rhs.m_string_array, TStringLength(rhs.m_string_array, Size - 1)) != 0);
                }

                return (m_length != rhs.m_length) || ::memcmp(m_string_array, rhs.m_string_array, m_length) == 0;
            }

            constexpr ALWAYS_INLINE bool operator>(const FixedString& rhs) {
                if (m_length != rhs.m_length) { return false; }
                return ::memcmp(m_string_array, rhs.m_string_array, m_length) < 0;
            }
            constexpr ALWAYS_INLINE bool operator>(const FixedString& rhs) const {
                if (m_length != rhs.m_length) { return false; }
                return ::memcmp(m_string_array, rhs.m_string_array, m_length) < 0;
            }
            constexpr ALWAYS_INLINE bool operator<(const FixedString& rhs) {
                if (m_length != rhs.m_length) { return false; }
                return ::memcmp(m_string_array, rhs.m_string_array, m_length) > 0;
            }
            constexpr ALWAYS_INLINE bool operator<(const FixedString& rhs) const {
                if (m_length != rhs.m_length) { return false; }
                return ::memcmp(m_string_array, rhs.m_string_array, m_length) > 0;
            }

            constexpr void RecomputeLength() {
                m_length = StringLength(m_string_array);
            }

            constexpr ALWAYS_INLINE bool IsSame(const char *string) const {
                return ::strncmp(m_string_array, string, m_length) == 0;
            }

            constexpr ALWAYS_INLINE void Clear() {
                m_length = 0;
                m_string_array[0] = '\0';
                return;
            }

            constexpr ALWAYS_INLINE FixedString &operator=(const FixedString& rhs) {
                if (std::is_constant_evaluated() == true) {
                    m_length = TStringCopy(m_string_array, rhs.m_string_array, TStringLength(rhs.m_string_array, Size - 1));
                } else {
                    const size_t len = (rhs.m_length < Size - 1) ? rhs.m_length : Size - 1;
                    ::memcpy(m_string_array, rhs.m_string_array, len);
                    m_string_array[len + 1] = '\0';
                    m_length = len;
                }
                this->AssureTermination();

                return *this;
            }

            constexpr ALWAYS_INLINE FixedString &operator=(const char* rhs) {
                if (rhs == nullptr) { 
                    m_string_array[0] = '\0';
                    m_length          = 0;
                    return *this; 
                }

                if (std::is_constant_evaluated()) {
                    m_length = TStringCopy(m_string_array, rhs, TStringLength(rhs, Size - 1));
                } else {
                    const size_t len = ::strlen(rhs);
                    ::memcpy(m_string_array, rhs, len);
                    m_string_array[len + 1] = '\0';
                    m_length = len;
                    this->AssureTermination();
                }
                return *this;
            }

            size_t Format(const char *format, ...) {
                va_list args;
                ::va_start(args, format);
                s32 length = ::vsnprintf(m_string_array, Size, format, args);
                VP_ASSERT(0 <= length);
                ::va_end(args);

                m_length = length;
                this->AssureTermination();

                return m_length;
            }

            size_t AppendFormat(const char *format, ...) {
                va_list args;
                ::va_start(args, format);
                s32 appended_length = ::vsnprintf(m_string_array + m_length, Size - m_length, format, args);
                VP_ASSERT(0 <= appended_length);
                ::va_end(args);

                m_length += appended_length;
                this->AssureTermination();

                return m_length;
            }

            void TruncateStart(u32 offset_from_start) {

                /* Truncate whole string */
                if (m_length <= offset_from_start) {
                    m_length = 0;
                    this->AssureTermination();
                    return;
                }

                /* Move string */
                const u32 new_length = m_length - offset_from_start;
                ::memmove(m_string_array, m_string_array + offset_from_start, new_length);
                m_length = new_length;

                this->AssureTermination();
                return;
            }
            constexpr ALWAYS_INLINE void TruncateEnd(u32 offset_from_end) {

                /* Truncate whole string */
                if (m_length <= offset_from_end) {
                    m_length = 0;
                    this->AssureTermination();
                    return;
                }

                /* Adjust end size */
                const u32 offset_from_start = m_length - offset_from_end;
                m_length = offset_from_start;

                this->AssureTermination();
                return;
            }

            constexpr ALWAYS_INLINE size_t Append(const char *append) {
                if (std::is_constant_evaluated() == true) {
                    size_t append_length = TStringLength(append, Size - m_length - 1);
                    TStringCopy(m_string_array + m_length, append, append_length);
                    m_length += append_length;
                } else {
                    size_t append_length = ::strnlen(append, Size - m_length - 1);
                    ::strncpy(m_string_array + m_length, append, append_length);
                    m_length += append_length;
                }
                this->AssureTermination();

                return m_length;
            }

            constexpr u32 FindCharFromBack(u32 base_reverse_index, char target) {
                for (u32 i = m_length - base_reverse_index - 1; cInvalidIndex != i; --i) {
                    if (m_string_array[i] == target) { return i; }
                }
                return cInvalidIndex;
            }
            
            constexpr void Insert(u32 base_index, const char *string) {
                
                /* Ensure valid base index */
                const u32 prev_length = m_length;
                VP_ASSERT(base_index <= prev_length)

                /* Append string with overwrite */
                const u32 insert_length = StringLength(string);
                const u32 final_length  = base_index + insert_length;
                const u32 append_length = (Size < final_length) ? Size - base_index : insert_length;
                ::memcpy(m_string_array + base_index, string, append_length);
                m_length = final_length;
                this->AssureTermination();

                return;
            }

            constexpr void ReplaceAllChar(char target, char replace) {
                for (u32 i = 0; i < m_length; ++i) {
                    if (m_string_array[i] == target) { m_string_array[i] = replace; }
                }
                return;
            }

            ALWAYS_INLINE void Print() const {
                ::puts(m_string_array);
            }

            constexpr ALWAYS_INLINE void AssureTermination() {
                m_string_array[Size - 1] = '\0';
            }
            constexpr ALWAYS_INLINE bool IsNullString() {
                return m_string_array[0] == '\0';
            }

            constexpr ALWAYS_INLINE size_t      GetLength() const { return m_length; }
            constexpr ALWAYS_INLINE char       *GetString()       { return m_string_array; }
            constexpr ALWAYS_INLINE const char *GetString() const { return m_string_array; }
    };

    class HeapString {
        private:
            char *m_string;
        public:
            explicit ALWAYS_INLINE HeapString(imem::IHeap *heap, const char *string, s32 alignment = 8) {
                const u32 length = ::strlen(string);

                m_string = new (heap, alignment) char [length + 1];

                ::strncpy(m_string, string, length);

                m_string[length] = '\0';

                return;
            }

            ALWAYS_INLINE ~HeapString() {
                if (m_string != nullptr) {
                    delete [] m_string;
                    m_string = nullptr;
                }
            }

            constexpr ALWAYS_INLINE const char* GetString() const { return m_string; }
    };
}
