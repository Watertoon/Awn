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

    template <typename T>
    class PointerArray {
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
            u32   m_max_count;
            u32   m_used_count;
            T   **m_array;
        public:
            static constexpr size_t GetWorkMemorySize(u32 count) {
                return sizeof(T*) * count;
            }
        public:
            constexpr ALWAYS_INLINE PointerArray() : m_max_count(), m_used_count(), m_array() {/*...*/}
            constexpr ~PointerArray() {/*...*/}

            constexpr Iterator begin() {
                return Iterator(m_array);
            }
            constexpr Iterator end() {
                return Iterator(m_array + m_used_count);
            }

            constexpr ALWAYS_INLINE T *operator[](u32 index) {
                if (m_used_count <= index) {
                    VP_ASSERT(index < m_used_count);
                    index = 0;
                    VP_ASSERT(0 < m_max_count);
                }
                return m_array[index];
            }

            constexpr ALWAYS_INLINE const T *operator[](u32 index) const {
                if (m_used_count <= index) {
                    VP_ASSERT(index < m_used_count);
                    index = 0;
                    VP_ASSERT(0 < m_max_count);
                }
                return m_array[index];
            }

            void Initialize(imem::IHeap *heap, u32 count) {

                /* Integrity checks */
                if (count == 0) { return; }
                VP_ASSERT(count < 0xffff'ffff);
                VP_ASSERT(m_array == nullptr && m_max_count == 0);

                /* Allocate pointer array */
                m_array = new (heap, alignof(T*)) T*[count];
                VP_ASSERT(m_array != nullptr);

                /* Set state */
                m_used_count = 0;
                m_max_count  = count;

                return;
            }

            void Finalize() {

                if (m_array != nullptr) {
                    delete [] m_array;
                }
                m_used_count = 0;
                m_max_count  = 0;
                m_array = nullptr;

                return;
            }

            constexpr void SetMemory(T **memory, u32 count) {
                m_array = memory;
                m_used_count = 0;
                m_max_count = count;
                return;
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
                VP_ASSERT(id < m_max_count);
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

            constexpr void Swap(u32 index0, u32 index1) {
                VP_ASSERT(index0 < m_used_count);
                VP_ASSERT(index1 < m_used_count);
                T *l = m_array[index0];
                T *r = m_array[index1];
                m_array[index0] = r;
                m_array[index1] = l;
                return;
            }

            constexpr bool RemoveUnordered(T *iter) {
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
            constexpr T **GetArray() { return m_array; }
            constexpr u32 GetUsedCount() const { return m_used_count; }
            constexpr u32 GetMaxCount()  const { return m_max_count; }

            constexpr bool IsValid() const { return m_array != nullptr; }
            constexpr bool IsEmpty() const { return m_used_count == 0; }
            constexpr bool IsFull() const { return (m_max_count <= m_used_count); }
        public:
            constexpr void SetUsedCount(u32 count) {
                m_used_count = count;
            }

            constexpr void SetPointer(u32 index, T *pointer) {
                VP_ASSERT(index < m_used_count);
                m_array[index] = pointer;
            }
        private:
            struct DefaultCompareLess {
                constexpr bool operator()(T *&lhs, T *&rhs) {
                    return *lhs < *rhs;
                }
                constexpr bool operator()(T *&lhs, T *&rhs) requires std::is_same_v<std::add_cv_t<char>, std::add_cv_t<T>> {
                    const s32 cmp = ::strcmp(lhs, rhs);
                    return cmp < 0;
                }
            };
            struct DefaultCompareEqual {
                constexpr bool operator()(T *&lhs, T *&rhs) {
                    return *lhs == *rhs;
                }
                constexpr bool operator()(T *&lhs, T *&rhs) requires std::is_same_v<std::add_cv_t<char>, std::add_cv_t<T>> {
                    return vp::util::StringCompare(lhs, rhs);
                }
            };
            template <auto Member>
            struct DefaultCompareLessMember {
                constexpr bool operator()(ParentType<Member> *&lhs, ParentType<Member> *&rhs) {
                    return lhs->*Member < rhs->*Member;
                }
                constexpr bool operator()(ParentType<Member> *&lhs, ParentType<Member> *&rhs) requires std::is_same_v<std::add_cv_t<char*>, std::add_cv_t<MemberType<Member>>> {
                    const s32 cmp = ::strcmp(lhs->*Member, rhs->*Member);
                    return cmp < 0;
                }
            };
        public:
            template <typename CompareLess = DefaultCompareLess>
            constexpr void Sort() {
                std::sort(m_array, m_array + m_used_count, CompareLess());
            }

            template <typename CompareLess = DefaultCompareLess>
            constexpr void HeapSort() {
                std::sort_heap(m_array, m_array + m_used_count, CompareLess());
            }

            template <auto Member, typename CompareLessMember = DefaultCompareLessMember<Member>>
                requires (std::is_same<ParentType<Member>, T>::value)
            constexpr void SortOn() {
                std::sort(m_array, m_array + m_used_count, CompareLessMember());
            }

            template <auto Member, typename CompareLessMember = DefaultCompareLessMember<Member>>
                requires (std::is_same<ParentType<Member>, T>::value)
            constexpr void HeapSortOn() {
                std::sort_heap(m_array, m_array + m_used_count, CompareLessMember());
            }

            /* Requires array to be sorted */
            template <typename CompareEqual = DefaultCompareEqual>
            constexpr void FilterOut() {
                u32 used_count = m_used_count;
                for (u32 i = 0; i < used_count - 1; ++i) {
                    u32 y = i;
                    for (; y < used_count; ++y) {
                        const bool is_toss = CompareEqual(m_array[i], m_array[y]);
                        if (!is_toss) { continue; }
                        this->RemoveRange(y, 1);
                        used_count = m_used_count;
                    }
                }
            }

            template <typename CompareEqual = DefaultCompareEqual, typename CompareLess = DefaultCompareLess>
            constexpr bool BinarySearch(u32 *out_index, const T *find_element) {

                /* Binary search for find element */
                u32  entry_index = m_used_count / 2;
                u32  high_iter   = m_used_count;
                u32  low_iter    = 0;
                u32  i           = 0;
                T   *current_obj = m_array[entry_index];
                while (!CompareEqual(find_element, current_obj)) {

                    u32 result = entry_index;
                    i = low_iter;
                    if (CompareLess(find_element, current_obj)) {
                        i = high_iter;
                        high_iter = entry_index;
                        result = low_iter;
                    }

                    low_iter = result;
                    if (i == entry_index) { return false; }

                    i = high_iter + low_iter;

                    entry_index = i / 2;
                    current_obj = m_array[entry_index];
                }

                *out_index = entry_index;
    
                return true;
            }
    };
}
