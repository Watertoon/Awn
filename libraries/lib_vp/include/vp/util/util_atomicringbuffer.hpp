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

	template<typename T>
	class AtomicRingBuffer {
		private:
			u32  m_offset;
			u32  m_count;
            u32  m_max_count;
			T   *m_array;
        public:
            constexpr ALWAYS_INLINE AtomicRingBuffer() : m_offset(0), m_count(0), m_max_count(0), m_array{} {/*...*/}
            constexpr ALWAYS_INLINE ~AtomicRingBuffer() {/*...*/}

            void Initialize(imem::IHeap *heap, u32 queue_count, s32 alignment = 8) {

                /* Integrity checks */
                VP_ASSERT(queue_count != 0);
                VP_ASSERT(vp::util::CountOneBits32(queue_count) == 1);

                /* Allocate pointer array */
                m_array = new (heap, alignment) T[queue_count];
                VP_ASSERT(m_array != nullptr);

                /* Set state */
                m_max_count = queue_count;
                m_offset    = 0;
                m_count     = 0;

                return;
            }

            void Finalize() {
                if (m_array != nullptr) {
                    delete [] m_array;
                    m_array = nullptr;
                }
                m_max_count = 0;
                m_count     = 0;
                m_offset    = 0;
            }

            bool PushBack(T value) {

                /* Integrity checks */
                VP_ASSERT(value != 0);

                /* Ensure available space */
                u32 count = m_count;
                u32 last  = 0;
                while ((count - m_offset) < (m_max_count - 3)) {

                    /* Attempt to reserve space */
                    const bool result = vp::util::InterlockedCompareExchangeRelaxed(std::addressof(last), std::addressof(m_count), count + 1, count);
                    if (!result) {
                        count = last;
                        continue;
                    }

                    /* Insert our element */
                    vp::util::MemoryBarrierRelease();
                    m_array[count & (m_max_count - 1)] = value;

                    return true;
                }

                return false;
            }

            T RemoveFront() {

                /* Attempt to read the next item in the queue */
                const u32 max_mask = (m_max_count - 1);
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
	};
}
