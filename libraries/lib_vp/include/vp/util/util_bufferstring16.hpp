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

    constexpr u32 StringLength16(const char16_t *string) {

        /* Handle unaligned string */
        u32 i = 0;
        while (string[i] != u'\0') { ++i; }

        return i;
    }

    constexpr bool StringCompare16(const char16_t *lhs, const char16_t *rhs) {
        while (*lhs != u'\0' || *rhs != u'\0') { 
            if (*lhs != *rhs) { return false; }
            lhs += 1;
            rhs += 1;
        }
        return true;
    }

    template <size_t Size>
        requires (Size < vp::util::c8MB) && (0 < Size)
    class FixedString16 {
        public:
            static constexpr u32 cInvalidIndex = 0xffff'ffff;
        public:
            u32  m_length;
            char16_t m_string_array[Size];
        public:
            explicit constexpr ALWAYS_INLINE FixedString16() : m_length() {
                this->AssureTermination();
            }
            explicit constexpr ALWAYS_INLINE FixedString16(const char16_t *string) : m_length() { 
                if (string == nullptr) { return; }
                const u32 len = StringLength16(string);
                const size_t clamp_len = vp::util::Min(len, static_cast<u32>(Size) - 1);
                ::memcpy(m_string_array, string, clamp_len);
                m_length = clamp_len; 
                this->AssureTermination();
            }
            template <size_t RhsSize>
            explicit constexpr FixedString16(const FixedString16<RhsSize> &rhs) : m_length() {
                const size_t clamp_len = vp::util::Min(rhs.m_length, static_cast<u32>(Size) - 1);
                ::memcpy(m_string_array, rhs.m_string_array, clamp_len);
                m_length = clamp_len;
                this->AssureTermination();
            }

            constexpr ALWAYS_INLINE ~FixedString16() {/*...*/}

            template <size_t RhsSize>
            constexpr ALWAYS_INLINE bool operator==(const FixedString16<RhsSize>& rhs) {
                if (m_length != rhs.m_length) { return false; }
                return ::memcmp(m_string_array, rhs.m_string_array, m_length) == 0;
            }

            template <size_t RhsSize>
            constexpr ALWAYS_INLINE bool operator==(const FixedString16<RhsSize>& rhs) const {
                if (m_length != rhs.m_length) { return false; }
                return ::memcmp(m_string_array, rhs.m_string_array, m_length) == 0;
            }

            constexpr ALWAYS_INLINE bool operator==(const char16_t *rhs) {
                const u32 length =  StringLength16(rhs);
                if (m_length != length) { return false; }
                return ::memcmp(m_string_array, rhs, length) == 0;
            }

            constexpr ALWAYS_INLINE bool operator==(const char16_t *rhs) const {
                const u32 length =  StringLength16(rhs);
                if (m_length != length) { return false; }
                return ::memcmp(m_string_array, rhs, length) == 0;
            }

            template <size_t RhsSize>
            constexpr ALWAYS_INLINE bool operator!=(const FixedString16<RhsSize>& rhs) {
                return (m_length != rhs.m_length) || ::memcmp(m_string_array, rhs.m_string_array, m_length) == 0;
            }

            template <size_t RhsSize>
            constexpr ALWAYS_INLINE bool operator!=(const FixedString16<RhsSize>& rhs) const {
                return (m_length != rhs.m_length) || ::memcmp(m_string_array, rhs.m_string_array, m_length) == 0;
            }

            template <size_t RhsSize>
            constexpr ALWAYS_INLINE bool operator>(const FixedString16<RhsSize>& rhs) {
                if (m_length != rhs.m_length) { return false; }
                return ::memcmp(m_string_array, rhs.m_string_array, m_length) < 0;
            }
    
            template <size_t RhsSize>
            constexpr ALWAYS_INLINE bool operator>(const FixedString16<RhsSize>& rhs) const {
                if (m_length != rhs.m_length) { return false; }
                return ::memcmp(m_string_array, rhs.m_string_array, m_length) < 0;
            }

            template <size_t RhsSize>
            constexpr ALWAYS_INLINE bool operator<(const FixedString16<RhsSize>& rhs) {
                if (m_length != rhs.m_length) { return false; }
                return ::memcmp(m_string_array, rhs.m_string_array, m_length) > 0;
            }

            template <size_t RhsSize>
            constexpr ALWAYS_INLINE bool operator<(const FixedString16<RhsSize>& rhs) const {
                if (m_length != rhs.m_length) { return false; }
                return ::memcmp(m_string_array, rhs.m_string_array, m_length) > 0;
            }

            constexpr void RecomputeLength() {
                m_length = vp::util::Min(StringLength16(m_string_array), static_cast<u32>(Size) - 1);
                this->AssureTermination();
            }

            constexpr ALWAYS_INLINE bool IsSame(const char16_t *string) const {
                return StringCompare16(m_string_array, string);
            }

            constexpr ALWAYS_INLINE void Clear() {
                m_length = 0;
                this->AssureTermination();
                return;
            }

            template <size_t RhsSize>
            constexpr ALWAYS_INLINE FixedString16 &operator=(const FixedString16<RhsSize>& rhs) {
                const size_t len = vp::util::Min(rhs.m_length, static_cast<u32>(Size) - 1);
                ::memcpy(m_string_array, rhs.m_string_array, len);
                m_length = len;
                this->AssureTermination();

                return *this;
            }

            constexpr ALWAYS_INLINE FixedString16 &operator=(const char16_t *rhs) {
                if (rhs == nullptr) { 
                    m_string_array[0] = u'\0';
                    m_length = 0;
                    return *this; 
                }

                const size_t len = vp::util::Min(StringLength16(rhs), static_cast<u32>(Size) - 1);
                ::memcpy(m_string_array, rhs, len);
                m_length = len;
                this->AssureTermination();
                return *this;
            }

            constexpr ALWAYS_INLINE FixedString16 &operator=(const char *rhs) {
                if (rhs == nullptr) { 
                    m_string_array[0] = u'\0';
                    m_length = 0;
                    return *this; 
                }

                const size_t len = StringLength(rhs);
                u32 out_len = 0;
                const u32 result = ConvertStringUtf8ToUtf16(std::addressof(out_len), m_string_array, static_cast<u32>(Size) - 1, rhs, len);
                VP_ASSERT(result != 2);
                m_length = out_len;
                this->AssureTermination();
                return *this;
            }

            //size_t Format(const char16_t *format, ...) {
            //    va_list args;
            //    ::va_start(args, format);
            //    s32 length = ::vsnprintf(m_string_array, Size, format, args);
            //    VP_ASSERT(0 <= length);
            //    ::va_end(args);
            //
            //    m_length = length;
            //    this->AssureTermination();
            //
            //    return m_length;
            //}
            //
            //size_t AppendFormat(const char16_t *format, ...) {
            //    va_list args;
            //    ::va_start(args, format);
            //    s32 appended_length = ::vsnprintf(m_string_array + m_length, Size - m_length, format, args);
            //    VP_ASSERT(0 <= appended_length);
            //    ::va_end(args);
            //
            //    m_length += appended_length;
            //    this->AssureTermination();
            //
            //    return m_length;
            //}

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

            constexpr ALWAYS_INLINE size_t Append(const char16_t *append) {
                size_t append_length = vp::util::Min(StringLength16(append), static_cast<u32>(Size) - m_length - 1);
                ::memcpy(m_string_array + m_length, append, append_length);
                m_length += append_length;
                this->AssureTermination();

                return m_length;
            }

            constexpr u32 FindCharFromBack(u32 base_reverse_index, char16_t target) {
                for (u32 i = m_length - base_reverse_index - 1; cInvalidIndex != i; --i) {
                    if (m_string_array[i] == target) { return i; }
                }
                return cInvalidIndex;
            }
            
            constexpr void Insert(u32 base_index, const char16_t *string) {
                
                /* Ensure valid base index */
                const u32 prev_length = m_length;
                VP_ASSERT(base_index <= prev_length)

                /* Append string with overwrite */
                const u32 insert_length = StringLength16(string);
                const u32 final_length  = base_index + insert_length;
                const u32 append_length = (Size < final_length) ? Size - base_index : insert_length;
                ::memcpy(m_string_array + base_index, string, append_length);
                m_length = final_length;
                this->AssureTermination();

                return;
            }

            constexpr void ReplaceAllChar(char16_t target, char16_t replace) {
                for (u32 i = 0; i < m_length; ++i) {
                    if (m_string_array[i] == target) { m_string_array[i] = replace; }
                }
                return;
            }

            ALWAYS_INLINE void Print() const {
#ifdef VP_TARGET_PLATFORM_win32
                ::_putws(reinterpret_cast<wchar_t*>(m_string_array));
#endif
            }

            constexpr void SetLength(u32 length) {
                m_length = length;
                this->AssureTermination();
                return;
            }

            constexpr ALWAYS_INLINE void AssureTermination() {
                m_string_array[m_length] = u'\0';
            }
            constexpr ALWAYS_INLINE bool IsEmpty() const {
                return m_string_array[0] == u'\0';
            }

            constexpr ALWAYS_INLINE size_t GetLength() const { return m_length; }
            constexpr ALWAYS_INLINE char16_t *GetString() { return m_string_array; }
            constexpr ALWAYS_INLINE const char16_t *GetString() const { return m_string_array; }
    };

    class HeapString16 {
        private:
            char16_t *m_string;
        public:
            explicit ALWAYS_INLINE HeapString16(imem::IHeap *heap, const char16_t *string, s32 alignment = 8) {
                const u32 length = vp::util::StringLength16(string);
                m_string = new (heap, alignment) char16_t [length + 1];
                ::memcpy(m_string, string, length);
                m_string[length] = u'\0';
                return;
            }

            ALWAYS_INLINE ~HeapString16() {
                if (m_string != nullptr) {
                    delete [] m_string;
                    m_string = nullptr;
                }
            }

            constexpr ALWAYS_INLINE const char16_t* GetString() const { return m_string; }
    };
}
