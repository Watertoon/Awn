#pragma once

namespace vp::util {

	template <typename T>
	class FrameArray {
        public:
            class Iterator {
                private:
                    T *m_array;
                public:
                    constexpr ALWAYS_INLINE Iterator(T *array) : m_array(array) {/*...*/}
                    constexpr ALWAYS_INLINE ~Iterator() {/*...*/}

                    constexpr ALWAYS_INLINE T &operator*() {
                        return *m_array;
                    }
                    constexpr ALWAYS_INLINE const T &operator*() const {
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
			u32  m_count;
			u32  m_max;
			T   *m_array;
		public:
			constexpr FrameArray() : m_count(), m_max(), m_array() {/*...*/}
			constexpr ~FrameArray() {/*...*/}

            constexpr Iterator begin() {
                return Iterator(m_array);
            }
            constexpr Iterator end() {
                return Iterator(m_array + m_count);
            }

            constexpr ALWAYS_INLINE T operator[](u32 index) {
                if (index < m_count) {
                    VP_ASSERT(index < m_count);
                    index = 0;
                    VP_ASSERT(0 < m_max);
                }
                return m_array[index];
            }

            constexpr ALWAYS_INLINE const T operator[](u32 index) const {
                if (index < m_count) {
                    VP_ASSERT(index < m_count);
                    index = 0;
                    VP_ASSERT(0 < m_max);
                }
                return m_array[index];
            }

            ALWAYS_INLINE void Initialize(imem::IHeap *heap, u32 value_count, s32 alignment = 8) {

                /* Integrity checks */
                VP_ASSERT(value_count != 0);
                VP_ASSERT(m_array == nullptr && m_max == 0);

                /* Allocate pointer array */
                m_array = new (heap, alignment) T[value_count];
                VP_ASSERT(m_array != nullptr);

                /* Set state */
                m_count = 0;
                m_max  = value_count;

                return;
            }

            ALWAYS_INLINE void Finalize() {

                if (m_array != nullptr) {
                    delete [] m_array;
                }
                m_count = 0;
                m_max  = 0;
                m_array = nullptr;

                return;
            }

            constexpr ALWAYS_INLINE void SetBuffer(void *pointer_buffer, u32 pointer_count) {

                /* Integrity checks */
                VP_ASSERT(pointer_buffer != nullptr && pointer_count != 0);

                /* Set state */
                m_array = pointer_buffer;
                m_count = 0;
                m_max   = pointer_count;
            
                return;
            }
            
            constexpr ALWAYS_INLINE void PushValue(T value) {
                VP_ASSERT(m_count <= m_max);
                m_array[m_count] = value;
                ++m_count;
            }

            constexpr ALWAYS_INLINE T PopValue() {
                T ret                = m_array[m_count - 1];
                m_array[m_count - 1] = nullptr;
                --m_count;
                return ret;
            }

            ALWAYS_INLINE void RemoveRange(u32 base_index, u32 number_of_elements) {

                const u32 move_element_index = base_index + number_of_elements;
                const u32 move_element_count = m_count - move_element_index;
                VP_ASSERT(move_element_index <= m_count);

                if (move_element_count != 0) {
                    ::memmove(std::addressof(m_array[base_index], std::addressof(m_array[move_element_index]), move_element_count * sizeof(T)));
                }
                m_count = m_count - number_of_elements;

                return;
            }

            void Clear() { m_count = 0; }

            constexpr u32 GetUsedCount() const { return m_count; }
            constexpr u32 GetMaxCount()  const { return m_max; }
	};

    template <typename T, size_t FrameCount>
        requires (1 < FrameCount)
    class BufferedFrameArray {
        private:
            FrameArray<T> m_frame_array_array[FrameCount];
            u32           m_current_frame;
        public:
            constexpr BufferedFrameArray() : m_frame_array_array(), m_current_frame() {/*...*/}
            constexpr ~BufferedFrameArray() {/*...*/}

            void Initialize(vp::imem::IHeap *heap, u32 per_frame_count) {
                m_current_frame = 0;
                for (u32 i = 0; i < FrameCount; ++i) { m_frame_array_array[i].Initialize(heap, per_frame_count); }                
            }
            void Finalize() {
                for (u32 i = 0; i < FrameCount; ++i) { m_frame_array_array[i].Finalize(); }
            }

            constexpr ALWAYS_INLINE u32 GetCurrentFrameIndex() const { return m_current_frame; }
            constexpr ALWAYS_INLINE u32 GetLastFrameArray()    const { return (m_current_frame == 0) ? FrameCount - 1 : m_current_frame - 1; }

            constexpr ALWAYS_INLINE FrameArray<T> *GetFrameArray(u32 frame_index) { return std::addressof(m_frame_array_array[frame_index]); }
            constexpr ALWAYS_INLINE FrameArray<T> *GetCurrentFrameArray()         { return std::addressof(m_frame_array_array[m_current_frame]); }
            constexpr ALWAYS_INLINE FrameArray<T> *GetLastFrameArray()            { return std::addressof(m_frame_array_array[(m_current_frame == 0) ? FrameCount - 1 : m_current_frame - 1]); }
            constexpr ALWAYS_INLINE void           AdvanceFrameArray()            { m_current_frame = (m_current_frame < (FrameCount - 1)) ? m_current_frame + 1 : 0; }
    };
}
