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

    constexpr bool StringCompare(const char *lhs, const char *rhs);

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
            constexpr ALWAYS_INLINE PointerArray() : m_max_count(0), m_used_count(0), m_array(nullptr) {/*...*/}
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

            void Initialize(imem::IHeap *heap, u32 pointer_count, s32 alignment = 8) {

                /* Integrity checks */
                if (pointer_count == 0) { return; }
                VP_ASSERT(pointer_count < 0xffff'ffff);
                VP_ASSERT(m_array == nullptr && m_max_count == 0);

                /* Allocate pointer array */
                m_array = new (heap, alignment) T*[pointer_count];
                VP_ASSERT(m_array != nullptr);

                /* Set state */
                m_used_count = 0;
                m_max_count  = pointer_count;

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

            constexpr void SetBuffer(void *pointer_buffer, u32 pointer_count) {

                /* Integrity checks */
                VP_ASSERT(pointer_buffer != nullptr && pointer_count != 0);

                /* Set state */
                m_array      = pointer_buffer;
                m_used_count = 0;
                m_max_count  = pointer_count;
            
                return;
            }

            constexpr u32 PushPointer(T *pointer) {
                VP_ASSERT(m_used_count < m_max_count);
                m_array[m_used_count] = pointer;
                const u32 index = m_used_count;
                m_used_count = index + 1;
                return index;
            }

            constexpr T *PopPointer() {
                T *ret = m_array[m_used_count - 1];
                m_array[m_used_count - 1] = nullptr;
                --m_used_count;
                return ret;
            }

            void RemoveRange(u32 base_index, u32 number_of_elements) {

                const u32 move_element_index = base_index + number_of_elements;
                const u32 move_element_count = m_used_count - move_element_index;
                VP_ASSERT(move_element_index <= m_used_count);

                if (move_element_count != 0) {
                    ::memmove(std::addressof(m_array[base_index]), std::addressof(m_array[move_element_index]), move_element_count * sizeof(T**));
                }
                m_used_count = m_used_count - number_of_elements;

                return;
            }

            constexpr void Clear() { m_used_count = 0; }

            constexpr u32 FindIndexOf(const T *pointer) const {
                for (u32 i = 0; i < m_used_count; ++i) {
                    if (m_array[i] == pointer) { return i; }
                }
                return cInvalidEntryIndex;
            }
            constexpr u32 GetUsedCount() const { return m_used_count; }
            constexpr u32 GetMaxCount()  const { return m_max_count; }

            constexpr bool IsValid() const { return m_array != nullptr; }
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
                constexpr bool operator()(const T *&lhs, const T *&rhs) {
                    return *lhs < *rhs;
                }
                constexpr bool operator()(const T *&lhs, const T *&rhs) requires std::is_same_v<std::add_cv_t<char>, std::add_cv_t<T>> {
                    const s32 cmp = ::strcmp(lhs, rhs);
                    return cmp < 0;
                }
            };
            struct DefaultCompareEqual {
                constexpr bool operator()(const T *&lhs, const T *&rhs) {
                    return *lhs == *rhs;
                }
                constexpr bool operator()(const T *&lhs, const T *&rhs) requires std::is_same_v<std::add_cv_t<char>, std::add_cv_t<T>> {
                    return vp::util::StringCompare(lhs, rhs);
                }
            };
            template <auto Member>
            struct DefaultCompareLessMember {
                constexpr bool operator()(const ParentType<Member> *&lhs, const ParentType<Member> *&rhs) {
                    return *lhs->*Member < *rhs->*Member;
                }
                constexpr bool operator()(const ParentType<Member> *&lhs, const ParentType<Member> *&rhs) requires std::is_same_v<std::add_cv_t<char>, std::add_cv_t<MemberType<Member>>> {
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
