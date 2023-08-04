#pragma once

namespace vp::util {

	template<typename T>
	class RingBuffer {
		private:
			u32   m_current_offset;
			u32   m_count;
            u32   m_max_count;
			T   **m_array;
        public:
            constexpr ALWAYS_INLINE RingBuffer() : m_current_offset(0), m_count(0), m_max_count(0), m_array{} {/*...*/}
            constexpr ALWAYS_INLINE ~RingBuffer() {/*...*/}

            void Initialize(imem::IHeap *heap, u32 pointer_count, s32 alignment = 8) {
                
                /* Integrity checks */
                VP_ASSERT(pointer_count != 0);

                /* Allocate pointer array */
                m_array = new (heap, alignment) T*[pointer_count];
                VP_ASSERT(m_array != nullptr);

                /* Set state */
                m_max_count      = pointer_count;
                m_current_offset = 0;
                m_count          = 0;

                return;
            }

            void Finalize() {

                if (m_array != nullptr) {
                    delete [] m_array;
                }
                m_current_offset = 0;
                m_count          = 0;
                m_max_count      = 0;
            }

            constexpr ALWAYS_INLINE void Insert(T *pointer) {

                /* Check count */
                VP_ASSERT(m_max_count > m_count);

                /* Increment count */
                u32 count = m_count;
                m_count = m_count + 1;

                /* Calculate offset */
                u32 offset = m_current_offset + count;

                /* Find size offset */
                u32 size_offset = 0;
                if (m_max_count <= offset) {
                    size_offset = m_max_count;
                }

                /* Placeback pointer */
                m_array[offset - size_offset] = pointer;
            }

            constexpr ALWAYS_INLINE T *RemoveFront() {

                /* Check count */
                VP_ASSERT(m_count != 0);

                /* Set size offset for wraparound */
                u32 size_offset = 0;
                if (m_max_count <= m_current_offset) {
                    size_offset = m_max_count;
                }

                /* Get pointer */
                T *ret = m_array[m_current_offset - size_offset];

                /* Adjust offset */
                if (0 < m_count) {
                    m_current_offset = (m_current_offset + 1 < m_max_count) ? (m_current_offset + 1) : 0;
                    m_count          = m_count - 1;
                }

                return ret;
            }

            constexpr ALWAYS_INLINE u32 GetUsedCount() const { return m_count; }
	};
}
