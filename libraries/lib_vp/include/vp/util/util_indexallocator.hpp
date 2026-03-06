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
        requires (std::is_unsigned_v<T>)
    class IndexAllocator {
        public:
            static constexpr T cInvalidEntryIndex = static_cast<T>(0xffff'ffff);
            static constexpr u32 cMaxMaxIndexCount = Min(~T(0), 0xffff'ffff);
        private:
            u32 m_next_index;
            u32 m_max_count;
            T *m_handle_array;
        public:
            constexpr IndexAllocator() : m_next_index(), m_max_count(), m_handle_array() {/*...*/}
            constexpr ~IndexAllocator() {/*...*/}

            Result Initialize(imem::IHeap *heap, u32 max_index_count) {

                /* Integrity check size */
                RESULT_RETURN_UNLESS(0 < max_index_count, ResultRangeNG);
                RESULT_RETURN_UNLESS(max_index_count < cMaxMaxIndexCount, ResultRangeNG);

                /* Allocate array */
                T *array = new (heap, alignof(T)) T[max_index_count];
                RESULT_RETURN_UNLESS(array != nullptr, ResultNoMemory);

                /* Initialize allocator */
                m_handle_array = array;
                m_max_count = max_index_count;
                this->Clear();

                RESULT_RETURN_SUCCESS;
            }

            void Finalize() {
                if (m_handle_array != nullptr) {
                    delete [] m_handle_array;
                    m_handle_array = nullptr;
                    m_next_index = static_cast<T>(cInvalidEntryIndex);
                    m_max_count  = 0;
                }
                return;
            }

            static constexpr size_t GetRequiredMemorySize(u32 count) {
                return sizeof(T) * count;
            }

            constexpr void SetMemory(T *memory, u32 count) {
                m_handle_array = memory;
                m_max_count = count;
                this->Clear();
                return;
            }
        public:
            constexpr T Allocate() {

                /* Try to acquire an index */
                const u32 handle_value = m_next_index;
                if (handle_value == cInvalidEntryIndex) { return handle_value; }
                m_next_index = static_cast<u32>(m_handle_array[handle_value]);

                return handle_value;
            }

            constexpr void Free(T index) {

                /* Release an index */
                VP_ASSERT(index < m_max_count);
                m_handle_array[index] = static_cast<T>(m_next_index);
                m_next_index = index;

                return;
            }

            constexpr void Clear() {

                /* Reset next id */
                m_next_index = 0;
                /* Rebuild index array */
                const u32 max = m_max_count;
                if (max < 1) { return; }
                T *head = m_handle_array;
                T *iter = head;
                T *end = head + max - 1;
                while (iter != end) {
                    T *next = iter + 1;
                    *iter = next - head;
                    iter = next;
                }
                *iter = static_cast<T>(cInvalidEntryIndex);

                return;
            }
        public:
            constexpr u32 GetFreeCount() const {
                u32 n = m_next_index;
                u32 i = 0;
                const T *arr = m_handle_array;
                while (n != cInvalidEntryIndex) { n = static_cast<u32>(arr[n]); ++i; }
                return i;
            }
            constexpr u32 GetMaxCount() const { return m_max_count; }

            constexpr bool IsValid() const { return m_handle_array != nullptr; }
            constexpr bool IsEmpty() const { return m_next_index == cInvalidEntryIndex; }
    };
    
    template <typename T, size_t Count>
        requires (std::is_unsigned_v<T> && 0 < Count && Count < 0xffff'ffff)
    class FixedIndexAllocator {
        public:
            static constexpr T cInvalidEntryIndex = static_cast<T>(0xffff'ffff);
            static constexpr u32 cMaxMaxIndexCount = Min(~T(0), 0xffff'ffff);
        private:
            u32  m_next_index;
            T    m_handle_array[Count];
        public:
            constexpr  FixedIndexAllocator() : m_next_index(), m_handle_array() { this->Clear(); }
            constexpr ~FixedIndexAllocator() {/*...*/}

            constexpr T Allocate() {

                /* Try to acquire an index */
                const u32 handle_value = m_next_index;
                if (handle_value == cInvalidEntryIndex) { return handle_value; }
                m_next_index = static_cast<u32>(m_handle_array[handle_value]);

                return handle_value;
            }
            
            constexpr void Free(T index) {

                /* Release an index */
                m_handle_array[index] = static_cast<T>(m_next_index);
                m_next_index = index;

                return;
            }

            constexpr void Clear() {

                /* Clear allocator */
                m_next_index = 0;
                for (u32 i = 0; i < Count - 1; ++i) {
                    m_handle_array[i] = i + 1;
                }
                m_handle_array[Count - 1] = static_cast<T>(cInvalidEntryIndex);

                return;
            }
        public:
            constexpr u32 GetFreeCount() const {
                u32 n = m_next_index;
                u32 i = 0;
                const T *arr = m_handle_array;
                while (n != cInvalidEntryIndex) { n = static_cast<u32>(arr[n]); ++i; }
                return i;
            }
            static constexpr u32 GetMaxCount() { return Count; }

            constexpr bool IsEmpty() const { return m_next_index == cInvalidEntryIndex; }
    };
}
