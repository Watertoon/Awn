#pragma once

namespace vp::util {

	template<typename T, size_t Size>
	class FixedRingBuffer {
		private:
			u32  m_current_offset;
			u32  m_count;
			T   *m_array[Size];
        public:
            constexpr ALWAYS_INLINE FixedRingBuffer() : m_current_offset(0), m_count(0), m_array{} {/*...*/}
            constexpr ~FixedRingBuffer() {/*...*/}

            constexpr ALWAYS_INLINE void PushBack(T *pointer) {

                /* Check count */
                VP_ASSERT(Size > m_count);

                /* Increment count */
                u32 count = m_count;
                m_count = m_count + 1;

                /* Calculate offset */
                u32 offset = m_current_offset + count;

                /* Find size offset */
                u32 size_offset = 0;
                if (Size <= offset) {
                    size_offset = Size;
                }

                /* Placeback pointer */
                m_array[offset - size_offset] = pointer;
            }

            constexpr ALWAYS_INLINE T *PopFront() {

                /* Check count */
                VP_ASSERT(m_count != 0);

                /* Set size offset for wraparound */
                u32 size_offset = 0;
                if (Size <= m_current_offset) {
                    size_offset = Size;
                }

                /* Get pointer */
                T *ret = m_array[m_current_offset - size_offset];

                /* Adjust offset */
                if (0 < m_count) {
                    m_current_offset = (m_current_offset + 1 < Size) ? (m_current_offset + 1) : 0;
                    m_count          = m_count - 1;
                }

                return ret;
            }
	};
}
