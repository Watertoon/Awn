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

    template <typename T, size_t Count>
        requires (sizeof(T) <= sizeof(u32)) && (Count <= (~T(0)))
    class FixedIndexAllocator {
        public:
            static constexpr T cInvalidHandle     = static_cast<T>(0xffff'ffff'ffff'ffff);
            static constexpr T cInvalidEntryIndex = static_cast<T>(0xffff'ffff'ffff'ffff);
        private:
            u32  m_next_index;
            T    m_handle_array[Count];
        public:
            constexpr  FixedIndexAllocator() : m_next_index(), m_handle_array() { this->Clear(); }
            constexpr ~FixedIndexAllocator() {/*...*/}

            constexpr void Clear() {

                /* Clear allocator */
                m_next_index = 0;
                for (u32 i = 0; i < Count - 1; ++i) {
                    m_handle_array[i] = i + 1;
                }
                m_handle_array[Count - 1] = static_cast<T>(cInvalidHandle);

                return;
            }

            constexpr ALWAYS_INLINE T Allocate() {

                /* Try to acquire an index */
                const u32 handle_value = m_next_index;
                m_next_index     = m_handle_array[handle_value];

                return handle_value;
            }

            
            constexpr ALWAYS_INLINE void Free(T index) {

                /* Release an index */
                m_handle_array[index] = m_next_index;
                m_next_index          = index;

                return;
            }

            constexpr ALWAYS_INLINE u32 GetMaxCount() const { return Count; }
    };
}
