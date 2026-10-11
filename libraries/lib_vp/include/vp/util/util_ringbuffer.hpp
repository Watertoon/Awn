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
                    RingBuffer<T> *m_ring;
                    u32            m_index;
                public:
                    constexpr ALWAYS_INLINE Iterator(RingBuffer<T> *ring, u32 index) : m_ring(ring), m_index(index) {/*...*/}
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
            constexpr ALWAYS_INLINE RingBuffer() : m_offset(), m_count(), m_max_count(), m_array{} {/*...*/}
            constexpr ALWAYS_INLINE ~RingBuffer() {/*...*/}

            Result Initialize(imem::IHeap *heap, u32 count, s32 alignment = 8) {

                /* Integrity checks */
                RESULT_RETURN_IF(count == 0, ResultRangeNG);

                /* Allocate pointer array */
                T *array = new (heap, alignment) T[count];
                RESULT_RETURN_UNLESS(array != nullptr, ResultNoMemory);

                /* Set state */
                m_offset = 0;
                m_count = 0;
                m_max_count = count;
                m_array = array;

                RESULT_RETURN_SUCCESS;
            }

            void Finalize() {
                if (m_array != nullptr) {
                    delete [] m_array;
                    m_array = nullptr;
                }
                m_offset    = 0;
                m_count     = 0;
                m_max_count = 0;
            }

            static constexpr size_t GetRequiredMemorySize(u32 count) {
                return sizeof(T) * count;
            }
            constexpr void SetMemory(T *array, u32 count) {
                m_array = array;
                m_offset = 0;
                m_count = 0;
                m_max_count = count;
            }
        public:
            constexpr iterator begin() {
                return iterator(this, 0);
            }
            constexpr iterator end() {
                return iterator(this, m_count);
            }

            constexpr T &operator[](u32 index) {
                const u32 base_offset = m_offset + index;
                const u32 wrap = (m_max_count <= base_offset) ? m_max_count : 0;
                return m_array[base_offset - wrap];
            }
            constexpr T &operator[](u32 index) const {
                const u32 base_offset = m_offset + index;
                const u32 wrap = (m_max_count <= base_offset) ? m_max_count : 0;
                return m_array[base_offset - wrap];
            }

            constexpr void PushBack(T value) {

                /* Check count */
                VP_ASSERT(!this->IsFull());

                /* Calculate offset */
                const u32 count = m_count;
                const u32 base_offset = m_offset + count;
                const u32 wrap = (m_max_count <= base_offset) ? m_max_count : 0;

                /* Placeback value */
                m_array[base_offset - wrap] = value;
                m_count = count + 1;
                return;
            }

            constexpr void PushFront(T value) {

                /* Check count */
                VP_ASSERT(!this->IsFull());

                /* Decrement offset */
                const u32 wrap = (m_offset < 1) ? m_max_count : 0;
                const u32 offset = (m_offset + wrap) - 1;

                /* Place front value */
                m_array[offset] = value;
                m_count = m_count + 1;
                m_offset = offset;
                return;
            }

            constexpr bool DropFront() {
                /* Ensure count */
                const u32 count = m_count;
                if (count == 0) { return false; }
                /* Adjust offset */
                const u32 n_offset = m_offset + 1;
                m_offset = (n_offset < m_max_count) ? n_offset : 0;
                m_count = count - 1;
                return true;
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

            constexpr T RemoveFront() {

                /* Ensure count */
                const u32 count = m_count;
                if (count == 0) { return 0; }

                /* Read message */
                const u32 offset = m_offset;
                const T ret = m_array[offset];

                /* Adjust offset */
                const u32 n_offset = offset + 1;
                m_offset = (n_offset < m_max_count) ? n_offset : 0;
                m_count = count - 1;

                return ret;
            }

            constexpr bool DropBack() {
                const u32 count = m_count;
                if (count == 0) { return false; }
                m_count = count - 1;
                return true;
            }

            constexpr T PeekBack() const {

                /* Check count */
                const u32 count = m_count;
                if (count == 0) { return 0; }

                /* Get pointer */
                const u32 max = m_max_count;
                const u32 offset = m_offset + count - 1;
                const u32 wrap = (max <= offset) ? max : 0;
                T ret = m_array[offset - wrap];

                return ret;
            }

            constexpr T RemoveBack() {

                /* Check count */
                const u32 count = m_count;
                if (count == 0) { return 0; }

                /* Get pointer */
                const u32 max = m_max_count;
                const u32 offset = m_offset + count - 1;
                const u32 wrap = (max <= offset) ? max : 0;
                T ret = m_array[offset - wrap];

                /* Adjust count */
                m_count = count - 1;

                return ret;
            }

            bool RemoveOrdered(T value) {

                /* Find and delete value */
                const u32 offset = m_offset;
                const u32 count = m_count;
                const u32 max_count = m_max_count;
                const u32 end_offset = offset + count;
                for (u32 i = offset; i < end_offset; ++i) {
                    const u32 wrap = (max_count <= i) ? max_count : 0;
                    const u32 wrap_i = i - wrap;
                    T *del = m_array + wrap_i;
                    if (value != *del) { continue; }

                    /* Shift values down by one, over our delete location, up to end of memory */
                    const bool is_wrap = (max_count < end_offset) && (offset <= wrap_i);
                    const u32 count_to_end = (is_wrap) ? (max_count - wrap_i - 1) : (end_offset - wrap_i - 1);
                    std::memmove(del, del + 1, count_to_end * sizeof(T));

                    /* On wrap, shift first value to end, shift values down by one, over the first value, to end of used */
                    if (is_wrap) {
                        m_array[max_count - 1] = m_array[0];
                        const u32 count_from_start = end_offset - max_count - 1;
                        std::memmove(m_array, m_array + 1, count_from_start * sizeof(T));
                    }
                    m_count = count - 1;

                    return true;
                }

                return false;
            }

            bool RemoveUnordered(T value) {

                /* Find and delete value */
                const u32 offset = m_offset;
                const u32 count = m_count;
                const u32 max = m_max_count;
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

            constexpr bool IsValid() const { return m_array != 0; }
            constexpr bool IsEmpty() const { return m_count == 0; }
            constexpr bool IsFull() const { return (m_max_count <= m_count); }

            constexpr u32 GetUsedCount() const { return m_count; }
	};

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
            constexpr FixedRingBuffer() : m_offset(), m_count(), m_array{} {/*...*/}
            constexpr ~FixedRingBuffer() {/*...*/}

            constexpr iterator begin() {
                return iterator(this, 0);
            }
            constexpr iterator end() {
                return iterator(this, m_count);
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

                /* Calculate offset */
                const u32 count = m_count;
                const u32 base_offset = m_offset + count;
                const u32 wrap = (Size <= base_offset) ? Size : 0;

                /* Placeback value */
                m_array[base_offset - wrap] = value;
                m_count = count + 1;
                return;
            }

            constexpr void PushFront(T value) {

                /* Check count */
                VP_ASSERT(!this->IsFull());

                /* Decrement offset */
                const u32 wrap = (m_offset < 1) ? Size : 0;
                const u32 offset = (m_offset + wrap) - 1;

                /* Place front value */
                m_array[offset] = value;
                m_count = m_count + 1;
                m_offset = offset;
                return;
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

                /* Read message */
                const u32 offset = m_offset;
                const T ret = m_array[offset];

                /* Adjust offset */
                const u32 n_offset = offset + 1;
                m_offset = (n_offset < Size) ? n_offset : 0;
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
                m_count = count - 1;

                return ret;
            }

            bool RemoveOrdered(T value) {

                /* Find and delete value */
                const u32 offset = m_offset;
                const u32 count = m_count;
                constexpr const u32 max_count = Size;
                const u32 end_offset = offset + count;
                for (u32 i = offset; i < end_offset; ++i) {
                    /* Find delete value */
                    const u32 wrap = (max_count <= i) ? max_count : 0;
                    const u32 wrap_i = i - wrap;
                    T *del = m_array + wrap_i;
                    if (value != *del) { continue; }

                    /* Shift values down by one, over our delete location, up to end of memory */
                    const bool is_wrap = (max_count < end_offset) && (offset <= wrap_i);
                    const u32 count_to_end = (is_wrap) ? (max_count - wrap_i - 1) : (end_offset - wrap_i - 1);
                    std::memmove(del, del + 1, count_to_end * sizeof(T));

                    /* On wrap, shift first value to end, shift values down by one, over the first value, to end of used */
                    if (is_wrap) {
                        m_array[max_count - 1] = m_array[0];
                        const u32 count_from_start = end_offset - max_count - 1;
                        std::memmove(m_array, m_array + 1, count_from_start * sizeof(T));
                    }
                    m_count = count - 1;

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
