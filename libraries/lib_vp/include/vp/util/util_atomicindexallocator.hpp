#pragma once

namespace vp::util {

    template <typename T>
        requires (sizeof(T) <= sizeof(u32)) && (std::is_integral_v<T>) && (std::is_unsigned_v<T>)
    class AtomicIndexAllocator {
        public:
            static constexpr T cInvalidEntryIndex = static_cast<T>(0xffff'ffff'ffff'ffff);
            static constexpr T cMaxSize = cInvalidEntryIndex;
        public:
            using NextIdType = vp::util::MinUIntType<(1ull << BITSIZEOF(T))+1>;
            static_assert(sizeof(NextIdType) == sizeof(T) * 2);
            static constexpr const NextIdType cAllMask = static_cast<NextIdType>(~0ull);
            static constexpr const NextIdType cTransactionAddend = static_cast<NextIdType>(1ull << BITSIZEOF(T));
            static constexpr const NextIdType cTransactionMask = static_cast<NextIdType>(cAllMask << BITSIZEOF(T));
            static constexpr const NextIdType cIdMask = static_cast<NextIdType>(~cTransactionMask);
        private:
            NextIdType m_next_id;
            u32 m_max_count;
            T *m_id_array;
        public:
            constexpr AtomicIndexAllocator() : m_next_id(cInvalidEntryIndex), m_max_count(), m_id_array() {/*...*/}
            constexpr ~AtomicIndexAllocator() {/*...*/}

            Result Initialize(imem::IHeap *heap, u32 max_index_count) {

                /* Integrity check size */
                RESULT_RETURN_UNLESS(max_index_count < cMaxSize, ResultRangeNG);
                RESULT_RETURN_UNLESS(0 < max_index_count, ResultRangeNG);

                /* Allocate array */
                T *handle_array = new (heap, alignof(T)) T[max_index_count];
                RESULT_RETURN_UNLESS(handle_array != nullptr, ResultNoMemory);

                /* Set max */
                m_max_count = max_index_count;
                m_id_array = handle_array;
                this->Clear();

                RESULT_RETURN_SUCCESS;
            }

            void Finalize() {
                if (m_id_array != 0) {
                    delete [] m_id_array;
                    m_id_array = nullptr;
                    m_next_id = cInvalidEntryIndex;
                    m_max_count  = 0;
                }
                return;
            }

            constexpr void SetMemory(T *array, u32 count) {
                m_next_id = cInvalidEntryIndex;
                m_max_count = count;
                m_id_array = array;
                return;
            }
        public:
            constexpr void Clear() {

                /* Clear allocator */
                m_next_id = 0;
                for (u32 i = 0; i < m_max_count - 1; ++i) {
                    m_id_array[i] = i + 1;
                }
                m_id_array[m_max_count - 1] = cInvalidEntryIndex;

                return;
            }

            constexpr T Allocate() {
                const T *id_array = m_id_array;
                for (;;) {
                    /* Ensure we load before transaction */
                    NextIdType prev_id = vp::util::InterlockedLoadAcquire(std::addressof(m_next_id));
                    /* Fail if we have exhausted ids */
                    const T acq_id = static_cast<T>(prev_id & cIdMask);
                    if (acq_id == cInvalidEntryIndex) { break; }
                    /* Read transaction */
                    const NextIdType next_free_id = static_cast<NextIdType>(id_array[acq_id]);
                    /* Ensure sequential transaction */
                    const NextIdType next = ((prev_id & cTransactionMask) + cTransactionAddend) | next_free_id;
                    const bool result = vp::util::InterlockedCompareExchangeWeak(static_cast<NextIdType*>(nullptr), std::addressof(m_next_id), next, prev_id);
                    if (result) { return acq_id; }
                }
                return cInvalidEntryIndex;
            }

            constexpr void Free(T index) {
                T *id_array = m_id_array;
                for (;;) {
                    /* Ensure we load before transaction */
                    NextIdType prev_id = vp::util::InterlockedLoadAcquire(std::addressof(m_next_id));
                    /* Write transaction */
                    const T acq_id = static_cast<T>(prev_id & cIdMask);
                    id_array[index] = acq_id;
                    /* Ensure sequential transaction */
                    const NextIdType next = ((prev_id & cTransactionMask) + cTransactionAddend) | static_cast<NextIdType>(index);
                    const bool result = vp::util::InterlockedCompareExchangeWeak(static_cast<NextIdType*>(nullptr), std::addressof(m_next_id), next, prev_id);
                    if (result) { return; }
                }
                return;
            }

            constexpr bool IsValid() const { return m_id_array != nullptr; }
            constexpr u32 GetMaxCount() const { return m_max_count; }
    };

