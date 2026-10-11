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

        template <typename T, typename C>
            requires (IsRawString<T>) && (IsRawString<T>) && ((sizeof(T) == sizeof(C)) || (IsCharEncodingConvertible<T,C>))
        constexpr u32 SetStringImpl(T *out_str, u32 max_length, const C *string, u32 len) {

            /* Copy up to size, convert encoding if necessary */
            u32 clamp_len = vp::util::Min(len, max_length-1);
            if constexpr (sizeof(T) == sizeof(C)) {
                for (u32 i = 0; i < clamp_len; ++i) {
                    out_str[i] = static_cast<T>(string[i]);
                }
            } else if constexpr (IsCharEncodingConvertible<T,C>) {
                ConvertStringEncoding(std::addressof(clamp_len), out_str, max_length-1, string, clamp_len);
            } else {
                _consteval_fail();
            }
            out_str[clamp_len] = static_cast<T>('\0');
            return clamp_len;
        }

        template <typename C>
            requires (IsChar<C>)
        class HeapStringImpl {
            public:
                using UnderlyingType = C;
            public:
                static constexpr C cNullChar = static_cast<C>(0u);
                static constexpr u32 cInvalidEntryIndex = 0xffff'ffff;
            public:
                C *m_string_array;
                u32 m_length;
                u32 m_memory_size;
            public:
                constexpr HeapStringImpl() : m_string_array(), m_length(), m_memory_size() {/*...*/}
                constexpr ~HeapStringImpl() = default;

                Result Initialize(vp::imem::IHeap *heap, u32 memory_size) {
                    /* Integrity checks */
                    RESULT_RETURN_UNLESS(0 < memory_size, ResultRangeNG);
                    /* Allocate */
                    C *string = new (heap, alignof(C)) C[memory_size];
                    RESULT_RETURN_UNLESS(string != nullptr, ResultNoMemory);
                    /* Setup */
                    m_string_array = string;
                    m_memory_size = memory_size;
                    this->Clear();
                    RESULT_RETURN_SUCCESS;
                }
                void Finalize() {
                    if (m_string_array != nullptr) {
                        delete [] m_string_array;
                        m_string_array = nullptr;
                        m_length = 0;
                        m_memory_size = 0;
                    }
                }

                static constexpr size_t GetRequiredMemorySize(u32 max_length) {
                    return sizeof(C) * max_length;
                }
                constexpr void SetMemoryAndClear(C *memory, u32 max_length) {
                    m_string_array = memory;
                    m_memory_size = max_length;
                    this->Clear();
                }
                constexpr void SetMemoryDirect(C *memory, u32 max_length, u32 length) {
                    m_string_array = memory;
                    m_memory_size = max_length;
                    m_length = length;
                }
            public:
                template <typename T>
                    requires (IsStringOrRaw<T>) && (sizeof(UnderlyingCharType<T>) == sizeof(C))
                constexpr bool operator==(const T &rhs) const {
                    const UnderlyingCharType<T> *str;
                    GetUnderlyingString(std::addressof(str), nullptr, std::addressof(rhs));
                    return StringCompare(m_string_array, str);
                }

                template <typename T>
                    requires (IsStringOrRaw<T>) && (sizeof(UnderlyingCharType<T>) == sizeof(C))
                constexpr s32 operator<=>(const T &rhs) const {
                    /* Order by valid contents */
                    const UnderlyingCharType<T> *str;
                    u32 len;
                    GetUnderlyingString(std::addressof(str), std::addressof(len), std::addressof(rhs));
                    const u32 length = m_length;
                    const u32 m_len = vp::util::Min(length, len);
                    const s32 cmp = std::memcmp(m_string_array, str, m_len);
                    if (cmp != 0) { return cmp; }
                    /* Order by length */
                    if (length < len) { return -1; }
                    if (length == len) { return 0; }
                    return 1;
                }

                template <typename T>
                    requires (IsStringOrRaw<T>) && ((sizeof(UnderlyingCharType<T>) == sizeof(C)) || (IsCharEncodingConvertible<C,UnderlyingCharType<T>>))
                constexpr HeapStringImpl &operator=(const T& rhs) {
                    this->SetString(std::addressof(rhs));
                    return *this;
                }
            public:
                template <typename T>
                    requires (IsStringOrRaw<T>) && ((sizeof(UnderlyingCharType<T>) == sizeof(C)) || (IsCharEncodingConvertible<C,UnderlyingCharType<T>>))
                constexpr void SetString(const T *string) {
                    const UnderlyingCharType<T> *str;
                    u32 len;
                    GetUnderlyingString(std::addressof(str), std::addressof(len), string);
                    const u32 new_len = impl::SetStringImpl(m_string_array, m_memory_size, str, len);
                    m_length = new_len;
                    return;
                }

                template <typename T>
                    requires (IsStringOrRaw<T>) && ((sizeof(UnderlyingCharType<T>) == sizeof(C)) || (IsCharEncodingConvertible<C,UnderlyingCharType<T>>))
                constexpr void SetString(const T *string, u32 length) {
                    const UnderlyingCharType<T> *str;
                    GetUnderlyingString(std::addressof(str), nullptr, string);
                    const u32 new_len = impl::SetStringImpl(m_string_array, m_memory_size, str, length);
                    m_length = new_len;
                    return;
                }

                template <typename T>
                    requires (IsRawString8<T>) && (sizeof(T) == sizeof(C))
                size_t Format(const T *format, ...) {
                    va_list args = {};
                    va_start(args, format);
                    s32 length = ::vsnprintf(reinterpret_cast<char*>(m_string_array), m_memory_size-1, reinterpret_cast<const char*>(format), args);
                    VP_ASSERT(0 <= length);
                    va_end(args);
                    this->SetLength(length);
                    return length;
                }

                template <typename T>
                    requires (IsChar<T>) && (sizeof(C) == sizeof(UnderlyingCharType<T>))
                constexpr void CopyTo(T *out_string, u32 max_length) const {
                    const u32 clamp_len = vp::util::Min(max_length-1, m_length);
                    std::memcpy(out_string, m_string_array, clamp_len);
                    out_string[clamp_len] = '\0';
                    return;
                }
                template <typename C1>
                    requires (IsStringOrRaw<C1>) && (sizeof(UnderlyingCharType<C1>) == sizeof(C))
                constexpr bool IsSame(const C1 *string) const {
                    return StringCompare(m_string_array, string);
                }
            public:
                template <typename T>
                    requires (IsChar<T>) && ((sizeof(T) == sizeof(C)) || (IsCharEncodingConvertible<C,T>))
                constexpr void AppendChar(const T append) {
                    const u32 prev_length = m_length;
                    const u32 append_length = impl::SetStringImpl(m_string_array + prev_length, m_memory_size - prev_length, std::addressof(append), 1);
                    m_length = prev_length + append_length;
                    return;
                }
                template <typename T>
                    requires (IsStringOrRaw<T>) && ((sizeof(UnderlyingCharType<T>) == sizeof(C)) || (IsCharEncodingConvertible<C,T>))
                constexpr void Append(const T *append) {
                    const u32 prev_length = m_length;
                    u32 len;
                    const UnderlyingCharType<T> *str;
                    GetUnderlyingString(std::addressof(str), std::addressof(len), append);
                    const u32 append_length = impl::SetStringImpl(m_string_array + prev_length, m_memory_size - prev_length, str, len);
                    m_length = prev_length + append_length;
                    return;
                }

                template <typename T>
                    requires (IsStringOrRaw<T>) && ((sizeof(UnderlyingCharType<T>) == sizeof(C)) || (IsCharEncodingConvertible<C,T>))
                constexpr void Append(const T *append, u32 len) {
                    const u32 prev_length = m_length;
                    const UnderlyingCharType<T> *str;
                    GetUnderlyingString(std::addressof(str), nullptr, append);
                    const u32 append_length = impl::SetStringImpl(m_string_array + prev_length, m_memory_size - prev_length, str, len);
                    m_length = prev_length + append_length;
                    return;
                }

                template <typename T>
                    requires (IsRawString<T>) && (sizeof(T) == sizeof(C))
                void AppendFormat(const T *format, ...) {
                    /* Format into start of free space */
                    va_list args = {};
                    va_start(args, format);
                    const s32 appended_length = ::vsnprintf(reinterpret_cast<char*>(m_string_array) + m_length, m_memory_size - m_length, reinterpret_cast<const char*>(format), args);
                    VP_ASSERT(0 <= appended_length);
                    va_end(args);
                    this->SetLength(m_length + appended_length);
                    return;
                }

                void TruncateStart(u32 length) {
                    /* Truncate whole string if length too long */
                    if (m_length <= length) {
                        m_length = 0;
                        this->AssureTermination();
                        return;
                    }
                    /* Move ahead new start of string */
                    const u32 new_length = m_length - length;
                    std::memmove(m_string_array, m_string_array + length, new_length * sizeof(C));
                    this->SetLength(new_length);
                    return;
                }

                constexpr void TruncateEnd(u32 length) {
                    /* Truncate whole string if length too long */
                    if (m_length <= length) {
                        m_length = 0;
                        this->AssureTermination();
                        return;
                    }
                    /* Adjust end size */
                    const u32 offset_from_start = m_length - length;
                    this->SetLength(offset_from_start);
                    return;
                }

                void RemoveAt(u32 base_index, u32 length) {
                    /* Ensure valid base offset */
                    VP_ASSERT(base_index < m_length);
                    /* Truncate to the base index if our removal length meets or exceeds our string length */
                    const u32 end_offset = base_index + length;
                    if (m_length <= end_offset || end_offset < base_index) {
                        m_length = base_index;
                        this->AssureTermination();
                        return;
                    }
                    /* Copy up end of string to base offset */
                    const u32 end_len = m_length - end_offset;
                    std::memmove(m_string_array + base_index, m_string_array + end_offset, end_len * sizeof(C));
                    this->SetLength(base_index + end_len);
                    return;
                }

                template <typename T>
                    requires (IsStringOrRaw<T>) && (sizeof(C) == sizeof(UnderlyingCharType<T>))
                constexpr void AppendAt(u32 base_index, const T *string) {
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
                    const u32 append_length = (m_memory_size < final_length) ? m_memory_size - base_index : insert_length;
                    std::memcpy(m_string_array + base_index, str, append_length);
                    this->SetLength(final_length);
                    return;
                }

                template <typename T>
                    requires (IsChar<T>)
                constexpr void ReplaceAllChar(T target, T replace) {
                    const u32 len = m_length;
                    for (u32 i = 0; i < len; ++i) {
                        if (m_string_array[i] == target) { m_string_array[i] = replace; }
                    }
                    return;
                }
            public:
                template <typename T>
                    requires (IsChar<T>) && (sizeof(C) == sizeof(T))
                constexpr u32 FirstIndexOfChar(u32 base_index, T target) {
                    for (u32 i = base_index; i < m_length; ++i) {
                        if (m_string_array[i] == static_cast<C>(target)) { return i; }
                    }
                    return cInvalidEntryIndex;
                }

                template <typename T>
                    requires (IsChar<T>) && (sizeof(C) == sizeof(T))
                constexpr u32 LastIndexOfChar(u32 base_reverse_index, T target) {
                    for (u32 i = m_length - base_reverse_index - 1; cInvalidEntryIndex != i; --i) {
                        if (m_string_array[i] == static_cast<C>(target)) { return i; }
                    }
                    return cInvalidEntryIndex;
                }
            public:
                void Print() const requires (IsChar8<C>) {
                    ::puts(m_string_array);
                }
            public:
                constexpr bool IsEmpty() const {
                    return m_string_array[0] == cNullChar;
                }
                constexpr bool IsFull() const {
                    return (m_memory_size-1) == m_length;
                }
                constexpr void AssureTermination() {
                    m_string_array[m_length] = cNullChar;
                }

                constexpr size_t GetMemorySize() const { return m_memory_size; }
                constexpr size_t GetLength() const { return m_length; }
                constexpr C *GetString() { return m_string_array; }
                constexpr const C *GetString() const { return m_string_array; }


                constexpr void Clear() {
                    this->SetLength(0);
                    return;
                }
                constexpr void SetLength(u32 length) {
                    VP_ASSERT(length < m_memory_size);
                    m_length = length;
                    this->AssureTermination();
                    return;
                }
                constexpr void RecomputeLength() {
                    const u32 recompute_length = vp::util::Min(StringLength(m_string_array), m_memory_size - 1);
                    this->SetLength(recompute_length);
                }

                constexpr bool VerifyUtf8() const requires (IsChar8<C>) {
                    return vp::util::GetCodePointCountOfUtf8String(m_string_array, m_length) != cInvalidCodePointCount;
                }
                constexpr u32 GetCodePointCount() const requires (IsChar8<C>) {
                    return vp::util::GetCodePointCountOfUtf8String(m_string_array, m_length);
                }
                constexpr void ClampCodePointCount(u32 max_code_point_count) const requires (IsChar8<C>) {
                    if (max_code_point_count > m_length) { return; }
                    u32 cp_i = 0;
                    u32 c;
                    const C *last_valid = m_string_array;
                    const C *iter = m_string_array;
                    while (vp::util::GetNextCharacterUtf8(std::addressof(c), std::addressof(iter))) {
                        if (c == 0) { break; }
                        ++cp_i;
                        if (max_code_point_count <= cp_i) {
                            this->SetLength(last_valid - m_string_array);
                            return;
                        }
                        last_valid = iter;
                    }
                    return;
                }
        };

        template <typename C, u32 Size>
            requires (IsChar<C>)
        class FixedStringImpl {
            public:
                using UnderlyingType = C;
            public:
                static constexpr C cNullChar = static_cast<C>(0u);
                static constexpr u32 cInvalidEntryIndex = 0xffff'ffff;
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
                    requires (IsStringOrRaw<T>) && ((sizeof(UnderlyingCharType<T>) == sizeof(C)) || (IsCharEncodingConvertible<C,UnderlyingCharType<T>>))
                constexpr FixedStringImpl(const T *string) : m_length() {
                    if (std::is_constant_evaluated()) {
                        for (u32 i = 0; i < Size; ++i) {
                            m_string_array[i] = 0;
                        }
                    }
                    this->SetString(string);
                }
                constexpr ~FixedStringImpl() {/*...*/}

                template <typename T>
                    requires (IsStringOrRaw<T>) && (sizeof(UnderlyingCharType<T>) == sizeof(C))
                constexpr bool operator==(const T &rhs) const {
                    const UnderlyingCharType<T> *str;
                    GetUnderlyingString(std::addressof(str), nullptr, std::addressof(rhs));
                    return StringCompare(m_string_array, str);
                }

                template <typename T>
                    requires (IsStringOrRaw<T>) && (sizeof(UnderlyingCharType<T>) == sizeof(C))
                constexpr s32 operator<=>(const T &rhs) const {
                    /* Order by valid contents */
                    const UnderlyingCharType<T> *str;
                    u32 len;
                    GetUnderlyingString(std::addressof(str), std::addressof(len), std::addressof(rhs));
                    const u32 length = m_length;
                    const u32 m_len = vp::util::Min(length, len);
                    const s32 cmp = std::memcmp(m_string_array, str, m_len);
                    if (cmp != 0) { return cmp; }
                    /* Order by length */
                    if (length < len) { return -1; }
                    if (length == len) { return 0; }
                    return 0;
                }

                template <typename T>
                    requires (IsStringOrRaw<T>) && ((sizeof(UnderlyingCharType<T>) == sizeof(C)) || (IsCharEncodingConvertible<C,UnderlyingCharType<T>>))
                constexpr FixedStringImpl &operator=(const T& rhs) {
                    this->SetString(std::addressof(rhs));
                    return *this;
                }
            public:
                template <typename T>
                    requires (IsStringOrRaw<T>) && ((sizeof(UnderlyingCharType<T>) == sizeof(C)) || (IsCharEncodingConvertible<C,UnderlyingCharType<T>>))
                constexpr void SetString(const T *string) {
                    const UnderlyingCharType<T> *str;
                    u32 len;
                    GetUnderlyingString(std::addressof(str), std::addressof(len), string);
                    const u32 new_len = impl::SetStringImpl(m_string_array, Size, str, len);
                    m_length = new_len;
                    return;
                }

                template <typename T>
                    requires (IsStringOrRaw<T>) && ((sizeof(UnderlyingCharType<T>) == sizeof(C)) || (IsCharEncodingConvertible<C,UnderlyingCharType<T>>))
                constexpr void SetString(const T *string, u32 length) {
                    const UnderlyingCharType<T> *str;
                    GetUnderlyingString(std::addressof(str), nullptr, string);
                    const u32 new_len = impl::SetStringImpl(m_string_array, Size, str, length);
                    m_length = new_len;
                    return;
                }

                template <typename T>
                    requires (IsRawString8<T>) && (sizeof(T) == sizeof(C))
                size_t Format(const T *format, ...) {
                    va_list args = {};
                    va_start(args, format);
                    s32 length = ::vsnprintf(reinterpret_cast<char*>(m_string_array), Size-1, reinterpret_cast<const char*>(format), args);
                    VP_ASSERT(0 <= length);
                    va_end(args);
                    this->SetLength(length);
                    return length;
                }

                template <typename T>
                    requires (IsChar<T>) && (sizeof(C) == sizeof(UnderlyingCharType<T>))
                constexpr void CopyTo(T *out_string, u32 max_length) const {
                    const u32 clamp_len = vp::util::Min(max_length-1, m_length);
                    std::memcpy(out_string, m_string_array, clamp_len);
                    out_string[clamp_len] = '\0';
                    return;
                }
                template <typename C1>
                    requires (IsStringOrRaw<C1>) && (sizeof(UnderlyingCharType<C1>) == sizeof(C))
                constexpr bool IsSame(const C1 *string) const {
                    return StringCompare(m_string_array, string);
                }
            public:
                template <typename T>
                    requires (IsChar<T>) && ((sizeof(T) == sizeof(C)) || (IsCharEncodingConvertible<C,T>))
                constexpr void AppendChar(const T append) {
                    const u32 prev_length = m_length;
                    const u32 append_length = impl::SetStringImpl(m_string_array + prev_length, Size - prev_length, std::addressof(append), 1);
                    m_length = prev_length + append_length;
                    return;
                }
                template <typename T>
                    requires (IsStringOrRaw<T>) && ((sizeof(UnderlyingCharType<T>) == sizeof(C)) || (IsCharEncodingConvertible<C,T>))
                constexpr void Append(const T *append) {
                    const u32 prev_length = m_length;
                    u32 len;
                    const UnderlyingCharType<T> *str;
                    GetUnderlyingString(std::addressof(str), std::addressof(len), append);
                    const u32 append_length = impl::SetStringImpl(m_string_array + prev_length, Size - prev_length, str, len);
                    m_length = prev_length + append_length;
                    return;
                }

                template <typename T>
                    requires (IsStringOrRaw<T>) && ((sizeof(UnderlyingCharType<T>) == sizeof(C)) || (IsCharEncodingConvertible<C,T>))
                constexpr void Append(const T *append, u32 len) {
                    const u32 prev_length = m_length;
                    const UnderlyingCharType<T> *str;
                    GetUnderlyingString(std::addressof(str), nullptr, append);
                    const u32 append_length = impl::SetStringImpl(m_string_array + prev_length, Size - prev_length, str, len);
                    m_length = prev_length + append_length;
                    return;
                }

                template <typename T>
                    requires (IsRawString<T>) && (sizeof(T) == sizeof(C))
                void AppendFormat(const T *format, ...) {
                    /* Format into start of free space */
                    va_list args = {};
                    va_start(args, format);
                    const s32 appended_length = ::vsnprintf(reinterpret_cast<char*>(m_string_array) + m_length, (Size - 1) - m_length, reinterpret_cast<const char*>(format), args);
                    VP_ASSERT(0 <= appended_length);
                    va_end(args);
                    this->SetLength(m_length + appended_length);
                    return;
                }

                void TruncateStart(u32 length) {
                    /* Truncate whole string if length too long */
                    if (m_length <= length) {
                        m_length = 0;
                        this->AssureTermination();
                        return;
                    }
                    /* Move ahead new start of string */
                    const u32 new_length = m_length - length;
                    std::memmove(m_string_array, m_string_array + length, new_length * sizeof(C));
                    this->SetLength(new_length);
                    return;
                }

                constexpr void TruncateEnd(u32 length) {
                    /* Truncate whole string if length too long */
                    if (m_length <= length) {
                        m_length = 0;
                        this->AssureTermination();
                        return;
                    }
                    /* Adjust end size */
                    const u32 offset_from_start = m_length - length;
                    this->SetLength(offset_from_start);
                    return;
                }

                void RemoveAt(u32 base_index, u32 length) {
                    /* Ensure valid base offset */
                    VP_ASSERT(base_index < m_length);
                    /* Truncate to the base index if our removal length meets or exceeds our string length */
                    const u32 end_offset = base_index + length;
                    if (m_length <= end_offset || end_offset < base_index) {
                        m_length = base_index;
                        this->AssureTermination();
                        return;
                    }
                    /* Copy up end of string to base offset */
                    const u32 end_len = m_length - end_offset;
                    std::memmove(m_string_array + base_index, m_string_array + end_offset, end_len * sizeof(C));
                    this->SetLength(base_index + end_len);
                    return;
                }

                template <typename T>
                    requires (IsStringOrRaw<T>) && (sizeof(C) == sizeof(UnderlyingCharType<T>))
                constexpr void AppendAt(u32 base_index, const T *string) {
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
                    std::memcpy(m_string_array + base_index, str, append_length);
                    this->SetLength(final_length);
                    return;
                }

                template <typename T>
                    requires (IsChar<T>)
                constexpr void ReplaceAllChar(T target, T replace) {
                    const u32 len = m_length;
                    for (u32 i = 0; i < len; ++i) {
                        if (m_string_array[i] == target) { m_string_array[i] = replace; }
                    }
                    return;
                }
            public:
                template <typename T>
                    requires (IsChar<T>) && (sizeof(C) == sizeof(T))
                constexpr u32 FirstIndexOfChar(u32 base_index, T target) {
                    for (u32 i = base_index; i < m_length; ++i) {
                        if (m_string_array[i] == static_cast<C>(target)) { return i; }
                    }
                    return cInvalidEntryIndex;
                }

                template <typename T>
                    requires (IsChar<T>) && (sizeof(C) == sizeof(T))
                constexpr u32 LastIndexOfChar(u32 base_reverse_index, T target) {
                    for (u32 i = m_length - base_reverse_index - 1; cInvalidEntryIndex != i; --i) {
                        if (m_string_array[i] == static_cast<C>(target)) { return i; }
                    }
                    return cInvalidEntryIndex;
                }
            public:
                void Print() const requires (IsChar8<C>) {
                    ::puts(m_string_array);
                }
            public:
                constexpr bool IsEmpty() const {
                    return m_string_array[0] == cNullChar;
                }
                constexpr bool IsFull() const {
                    return (Size-1) == m_length;
                }
                constexpr void AssureTermination() {
                    m_string_array[m_length] = cNullChar;
                }

                constexpr size_t GetMemorySize() const { return Size; }
                constexpr size_t GetLength() const { return m_length; }
                constexpr C *GetString() { return m_string_array; }
                constexpr const C *GetString() const { return m_string_array; }


                constexpr void Clear() {
                    this->SetLength(0);
                    return;
                }
                constexpr void SetLength(u32 length) {
                    VP_ASSERT(length < Size);
                    m_length = length;
                    this->AssureTermination();
                    return;
                }
                constexpr void RecomputeLength() {
                    const u32 recompute_length = vp::util::Min(StringLength(m_string_array), Size - 1);
                    this->SetLength(recompute_length);
                }

                constexpr bool VerifyUtf8() const requires (IsChar8<C>) {
                    return vp::util::GetCodePointCountOfUtf8String(m_string_array, m_length) != cInvalidCodePointCount;
                }
                constexpr u32 GetCodePointCount() const requires (IsChar8<C>) {
                    return vp::util::GetCodePointCountOfUtf8String(m_string_array, m_length);
                }
                constexpr void ClampCodePointCount(u32 max_code_point_count) const requires (IsChar8<C>) {
                    if (max_code_point_count > m_length) { return; }
                    u32 cp_i = 0;
                    u32 c;
                    const C *last_valid = m_string_array;
                    const C *iter = m_string_array;
                    while (vp::util::GetNextCharacterUtf8(std::addressof(c), std::addressof(iter))) {
                        if (c == 0) { break; }
                        ++cp_i;
                        if (max_code_point_count <= cp_i) {
                            this->SetLength(last_valid - m_string_array);
                            return;
                        }
                        last_valid = iter;
                    }
                    return;
                }
        };
    }

    class HeapString8 : public impl::HeapStringImpl<char> {
        public:
            static constexpr bool cIsHeapString8 = true;
        public:
            constexpr HeapString8() : impl::HeapStringImpl<char>() {/*...*/}
            template <typename T>
                requires (IsStringOrRaw<T>) && ((sizeof(UnderlyingCharType<T>) == sizeof(char)) || (IsCharEncodingConvertible<char,UnderlyingCharType<T>>))
            constexpr HeapString8(const T *string) : impl::HeapStringImpl<char>(string) {/*...*/}
            constexpr ~HeapString8() = default;

            template <typename T>
                requires (IsStringOrRaw<T>) && (sizeof(UnderlyingCharType<T>) == sizeof(char))
            constexpr bool operator==(const T &rhs) const {
                return this->impl::HeapStringImpl<char>::operator ==(rhs);
            }

            template <typename T>
                requires (IsStringOrRaw<T>) && (sizeof(UnderlyingCharType<T>) == sizeof(char))
            constexpr s32 operator<=>(const T &rhs) const {
                return this->impl::HeapStringImpl<char>::operator <=>(rhs);
            }
    };

    class HeapString16 : public impl::HeapStringImpl<char16_t> {
        public:
            static constexpr bool cIsHeapString16 = true;
        public:
            constexpr HeapString16() : impl::HeapStringImpl<char16_t>() {/*...*/}
            template <typename T>
                requires (IsStringOrRaw<T>) && ((sizeof(UnderlyingCharType<T>) == sizeof(char16_t)) || (IsCharEncodingConvertible<char16_t,UnderlyingCharType<T>>))
            constexpr HeapString16(const T *string) : impl::HeapStringImpl<char16_t>(string) {/*...*/}
            constexpr ~HeapString16() = default;

            template <typename T>
                requires (IsStringOrRaw<T>) && (sizeof(UnderlyingCharType<T>) == sizeof(char16_t))
            constexpr bool operator==(const T &rhs) const {
                return this->impl::HeapStringImpl<char16_t>::operator ==(rhs);
            }

            template <typename T>
                requires (IsStringOrRaw<T>) && (sizeof(UnderlyingCharType<T>) == sizeof(char16_t))
            constexpr s32 operator<=>(const T &rhs) const {
                return this->impl::HeapStringImpl<char16_t>::operator <=>(rhs);
            }
    };

    class HeapString32 : public impl::HeapStringImpl<char32_t> {
        public:
            static constexpr bool cIsHeapString32 = true;
        public:
            constexpr HeapString32() : impl::HeapStringImpl<char32_t>() {/*...*/}
            template <typename T>
                requires (IsStringOrRaw<T>) && ((sizeof(UnderlyingCharType<T>) == sizeof(char32_t)) || (IsCharEncodingConvertible<char32_t,UnderlyingCharType<T>>))
            constexpr HeapString32(const T *string) : impl::HeapStringImpl<char32_t>(string) {/*...*/}
            constexpr ~HeapString32() = default;

            template <typename T>
                requires (IsStringOrRaw<T>) && (sizeof(UnderlyingCharType<T>) == sizeof(char32_t))
            constexpr bool operator==(const T &rhs) const {
                return this->impl::HeapStringImpl<char32_t>::operator ==(rhs);
            }

            template <typename T>
                requires (IsStringOrRaw<T>) && (sizeof(UnderlyingCharType<T>) == sizeof(char32_t))
            constexpr s32 operator<=>(const T &rhs) const {
                return this->impl::HeapStringImpl<char32_t>::operator <=>(rhs);
            }
    };

    template <u32 Len>
        requires (Len < 0xffff'ffff) && (0 < Len)
    class FixedString8 : public impl::FixedStringImpl<char,Len> {
        public:
            static constexpr bool cIsFixedString8 = true;
        public:
            constexpr FixedString8() : impl::FixedStringImpl<char,Len>() {/*...*/}
            template <typename T>
                requires (IsStringOrRaw<T>) && ((sizeof(UnderlyingCharType<T>) == sizeof(char)) || (IsCharEncodingConvertible<char,UnderlyingCharType<T>>))
            constexpr FixedString8(const T *string) : impl::FixedStringImpl<char,Len>(string) {/*...*/}
            constexpr ~FixedString8() = default;

            template <typename T>
                requires (IsStringOrRaw<T>) && (sizeof(UnderlyingCharType<T>) == sizeof(char))
            constexpr bool operator==(const T &rhs) const {
                return this->impl::FixedStringImpl<char,Len>::operator ==(rhs);
            }

            template <typename T>
                requires (IsStringOrRaw<T>) && (sizeof(UnderlyingCharType<T>) == sizeof(char))
            constexpr s32 operator<=>(const T &rhs) const {
                return this->impl::FixedStringImpl<char,Len>::operator <=>(rhs);
            }
    };

    template <u32 Len>
        requires (Len < 0xffff'ffff) && (0 < Len)
    class FixedString16 : public impl::FixedStringImpl<char16_t,Len> {
        public:
            static constexpr bool cIsFixedString16 = true;
        public:
            constexpr FixedString16() : impl::FixedStringImpl<char16_t,Len>() {/*...*/}
            template <typename T>
                requires (IsStringOrRaw<T>) && ((sizeof(UnderlyingCharType<T>) == sizeof(char16_t)) || (IsCharEncodingConvertible<char16_t,UnderlyingCharType<T>>))
            constexpr FixedString16(const T *string) : impl::FixedStringImpl<char16_t,Len>(string) {/*...*/}
            constexpr ~FixedString16() = default;

            template <typename T>
                requires (IsStringOrRaw<T>) && (sizeof(UnderlyingCharType<T>) == sizeof(char16_t))
            constexpr bool operator==(const T &rhs) const {
                return this->impl::FixedStringImpl<char16_t,Len>::operator ==(rhs);
            }

            template <typename T>
                requires (IsStringOrRaw<T>) && (sizeof(UnderlyingCharType<T>) == sizeof(char16_t))
            constexpr s32 operator<=>(const T &rhs) const {
                return this->impl::FixedStringImpl<char16_t,Len>::operator <=>(rhs);
            }
    };

    template <u32 Len>
        requires (Len < 0xffff'ffff) && (0 < Len)
    class FixedString32 : public impl::FixedStringImpl<char32_t,Len> {
        public:
            static constexpr bool cIsFixedString32 = true;
        public:
            constexpr FixedString32() : impl::FixedStringImpl<char32_t,Len>() {/*...*/}
            template <typename T>
                requires (IsStringOrRaw<T>) && ((sizeof(UnderlyingCharType<T>) == sizeof(char32_t)) || (IsCharEncodingConvertible<char32_t,UnderlyingCharType<T>>))
            constexpr FixedString32(const T *string) : impl::FixedStringImpl<char32_t,Len>(string) {/*...*/}
            constexpr ~FixedString32() = default;

            template <typename T>
                requires (IsStringOrRaw<T>) && (sizeof(UnderlyingCharType<T>) == sizeof(char32_t))
            constexpr bool operator==(const T &rhs) const {
                return this->impl::FixedStringImpl<char32_t,Len>::operator ==(rhs);
            }

            template <typename T>
                requires (IsStringOrRaw<T>) && (sizeof(UnderlyingCharType<T>) == sizeof(char32_t))
            constexpr s32 operator<=>(const T &rhs) const {
                return this->impl::FixedStringImpl<char32_t,Len>::operator <=>(rhs);
            }
    };
}
