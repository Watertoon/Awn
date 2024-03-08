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
	class RingBuffer {
        public:
            class Iterator {
                private:
                    const RingBuffer<T> *m_ring;
                    u32                  m_index;
                public:
                    constexpr ALWAYS_INLINE Iterator(const RingBuffer<T> *ring, u32 index) : m_ring(ring), m_index(index) {/*...*/}
                    constexpr ALWAYS_INLINE ~Iterator() {/*...*/}

                    constexpr ALWAYS_INLINE T &operator*() {
                        return (*m_ring)[m_index];
                    }
                    constexpr ALWAYS_INLINE const T &operator*() const {
                        return (*m_ring)[m_index];
                    }

                    constexpr ALWAYS_INLINE bool operator==(const Iterator &rhs) const {
                        return m_index == rhs.m_index;
                    }

                    constexpr ALWAYS_INLINE bool operator!=(const Iterator &rhs) const {
                        return m_index != rhs.m_index;
                    }

                    constexpr ALWAYS_INLINE Iterator &operator++() {
                        ++m_index;
                        return *this;
                    }

                    constexpr ALWAYS_INLINE Iterator &operator++([[maybe_unused]]int) {
                        ++m_index;
                        return *this;
                    }
            };
        public:
            using iterator = Iterator;
		private:
			u32  m_offset;
			u32  m_count;
            u32  m_max_count;
			T   *m_array;
        public:
            constexpr ALWAYS_INLINE RingBuffer() : m_offset(0), m_count(0), m_max_count(0), m_array{} {/*...*/}
            constexpr ALWAYS_INLINE ~RingBuffer() {/*...*/}

            constexpr iterator begin() {
                return iterator(this, m_offset);
            }
            constexpr iterator end() {
                return iterator(this, m_max_count);
            }

            void Initialize(imem::IHeap *heap, u32 queue_count, s32 alignment = 8) {

                /* Integrity checks */
                if (queue_count == 0) { return; }

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
                }
                m_offset    = 0;
                m_count     = 0;
                m_max_count = 0;
            }

            constexpr T &operator[](u32 index) {
                const u32 base_offset = m_offset + index;
                const u32 adj_index   = (m_max_count < base_offset) ? base_offset - m_max_count : base_offset;
                return m_array[adj_index];
            }
            constexpr T &operator[](u32 index) const {
                const u32 base_offset = m_offset + index;
                const u32 adj_index   = (m_max_count < base_offset) ? base_offset - m_max_count : base_offset;
                return m_array[adj_index];
            }

            constexpr ALWAYS_INLINE void Insert(T value) {

                /* Check count */
                VP_ASSERT(m_max_count > m_count);

                /* Increment count */
                u32 count = m_count;
                m_count = m_count + 1;

                /* Calculate offset */
                u32 offset = m_offset + count;

                /* Find size offset */
                u32 size_offset = 0;
                if (m_max_count <= offset) {
                    size_offset = m_max_count;
                }

                /* Placeback value */
                m_array[offset - size_offset] = value;
            }

            constexpr ALWAYS_INLINE T RemoveFront() {

                /* Check count */
                if (m_count == 0) { return 0; }

                /* Set size offset for wraparound */
                u32 size_offset = 0;
                if (m_max_count <= m_offset) {
                    size_offset = m_max_count;
                }

                /* Get pointer */
                T ret = m_array[m_offset - size_offset];

                /* Adjust offset */
                if (0 < m_count) {
                    m_offset = (m_offset + 1 < m_max_count) ? (m_offset + 1) : 0;
                    m_count  = m_count - 1;
                }

                return ret;
            }

            T Remove(T iter) {

                /* Find iter */
                for (u32 i = 0; i < m_count; ++i) {

                    /* Calculate index */
                    const u32 base   = i + m_offset;
                    const u32 adjust = (m_max_count < base) ? i - m_max_count : i;

                    /* Check if iter at index */
                    if (iter != m_array[adjust]) { continue; }

                    /* Move ring to remove iter */
                    T *adjust_location = m_array + adjust;

                    const bool is_wrap   = (m_max_count < m_count + m_offset) & (m_offset < adjust);
                    const u32  adj_count = (is_wrap == true) ?  m_max_count - adjust - 1 : m_count - adjust - 1;

                    ::memmove(adjust_location, adjust_location + 1, adj_count * sizeof(T*));

                    if (is_wrap == true) {
                        m_array[m_max_count - 1] = m_array[0];

                        const u32 final_count = (m_offset + m_count) - m_max_count - 1;

                        ::memmove(m_array, m_array + 1, final_count * sizeof(T*));
                    }

                    /* Adjust count */
                    m_count  = m_count - 1;

                    return iter;
                }

                VP_ASSERT(false);

                return iter;
            }

            constexpr void Clear() {
                m_offset = 0;
                m_count  = 0;
            }

            constexpr ALWAYS_INLINE u32 GetUsedCount() const { return m_count; }
	};

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
                m_max_count = 0;
                m_count     = 0;
                m_offset    = 0;
                if (m_array != nullptr) {
                    delete [] m_array;
                }
            }

            constexpr void Insert(T value) {

                /* Integrity checks */
                VP_ASSERT(value != 0);

                /* Ensure available space */
                u32 count = m_count;
                u32 last  = 0;
                while ((count - m_offset) < (m_max_count - 3)) {

                    /* Attempt to reserve space */
                    const bool result = vp::util::InterlockedCompareExchangeRelaxed(std::addressof(last), std::addressof(m_count), count + 1, count);
                    if (result == false) { count = last; continue; }

                    /* Insert our element */
                    vp::util::MemoryBarrierRelease();
                    m_array[(m_max_count - 1) & count] = value;

                    return;
                }
                VP_ASSERT(false);
            }

            constexpr T RemoveFront() {

                /* Attempt to read the next item in the queue */
                u32 offset = m_offset;
                while (0 < m_count - offset) {

                    /* Check if next element is available */
                    T ret = m_array[(m_max_count - 1) & offset];
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
                    if (result == false) { continue; }

                    /* Take the element */
                    vp::util::MemoryBarrierRelease();
                    m_array[(m_max_count - 1) & offset] = 0;

                    return ret;
                }

                return 0;
            }
	};
}
