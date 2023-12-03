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

	template<typename T, size_t Size>
	class FixedRingBuffer {
		private:
			u32  m_current_offset;
			u32  m_count;
			T   *m_array[Size];
        public:
            constexpr ALWAYS_INLINE FixedRingBuffer() : m_current_offset(0), m_count(0), m_array{} {/*...*/}
            constexpr ~FixedRingBuffer() {/*...*/}

            constexpr ALWAYS_INLINE void Insert(T *pointer) {

                /* Check count */
                VP_ASSERT(Size > m_count);

                /* Increment count */
                u32 count = m_count;
                m_count = m_count + 1;

                /* Calculate offset */
                u32 offset = m_current_offset + count;

                /* Find size offset */
                u32 size_offset = 0;
                if (Size <= offset) {
                    size_offset = Size;
                }

                /* Placeback pointer */
                m_array[offset - size_offset] = pointer;
            }

            constexpr ALWAYS_INLINE T *RemoveFront() {

                /* Check count */
                VP_ASSERT(m_count != 0);

                /* Set size offset for wraparound */
                u32 size_offset = 0;
                if (Size <= m_current_offset) {
                    size_offset = Size;
                }

                /* Get pointer */
                T *ret = m_array[m_current_offset - size_offset];

                /* Adjust offset */
                if (0 < m_count) {
                    m_current_offset = (m_current_offset + 1 < Size) ? (m_current_offset + 1) : 0;
                    m_count          = m_count - 1;
                }

                return ret;
            }

            T *Remove(T *iter) {

                /* Find iter */
                for (u32 i = 0; i < m_count; ++i) {

                    const u32 base   = i + m_current_offset;
                    const u32 adjust = (Size < base) ? i - Size : i;

                    if (iter != m_array[adjust]) { continue; }

                    T **adjust_location = m_array + adjust;
                    
                    const bool is_wrap   = (Size < m_count + m_current_offset) & (m_current_offset < adjust);
                    const u32  adj_count = (is_wrap == true) ?  Size - adjust - 1 : m_count - adjust - 1;

                    ::memmove(adjust_location, adjust_location + 1, adj_count << 3);

                    if (is_wrap == true) {
                        m_array[Size - 1] = m_array[0];

                        const u32 final_count = (m_current_offset + m_count) - Size - 1;

                        ::memmove(m_array, m_array + 1, final_count << 3);
                    }

                    return iter;
                }

                VP_ASSERT(false);

                return iter;
            }

            constexpr ALWAYS_INLINE u32 GetUsedCount() const { return m_count; } 
	};
}
