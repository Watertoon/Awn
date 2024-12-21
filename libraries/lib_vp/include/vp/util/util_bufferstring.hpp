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

        /* Constexpr impl */
        if (std::is_constant_evaluated()) {
            u32 count = 0;
            while (string[0] != '\0') { string += 1; ++count; }
            return count;
        }

        /* Handle unaligned string */
        const u32 unaligned = 0x20 - (reinterpret_cast<uintptr_t>(string) & 0x1f);
        for (u32 i = 0; i < unaligned; ++i) {
            if (string[i] == '\0') { return i; }
        }
        string = string + unaligned;

        /* Handle aligned string */
        u32 i = unaligned;
        u32 mask = 0;
        const v32cc zero = {};
        for (;;) {

            /* Load 32 characters, compare if equal to zero, and convert result to a mask */
            const v32cc characters = Load32Aligned(string);
            const v32cc result     = CompareEqual32(zero, characters);
            mask                   = MoveMask256(result);

            /* Check if the mask has a bit set, meaning a character is non-zero */
            if (mask != 0) {
                const u32 offset = vp::util::CountRightZeroBits32(mask);
                return i + offset;
            }

            /* Advance while mask is not empty */
            i      = i + 0x20;
            string = string + 0x20;
        }
    }

    constexpr bool StringCompare(const char *lhs, const char *rhs) {
        
        /* Constexpr impl */
        if (std::is_constant_evaluated()) {
            while (*lhs != '\0' || *rhs != '\0') { 
                if (*lhs != *rhs) { return false; }
                lhs += 1;
                rhs += 1;
            }
            return true;
        }

        /* Handle aligned string */
        u32 i    = 0;
        const u32 iter = 0x20;
        const char *l_iter = lhs;
        const char *r_iter = rhs;
        const v32cc zero = {};
        for (;;) {

            /* Load 32 characters, compare if equal to zero, and convert result to a mask */
            const v32cc lhs_char = Load32(l_iter);
            const v32cc rhs_char = Load32(r_iter);
            const v32cc result0 = CompareEqual32(lhs_char, zero);
            const v32cc result1 = CompareEqual32(rhs_char, zero);
            const v32cc result2 = CompareEqual32(lhs_char, rhs_char);
            const u32 mask0 = MoveMask256(result0);
            const u32 mask1 = MoveMask256(result1);
            const u32 mask2 = MoveMask256(result2);

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
    constexpr u32 StringLength(const char *string) {

        /* Handle unaligned string */
        u32 i = 0;
        while (string[i] != '\0') { ++i; }

        return i;
    }

    constexpr bool StringCompare(const char *lhs, const char *rhs) {
        while (*lhs != '\0' || *rhs != '\0') { 
            if (*lhs != *rhs) { return false; }
            lhs += 1;
            rhs += 1;
        }
        return true;
    }
#endif

    template <size_t Size>
        requires (Size < vp::util::c8MB) && (0 < Size)
    class FixedString {
        public:
            static constexpr u32 cInvalidIndex = 0xffff'ffff;
        public:
            u32  m_length;
            char m_string_array[Size];
        public:
            explicit constexpr ALWAYS_INLINE FixedString() : m_length() {
                this->AssureTermination();
            }
            explicit constexpr ALWAYS_INLINE FixedString(const char *string) : m_length() { 
                if (string == nullptr) { return; }
                const u32 len = StringLength(string);
                const size_t clamp_len = vp::util::Min(len, static_cast<u32>(Size) - 1);
                ::memcpy(m_string_array, string, clamp_len);
                m_length = clamp_len; 
                this->AssureTermination();
            }
            template <size_t RhsSize>
            explicit constexpr FixedString(const FixedString<RhsSize> &rhs) : m_length() {
                const size_t clamp_len = vp::util::Min(rhs.m_length, static_cast<u32>(Size) - 1);
                ::memcpy(m_string_array, rhs.m_string_array, clamp_len);
                m_length = clamp_len;
                this->AssureTermination();
            }

            constexpr ALWAYS_INLINE ~FixedString() {/*...*/}

            template <size_t RhsSize>
            constexpr ALWAYS_INLINE bool operator==(const FixedString<RhsSize>& rhs) {
                if (m_length != rhs.m_length) { return false; }
                return ::memcmp(m_string_array, rhs.m_string_array, m_length) == 0;
            }

            template <size_t RhsSize>
            constexpr ALWAYS_INLINE bool operator==(const FixedString<RhsSize>& rhs) const {
                if (m_length != rhs.m_length) { return false; }
                return ::memcmp(m_string_array, rhs.m_string_array, m_length) == 0;
            }

            constexpr ALWAYS_INLINE bool operator==(const char *rhs) {
                const u32 length =  StringLength(rhs);
                if (m_length != length) { return false; }
                return ::memcmp(m_string_array, rhs, length) == 0;
            }

            constexpr ALWAYS_INLINE bool operator==(const char *rhs) const {
                const u32 length =  StringLength(rhs);
                if (m_length != length) { return false; }
                return ::memcmp(m_string_array, rhs, length) == 0;
            }

            template <size_t RhsSize>
            constexpr ALWAYS_INLINE bool operator!=(const FixedString<RhsSize>& rhs) {
                return (m_length != rhs.m_length) || ::memcmp(m_string_array, rhs.m_string_array, m_length) == 0;
            }

            template <size_t RhsSize>
            constexpr ALWAYS_INLINE bool operator!=(const FixedString<RhsSize>& rhs) const {
                return (m_length != rhs.m_length) || ::memcmp(m_string_array, rhs.m_string_array, m_length) == 0;
            }

            template <size_t RhsSize>
            constexpr ALWAYS_INLINE bool operator>(const FixedString<RhsSize>& rhs) {
                if (m_length != rhs.m_length) { return false; }
                return ::memcmp(m_string_array, rhs.m_string_array, m_length) < 0;
            }
    
            template <size_t RhsSize>
            constexpr ALWAYS_INLINE bool operator>(const FixedString<RhsSize>& rhs) const {
                if (m_length != rhs.m_length) { return false; }
                return ::memcmp(m_string_array, rhs.m_string_array, m_length) < 0;
            }

            template <size_t RhsSize>
            constexpr ALWAYS_INLINE bool operator<(const FixedString<RhsSize>& rhs) {
                if (m_length != rhs.m_length) { return false; }
                return ::memcmp(m_string_array, rhs.m_string_array, m_length) > 0;
            }

            template <size_t RhsSize>
            constexpr ALWAYS_INLINE bool operator<(const FixedString<RhsSize>& rhs) const {
                if (m_length != rhs.m_length) { return false; }
                return ::memcmp(m_string_array, rhs.m_string_array, m_length) > 0;
            }

            constexpr void RecomputeLength() {
                m_length = vp::util::Min(StringLength(m_string_array), Size - 1);
                this->AssureTermination();
            }

            constexpr ALWAYS_INLINE bool IsSame(const char *string) const {
                return StringCompare(m_string_array, string);
            }

            constexpr ALWAYS_INLINE void Clear() {
                m_length = 0;
                m_string_array[0] = '\0';
                return;
            }

            template <size_t RhsSize>
            constexpr ALWAYS_INLINE FixedString &operator=(const FixedString<RhsSize>& rhs) {
                const size_t len = vp::util::Min(rhs.m_length, static_cast<u32>(Size) - 1);
                ::memcpy(m_string_array, rhs.m_string_array, len);
                m_length = len;
                this->AssureTermination();

                return *this;
            }

            constexpr ALWAYS_INLINE FixedString &operator=(const char *rhs) {
                if (rhs == nullptr) { 
                    m_string_array[0] = '\0';
                    m_length          = 0;
                    return *this; 
                }

                const size_t len = vp::util::Min(StringLength(rhs), static_cast<u32>(Size) - 1);
                ::memcpy(m_string_array, rhs, len);
                m_length = len;
                this->AssureTermination();
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
                size_t append_length = vp::util::Min(StringLength(append), static_cast<u32>(Size) - m_length - 1);
                ::memcpy(m_string_array + m_length, append, append_length);
                m_length += append_length;
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
                m_string_array[m_length] = '\0';
            }
            constexpr ALWAYS_INLINE bool IsNullString() {
                return m_string_array[0] == '\0';
            }

            constexpr ALWAYS_INLINE size_t GetLength() const { return m_length; }
            constexpr ALWAYS_INLINE char *GetString() { return m_string_array; }
            constexpr ALWAYS_INLINE const char *GetString() const { return m_string_array; }
    };

    class HeapString {
        private:
            char *m_string;
        public:
            explicit ALWAYS_INLINE HeapString(imem::IHeap *heap, const char *string, s32 alignment = 8) {
                const u32 length = StringLength(string);
                m_string = new (heap, alignment) char [length + 1];
                ::memcpy(m_string, string, length);
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
