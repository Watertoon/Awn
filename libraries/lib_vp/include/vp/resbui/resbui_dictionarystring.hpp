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

namespace vp::resbui {

    class NintendoWareDictionaryBuilder;

    class DictionaryStringBase : public StringPoolStringBase {
        public:
            friend class NintendoWareDictionaryBuilder;
        private:
            vp::util::IntrusiveListNode m_dictionary_builder_node;
        public:
            constexpr DictionaryStringBase() : StringPoolStringBase(), m_dictionary_builder_node() {/*...*/}
            constexpr virtual ~DictionaryStringBase() {/*...*/}
    };

    class DictionaryString : public DictionaryStringBase {
        public:
            const char *m_string;
        public:
            constexpr DictionaryString() : DictionaryStringBase(), m_string("") {/*...*/}
            constexpr DictionaryString(const char *string) : DictionaryStringBase(), m_string(string) {/*...*/}
            constexpr virtual ~DictionaryString() {/*...*/}

            constexpr virtual const char *GetString() const override {
                return m_string;
            }

            constexpr virtual u32 GetLength() const override {
                return ::strlen(m_string);
            }
    
            constexpr void SetString(const char *string) {
                m_string = string;
            }
    };

    template <size_t Size>
    class FixedDictionaryString : public DictionaryStringBase {
        public:
            vp::util::FixedString<Size> m_string;
        public:
            constexpr FixedDictionaryString() : DictionaryStringBase(), m_string() {/*...*/}
            constexpr FixedDictionaryString(const char *string) : DictionaryStringBase(), m_string(string) {/*...*/}
            constexpr virtual ~FixedDictionaryString() {/*...*/}

            constexpr virtual const char *GetString() const override {
                return m_string.GetString();
            }

            constexpr virtual u32 GetLength() const override {
                return m_string.GetLength();
            }
    
            constexpr void SetString(const char *string) {
                m_string = string;
            }
    };
}
