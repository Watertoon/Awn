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
        public:
            class Iterator {
                private:
                    FixedRingBuffer<T, Size> *m_ring;
                    u32                       m_index;
                public:
                    constexpr ALWAYS_INLINE Iterator(FixedRingBuffer<T, Size> *ring, u32 index) : m_ring(ring), m_index(index) {/*...*/}
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
			u32 m_offset;
			u32 m_count;
			T   m_array[Size];
        public:
            constexpr ALWAYS_INLINE FixedRingBuffer() : m_offset(0), m_count(0), m_array{} {/*...*/}
            constexpr ~FixedRingBuffer() {/*...*/}

            constexpr iterator begin() {
                return iterator(this, m_offset);
            }
            constexpr iterator end() {
                return iterator(this, Size);
            }

            constexpr T &operator[](u32 index) {
                const u32 base_offset = m_offset + index;
                const u32 wrap = (Size <= base_offset) ? Size : 0;
                return m_array[base_offset - wrap];
            }
            constexpr const T &operator[](u32 index) const {
                const u32 base_offset = m_offset + index;
                const u32 wrap = (Size <= base_offset) ? Size : 0;
                return m_array[base_offset - wrap];
            }
            
            constexpr ALWAYS_INLINE void PushBack(T value) {

                /* Check count */
                VP_ASSERT(Size > m_count);

                /* Increment count */
                const u32 count = m_count;
                m_count = count + 1;

                /* Calculate offset */
                const u32 base_offset = m_offset + count;
                const u32 wrap = (Size <= base_offset) ? Size : 0;

                /* Placeback value */
                m_array[base_offset - wrap] = value;
            }

            constexpr ALWAYS_INLINE void PushFront(T value) {

                /* Check count */
                VP_ASSERT(Size > m_count);

                /* Increment count */
                const u32 count = m_count;
                m_count = count + 1;

                /* Decrement offset */
                const u32 offset = (m_offset != 0) ? m_offset - 1 : Size - 1;
                m_offset = offset;

                /* Place front value */
                m_array[offset] = value;
            }

            constexpr ALWAYS_INLINE T PeekFront() const {

                /* Check count */
                const u32 count = m_count;
                if (count == 0) { return 0; }

                /* Get pointer */
                const u32 offset = m_offset;
                T ret = m_array[offset];

                return ret;
            }

            constexpr ALWAYS_INLINE T RemoveFront() {

                /* Check count */
                const u32 count = m_count;
                if (count == 0) { return 0; }

                /* Set size offset for wraparound */
                const u32 offset = m_offset;
                T ret = m_array[offset];

                /* Adjust offset */
                m_offset = (offset + 1 < Size) ? (offset + 1) : 0;
                m_count = count - 1;

                return ret;
            }

            constexpr ALWAYS_INLINE T PeekBack() const {

                /* Check count */
                const u32 count = m_count - 1;
                if (count == 0) { return 0; }

                /* Get pointer */
                const u32 offset = m_offset + count;
                const u32 wrap = (Size <= offset) ? Size : 0;
                T ret = m_array[offset - wrap];

                return ret;
            }
        
            constexpr ALWAYS_INLINE T RemoveBack() {

                /* Check count */
                const u32 count = m_count - 1;
                if (count == 0) { return 0; }

                /* Get pointer */
                const u32 offset = m_offset + count;
                const u32 wrap = (Size <= offset) ? Size : 0;
                T ret = m_array[offset - wrap];

                /* Adjust count */
                m_count = count - 1;

                return ret;
            }

            T Remove(T iter) {

                /* Find iter */
                for (u32 i = 0; i < m_count; ++i) {

                    const u32 base   = i + m_offset;
                    const u32 adjust = (Size < base) ? i - Size : i;

                    if (iter != m_array[adjust]) { continue; }

                    T *adjust_location = m_array + adjust;
                    
                    const bool is_wrap   = (Size < m_count + m_offset) & (m_offset < adjust);
                    const u32  adj_count = (is_wrap == true) ?  Size - adjust - 1 : m_count - adjust - 1;

                    ::memmove(adjust_location, adjust_location + 1, adj_count * sizeof(T*));

                    if (is_wrap == true) {
                        m_array[Size - 1] = m_array[0];

                        const u32 final_count = (m_offset + m_count) - Size - 1;

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

	template<typename T, u32 Size>
        requires (vp::util::CountOneBits32(Size) == 1)
	class AtomicFixedRingBuffer {
		private:
			u32 m_offset;
			u32 m_count;
			T   m_array[Size];
        public:
            constexpr ALWAYS_INLINE AtomicFixedRingBuffer() : m_offset(0), m_count(0), m_array{} {/*...*/}
            constexpr ~AtomicFixedRingBuffer() {/*...*/}

            constexpr ALWAYS_INLINE void Insert(T value) {

                /* Integrity checks */
                VP_ASSERT(value != 0);

                /* Increment count */
                const u32 count = vp::util::InterlockedIncrement(std::addressof(m_count));

                /* Calculate offset */
                const u32 offset = count - m_offset;
                VP_ASSERT(offset < Size);

                /* Placeback value */
                m_array[(Size - 1) & count] = value;
            }

            constexpr ALWAYS_INLINE T RemoveFront() {

                /* Check count */
                const u32 offset = m_offset;
                if ((m_count - offset) != 0) { return 0; }

                /* Try to acquire value */
                T ret = m_array[(Size - 1) & offset];
                if (ret == 0) { return 0; }

                /* Increment queue */
                m_array[(Size - 1) & offset] = 0;
                m_offset = offset + 1;

                return ret;
            }

            constexpr ALWAYS_INLINE u32 GetUsedCount() const { return m_count; } 
	};
}
