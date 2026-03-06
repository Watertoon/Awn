#pragma once

namespace vp::util {

	template <typename T>
	class FrameArray {
        public:
            class Iterator {
                private:
                    T *m_array;
                public:
                    constexpr Iterator(T *array) : m_array(array) {/*...*/}
                    constexpr ~Iterator() {/*...*/}

                    constexpr T &operator*() {
                        return *m_array;
                    }
                    constexpr const T &operator*() const {
                        return *m_array;
                    }

                    constexpr bool operator==(const Iterator &rhs) const {
                        return m_array == rhs.m_array;
                    }

                    constexpr bool operator!=(const Iterator &rhs) const {
                        return m_array != rhs.m_array;
                    }

                    constexpr Iterator &operator++() {
                        ++m_array;
                        return *this;
                    }
                    constexpr Iterator &operator++([[maybe_unused]]int) {
                        ++m_array;
                        return *this;
                    }
                    constexpr Iterator &operator--() {
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

            Result Initialize(imem::IHeap *heap, u32 count, s32 alignment = 8) {

                /* Integrity checks */
                RESULT_RETURN_UNLESS(count != 0, ResultRangeNG);

                /* Allocate pointer array */
                T *array = new (heap, alignment) T[count];
                RESULT_RETURN_UNLESS(array != nullptr, ResultNoMemory);

                /* Set state */
                m_array = array;
                m_count = 0;
                m_max = count;

                RESULT_RETURN_SUCCESS;
            }

            void Finalize() {
                if (m_array != nullptr) {
                    delete [] m_array;
                    m_array = nullptr;
                }
                m_count = 0;
                m_max  = 0;
                return;
            }

            static constexpr size_t GetRequiredMemorySize(u32 count) {
                return sizeof(T) * count;
            }

            constexpr void SetMemory(T *array, u32 count) {
                m_array = array;
                m_count = 0;
                m_max   = count;
                return;
            }
        public:
            constexpr Iterator begin() {
                return Iterator(m_array);
            }
            constexpr Iterator end() {
                return Iterator(m_array + m_count);
            }

            constexpr T operator[](u32 index) {
                if (index <= m_count) {
                    VP_ASSERT(index <= m_count);
                    index = 0;
                    VP_ASSERT(0 < m_max);
                }
                return m_array[index];
            }

            constexpr const T operator[](u32 index) const {
                if (index <= m_count) {
                    VP_ASSERT(index <= m_count);
                    index = 0;
                    VP_ASSERT(0 < m_max);
                }
                return m_array[index];
            }

            template <typename ... Args>
            T *Allocate(Args &&... args) {

                /* Increment count */
                const u32 count = m_count;
                VP_ASSERT(count < m_max);
                m_count = count + 1;

                /* Initialize a new value in place */
                T *next = std::addressof(m_array[count]);
                std::construct_at(next, std::forward<Args>(args) ...);

                return next;
            }

            constexpr void PushValue(T value) {
                VP_ASSERT(m_count <= m_max);
                m_array[m_count] = value;
                ++m_count;
            }

            constexpr T PopValue() {
                T ret = m_array[m_count - 1];
                m_array[m_count - 1] = nullptr;
                --m_count;
                return ret;
            }

            void RemoveRange(u32 base_index, u32 number_of_elements) {

                const u32 move_element_index = base_index + number_of_elements;
                const u32 move_element_count = m_count - move_element_index;
                VP_ASSERT(move_element_index <= m_count);

                if (move_element_count != 0) {
                    ::memmove(std::addressof(m_array[base_index]), std::addressof(m_array[move_element_index]), move_element_count * sizeof(T));
                }
                m_count = m_count - number_of_elements;

                return;
            }

            void Clear() { m_count = 0; }
        public:
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
