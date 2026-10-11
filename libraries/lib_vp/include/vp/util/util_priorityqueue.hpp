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

    template <typename T, auto KeyMemberPtr>
    class PriorityQueue {
        public:
            using KeyType = vp::util::MemberType<KeyMemberPtr>;
        public:
            T **m_queue;
            u32 m_max;
            u32 m_count;
        private:
            constexpr T **InsertFixup(T **pivot) {

                /* Calculate sort range */
                T **head = m_queue;
                VP_ASSERT(head <= pivot);
                const u32 p_count = static_cast<u32>(pivot - head);

                /* Integrity check */
                if (p_count == 0) { return pivot; }
                VP_ASSERT(p_count < m_count);

                /* Sort */
                T  *pivot_node = *pivot;
                T **low_pair =  pivot;
                T **high_pair =  head + ((p_count + 1) >> 1) - 1;
                u32   i =  p_count;

                while ((*high_pair)->*KeyMemberPtr < pivot_node->*KeyMemberPtr) {

                    /* Swap */
                    *low_pair = *high_pair;

                    /* Adjust pairs */
                    const u32 next_low = ((i + 1) >> 1) - 1;
                    low_pair = head + next_low;
                    if (next_low == 0) { break; }

                    const u32 next_high = ((i + 1) >> 2) - 1;
                    high_pair = head + next_high;
                    i = next_low;
                }

                *low_pair = pivot_node;

                return low_pair;
            }

            constexpr T **RemoveFixup(T **iter) {

                /* Calculate sort range */
                T **head = m_queue;
                VP_ASSERT(head <= iter);
                const u32 p_count = static_cast<u32>(iter - head);
                const u32 count = m_count;
                VP_ASSERT(p_count < count);

                /* Calculate starting positions */
                const u32 start = (p_count << 1);
                const u32 start_l = start | 1;
                const u32 start_h = (start + 2) & 0xffff'fffe;

                T **high_pair = (start_h < count) ? (head + start_h) : nullptr;

                /* Ensure start l is in range */
                if (count <= start_l) { return iter; }

                /* Sort */
                T *low_pair = head[start_l];
                T *pivot_node = *iter;
                u32 l_i = start_l;
                u32 h_i = start_h;

                while (pivot_node->*KeyMemberPtr < low_pair->*KeyMemberPtr || (high_pair != nullptr && pivot_node->*KeyMemberPtr < (*high_pair)->*KeyMemberPtr)) {

                    /* Swap pair */
                    if (high_pair == nullptr || (*high_pair)->*KeyMemberPtr <= low_pair->*KeyMemberPtr) {
                        *iter = low_pair;
                        h_i = l_i;
                    } else {
                        *iter = *high_pair;
                    }

                    iter = head + h_i;

                    /* Find next pairs */
                    l_i = (h_i << 1) | 1;
                    h_i = (h_i << 1) + 2;

                    if (count <= l_i) { break; }

                    high_pair = (h_i < count) ? (head + h_i) : nullptr;
                    low_pair = head[l_i];
                }

                *iter = pivot_node;

                return iter;
            }
        public:
            constexpr  PriorityQueue() : m_queue(), m_max(), m_count() {/*...*/}
            constexpr ~PriorityQueue() {/*...*/}

            Result Initialize(imem::IHeap *heap, u32 max_count) {

                /* Integrity checks */
                RESULT_RETURN_UNLESS(0 < max_count, ResultRangeNG);
    
                /* Allocate queue */
                T **queue = new (heap, alignof(T*)) T*[max_count];
                RESULT_RETURN_UNLESS(queue != nullptr, ResultNoMemory);

                /* Setup */
                m_queue = queue;
                m_count = 0;
                m_max = max_count;

                RESULT_RETURN_SUCCESS;
            }

            void Finalize() {
                if (m_queue != nullptr) {
                    delete [] m_queue;
                    m_queue = nullptr;
                    m_count = 0;
                    m_max   = 0;
                }
                return;
            }

            static constexpr size_t GetRequiredMemorySize(u32 count) {
                return sizeof(T*) * count;
            }

            constexpr void SetMemory(T *array, u32 count) {
                m_queue = array;
                m_count = 0;
                m_max = count;
            }
            constexpr void SetUsedCount(u32 count) {
                m_count = count;
            }
        public:
            constexpr void Insert(T *new_node) {

                /* Integrity check */
                VP_ASSERT(new_node != nullptr);
                VP_ASSERT(m_count < m_max);

                /* Add new node */
                const u32 last_count = m_count;
                m_queue[last_count] = new_node;
                m_count = last_count + 1;

                /* Fixup queue */
                this->InsertFixup(m_queue + last_count);

                return;
            }

            constexpr T *Peek() {
                return m_queue[0];
            }

            constexpr T *RemoveFront() {

                /* Intergrity check bounds */
                VP_ASSERT(0 < m_count);

                /* Remove element from the queue */
                const u32 next_count = m_count - 1;
                m_count = next_count;

                /* Fixup */
                T **head = m_queue;
                T *value = head[0];
                if (next_count != 0) {
                    head[0] = head[next_count];
                    this->RemoveFixup(head);
                }

                return value;
            }

            constexpr T **IterTo(T *node) {
                for (u32 i = 0; i < m_count; ++i) {
                    if (m_queue[i] == node) { return std::addressof(m_queue[i]); }
                }
                VP_ASSERT(false);
                return nullptr;
            }

            constexpr void Remove(T **iter) {

                /* Intergrity check bounds */
                VP_ASSERT(m_queue <= iter && iter < m_queue + m_count);

                /* Remove element from the queue */
                --m_count;

                /* No fixup if last removed node, or node at end of queue */
                if (m_count == 0 || (m_queue + m_count) == iter) { return; }

                /* Fixup */
                *iter = m_queue[m_count];
                T **insert_location = this->InsertFixup(iter);
                if (insert_location != iter) { return; } 

                this->RemoveFixup(iter);

                return;
            }

            constexpr void Clear() {
                m_count = 0;
            }
        public:
            constexpr T **GetArray() { return m_queue; }
            constexpr const T **GetArray() const { return m_queue; }
            constexpr u32 GetMaxCount() const { return m_max; }
            constexpr u32 GetUsedCount() const { return m_count; }

            constexpr bool IsValid() const { return m_queue != nullptr; }
            constexpr bool IsEmpty() const { return 0 == m_count; }
            constexpr bool IsFull() const { return m_max == m_count; }
    };

    template <typename T, auto KeyMemberPtr, const size_t Size>
    class FixedPriorityQueue {
        public:
            using KeyType = vp::util::MemberType<KeyMemberPtr>;
        public:
            u32  m_count;
            T   *m_queue[Size];
        private:
            constexpr T **InsertFixup(T **pivot) {

                /* Calculate sort range */
                T **head = m_queue;
                VP_ASSERT(head <= pivot);
                const u32 p_count = static_cast<u32>(pivot - head);

                /* Integrity check */
                if (p_count == 0) { return pivot; }
                VP_ASSERT(p_count < m_count);

                /* Sort */
                T  *pivot_node = *pivot;
                T **low_pair = pivot;
                T **high_pair = head + ((p_count + 1) >> 1) - 1;
                u32 i = p_count;

                while ((*high_pair)->*KeyMemberPtr < pivot_node->*KeyMemberPtr) {

                    /* Swap */
                    *low_pair = *high_pair;

                    /* Adjust pairs */
                    const u32 next_low = ((i + 1) >> 1) - 1;
                    low_pair = head + next_low;
                    if (next_low == 0) { break; }

                    const u32 next_high = ((i + 1) >> 2) - 1;
                    high_pair = head + next_high;
                    i = next_low;
                }

                *low_pair = pivot_node;

                return low_pair;
            }

            constexpr T **RemoveFixup(T **iter) {

                /* Calculate sort range */
                T **head = m_queue;
                VP_ASSERT(head <= iter);
                const u32 p_count = static_cast<u32>(iter - head);
                constexpr const u32 count = m_count;
                VP_ASSERT(p_count < count);

                /* Calculate starting positions */
                const u32 start = (p_count << 1);
                const u32 start_l = start | 1;
                const u32 start_h = (start + 2) & 0xffff'fffe;

                T **high_pair = (start_h < count) ? (head + start_h) : nullptr;

                /* Ensure start l is in range */
                if (count <= start_l) { return iter; }

                /* Sort */
                T *low_pair = head[start_l];
                T *pivot_node = *iter;
                u32 l_i =  start_l;
                u32 h_i =  start_h;

                while (pivot_node->*KeyMemberPtr < low_pair->*KeyMemberPtr || (high_pair != nullptr && pivot_node->*KeyMemberPtr < (*high_pair)->*KeyMemberPtr)) {

                    /* Swap pair */
                    if (high_pair == nullptr || (*high_pair)->*KeyMemberPtr <= low_pair->*KeyMemberPtr) {
                        *iter = low_pair;
                        h_i = l_i;
                    } else {
                        *iter = *high_pair;
                    }

                    iter = head + h_i;
                    
                    /* Find next pairs */
                    l_i = (h_i << 1) | 1;
                    h_i = (h_i << 1) + 2;

                    if (count <= l_i) { break; }

                    high_pair = (h_i < count) ? (head + h_i) : nullptr;
                    low_pair = head[l_i];
                }

                *iter = pivot_node;

                return iter;
            }
        public:
            constexpr  FixedPriorityQueue() : m_count(), m_queue{} {/*...*/}
            constexpr ~FixedPriorityQueue() {/*...*/}
            
            constexpr void Insert(T *new_node) {

                /* Integrity check */
                VP_ASSERT(new_node != nullptr);
                VP_ASSERT(m_count < Size);

                /* Add new node */
                const u32 last_count = m_count;
                m_queue[last_count] = new_node;
                m_count = last_count + 1;

                /* Fixup queue */
                this->InsertFixup(m_queue + last_count);

                return;
            }

            constexpr ALWAYS_INLINE T *Peek() {
                return m_queue[0];
            }

            constexpr T *RemoveFront() {

                /* Intergrity check bounds */
                VP_ASSERT(0 < m_count);

                /* Remove element from the queue */
                const u32 next_count = m_count - 1;
                m_count = next_count;

                /* Fixup */
                T **head = m_queue;
                T *value = head[0];
                if (next_count != 0) {
                    head[0] = head[next_count];
                    this->RemoveFixup(head);
                }

                return value;
            }

            constexpr T **FindIterTo(T *node) {
                for (u32 i = 0; i < m_count; ++i) {
                    if (m_queue[i] == node) { return std::addressof(m_queue[i]); }
                }
                VP_ASSERT(false);
                return nullptr;
            }

            constexpr void Remove(T **iter) {

                /* Intergrity check bounds */
                VP_ASSERT(m_queue <= iter && iter < m_queue + m_count);

                /* Remove element from the queue */
                --m_count;

                /* No fixup if last removed node, or node at end of queue */
                if (m_count == 0 || (m_queue + m_count) == iter) { return; }

                /* Fixup */
                *iter = m_queue[m_count];
                T **insert_location = this->InsertFixup(iter);
                if (insert_location != iter) { return; } 

                this->RemoveFixup(iter);

                return;
            }

            constexpr void Clear() {
                m_count = 0;
            }

            constexpr ALWAYS_INLINE u32 GetUsedCount() const { return m_count; }
    };
}
