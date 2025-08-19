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

    template <typename T, u32 Count>
    class FixedPointerArray {
        public:
            static constexpr u32 cInvalidEntryIndex = 0xffff'ffff;
        public:
            class Iterator {
                private:
                    T **m_array;
                public:
                    constexpr ALWAYS_INLINE Iterator(T **array) : m_array(array) {/*...*/}
                    constexpr ALWAYS_INLINE ~Iterator() {/*...*/}

                    constexpr ALWAYS_INLINE T *&operator*() {
                        return *m_array;
                    }
                    constexpr ALWAYS_INLINE const T *&operator*() const {
                        return *m_array;
                    }

                    constexpr ALWAYS_INLINE bool operator==(const Iterator &rhs) const {
                        return m_array == rhs.m_array;
                    }

                    constexpr ALWAYS_INLINE bool operator!=(const Iterator &rhs) const {
                        return m_array != rhs.m_array;
                    }

                    constexpr ALWAYS_INLINE Iterator &operator++() {
                        ++m_array;
                        return *this;
                    }
                    constexpr ALWAYS_INLINE Iterator &operator++([[maybe_unused]]int) {
                        ++m_array;
                        return *this;
                    }
                    constexpr ALWAYS_INLINE Iterator &operator--() {
                        --m_array;
                        return *this;
                    }
            };
        private:
            u32  m_used_count;
            T   *m_array[Count];
        public:
            constexpr FixedPointerArray() : m_used_count(), m_array{} {/*...*/}
            constexpr ~FixedPointerArray() {/*...*/}

            constexpr Iterator begin() {
                return Iterator(m_array);
            }
            constexpr Iterator end() {
                return Iterator(m_array + m_used_count);
            }

            constexpr T *operator[](u32 index) {
                if (m_used_count <= index) {
                    VP_ASSERT(index < m_used_count);
                    index = 0;
                    VP_ASSERT(0 < Count);
                }
                return m_array[index];
            }

            constexpr const T *operator[](u32 index) const {
                if (m_used_count <= index) {
                    VP_ASSERT(index < m_used_count);
                    index = 0;
                    VP_ASSERT(0 < Count);
                }
                return m_array[index];
            }

            constexpr u32 PushPointer(T *pointer) {
                VP_ASSERT(!this->IsFull());
                const u32 index = m_used_count;
                m_array[index] = pointer;
                m_used_count = index + 1;
                return index;
            }

            constexpr u32 PushPointerAtomic(T *pointer) {
                const u32 id = vp::util::InterlockedFetchAddAcquire(std::addressof(m_used_count), 1u);
                VP_ASSERT(id < Count);
                m_array[id] = pointer;
                return id;
            }

            constexpr T *PopPointer() {
                const u32 count = m_used_count;
                VP_ASSERT(0 < count);
                const u32 index = count - 1;
                T *ret = m_array[index];
                m_array[index] = nullptr;
                m_used_count = index;
                return ret;
            }

            constexpr void Clear() { m_used_count = 0; }

            bool RemoveRange(u32 base_index, u32 number_of_elements) {

                /* Integrity check */
                const u32 count = m_used_count;
                const u32 end_i = base_index + number_of_elements;
                if (count < end_i) { return false; }

                /* Shift elements down from end to base of remove range */
                const u32 elements_after = count - end_i;
                if (elements_after != 0) {
                    T **base = m_array + base_index;
                    T **end = base + end_i;
                    ::memmove(base, end, elements_after * sizeof(T**));
                }

                m_used_count = count - number_of_elements;

                return true;
            }

            bool RemoveOrdered(T *iter) {
                const u32 i = this->IndexOf(iter);
                if (i == cInvalidEntryIndex) { return false; }
                return this->RemoveRange(i, 1);
            }

            bool RemoveUnordered(T *iter) {
                const u32 i = this->IndexOf(iter);
                if (i == cInvalidEntryIndex) { return false; }
                const u32 next_count = m_used_count - 1;
                m_array[i] = m_array[next_count];
                m_used_count = next_count;
                return true;
            }

            constexpr u32 IndexOf(const T *pointer) const {
                T **start = m_array;
                T **iter = start;
                T **end = start + m_used_count;
                while (iter != end) {
                    if (*iter != pointer) { ++iter; continue; }
                    return start - iter;
                }
                return cInvalidEntryIndex;
            }
            constexpr u32 GetUsedCount() const { return m_used_count; }
            constexpr u32 GetMaxCount()  const { return Count; }

            constexpr bool IsEmpty() const { return m_used_count == 0; }
            constexpr bool IsFull() const { return (Count <= m_used_count); }
        public:
            constexpr void SetUsedCount(u32 count) { m_used_count = count; }
            constexpr void SetPointer(u32 index, T *pointer) {
                VP_ASSERT(index < m_used_count);
                m_array[index] = pointer;
            }
    };
}
