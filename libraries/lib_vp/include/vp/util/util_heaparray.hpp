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
    class HeapArray {        
        public:
            class Iterator {
                private:
                    T *m_array;
                public:
                    constexpr ALWAYS_INLINE Iterator(T *array) : m_array(array) {/*...*/}

                    constexpr ALWAYS_INLINE T &operator*() {
                        return *m_array;
                    }
                    constexpr ALWAYS_INLINE const T &operator*() const {
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
            T   *m_object_array;
            u32  m_array_count;
        public:
            static constexpr size_t GetWorkMemorySize(u32 count) {
                return sizeof(T) * count;
            }
        public:
            constexpr ALWAYS_INLINE HeapArray() : m_object_array(), m_array_count() {/*...*/}
            constexpr ALWAYS_INLINE ~HeapArray() {/*...*/}

            bool Initialize(imem::IHeap *alloc_heap, u32 count) {

                /* Fail on invalid element count */
                if (count == 0) { return false; }

                /* Allocate an array of objects */
                m_object_array = reinterpret_cast<T*>(::operator new(sizeof(T) * count, alloc_heap, alignof(T)));

                /* Fail on nullptr */
                if (m_object_array == nullptr) { return false; }

                /* Set count */
                m_array_count = count;

                /* Initialize every object */
                for (u32 i = 0; i < m_array_count; ++i) {
                    std::construct_at(std::addressof(m_object_array[i]));
                }

                return true;
            }

            void Finalize() {

                /* Bail if there's no buffer */
                if (m_object_array == nullptr) { return; }

                /* Destruct every object */
                for (u32 i = 0; i < m_array_count; ++i) {
                    std::destroy_at(std::addressof(m_object_array[i]));
                }

                /* Free buffer */
                if (m_object_array != nullptr) {
                    ::operator delete(m_object_array);
                    m_object_array = nullptr;
                    m_array_count  = 0;
                }

                return;
            }

            constexpr void SetMemory(T *memory, u32 count) {
                m_object_array = memory;
                m_array_count = count;
                return;
            }

            constexpr Iterator begin() {
                return Iterator(m_object_array);
            }
            constexpr Iterator end() {
                return Iterator(m_object_array + m_array_count);
            }

            constexpr T &operator[](u32 index) {
                VP_ASSERT(m_array_count > index);
                return m_object_array[index];
            }

            constexpr T &operator[](u32 index) const {
                VP_ASSERT(m_array_count > index);
                return m_object_array[index];
            }

            constexpr T *GetArray() { return m_object_array; }
            constexpr u32 GetCount() const { return m_array_count; }

            constexpr u32 IndexOf(T *element) const {
                return element - m_object_array;
            }

            constexpr bool IsValid() const { return m_object_array != nullptr; }
        private:
            struct DefaultCompareLess {
                constexpr bool operator()(const T &lhs, const T &rhs) {
                    return lhs < rhs;
                }
                constexpr bool operator()(const T &lhs, const T &rhs) requires std::is_same_v<std::add_cv_t<char*>, std::add_cv_t<T>> {
                    const s32 cmp = ::strcmp(lhs, rhs);
                    return cmp < 0;
                }
            };
            struct DefaultCompareEqual {
                constexpr bool operator()(const T &lhs, const T &rhs) {
                    return lhs == rhs;
                }
                constexpr bool operator()(const T &lhs, const T &rhs) requires std::is_same_v<std::add_cv_t<char*>, std::add_cv_t<T>> {
                    return vp::util::StringCompare(lhs, rhs);
                }
            };
            template <auto Member>
            struct DefaultCompareLessMember {
                constexpr bool operator()(const ParentType<Member> &lhs, const ParentType<Member> &rhs) {
                    return lhs.*Member < rhs.*Member;
                }
                constexpr bool operator()(const ParentType<Member> &lhs, const ParentType<Member> &rhs) requires std::is_same_v<std::add_cv_t<char*>, std::add_cv_t<MemberType<Member>>> {
                    const s32 cmp = ::strcmp(lhs.*Member, rhs.*Member);
                    return cmp < 0;
                }
            };
        public:
            template <typename CompareLess = DefaultCompareLess>
            constexpr void Sort() {
                std::sort(m_object_array, m_object_array + m_array_count, CompareLess());
            }

            template <typename CompareLess = DefaultCompareLess>
            constexpr void HeapSort() {
                std::sort_heap(m_object_array, m_object_array + m_array_count, CompareLess());
            }

            template <auto Member, typename CompareLessMember = DefaultCompareLessMember<Member>>
                requires (std::is_same<ParentType<Member>, T>::value)
            constexpr void SortOn() {
                std::sort(m_object_array, m_object_array + m_array_count, CompareLessMember());
            }

            template <auto Member, typename CompareLessMember = DefaultCompareLessMember<Member>>
                requires (std::is_same<ParentType<Member>, T>::value)
            constexpr void HeapSortOn() {
                std::sort_heap(m_object_array, m_object_array + m_array_count, CompareLessMember());
            }

            /* This function requires the heap array to be sorted */
            template <auto MemberPtr>
                requires (std::is_same<ParentType<MemberPtr>, T>::value)
            constexpr bool BinarySearchOn(u32 *out_index, const MemberType<MemberPtr> &find_element) const {

                /* Binary search for find element */
                u32  entry_index = m_array_count / 2;
                u32  high_iter   = m_array_count;
                u32  low_iter    = 0;
                u32  i           = 0;
                T   *current_obj = std::addressof(m_object_array[entry_index]);
                while (find_element != current_obj->*MemberPtr) {

                    u32 result = entry_index;
                    i = low_iter;
                    if (find_element < current_obj->*MemberPtr) {
                        i = high_iter;
                        high_iter = entry_index;
                        result = low_iter;
                    }

                    low_iter = result;
                    if (i == entry_index) { return false; }

                    i = high_iter + low_iter;

                    entry_index = i / 2;
                    current_obj = std::addressof(m_object_array[entry_index]);
                }

                *out_index = entry_index;
    
                return true;
            }
    };
}
