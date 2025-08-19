#pragma once

namespace vp::util {

    template <typename T>
        requires (sizeof(T) <= sizeof(u64)) && (std::is_integral_v<T>)
    class AtomicIndexAllocator {
        public:
            static constexpr T cInvalidHandle = static_cast<T>(0xffff'ffff'ffff'ffff);
            static constexpr T cInvalidEntryIndex = static_cast<T>(0xffff'ffff'ffff'ffff);
            static constexpr T cMaxSize = cInvalidHandle;
        private:
            T m_next_index;
            T m_max_count;
            T *m_handle_array;
        public:
            constexpr AtomicIndexAllocator() : m_next_index(), m_max_count(), m_handle_array() {/*...*/}
            constexpr ~AtomicIndexAllocator() {/*...*/}

            void Initialize(imem::IHeap *heap, u32 max_index_count) {

                /* Integrity check size */
                VP_ASSERT(max_index_count < cMaxSize);
                VP_ASSERT(0 < max_index_count);

                /* Allocate array */
                m_handle_array = new (heap, alignof(T)) T[max_index_count];
                VP_ASSERT(m_handle_array != nullptr);

                /* Initialize allocator */
                m_next_index = 0;
                for (u32 i = 0; i < max_index_count - 1; ++i) {
                    m_handle_array[i] = i + 1;
                }
                m_handle_array[max_index_count - 1] = cInvalidHandle;

                /* Set max */
                m_max_count  = max_index_count;

                return;
            }

            void Finalize() {

                m_next_index = cInvalidHandle;
                m_max_count  = 0;
                if (m_handle_array != 0) {
                    delete [] m_handle_array;
                    m_handle_array = nullptr;
                }

                return;
            }

            constexpr void Clear() {

                /* Clear allocator */
                m_next_index = 0;
                for (u32 i = 0; i < m_max_count - 1; ++i) {
                    m_handle_array[i] = i + 1;
                }
                m_handle_array[m_max_count - 1] = cInvalidHandle;

                return;
            }

            constexpr T Allocate() {

                /* Try to atomicly acquire an index */
                T handle_value = InterlockedLoadAcquire(std::addressof(m_next_index));
                for (;;) {
                    if (handle_value == cInvalidHandle) { break; }
                    const bool result = InterlockedCompareExchange(std::addressof(handle_value), std::addressof(m_next_index), m_handle_array[handle_value], handle_value);
                    if (result) { return handle_value; }
                }

                return cInvalidHandle;
            }

            constexpr void Free(T index) {

                /* Atomicly release an index */
                T handle = InterlockedLoadAcquire(std::addressof(m_next_index));
                bool result;
                do {
                    m_handle_array[index] = handle;
                    result = InterlockedCompareExchange(std::addressof(handle), std::addressof(m_next_index), index, handle);
                } while (result == false);

                return;
            }

            constexpr ALWAYS_INLINE u32 GetMaxCount() const { return m_max_count; }
    };

    template <typename T, size_t Count>
        requires (sizeof(T) <= sizeof(u64)) && (Count < static_cast<T>(0xffff'ffff'ffff'ffff)) && (std::is_integral_v<T>)
    class FixedAtomicIndexAllocator {
        public:
            static constexpr T cInvalidHandle = static_cast<T>(0xffff'ffff'ffff'ffff);
            static constexpr T cInvalidEntryIndex = static_cast<T>(0xffff'ffff'ffff'ffff);
        private:
            T m_next_index;
            T m_handle_array[Count];
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

            constexpr ALWAYS_INLINE T Allocate() {

                /* Try to atomicly acquire an index */
                T handle_value = InterlockedLoadAcquire(std::addressof(m_next_index));
                for (;;) {
                    if (handle_value == static_cast<T>(cInvalidHandle)) { break; }
                    const bool result = InterlockedCompareExchange(std::addressof(handle_value), std::addressof(m_next_index), static_cast<u32>(m_handle_array[handle_value]), handle_value);
                    if (result == true) { return static_cast<T>(handle_value); }
                }

                return static_cast<T>(cInvalidHandle);
            }

            constexpr ALWAYS_INLINE void Free(T index) {

                /* Atomicly release an index */
                T handle = InterlockedLoadAcquire(std::addressof(m_next_index));
                bool result;
                do {
                    m_handle_array[index] = handle;
                    result = InterlockedCompareExchange(std::addressof(handle), std::addressof(m_next_index), index, handle);
                } while (result == false);

                return;
            }

            constexpr ALWAYS_INLINE u32 GetMaxCount() const { return Count; }
    };
}
