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

    template <typename T, auto KeyMemberPtr, const size_t Size>
    class FixedPriorityQueue {
        public:
            using KeyType = vp::util::MemberType<KeyMemberPtr>;
        public:
            u32  m_count;
            T   *m_queue[Size];
        private:
            T **InsertFixup(T **pivot) {

                /* Calculate sort range */
                const ptrdiff_t extent  = reinterpret_cast<ptrdiff_t>(pivot) - reinterpret_cast<ptrdiff_t>(m_queue);
                const u32 p_count = (extent >> 3);

                /* Integrity check */
                VP_ASSERT(-1 < extent && p_count < m_count);
                if (p_count == 0) { return pivot; }

                /* Sort */
                T    *pivot_node  = *pivot;
                T   **low_pair    =  pivot;
                T   **high_pair   =  m_queue + ((p_count + 1) >> 1) - 1;
                u32   i           =  p_count;

                while ((*high_pair)->*KeyMemberPtr < pivot_node->*KeyMemberPtr) {

                    /* Swap */
                    *low_pair = *high_pair;

                    /* Adjust pairs */
                    const u32 next_low = ((i + 1) >> 1) - 1;
                    low_pair           = m_queue + next_low;
                    if (next_low == 0) { break; }

                    const u32 next_high = ((i + 1) >> 2) - 1;
                    high_pair           = m_queue + next_high;
                    i                   = next_low;
                }

                *low_pair = pivot_node;

                return low_pair;
            }

            T **RemoveFixup(T **iter) {

                /* Calculate sort range */
                const ptrdiff_t extent  = reinterpret_cast<ptrdiff_t>(iter) - reinterpret_cast<ptrdiff_t>(m_queue);
                const u32       p_count = (extent >> 3);

                /* Integrity check iter */
                VP_ASSERT(-1 < extent && p_count < m_count);

                /* Calculate starting positions */
                const u32 start   = (extent >> 2);
                const u32 start_l = start | 1;
                const u32 start_h = (start + 2) & 0xffff'fffe;

                T **high_pair = (start_h < m_count) ? (m_queue + start_h) : nullptr;

                /* Ensure start l is in range */
                if (m_count <= start_l) { return iter; }

                /* Sort */
                T   *low_pair   =  m_queue[start_l];
                T   *pivot_node = *iter;
                u32  l_i        =  start_l;
                u32  h_i        =  start_h;

                while (pivot_node->*KeyMemberPtr < low_pair->*KeyMemberPtr || (high_pair != nullptr && pivot_node->*KeyMemberPtr < (*high_pair)->*KeyMemberPtr)) {

                    /* Swap pair */
                    if (high_pair == nullptr || (*high_pair)->*KeyMemberPtr <= low_pair->*KeyMemberPtr) {
                        *iter = low_pair;
                        h_i   = l_i;
                    } else {
                        *iter = *high_pair;
                    }

                    iter = m_queue + h_i;
                    
                    /* Find next pairs */
                    l_i = (h_i << 1) | 1;
                    h_i = (h_i << 1) + 2;

                    if (m_count <= l_i) { break; }

                    high_pair = (h_i < m_count) ? (m_queue + h_i) : nullptr;
                    low_pair  = m_queue[l_i];
                }

                *iter = pivot_node;

                return iter;
            }
        public:
            constexpr  FixedPriorityQueue() : m_count(), m_queue{} {/*...*/}
            constexpr ~FixedPriorityQueue() {/*...*/}
            
            void Insert(T *new_node) {

                /* Integrity check */
                VP_ASSERT(m_count < Size && new_node != nullptr);

                /* Add new node */
                m_queue[m_count] = new_node;

                /* Increment count */
                const u32 last_count = m_count;
                ++m_count;

                /* Fixup queue */
                this->InsertFixup(m_queue + last_count);

                return;
            }

            constexpr ALWAYS_INLINE T *Peek() {
                return m_queue[0];
            }

            T *RemoveFront() {

                /* Intergrity check bounds */
                VP_ASSERT(0 < m_count);

                /* Remove element from the queue */
                --m_count;

                /* Fixup */
                T  *out_node = m_queue[0];

                if (m_count != 0) {
                    T **iter = m_queue;
                    *iter    = m_queue[m_count];
                    this->RemoveFixup(iter);
                }

                return out_node;
            }

            T **FindIterTo(T *node) {
                for (u32 i = 0; i < m_count; ++i) {
                    if (m_queue[i] == node) { return std::addressof(m_queue[i]); }
                }
                VP_ASSERT(false);
                return nullptr;
            }

            void Remove(T **iter) {

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

            void Clear() {
                m_count = 0;
            }

            constexpr ALWAYS_INLINE u32 GetUsedCount() const { return m_count; }
    };
}
