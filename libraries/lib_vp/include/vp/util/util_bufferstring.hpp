#pragma once

namespace vp::util {

    template <size_t Size>
        requires (Size < 0xffff)
    class FixedString {
        public:
            char m_string_array[Size];
            u32  m_length;
        public:
            explicit constexpr ALWAYS_INLINE FixedString() : m_string_array{'\0'}, m_length(0) {/*...*/}
            explicit constexpr ALWAYS_INLINE FixedString(const char *string) : m_string_array{'\0'}, m_length(0) { 
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
                if (std::is_constant_evaluated()) {
                    m_length = TStringCopy(m_string_array, rhs.m_string_array, TStringLength(rhs.m_string_array, Size - 1));
                } else {
                    const size_t len = (rhs.m_length < Size - 1) ? rhs.m_length : Size - 1;
                    ::memcpy(m_string_array, rhs.m_string_array, len);
                    m_string_array[len + 1] = '\0';
                    m_length = len;
                }
                this->AssureTermination();
            }

            constexpr ALWAYS_INLINE bool operator==(const FixedString& rhs) {
                if (std::is_constant_evaluated()) {
                    return TStringCompare(m_string_array, rhs.m_string_array, TStringLength(rhs.m_string_array, Size - 1));
                }

                return ::strcmp(m_string_array, rhs.m_string_array) == 0;
            }
            constexpr ALWAYS_INLINE bool operator==(const FixedString& rhs) const {
                if (std::is_constant_evaluated()) {
                    return TStringCompare(m_string_array, rhs.m_string_array, TStringLength(rhs.m_string_array, Size - 1));
                }

                return ::strcmp(m_string_array, rhs.m_string_array) == 0;
            }
            constexpr ALWAYS_INLINE bool operator!=(const FixedString& rhs) {
                if (std::is_constant_evaluated()) {
                    return (TStringCompare(m_string_array, rhs.m_string_array, TStringLength(rhs.m_string_array, Size - 1)) != 0);
                }

                return ::strcmp(m_string_array, rhs.m_string_array) != 0;
            }
            constexpr ALWAYS_INLINE bool operator!=(const FixedString& rhs) const {
                if (std::is_constant_evaluated()) {
                    return (TStringCompare(m_string_array, rhs.m_string_array, TStringLength(rhs.m_string_array, Size - 1)) != 0);
                }

                return ::strcmp(m_string_array, rhs.m_string_array) != 0;
            }

            constexpr ALWAYS_INLINE FixedString &operator=(const FixedString& rhs) {
                if (std::is_constant_evaluated()) {
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
            constexpr ALWAYS_INLINE char*       GetString()       { return m_string_array; }
            constexpr ALWAYS_INLINE const char* GetString() const { return m_string_array; }
    };

    class HeapString {
        private:
            char *m_string;
        public:
            explicit HeapString(imem::IHeap *heap, const char *string, s32 alignment = 8) {
                const u32 length = ::strlen(string);

                /* This is the only time we should modify the string */
                m_string = new (heap, alignment) char [length + 1];

                ::strncpy(m_string, string, length); 
            }

            ~HeapString() {
                if (m_string != nullptr) {
                    delete [] m_string;
                    m_string = nullptr;
                }
            }

            constexpr ALWAYS_INLINE const char* GetString() const { return m_string; }
    };
}
