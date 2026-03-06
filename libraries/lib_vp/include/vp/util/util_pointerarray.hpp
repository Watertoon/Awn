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
            u32 m_used_count;
            u32 m_max_count;
            T **m_array;
        public:
            constexpr PointerArray() : m_used_count(), m_max_count(), m_array() {/*...*/}
            constexpr ~PointerArray() {/*...*/}

            Result Initialize(imem::IHeap *heap, u32 count) {

                /* Integrity checks */
                RESULT_RETURN_IF(count < 1, ResultRangeNG);
                RESULT_RETURN_UNLESS(count < 0xffff'ffff, ResultRangeNG);

                /* Allocate pointer array */
                T **array = new (heap, alignof(T*)) T*[count];
                RESULT_RETURN_IF(m_array == nullptr, ResultNoMemory);

                /* Set state */
                m_used_count = 0;
                m_max_count = count;
                m_array = array;

                RESULT_RETURN_SUCCESS;
            }

            void Finalize() {
                if (m_array != nullptr) {
                    delete [] m_array;
                    m_array = nullptr;
                    m_used_count = 0;
                    m_max_count  = 0;
                }
                return;
            }

            static constexpr size_t GetRequiredMemorySize(u32 count) {
                return sizeof(T*) * count;
            }

            constexpr void SetMemory(T **memory, u32 count) {
                m_array = memory;
                m_used_count = 0;
                m_max_count = count;
                return;
            }
            constexpr void SetUsedCount(u32 count) {
                m_used_count = count;
            }
        public:
            constexpr Iterator begin() {
                return Iterator(m_array);
            }
            constexpr Iterator end() {
                return Iterator(m_array + m_used_count);
            }
            constexpr Iterator IteratorTo(T **location) {
                return Iterator(location);
            }

            constexpr T *operator[](u32 index) {
                if (VP_UNLIKELY(m_used_count <= index)) {
                    VP_ASSERT(index < m_used_count);
                    index = 0;
                    VP_ASSERT(0 < m_max_count);
                }
                return m_array[index];
            }

            constexpr const T *operator[](u32 index) const {
                if (VP_UNLIKELY(m_used_count <= index)) {
                    VP_ASSERT(index < m_used_count);
                    index = 0;
                    VP_ASSERT(0 < m_max_count);
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

            constexpr void SetPointer(u32 index, T *pointer) {
                VP_ASSERT(index < m_used_count);
                m_array[index] = pointer;
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
        public:
            constexpr u32 IndexOf(const Iterator &pointer_iterator) const {
                return pointer_iterator.m_array - m_array;
            }
            constexpr u32 IndexOf(const T **pointer_iterator) const {
                return pointer_iterator - m_array;
            }
            constexpr u32 IndexOf(const T *pointer) const {
                T **start = m_array;
                T **iter = start;
                T **end = start + m_used_count;
                while (iter != end) {
                    if (*iter != pointer) { iter += 1; continue; }
                    return iter - start;
                }
                return cInvalidEntryIndex;
            }
            constexpr T **GetArray() { return m_array; }
            constexpr const T **GetArray() const { return m_array; }
            constexpr u32 GetUsedCount() const { return m_used_count; }
            constexpr u32 GetMaxCount()  const { return m_max_count; }

            constexpr bool IsValid() const { return m_array != nullptr; }
            constexpr bool IsEmpty() const { return m_used_count == 0; }
            constexpr bool IsFull() const { return (m_max_count <= m_used_count); }
        public:
            struct DefaultCompareLess {
                constexpr bool operator()(const T *&lhs, const T *&rhs) {
                    return *lhs < *rhs;
                }
                constexpr bool operator()(const T *&lhs, const T *&rhs) requires std::is_same_v<std::add_cv_t<char>, std::add_cv_t<T>> {
                    const s32 cmp = ::strcmp(lhs, rhs);
                    return cmp < 0;
                }
            };
            template <auto Member>
            struct DefaultCompareLessMember {
                constexpr bool operator()(const T *&lhs, const T *&rhs) {
                    return lhs->*Member < rhs->*Member;
                }
                constexpr bool operator()(const T *&lhs, const T *&rhs) requires std::is_same_v<std::add_cv_t<char*>, std::add_cv_t<MemberType<Member>>> {
                    const s32 cmp = ::strcmp(lhs->*Member, rhs->*Member);
                    return cmp < 0;
                }
            };
            struct DefaultFilterOutNull {
                constexpr bool operator()(const T *&lhs) {
                    return *lhs == nullptr;
                }
            };
            template <auto Member>
            struct DefaultCompareLessFindMember {
                constexpr bool operator()(const MemberType<Member> *&lhs, const MemberType<Member> *&rhs) {
                    return lhs < rhs;
                }
                constexpr bool operator()(const MemberType<Member> *&lhs, const MemberType<Member> *&rhs) requires std::is_same_v<std::add_cv_t<char*>, std::add_cv_t<MemberType<Member>>> {
                    const s32 cmp = ::strcmp(lhs, rhs);
                    return cmp < 0;
                }
            };
        public:
            template <typename CompareLess>
            constexpr void Sort(CompareLess cmp) {
                std::sort(m_array, m_array + m_used_count, cmp);
            }

            template <typename CompareLess>
            constexpr void HeapSort(CompareLess cmp) {
                std::sort_heap(m_array, m_array + m_used_count, cmp);
            }

            /* The following operations require array to be sorted */
            template <typename Filter>
            constexpr void FilterOut(Filter filter) {
                const T **base_iter = nullptr;
                const T **end_iter = nullptr;
                const T **iter = m_array;
                const T **end = m_array + m_used_count;
                while (iter != end) {
                    ON_SCOPE_EXIT { ++iter; };
                    const bool is_toss = filter(*iter);
                    if (base_iter == nullptr && !is_toss) { continue; }
                    if (is_toss) {
                        if (base_iter == nullptr) { base_iter = iter; }
                        end_iter = iter;
                        continue;
                    }
                    const u32 base_id = this->IndexOf(base_iter);
                    const u32 end_id = static_cast<u32>(end_iter - base_iter) + 1;
                    this->RemoveRange(base_id, end_id);
                    base_iter = nullptr;
                    end = m_array + m_used_count;
                }
                if (base_iter != nullptr) {
                    const u32 base_id = this->IndexOf(base_iter);
                    const u32 end_id = static_cast<u32>(end_iter - base_iter) + 1;
                    this->RemoveRange(base_id, end_id);
                }
                return;
            }

            template <typename CompareLess>
            constexpr Iterator BinarySearch(const T *find_element, CompareLess cmp) {

                /* Binary search for element */
                T **iter = m_array;
                u32 i = m_used_count;
                while (1 < i) {
                    const u32 h = i / 2;
                    const T *&mber = iter[h-1];
                    iter = (cmp(mber, find_element)) ? std::addressof(iter[h]) : iter;
                    i -= h;
                }
                const T *&mber = iter[0];
                iter += (cmp(mber, find_element));

                return Iterator(iter);
            }
    };
    
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
                    if (*iter != pointer) { iter += 1; continue; }
                    return iter - start;
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
