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
            
            constexpr void PushBack(T value) {

                /* Check count */
                VP_ASSERT(!this->IsFull());

                /* Increment count */
                const u32 count = m_count;
                m_count = count + 1;

                /* Calculate offset */
                const u32 base_offset = m_offset + count;
                const u32 wrap = (Size <= base_offset) ? Size : 0;

                /* Placeback value */
                m_array[base_offset - wrap] = value;
            }

            constexpr void PushFront(T value) {

                /* Check count */
                VP_ASSERT(!this->IsFull());

                /* Increment count */
                const u32 count = m_count;
                m_count = count + 1;

                /* Decrement offset */
                const u32 offset = (m_offset != 0) ? m_offset - 1 : Size - 1;
                m_offset = offset;

                /* Place front value */
                m_array[offset] = value;
            }

            constexpr T PeekFront() const {

                /* Check count */
                const u32 count = m_count;
                if (count == 0) { return 0; }

                /* Get pointer */
                const u32 offset = m_offset;
                T ret = m_array[offset];

                return ret;
            }

            constexpr bool DropFront() {
                const u32 count = m_count;
                if (count == 0) { return false; }
                const u32 offset = m_offset;
                m_offset = (offset + 1 < Size) ? (offset + 1) : 0;
                m_count = count - 1;
                return true;
            }

            constexpr T RemoveFront() {

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

            constexpr T PeekBack() const {

                /* Check count */
                const u32 count = m_count;
                if (count == 0) { return 0; }

                /* Get pointer */
                const u32 offset = m_offset + count - 1;
                const u32 wrap = (Size <= offset) ? Size : 0;
                T ret = m_array[offset - wrap];

                return ret;
            }

            constexpr bool DropBack() {
                const u32 count = m_count;
                if (count == 0) { return false; }
                m_count = count - 1;
                return true;
            }

            constexpr T RemoveBack() {

                /* Check count */
                const u32 count = m_count;
                if (count == 0) { return 0; }

                /* Get pointer */
                const u32 offset = m_offset + count - 1;
                const u32 wrap = (Size <= offset) ? Size : 0;
                T ret = m_array[offset - wrap];

                /* Adjust count */
                m_count = count;

                return ret;
            }

            bool RemoveOrdered(T value) {

                /* Find and delete value */
                const u32 offset = m_offset;
                const u32 count = m_count;
                constexpr const u32 max = Size;
                const u32 end_offset = offset + count;
                for (u32 i = offset; i < end_offset; ++i) {

                    /* Match value to delete location */
                    const u32 sub0 = (max <= i) ? max : 0;
                    const u32 base_i = i - sub0;
                    T *del = m_array + base_i;
                    if (value != *del) { continue; }

                    /* Shift base offset range up to end of memory down by 1 */
                    const bool is_wrap = (max < end_offset) & (offset < base_i);
                    const u32 end_count = (is_wrap) ? (max - base_i - 1) : (count - base_i - 1);
                    ::memmove(del, del + 1, end_count * sizeof(T*));

                    /* On wrap shift start of memory to end, and shift start range up to end of values down by 1 */
                    if (is_wrap) {
                        m_array[max - 1] = m_array[0];
                        const u32 final_count = end_offset - max - 1;
                        ::memmove(m_array, m_array + 1, final_count * sizeof(T*));
                    }
                    m_count  = count - 1;

                    return true;
                }

                return false;
            }

            bool RemoveUnordered(T value) {

                /* Find and delete value */
                const u32 offset = m_offset;
                const u32 count = m_count;
                constexpr const u32 max = Size;
                const u32 arr_count = offset + count;
                for (u32 i = offset; i < arr_count; ++i) {

                    /* Match value to delete location */
                    const u32 sub0 = (max <= i) ? max : 0;
                    const u32 base_i = i - sub0;
                    T *del = m_array + base_i;
                    if (value != *del) { continue; }

                    /* Swap last element in to deleted's place */
                    const u32 end_i = count + offset - 1;
                    const u32 sub1 = (max <= end_i) ? max : 0;
                    const u32 swap_i = end_i - sub1;
                    *del = m_array[swap_i];
                    m_count = count - 1;

                    return true;
                }

                return false;
            }

            constexpr void Clear() {
                m_offset = 0;
                m_count  = 0;
            }

            constexpr ALWAYS_INLINE u32 GetUsedCount() const { return m_count; }

            constexpr ALWAYS_INLINE bool IsEmpty() const { return m_count == 0; }
            constexpr ALWAYS_INLINE bool IsFull() const { return (Size <= m_count); }
	};
}
