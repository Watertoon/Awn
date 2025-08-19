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

    namespace impl {

        template <typename C, u32 Size>
            requires (IsCharRaw<C>)
        class FixedStringImpl {
            public:
                using UnderlyingType = C;
            public:
                static constexpr C cNullChar = static_cast<C>(0u);
                static constexpr u32 cInvalidIndex = 0xffff'ffff;
            public:
                u32 m_length;
                C m_string_array[Size];
            public:
            constexpr FixedStringImpl() : m_length() {
                if (std::is_constant_evaluated()) {
                    for (u32 i = 0; i < Size; ++i) {
                        m_string_array[i] = 0;
                    }
                }
                this->AssureTermination();
            }
            template <typename T>
                requires (IsFixedStringOrRaw<T>) && ((sizeof(UnderlyingCharType<T>) == sizeof(C)) || (IsCharEncodingConvertible<C,UnderlyingCharType<T>>))
            constexpr FixedStringImpl(const T *string) : m_length() {
                if (std::is_constant_evaluated()) {
                    for (u32 i = 0; i < Size; ++i) {
                        m_string_array[i] = 0;
                    }
                }
                this->SetString(string);
            }
            constexpr ALWAYS_INLINE ~FixedStringImpl() {/*...*/}

            template <typename T>
                requires (IsFixedStringOrRaw<T>) && (sizeof(UnderlyingCharType<T>) == sizeof(C))
            constexpr bool operator==(T &rhs) const {
                const UnderlyingCharType<T> *str;
                GetUnderlyingString(std::addressof(str), nullptr, std::addressof(rhs));
                return StringCompare(m_string_array, str);
            }

            template <typename T>
                requires (IsFixedStringOrRaw<T>) && (sizeof(UnderlyingCharType<T>) == sizeof(C))
            constexpr ALWAYS_INLINE s32 operator<=>(const T &rhs) const {
                const UnderlyingCharType<T> *str;
                u32 len;
                GetUnderlyingString(std::addressof(str), std::addressof(len), std::addressof(rhs));
                if (m_length != len) { return false; }
                return ::memcmp(m_string_array, str, len * sizeof(C));
            }

            template <typename T>
                requires (IsFixedStringOrRaw<T>) && ((sizeof(UnderlyingCharType<T>) == sizeof(C)) || (IsCharEncodingConvertible<C,UnderlyingCharType<T>>))
            constexpr ALWAYS_INLINE FixedStringImpl &operator=(const T& rhs) {
                this->SetString(std::addressof(rhs));
                return *this;
            }

            template <typename T>
                requires (IsStringRaw<T>) && ((sizeof(T) == sizeof(C)) || (IsCharEncodingConvertible<C,T>))
            constexpr void SetStringImpl(const T *string, u32 len) {

                /* Copy up to size, convert encoding if necessary */
                u32 clamp_len = vp::util::Min(len, static_cast<u32>(Size-1));
                if constexpr (sizeof(UnderlyingCharType<T>) == sizeof(C)) {
                    for (u32 i = 0; i < clamp_len; ++i) {
                        m_string_array[i] = string[i];
                    }
                } else if constexpr (IsCharEncodingConvertible<C,T>) {
                    ConvertStringEncoding(std::addressof(clamp_len), m_string_array, Size-1, string, clamp_len);
                } else {
                    _consteval_fail();
                }
                m_length = clamp_len;
                this->AssureTermination();
                return;
            }

            template <typename T>
                requires (IsFixedStringOrRaw<T>) && ((sizeof(UnderlyingCharType<T>) == sizeof(C)) || (IsCharEncodingConvertible<C,UnderlyingCharType<T>>))
            constexpr void SetString(const T *string) {

                /* Get string */
                const UnderlyingCharType<T> *str;
                u32 len;
                GetUnderlyingString(std::addressof(str), std::addressof(len), string);

                /* Set string impl */
                this->SetStringImpl(str, len);
                return;
            }

            template <typename T>
                requires (IsFixedStringOrRaw<T>) && ((sizeof(UnderlyingCharType<T>) == sizeof(C)) || (IsCharEncodingConvertible<C,UnderlyingCharType<T>>))
            constexpr void SetString(const T *string, u32 length) {

                /* Get string */
                const UnderlyingCharType<T> *str;
                GetUnderlyingString(std::addressof(str), nullptr, string);

                /* Set string impl */
                this->SetStringImpl(str, length);
                return;
            }

            template <typename T>
                requires (IsStringRaw<T>) && (sizeof(T) == sizeof(C))
            size_t Format(const T *format, ...) {
                va_list args;
                ::va_start(args, format);
                s32 length = ::vsnprintf(reinterpret_cast<char*>(m_string_array), Size-1, reinterpret_cast<const char*>(format), args);
                VP_ASSERT(0 <= length);
                ::va_end(args);

                m_length = length;
                this->AssureTermination();

                return m_length;
            }

            template <typename T>
                requires (IsStringRaw<T>) && (sizeof(T) == sizeof(C))
            size_t AppendFormat(const T *format, ...) {
                va_list args;
                ::va_start(args, format);
                const s32 appended_length = ::vsnprintf(reinterpret_cast<char*>(m_string_array) + m_length, Size - m_length - 1, reinterpret_cast<const char*>(format), args);
                VP_ASSERT(0 <= appended_length);
                ::va_end(args);

                m_length += appended_length;
                this->AssureTermination();

                return m_length;
            }

            template <typename T>
                requires (IsStringRaw<T>) && ((sizeof(UnderlyingCharType<T>) == sizeof(C)) || (IsCharEncodingConvertible<C,T>))
            constexpr size_t AppendImpl(const T *string, u32 len) {

                /* Append */
                const u32 prev_length = m_length;
                size_t append_length = vp::util::Min(len, (Size-1) - prev_length);
                if constexpr (sizeof(UnderlyingCharType<T>) == sizeof(C)) {
                    for (u32 i = 0; i < append_length; ++i) {
                        m_string_array[i + prev_length] = string[i];
                    }
                } else if constexpr (IsCharEncodingConvertible<C,T>) {
                    ConvertStringEncoding(std::addressof(append_length), m_string_array + prev_length, (Size-1) - prev_length, string, append_length);
                } else {
                    _consteval_fail();
                }
                m_length += append_length;
                this->AssureTermination();

                return m_length;
            }

            template <typename T>
                requires (IsFixedStringOrRaw<T>) && ((sizeof(UnderlyingCharType<T>) == sizeof(C)) || (IsCharEncodingConvertible<C,T>))
            constexpr size_t Append(const T *append) {

                /* Get string */
                const UnderlyingCharType<T> *str;
                u32 len;
                GetUnderlyingString(std::addressof(str), std::addressof(len), append);

                return this->AppendImpl(str, len);
            }

            template <typename T>
                requires (IsFixedStringOrRaw<T>) && ((sizeof(UnderlyingCharType<T>) == sizeof(C)) || (IsCharEncodingConvertible<C,T>))
            constexpr size_t Append(const T *append, u32 len) {

                /* Get string */
                const UnderlyingCharType<T> *str;
                GetUnderlyingString(std::addressof(str), nullptr, append);

                return this->AppendImpl(str, len);
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
                ::memmove(m_string_array, m_string_array + offset_from_start, new_length * sizeof(C));
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

            constexpr void SetLength(u32 length) {
                m_length = length;
                this->AssureTermination();
                return;
            }

            constexpr void RecomputeLength() {
                m_length = vp::util::Min(StringLength(m_string_array), Size - 1);
                this->AssureTermination();
            }

            template <typename C1>
                requires (IsFixedStringOrRaw<C1>) && (sizeof(UnderlyingCharType<C1>) == sizeof(C))
            constexpr bool IsSame(const C1 *string) const {
                return StringCompare(m_string_array, string);
            }

            constexpr void Clear() {
                m_length = 0;
                m_string_array[0] = '\0';
                return;
            }

            template <typename T>
                requires (IsFixedStringOrRaw<T>) && (sizeof(C) == sizeof(UnderlyingCharType<T>))
            constexpr void Insert(u32 base_index, const T *string) {
                
                /* Ensure valid base index */
                const u32 prev_length = m_length;
                VP_ASSERT(base_index <= prev_length)

                /* Get string */
                const UnderlyingCharType<T> *str;
                u32 len;
                GetUnderlyingString(std::addressof(str), std::addressof(len), string);

                /* Append string with overwrite */
                const u32 insert_length = len;
                const u32 final_length  = base_index + insert_length;
                const u32 append_length = (Size < final_length) ? Size - base_index : insert_length;
                ::memcpy(m_string_array + base_index, str, append_length);
                m_length = final_length;
                this->AssureTermination();

                return;
            }

            template <typename T>
                requires (IsFixedStringOrRaw<T>) && (sizeof(C) == sizeof(UnderlyingCharType<T>))
            constexpr void CopyStringTo(T *out_string, u32 max_length) const {
                const u32 clamp_len = vp::util::Min(max_length-1, m_length);
                ::memcpy(out_string, m_string_array, clamp_len);
                out_string[clamp_len] = '\0';
                return;
            }

            template <typename T>
                requires (IsCharRaw<T>) && (sizeof(C) == sizeof(T))
            constexpr u32 FindCharFromBack(u32 base_reverse_index, T target) {
                for (u32 i = m_length - base_reverse_index - 1; cInvalidIndex != i; --i) {
                    if (m_string_array[i] == static_cast<C>(target)) { return i; }
                }
                return cInvalidIndex;
            }

            template <typename T>
                requires (IsChar8Raw<T>)
            constexpr void ReplaceAllChar(T target, T replace) {
                const u32 len = m_length;
                for (u32 i = 0; i < len; ++i) {
                    if (m_string_array[i] == target) { m_string_array[i] = replace; }
                }
                return;
            }

            void Print() const requires (IsChar8Raw<C>) {
                ::puts(m_string_array);
            }

            constexpr void AssureTermination() {
                m_string_array[m_length] = cNullChar;
            }
            constexpr bool IsEmpty() const {
                return m_string_array[0] == cNullChar;
            }

            constexpr size_t GetLength() const { return m_length; }
            constexpr C *GetString() { return m_string_array; }
            constexpr const C *GetString() const { return m_string_array; }
        };
    }

    template <u32 Len>
        requires (Len < 0xffff'ffff) && (0 < Len)
    class FixedString8 : public impl::FixedStringImpl<char,Len> {
        public:
            static constexpr bool cIsFixedString8 = true;
        public:
            constexpr FixedString8() : impl::FixedStringImpl<char,Len>() {/*...*/}
            template <typename T>
                requires (IsFixedStringOrRaw<T>) && ((sizeof(UnderlyingCharType<T>) == sizeof(char)) || (IsCharEncodingConvertible<char,UnderlyingCharType<T>>))
            constexpr FixedString8(const T *string) : impl::FixedStringImpl<char,Len>(string) {/*...*/}
            constexpr ALWAYS_INLINE ~FixedString8() {/*...*/}
    };

    template <u32 Len>
        requires (Len < 0xffff'ffff) && (0 < Len)
    class FixedString16 : public impl::FixedStringImpl<char16_t,Len> {
        public:
            static constexpr bool cIsFixedString16 = true;
        public:
            constexpr FixedString16() : impl::FixedStringImpl<char16_t,Len>() {/*...*/}
            template <typename T>
                requires (IsFixedStringOrRaw<T>) && ((sizeof(UnderlyingCharType<T>) == sizeof(char)) || (IsCharEncodingConvertible<char,UnderlyingCharType<T>>))
            constexpr FixedString16(const T *string) : impl::FixedStringImpl<char16_t,Len>(string) {/*...*/}
            constexpr ALWAYS_INLINE ~FixedString16() {/*...*/}
    };

    template <u32 Len>
        requires (Len < 0xffff'ffff) && (0 < Len)
    class FixedString32 : public impl::FixedStringImpl<char32_t,Len> {
        public:
            static constexpr bool cIsFixedString32 = true;
        public:
            constexpr FixedString32() : impl::FixedStringImpl<char32_t,Len>() {/*...*/}
            template <typename T>
                requires (IsFixedStringOrRaw<T>) && ((sizeof(UnderlyingCharType<T>) == sizeof(char)) || (IsCharEncodingConvertible<char,UnderlyingCharType<T>>))
            constexpr FixedString32(const T *string) : impl::FixedStringImpl<char32_t,Len>(string) {/*...*/}
            constexpr ALWAYS_INLINE ~FixedString32() {/*...*/}
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
