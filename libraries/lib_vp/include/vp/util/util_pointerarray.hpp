#pragma once

namespace vp::util {

    template <typename T>
    class PointerArray {
        private:
            u32   m_max_pointers;
            u32   m_used_pointers;
            T   **m_pointer_array;
        public:
            constexpr ALWAYS_INLINE PointerArray() : m_max_pointers(0), m_used_pointers(0), m_pointer_array(nullptr) {/*...*/}

            constexpr ALWAYS_INLINE T *operator[](u32 index) {
                VP_ASSERT(index < m_used_pointers);
                return m_pointer_array[index];
            }

            constexpr ALWAYS_INLINE const T *operator[](u32 index) const {
                VP_ASSERT(index < m_used_pointers);
                return m_pointer_array[index];
            }

            ALWAYS_INLINE void Initialize(imem::IHeap *heap, u32 pointer_count, s32 alignment = 8) {

                VP_ASSERT(pointer_count != 0);

                m_pointer_array = new (heap, alignment) T*[pointer_count];
                VP_ASSERT(m_pointer_array != nullptr);

                m_used_pointers = 0;
                m_max_pointers  = pointer_count;
            }

            ALWAYS_INLINE void Finalize() {
                if (m_pointer_array != nullptr) {
                    delete[] m_pointer_array;
                    m_used_pointers = 0;
                    m_max_pointers  = 0;
                    m_pointer_array = nullptr;
                }
            }

            constexpr ALWAYS_INLINE void SetBuffer(void *pointer_buffer, u32 pointer_count) {
                VP_ASSERT(pointer_buffer != nullptr && pointer_count != 0);
                
                m_pointer_array = pointer_buffer;
                m_used_pointers = 0;
                m_max_pointers  = pointer_count;
            }
            
            constexpr ALWAYS_INLINE void PushPointer(T *pointer) {
                VP_ASSERT(m_used_pointers <= m_max_pointers);
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
                m_used_pointers -= number_of_elements;
            }
    };
}
