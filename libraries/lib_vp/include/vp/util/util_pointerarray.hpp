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
            u32   m_max_pointers;
            u32   m_used_pointers;
            T   **m_pointer_array;
        public:
            constexpr ALWAYS_INLINE PointerArray() : m_max_pointers(0), m_used_pointers(0), m_pointer_array(nullptr) {/*...*/}
            constexpr ~PointerArray() {/*...*/}

            constexpr Iterator begin() {
                return Iterator(m_pointer_array);
            }
            constexpr Iterator end() {
                return Iterator(m_pointer_array + m_used_pointers);
            }

            constexpr ALWAYS_INLINE T *operator[](u32 index) {
                if (index < m_used_pointers) {
                    VP_ASSERT(index < m_used_pointers);
                    index = 0;
                    VP_ASSERT(0 < m_max_pointers);
                }
                return m_pointer_array[index];
            }

            constexpr ALWAYS_INLINE const T *operator[](u32 index) const {
                if (index < m_used_pointers) {
                    VP_ASSERT(index < m_used_pointers);
                    index = 0;
                    VP_ASSERT(0 < m_max_pointers);
                }
                return m_pointer_array[index];
            }

            ALWAYS_INLINE void Initialize(imem::IHeap *heap, u32 pointer_count, s32 alignment = 8) {

                /* Integrity checks */
                if (pointer_count == 0) { return; }
                VP_ASSERT(m_pointer_array == nullptr && m_max_pointers == 0);

                /* Allocate pointer array */
                m_pointer_array = new (heap, alignment) T*[pointer_count];
                VP_ASSERT(m_pointer_array != nullptr);

                /* Set state */
                m_used_pointers = 0;
                m_max_pointers  = pointer_count;

                return;
            }

            ALWAYS_INLINE void Finalize() {

                if (m_pointer_array != nullptr) {
                    delete [] m_pointer_array;
                }
                m_used_pointers = 0;
                m_max_pointers  = 0;
                m_pointer_array = nullptr;

                return;
            }

            constexpr ALWAYS_INLINE void SetBuffer(void *pointer_buffer, u32 pointer_count) {

                /* Integrity checks */
                VP_ASSERT(pointer_buffer != nullptr && pointer_count != 0);

                /* Set state */
                m_pointer_array = pointer_buffer;
                m_used_pointers = 0;
                m_max_pointers  = pointer_count;
            
                return;
            }
            
            constexpr ALWAYS_INLINE void PushPointer(T *pointer) {
                VP_ASSERT(m_used_pointers < m_max_pointers);
                m_pointer_array[m_used_pointers] = pointer;
                ++m_used_pointers;
            }

            constexpr ALWAYS_INLINE T *PopPointer() {
                T *ret = m_pointer_array[m_used_pointers - 1];
                m_pointer_array[m_used_pointers - 1] = nullptr;
                --m_used_pointers;
                return ret;
            }

            ALWAYS_INLINE void RemoveRange(u32 base_index, u32 number_of_elements) {

                const u32 move_element_index = base_index + number_of_elements;
                const u32 move_element_count = m_used_pointers - move_element_index;
                VP_ASSERT(move_element_index <= m_used_pointers);

                if (move_element_count != 0) {
                    ::memmove(std::addressof(m_pointer_array[base_index], std::addressof(m_pointer_array[move_element_index]), move_element_count * sizeof(T**)));
                }
                m_used_pointers = m_used_pointers - number_of_elements;

                return;
            }

            void Clear() { m_used_pointers = 0; }

            constexpr u32 GetUsedCount() const { return m_used_pointers; }
            constexpr u32 GetMaxCount()  const { return m_max_pointers; }
    };
}
