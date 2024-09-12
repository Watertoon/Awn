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

	template<typename T, u32 Size>
        requires (vp::util::CountOneBits32(Size) == 1)
	class AtomicFixedRingBuffer {
        public:
            static constexpr uintptr_t cRemoveValue = 0xffff'ffff'ffff'ffff;
		private:
			u32 m_offset;
			u32 m_count;
			T   m_array[Size];
        public:
            constexpr ALWAYS_INLINE AtomicFixedRingBuffer() : m_offset(0), m_count(0), m_array{} {/*...*/}
            constexpr ~AtomicFixedRingBuffer() {/*...*/}

            void PushBack(T value) {

                /* Integrity checks */
                VP_ASSERT(value != 0);

                /* Increment count */
                const u32 count = vp::util::InterlockedIncrement(std::addressof(m_count));

                /* Calculate offset */
                const u32 offset = count - m_offset;
                VP_ASSERT(offset < Size);

                /* Placeback value */
                m_array[count & (Size - 1)] = value;
            }

            void PushFront(T value) {

                /* Integrity checks */
                VP_ASSERT(value != 0);

                /* Increment count */
                const u32 count = vp::util::InterlockedIncrement(std::addressof(m_count));

                /* Calculate offset */
                const u32 offset = count - m_offset;
                VP_ASSERT(offset < Size);

                /* Placeback value */
                m_array[count & (Size - 1)] = value;
            }

            T RemoveFront() {

                /* Attempt to read the next item in the queue */
                const u32 max_mask = (Size - 1);
                u32 offset = m_offset;
                while (0 < m_count - offset) {

                    /* Check if next element is available */
                    T ret = m_array[offset & max_mask];
                    if (ret == 0) {
                        vp::util::MemoryBarrierRelease();

                        /* Retry if a new element was added */
                        const u32 next_offset = m_offset;
                        if (next_offset == offset) { return 0; }
                        offset = next_offset;
                        continue;
                    }

                    /* Reserve the element as ours */
                    const bool result = vp::util::InterlockedCompareExchangeRelaxed(std::addressof(offset), std::addressof(m_offset), offset + 1, offset);
                    if (!result) { continue; }

                    /* Take the element */
                    vp::util::MemoryBarrierRelease();
                    m_array[offset & max_mask] = 0;

                    return ret;
                }

                return 0;
            }

            constexpr ALWAYS_INLINE u32 GetUsedCount() const { return m_count; } 
	};
}
