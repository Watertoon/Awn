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
                    constexpr ALWAYS_INLINE Iterator(const T *array) : m_array(const_cast<T*>(array)) {/*...*/}

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
            u32  m_count;
            T   *m_array;
        public:
        public:
            constexpr HeapArray() : m_count(), m_array() {/*...*/}
            constexpr ~HeapArray() {/*...*/}

            Result Initialize(imem::IHeap *heap, u32 count) {

                /* Integrity checks */
                RESULT_RETURN_IF(count < 1, ResultRangeNG);

                /* Allocate an array of objects */
                T *array = new (heap, alignof(T)) T[count];
                RESULT_RETURN_IF(array == nullptr, ResultNoMemory);

                /* Set count */
                m_count = count;
                m_array = array;

                RESULT_RETURN_SUCCESS;
            }

            void Finalize() {
                if (m_array == nullptr) { return; }
                delete [] (m_array);
                m_array = nullptr;
                m_count  = 0;
                return;
            }

            static constexpr size_t GetRequiredMemorySize(u32 count) {
                return sizeof(T) * count;
            }

            constexpr void SetMemory(T *memory, u32 count) {
                m_array = memory;
                m_count = count;
                return;
            }
        public:
            constexpr Iterator begin() {
                return Iterator(m_array);
            }
            constexpr Iterator begin() const {
                return Iterator(m_array);
            }
            constexpr Iterator end() {
                return Iterator(m_array + m_count);
            }
            constexpr Iterator end() const {
                return Iterator(m_array + m_count);
            }

            constexpr Iterator IteratorTo(T *array_location) {
                return Iterator(array_location);
            }

            constexpr T &operator[](u32 index) {
                VP_ASSERT(m_count > index);
                return m_array[index];
            }

            constexpr const T &operator[](u32 index) const {
                VP_ASSERT(m_count > index);
                return m_array[index];
            }

            constexpr void Fill(const T &value) {
                T *iter = m_array;
                T *end = m_array + m_count;
                while (iter != end) {
                    *iter = value;
                    ++iter;
                }
                return;
            }
            constexpr void FillRange(const T &value, u32 base, u32 count) {
                VP_ASSERT(base < m_count);
                VP_ASSERT(count < m_count);
                T *iter = m_array + base;
                T *end = m_array + count;
                while (iter != end) {
                    *iter = value;
                    ++iter;
                }
                return;
            }
            constexpr void CopyFrom(const HeapArray<T> &array) {
                const u32 copy_count = vp::util::Min(m_count, array.m_count);
                T *iter = m_array;
                T *r_iter = array.m_array;
                T *end = m_array + copy_count;
                while (iter != end) {
                    *iter = *r_iter;
                    ++r_iter;
                    ++iter;
                }
                return;
            }
        public:
            constexpr u32 IndexOf(T *element) const {
                return element - m_array;
            }

            constexpr T *GetArray() { return m_array; }
            constexpr const T *GetArray() const { return m_array; }
            constexpr u32 GetCount() const { return m_count; }

            constexpr bool IsValid() const { return m_array != nullptr; }
            constexpr bool IsContain(const T *element) const {
                const bool is_start = (m_array <= element);
                const bool is_end = (element < m_array + m_count);
                return is_start && is_end;
            }
        public:
            struct DefaultCompareLess {
                constexpr bool operator()(const T &lhs, const T &rhs) {
                    return lhs < rhs;
                }
                constexpr bool operator()(const T &lhs, const T &rhs) requires std::is_same_v<std::add_cv_t<char*>, std::add_cv_t<T>> {
                    const s32 cmp = std::strcmp(lhs, rhs);
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
                static constexpr auto MemberPtr = Member;

                constexpr bool operator()(const T &lhs, const T &rhs) {
                    return lhs.*Member < rhs.*Member;
                }
                constexpr bool operator()(const T &lhs, const T &rhs) requires std::is_same_v<std::add_cv_t<char*>, std::add_cv_t<MemberType<Member>>> {
                    const s32 cmp = std::strcmp(lhs.*Member, rhs.*Member);
                    return cmp < 0;
                }
            };

            template <auto Member>
            struct DefaultCompareLessSearchMember {
                static constexpr auto MemberPtr = Member;

                constexpr bool operator()(const MemberType<Member> &lhs, const MemberType<Member> &rhs) {
                    return lhs < rhs;
                }
                constexpr bool operator()(const MemberType<Member> &lhs, const MemberType<Member> &rhs) requires std::is_same_v<std::add_cv_t<char*>, std::add_cv_t<MemberType<Member>>> {
                    const s32 cmp = std::strcmp(lhs, rhs);
                    return cmp < 0;
                }
            };
        public:
            template <typename CompareLess>
            constexpr void Sort(CompareLess cmp) {
                std::sort(m_array, m_array + m_count, cmp);
            }

            template <typename CompareLess>
            constexpr void HeapSort(CompareLess cmp) {
                std::sort_heap(m_array, m_array + m_count, cmp);
            }

            template <typename CompareLessSearchMember>
            constexpr Iterator LowerBound(const MemberType<CompareLessSearchMember::MemberPtr> &find_element, CompareLessSearchMember cmp) const {

                /* Binary search for closest find element */
                using Member = MemberType<CompareLessSearchMember::MemberPtr>;
                constexpr auto MemberPtr = CompareLessSearchMember::MemberPtr;
                T *iter = m_array;
                u32 i = m_count;
                while (1 < i) {
                    const u32 h = i / 2;
                    const Member &mber = iter[h-1].*MemberPtr;
                    const u32 n = (cmp(mber, find_element)) * h;
                    iter += n;
                    i -= h;
                }

                return Iterator(iter);
            }
            /* This function requires the heap array to be sorted */
            template <typename CompareLessSearchMember>
            constexpr Iterator BinarySearch(const MemberType<CompareLessSearchMember::MemberPtr> &find_element, CompareLessSearchMember cmp) const {
                auto iter = this->LowerBound(find_element, cmp);
                const auto end = this->end();
                constexpr auto MemberPtr = CompareLessSearchMember::MemberPtr;
                bool is_valid = (iter != end) && !(cmp(find_element, (*iter).*MemberPtr));
                return is_valid ? iter : end;
            }
    };

    template <typename T, size_t Count>
    class FixedArray {
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
            T m_array[Count];
        public:
            constexpr FixedArray() : m_array{} {/*...*/}
            constexpr ~FixedArray() {/*...*/}

            constexpr Iterator begin() {
                return Iterator(m_array);
            }
            constexpr Iterator end() {
                return Iterator(m_array + Count);
            }

            constexpr Iterator IteratorTo(T *array_location) {
                return Iterator(array_location);
            }

            constexpr T &operator[](u32 index) {
                VP_ASSERT(Count > index);
                return m_array[index];
            }

            constexpr const T &operator[](u32 index) const {
                VP_ASSERT(Count > index);
                return m_array[index];
            }

            constexpr void Fill(const T &value) {
                T *iter = m_array;
                T *end = m_array + Count;
                while (iter != end) {
                    *iter = value;
                    ++iter;
                }
                return;
            }
            constexpr void FillRange(const T &value, u32 base, u32 count) {
                VP_ASSERT(base < Count);
                VP_ASSERT(count < Count);
                T *iter = m_array + base;
                T *end = m_array + count;
                while (iter != end) {
                    *iter = value;
                    ++iter;
                }
                return;
            }
            constexpr void CopyFrom(const HeapArray<T> &array) {
                const u32 copy_count = vp::util::Min(Count, array.Count);
                T *iter = m_array;
                T *r_iter = array.m_array;
                T *end = m_array + copy_count;
                while (iter != end) {
                    *iter = *r_iter;
                    ++r_iter;
                    ++iter;
                }
                return;
            }
        public:
            constexpr u32 IndexOf(T *element) const {
                return element - m_array;
            }

            constexpr T *GetArray() { return m_array; }
            constexpr const T *GetArray() const { return m_array; }
            constexpr u32 GetCount() const { return Count; }

            constexpr bool IsValid() const { return m_array != nullptr; }
            constexpr bool IsContainAddress(const T *element) const {
                const bool is_start = (m_array <= element);
                const bool is_end = (element < m_array + Count);
                return is_start && is_end;
            }
        public:
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
                static constexpr auto MemberPtr = Member;

                constexpr bool operator()(const T &lhs, const T &rhs) {
                    return lhs.*Member < rhs.*Member;
                }
                constexpr bool operator()(const T &lhs, const T &rhs) requires std::is_same_v<std::add_cv_t<char*>, std::add_cv_t<MemberType<Member>>> {
                    const s32 cmp = ::strcmp(lhs.*Member, rhs.*Member);
                    return cmp < 0;
                }
            };

            template <auto Member>
            struct DefaultCompareLessSearchMember {
                static constexpr auto MemberPtr = Member;

                constexpr bool operator()(const MemberType<Member> &lhs, const MemberType<Member> &rhs) {
                    return lhs < rhs;
                }
                constexpr bool operator()(const MemberType<Member> &lhs, const MemberType<Member> &rhs) requires std::is_same_v<std::add_cv_t<char*>, std::add_cv_t<MemberType<Member>>> {
                    const s32 cmp = ::strcmp(lhs, rhs);
                    return cmp < 0;
                }
            };
        public:
            template <typename CompareLess>
            constexpr void Sort(CompareLess cmp) {
                std::sort(m_array, m_array + Count, cmp);
            }

            template <typename CompareLess>
            constexpr void HeapSort(CompareLess cmp) {
                std::sort_heap(m_array, m_array + Count, cmp);
            }

            /* This function requires the heap array to be sorted */
            template <typename CompareLessSearchMember>
            constexpr Iterator BinarySearch(const MemberType<CompareLessSearchMember::MemberPtr> &find_element, CompareLessSearchMember cmp) const {

                /* Binary search for find element */
                using Member = MemberType<CompareLessSearchMember::MemberPtr>;
                constexpr auto MemberPtr = CompareLessSearchMember::MemberPtr;
                T *iter = m_array;
                u32 i = Count;
                while (1 < i) {
                    const u32 h = i / 2;
                    const Member &mber = iter[h-1].*MemberPtr;
                    iter = (cmp(mber, find_element)) ? std::addressof(iter[h]) : iter;
                    i -= h;
                }
                const Member &mber = iter[0].*MemberPtr;
                iter += (cmp(mber, find_element));
    
                return Iterator(iter);
            }
    };
}
