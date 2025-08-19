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
    class ObjectAllocator {
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
            constexpr ALWAYS_INLINE ObjectAllocator() : m_free_list(), m_start_address() {/*...*/}
            constexpr ~ObjectAllocator() {/*...*/}

            bool Initialize(imem::IHeap *heap, u32 count) {

                /* Integrity checks */
                if (count < 1) { return false; }

                /* Allocate uninitialized array of objects */
                void *allocation = ::operator new(sizeof(T) * count, heap, alignof(T));
                if (allocation == nullptr) { return false; }

                /* Set free list and start address */
                m_start_address = allocation;
                this->Clear(count);

                return true;
            }

            void SetMemoryAndClear(T *memory, u32 count) {
                m_start_address = memory;
                this->Clear(count);
                return;
            }

            void Finalize() {
                if (m_start_address != nullptr) {
                    ::operator delete(m_start_address);
                    m_start_address = nullptr;
                    m_free_list     = nullptr;
                }
            }

            template <typename ... Args>
            T *Allocate(Args &&... args) {

                /* Fail on empty free list */
                if (m_free_list == nullptr) { return nullptr; }

                /* Pop a free object */
                T *allocation = reinterpret_cast<T*>(m_free_list);
                m_free_list = m_free_list->next;

                /* Initialize the object */
                std::construct_at(allocation, std::forward<Args>(args) ...);

                return allocation;
            }

            void Free(T *allocated_object) {

                /* Destruct the object */
                std::destroy_at(allocated_object);

                /* Add it to the front of the free list */
                FreeList *new_node = reinterpret_cast<FreeList*>(allocated_object);
                new_node->next = m_free_list;
                m_free_list = new_node;
            }

            void Clear(u32 count) {

                /* Construct object free list */
                if (count < 1) { return; }
                FreeList *list_array = reinterpret_cast<FreeList*>(m_start_address);
                m_free_list = list_array;
                for (u32 i = 0; i < (count - 1); ++i) {
                    /* Remember we are adding to a pointer */
                    list_array[i].next = list_array + i + 1;
                }
                list_array[count - 1].next = nullptr;
                return;
            }

            constexpr bool IsEmpty() const { return m_free_list == nullptr; }
            constexpr bool IsValid() const { return m_start_address != nullptr; }
    };
}
