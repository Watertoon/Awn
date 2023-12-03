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

    class StringPoolStringBase {
        public:
            vp::util::IntrusiveListNode m_pool_builder_node;
        public:
            constexpr StringPoolStringBase() : m_pool_builder_node() {/*...*/}
            constexpr virtual ~StringPoolStringBase() {/*...*/}

            constexpr virtual const char *GetString() const {
                return "";
            }

            constexpr virtual u32 GetStringLength() const {
                return 0;
            }
    };

    class StringPoolString : public StringPoolStringBase {
        public:
            const char *m_string;
        public:
            constexpr StringPoolString()                   : StringPoolStringBase(), m_string("") {/*...*/}
            constexpr StringPoolString(const char *string) : StringPoolStringBase(), m_string(string) {/*...*/}
            constexpr virtual ~StringPoolString() {/*...*/}

            constexpr virtual const char *GetString() const {
                return m_string;
            }

            constexpr virtual u32 GetStringLength() const {
                if (std::is_constant_evaluated() == true) { return vp::util::TStringLength(m_string, 0xffff'ffff); }
                return (m_string != nullptr) ? ::strlen(m_string) : 0;
            }
            
            constexpr void SetString(const char *string) { m_string = string; }
    };

    template <size_t Size>
    class FixedStringPoolString : public StringPoolStringBase {
        public:
            vp::util::FixedString<Size> m_string;
        public:
            constexpr FixedStringPoolString() : StringPoolStringBase(), m_string() {/*...*/}
            constexpr virtual ~FixedStringPoolString() {/*...*/}

            constexpr virtual const char *GetString() const {
                return m_string.GetString();
            }

            constexpr virtual u16 GetLength() const {
                return m_string.GetLength();
            }
    };
}
