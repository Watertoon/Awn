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
    class PointerArrayAllocator {
        private:
            static constexpr size_t cPaddingSize = (sizeof(T) < sizeof(size_t)) ? sizeof(size_t) : (sizeof(T) - sizeof(void*)) & ~(alignof(T) - 1);
            struct FreeList {
                FreeList *next;
                char      _padding[cPaddingSize];
            };
            static_assert(sizeof(T) <= sizeof(FreeList) && alignof(T) <= alignof(FreeList));
        private:
            FreeList  *m_free_list;
            T        **m_pointer_array;
            u32        m_used_count;
            u32        m_max;
        public:
            constexpr ALWAYS_INLINE PointerArrayAllocator() : m_free_list(), m_pointer_array(), m_used_count(), m_max() {/*...*/}
            constexpr ~PointerArrayAllocator() {/*...*/}
            
            constexpr ALWAYS_INLINE T *operator[](u32 index) {
                VP_ASSERT(index < m_used_count);
                return m_pointer_array[index];
            }

            constexpr ALWAYS_INLINE const T *operator[](u32 index) const {
                VP_ASSERT(index < m_used_count);
                return m_pointer_array[index];
            }
            
            ALWAYS_INLINE void Initialize(imem::IHeap *heap, u32 array_size) {

                VP_ASSERT(array_size != 0);

                /* Allocate memory for both arrays */
                uintptr_t memory = reinterpret_cast<uintptr_t>(::operator new((sizeof(FreeList) + sizeof(T**)) * array_size, heap, alignof(T)));
                VP_ASSERT(memory != 0);

                /* Split memory */
                m_pointer_array = reinterpret_cast<T**>(memory);
                m_free_list = reinterpret_cast<FreeList*>(reinterpret_cast<uintptr_t>(memory) + sizeof(T**) * array_size);

                /* Construct a free list of objects */
                for (u32 i = 0; i < (array_size - 1); ++i) {
                    /* Remember we are adding to a pointer */
                    m_free_list[i].next = m_free_list + i + 1;
                    m_pointer_array[i]  = nullptr;
                }
                m_pointer_array[array_size - 1]  = nullptr;

                m_max = array_size;
            }
            
            ALWAYS_INLINE void Finalize() {
                this->Clear();
                if (m_pointer_array != nullptr) {
                    ::operator delete(reinterpret_cast<void*>(m_pointer_array));
                }
                m_pointer_array = nullptr;
            }

            ALWAYS_INLINE T *Allocate() {

                /* Fail on empty free list */
                if (m_free_list == nullptr) { return nullptr; }

                /* Pop a free object */
                T *allocation = reinterpret_cast<T*>(m_free_list);
                m_free_list = m_free_list->next;

                /* Initialize the object */
                std::construct_at(allocation);

                /* Push to pointer array */
                this->PushPointer(allocation);
                
                return allocation;
            }

            ALWAYS_INLINE void RemoveLast() {

                if (m_used_count == 0) { return; }

                /* Delete the last pointer */
                const u32 new_count = m_used_count - 1;
                this->Free(m_pointer_array[new_count]);
                m_pointer_array[new_count] = nullptr;
                m_used_count = new_count;
            }

            ALWAYS_INLINE void Clear() {
                for (u32 i = 0; i < m_used_count; ++i) {
                    this->Free(m_pointer_array[i]);
                }
                m_used_count = 0;
            }

            constexpr ALWAYS_INLINE u32 GetUsedCount() const { return m_used_count; }
            constexpr ALWAYS_INLINE u32 GetMaxCount()  const { return m_max; }
        private:
            ALWAYS_INLINE void Free(T *allocated_object) {

                /* Destruct the object */
                std::destroy_at(allocated_object);

                /* Add it to the front of the free list */
                FreeList *new_node = reinterpret_cast<FreeList*>(allocated_object);
                new_node->next = m_free_list;
                m_free_list = new_node;
            }
            
            constexpr ALWAYS_INLINE void PushPointer(T *pointer) {
                VP_ASSERT(m_used_count <= m_max);
                m_pointer_array[m_used_count] = pointer;
                ++m_used_count;
            }
    };
}
