#pragma once

namespace vp::util {

    template <typename T>
    class HeapArray {
        private:
            T   *m_object_array;
            u32  m_array_count;
        public:
            constexpr ALWAYS_INLINE HeapArray() : m_object_array(nullptr), m_array_count(0) {/*...*/}
            constexpr ALWAYS_INLINE ~HeapArray() {/*...*/}

            ALWAYS_INLINE bool Initialize(imem::IHeap *alloc_heap, u32 element_count) {

                /* Fail on invalid element count */
                if (element_count == 0) { return false; }

                /* Allocate an array of objects */
                m_object_array = reinterpret_cast<T*>(::operator new(sizeof(T) * element_count, alloc_heap, alignof(T)));

                /* Fail on nullptr */
                if (m_object_array == nullptr) { return false; }

                /* Set count */
                m_array_count = element_count;

                /* Initialize every object */
                for (u32 i = 0; i < m_array_count; ++i) {
                    std::construct_at(std::addressof(m_object_array[i]));
                }

                return true;
            }

            ALWAYS_INLINE void Finalize() {

                /* Bail if there's no buffer */
                if (m_object_array == nullptr) { return; }

                /* Destruct every object */
                for (u32 i = 0; i < m_array_count; ++i) {
                    std::destroy_at(std::addressof(m_object_array[i]));
                }

                /* Free buffer */
                if (m_object_array != nullptr) {
                    ::operator delete(m_object_array);
                }

                /* Clear state */
                m_array_count  = 0;
                m_object_array = nullptr;
            }

            constexpr ALWAYS_INLINE T &operator[](u32 index) {
                VP_ASSERT(m_array_count > index);
                return m_object_array[index];
            }

            constexpr ALWAYS_INLINE T &operator[](u32 index) const {
                VP_ASSERT(m_array_count > index);
                return m_object_array[index];
            }

            constexpr ALWAYS_INLINE u32 GetCount() const { return m_array_count; }
    };
}
