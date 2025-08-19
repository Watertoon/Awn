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
            class Iterator {
                private:
                    AtomicRingBuffer<T> *m_ring;
                    u32 m_index;
                public:
                    constexpr ALWAYS_INLINE Iterator(AtomicRingBuffer<T> *ring, u32 index) : m_ring(ring), m_index(index) {/*...*/}
                    constexpr ALWAYS_INLINE ~Iterator() {/*...*/}

                    constexpr bool GetNext(T *out_value) {
                        const u32 max_mask = (m_ring->m_max_count - 1);
                        u32 i = m_index;
                        u32 count = m_ring->m_count;
                        ON_SCOPE_EXIT { m_index = i + 1; };
                        while (i < count) {
                            const T ret = m_ring->m_array[i & max_mask];
                            if (ret != 0) { *out_value = ret; return true; }
                            ++i;
                            count = m_ring->m_count;
                        }
                        return false;
                    }
            };
        public:
            constexpr ALWAYS_INLINE AtomicRingBuffer() : m_offset(), m_count(), m_max_count(), m_array{} {/*...*/}
            constexpr ALWAYS_INLINE ~AtomicRingBuffer() {/*...*/}

            void Initialize(imem::IHeap *heap, u32 queue_count, s32 alignment = 8) {

                /* Integrity checks */
                VP_ASSERT(queue_count != 0);

                /* Align up count to nearest power of 2 */
                const u32 shift = vp::util::CountLeftZeroBits32(queue_count - 1);
                const u32 count = static_cast<u32>(0x8000'0000) >> shift;

                /* Allocate pointer array */
                m_array = new (heap, alignment) T[count];
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
                    const T ret = m_array[offset & max_mask];
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

            constexpr Iterator Begin() {
                return Iterator(this, m_offset);
            }

            constexpr void Clear() {
                if (0 < static_cast<s32>(m_offset) - static_cast<s32>(m_count)) {
                    m_offset = 0;
                    m_count = 0;
                    for (u32 i = 0; i < m_max_count; ++i) {
                        m_array[i] = nullptr;
                    }
                }
                return;
            }

            constexpr u32 GetUsedCount() const { return m_count; }
            constexpr u32 GetOffset() const { return m_offset; }
            constexpr u32 GetMaxCount() const { return m_max_count; }
	};
}