    template <typename T, size_t Count>
        requires (sizeof(T) <= sizeof(u32)) && (Count < static_cast<T>(0xffff'ffff'ffff'ffff)) && (std::is_integral_v<T>) && (std::is_unsigned_v<T>)
    class FixedAtomicIndexAllocator {
        public:
            static constexpr T cInvalidEntryIndex = static_cast<T>(0xffff'ffff'ffff'ffff);
        public:
            using NextIdType = vp::util::MinUIntType<(1ull << BITSIZEOF(T))+1>;
            static_assert(sizeof(NextIdType) == sizeof(T) * 2);
            static constexpr const NextIdType cAllMask = static_cast<NextIdType>(~0ull);
            static constexpr const NextIdType cTransactionAddend = static_cast<NextIdType>(1ull << BITSIZEOF(T));
            static constexpr const NextIdType cTransactionMask = static_cast<NextIdType>(cAllMask << BITSIZEOF(T));
            static constexpr const NextIdType cIdMask = static_cast<NextIdType>(~cTransactionMask);
        private:
            NextIdType m_next_id;
            T m_id_array[Count];
        public:
            constexpr  FixedAtomicIndexAllocator() : m_next_id(), m_id_array() { this->Clear(); }
            constexpr ~FixedAtomicIndexAllocator() {/*...*/}

            constexpr void Clear() {

                /* Clear allocator */
                m_next_id = 0;
                for (u32 i = 0; i < Count - 1; ++i) {
                    m_id_array[i] = i + 1;
                }
                m_id_array[Count - 1] = static_cast<T>(cInvalidEntryIndex);

                return;
            }

            constexpr T Allocate() {
                const T *id_array = m_id_array;
                for (;;) {
                    /* Ensure we load before transaction */
                    NextIdType prev_id = vp::util::InterlockedLoadAcquire(std::addressof(m_next_id));
                    /* Fail if we have exhausted ids */
                    const T acq_id = static_cast<T>(prev_id & cIdMask);
                    if (acq_id == cInvalidEntryIndex) { break; }
                    /* Read transaction */
                    const NextIdType next_free_id = static_cast<NextIdType>(id_array[acq_id]);
                    /* Ensure sequential transaction */
                    const NextIdType next = ((prev_id & cTransactionMask) + cTransactionAddend) | next_free_id;
                    const bool result = vp::util::InterlockedCompareExchangeWeak(static_cast<NextIdType*>(nullptr), std::addressof(m_next_id), next, prev_id);
                    if (result) { return acq_id; }
                }
                return cInvalidEntryIndex;
            }

            constexpr void Free(T index) {
                T *id_array = m_id_array;
                for (;;) {
                    /* Ensure we load before transaction */
                    NextIdType prev_id = vp::util::InterlockedLoadAcquire(std::addressof(m_next_id));
                    /* Write transaction */
                    const T acq_id = static_cast<T>(prev_id & cIdMask);
                    id_array[index] = acq_id;
                    /* Ensure sequential transaction */
                    const NextIdType next = ((prev_id & cTransactionMask) + cTransactionAddend) | static_cast<NextIdType>(index);
                    const bool result = vp::util::InterlockedCompareExchangeWeak(static_cast<NextIdType*>(nullptr), std::addressof(m_next_id), next, prev_id);
                    if (result) { return; }
                }
                return;
            }

            constexpr u32 GetMaxCount() const { return Count; }
    };
}
