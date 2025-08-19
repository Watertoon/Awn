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
