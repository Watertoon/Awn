#pragma once

namespace vp::util {

    template <typename T>
        requires (sizeof(T) <= sizeof(u32))
    class AtomicIndexAllocator {
        public:
            static constexpr u32 cInvalidHandle = 0xffff'ffff;
        private:
            u32  m_next_index;
            u32  m_max_count;
            T   *m_handle_array;
        public:
            constexpr AtomicIndexAllocator() : m_next_index(), m_max_count(), m_handle_array() {/*...*/}
            constexpr ~AtomicIndexAllocator() {/*...*/}

            void Initialize(imem::IHeap *heap, u32 max_index_count) {

                /* Allocate array */
                m_handle_array = new (heap, alignof(T)) T[max_index_count];
                VP_ASSERT(m_handle_array != nullptr);

                /* Initialize allocator */
                m_next_index = 0;
                for (u32 i = 0; i < max_index_count - 1; ++i) {
                    m_handle_array[i] = i + 1;
                }
                m_handle_array[max_index_count - 1] = static_cast<T>(cInvalidHandle);

                /* Set max */
                m_max_count  = max_index_count;

                return;
            }

            void Finalize() {

                m_next_index = static_cast<T>(cInvalidHandle);
                m_max_count  = 0;
                if (m_handle_array != 0) {
                    delete [] m_handle_array;
                    m_handle_array = nullptr;
                }

                return;
            }

            void Clear() {

                /* Clear allocator */
                m_next_index = 0;
                for (u32 i = 0; i < m_max_count - 1; ++i) {
                    m_handle_array[i] = i + 1;
                }
                m_handle_array[m_max_count - 1] = static_cast<T>(cInvalidHandle);

                return;
            }

            ALWAYS_INLINE T Allocate() {

                /* Try to atomicly acquire an index */
                u32 handle_value      = 0;
                u32 last_handle_value = static_cast<u32>(m_next_index);
                while (last_handle_value != cInvalidHandle) {
                    handle_value      = static_cast<u32>(m_next_index);
                    last_handle_value = vp::util::InterlockedCompareExchange(std::addressof(m_next_index), handle_value, static_cast<u32>(m_handle_array[handle_value]));
                    if (last_handle_value == handle_value) { return static_cast<T>(cInvalidHandle); }
                }

                return static_cast<T>(cInvalidHandle);
            }

            ALWAYS_INLINE void Free(T index) {

                /* Atomicly release an index */
                u32 handle     = 0;
                u32 last_index = 0;
                do {
                    handle                = m_next_index;
                    m_handle_array[index] = handle;
                    last_index            = vp::util::InterlockedCompareExchange(std::addressof(m_next_index), handle, static_cast<u32>(index));
                } while (last_index != handle);

                return;
            }

            constexpr ALWAYS_INLINE u32 GetMaxCount() const { return m_max_count; }
    };

    template <typename T, size_t Count>
        requires (sizeof(T) <= sizeof(u32)) && (Count <= (~T(0)))
    class FixedAtomicIndexAllocator {
        public:
            static constexpr u32 cInvalidHandle = 0xffff'ffff;
        private:
            u32  m_next_index;
            T    m_handle_array[Count];
        public:
            constexpr  FixedAtomicIndexAllocator() : m_next_index(), m_handle_array() { this->Clear(); }
            constexpr ~FixedAtomicIndexAllocator() {/*...*/}

            constexpr void Clear() {

                /* Clear allocator */
                m_next_index = 0;
                for (u32 i = 0; i < Count - 1; ++i) {
                    m_handle_array[i] = i + 1;
                }
                m_handle_array[Count - 1] = static_cast<T>(cInvalidHandle);

                return;
            }

            ALWAYS_INLINE T Allocate() {

                /* Try to atomicly acquire an index */
                u32 handle_value      = 0;
                u32 last_handle_value = static_cast<u32>(m_next_index);
                while (last_handle_value != cInvalidHandle) {
                    handle_value      = static_cast<u32>(m_next_index);
                    last_handle_value = vp::util::InterlockedCompareExchange(std::addressof(m_next_index), handle_value, static_cast<u32>(m_handle_array[handle_value]));
                    if (last_handle_value == handle_value) { return static_cast<T>(cInvalidHandle); }
                }

                return static_cast<T>(cInvalidHandle);
            }

            ALWAYS_INLINE void Free(T index) {

                /* Atomicly release an index */
                u32 handle     = 0;
                u32 last_index = 0;
                do {
                    handle                = m_next_index;
                    m_handle_array[index] = handle;
                    last_index            = vp::util::InterlockedCompareExchange(std::addressof(m_next_index), handle, static_cast<u32>(index));
                } while (last_index != handle);

                return;
            }

            constexpr ALWAYS_INLINE u32 GetMaxCount() const { return Count; }
    };
}
