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
    
    template <class T, size_t Count>
        requires (sizeof(void*) <= sizeof(T)) && (Count != 0)
    class FixedObjectAllocator {
        private:
            struct FreeList {
                FreeList *next;
                char      _padding[sizeof(T) - sizeof(void*)];
            };
            static_assert(sizeof(T) == sizeof(FreeList) && alignof(T) == alignof(FreeList));
        private:
            FreeList     *m_free_list;
            union {
                T         m_object_array[Count];
                FreeList  m_free_node_array[Count];
            };
        public:
            constexpr ALWAYS_INLINE FixedObjectAllocator() : m_free_list(m_free_node_array), m_free_node_array{} {

                /* Construct a free list of objects */
                for (u32 i = 0; i < (Count - 1); ++i) {
                    /* Remember we are adding to a pointer */
                    m_free_node_array[i].next = m_free_node_array + i + 1;
                }
            }
            constexpr ALWAYS_INLINE ~FixedObjectAllocator() {/*...*/}

            template <typename ... Args>
            ALWAYS_INLINE T *Allocate(Args &&... args) {

                /* Fail on empty free list */
                if (m_free_list == nullptr) { return nullptr; }

                /* Pop a free object */
                T *allocation = reinterpret_cast<T*>(m_free_list);
                m_free_list = m_free_list->next;

                /* Initialize the object */
                std::construct_at(allocation, std::forward<Args>(args) ...);

                return allocation;
            }

            ALWAYS_INLINE void Free(T *allocated_object) {

                /* Integrity check parameter */
                VP_ASSERT(m_object_array <= allocated_object && allocated_object < m_object_array + Count);

                /* Destruct the object */
                std::destroy_at(allocated_object);

                /* Add it to the front of the free list */
                FreeList *new_node = reinterpret_cast<FreeList*>(allocated_object);
                new_node->next = m_free_list;
                m_free_list = new_node;
            }
    };
}
