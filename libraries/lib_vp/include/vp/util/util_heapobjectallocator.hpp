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
    
    template <class T>
        requires (sizeof(void*) <= sizeof(T))
    class HeapObjectAllocator {
        private:
            struct FreeList {
                FreeList *next;
                char      _padding[sizeof(T) - sizeof(void*)];
            };
            static_assert(sizeof(T) == sizeof(FreeList) && alignof(T) == alignof(FreeList));
        private:
            FreeList *m_free_list;
            void     *m_start_address;
        public:
            constexpr ALWAYS_INLINE HeapObjectAllocator() : m_free_list(nullptr), m_start_address(nullptr) {/*...*/}
            constexpr ~HeapObjectAllocator() {/*...*/}
            
            ALWAYS_INLINE bool Initialize(imem::IHeap *alloc_heap, u32 element_count) {
                
                /* Fail if we are passed no elements */
                if (element_count == 0) { return false; }

                /* Allocate uninitialized array of objects */
                void *allocation = ::operator new(sizeof(T) * element_count, alloc_heap, alignof(T));
                FreeList *list_array = reinterpret_cast<FreeList*>(allocation);

                /* Fail on failed allocation */
                if (allocation == nullptr) { return false; }

                /* Set free list and start address */
                m_start_address = allocation;
                m_free_list     = list_array;

                /* Construct object free list */
                for (u32 i = 0; i < (element_count - 1); ++i) {
                    /* Remember we are adding to a pointer */
                    list_array[i].next = list_array + i + 1;
                }

                return true;
            }

            ALWAYS_INLINE bool Initialize(T *preallocted_buffer, u32 element_count) {
                
                /* Ensure valid arguments */
                if (element_count == 0 || preallocted_buffer == nullptr) { return false; }

                /* Cast buffer to free list pointer */
                FreeList *list_array = reinterpret_cast<FreeList*>(preallocted_buffer);

                /* Set free list and start address */
                m_start_address = preallocted_buffer;
                m_free_list     = list_array;

                /* Construct object free list */
                for (u32 i = 0; i < (element_count - 1); ++i) {
                    /* Remember we are adding to a pointer */
                    list_array[i].next = list_array + i + 1;
                }

                return true;
            }

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

                /* Destruct the object */
                std::destroy_at(allocated_object);

                /* Add it to the front of the free list */
                FreeList *new_node = reinterpret_cast<FreeList*>(allocated_object);
                new_node->next = m_free_list;
                m_free_list = new_node;
            }
    };
}
