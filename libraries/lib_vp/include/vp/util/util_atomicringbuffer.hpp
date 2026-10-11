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

	template<typename T, bool IsCancellable = true>
        requires (sizeof(T) == sizeof(u64))
	class AtomicRingBuffer {
		public:
            static constexpr const u64 cCancelValue = 0xffff'ffff'ffff'ffff;
		public:
            class Iterator {
                public:
                    u32 m_offset;
                    T m_value;
                    const AtomicRingBuffer *m_rb;
                public:
                    constexpr Iterator(u32 offset, AtomicRingBuffer *rb) : m_offset(offset), m_rb(rb) {/*...*/}
                    constexpr ~Iterator() {/*...*/}

                    constexpr bool operator!=(const Iterator &rhs) const {
                        return m_offset < rhs.m_rb->GetCounter();
                    }
                    constexpr T operator*() const {
                        return m_value;
                    }
                    constexpr Iterator &operator++() {
                         while (m_offset < m_rb->GetCounter()) {
                            m_value = m_rb->GetValue(m_offset++);
                            if (m_value != 0 && (IsCancellable && reinterpret_cast<u64>(m_value) != cCancelValue)) { break; }
                        }
                        return *this;
                    }
            };
		private:
            u32  m_max_count;
			u32  m_offset;
			u32  m_count;
			u64 *m_array;
        public:
            constexpr AtomicRingBuffer() : m_max_count(), m_offset(), m_count(), m_array{} {/*...*/}
            constexpr ~AtomicRingBuffer() {/*...*/}

            Result Initialize(imem::IHeap *heap, u32 queue_count, s32 alignment = 8) {

                /* Integrity checks */
                RESULT_RETURN_UNLESS(queue_count != 0, ResultRangeNG);
                RESULT_RETURN_UNLESS(queue_count <= 0x8000'0000, ResultRangeNG);

                /* Align up count to nearest power of 2 */
                const u32 count = vp::util::AlignPow2(queue_count);

                /* Allocate pointer array */
                u64 *array = new (heap, alignment) u64[count];
                RESULT_RETURN_IF(array == nullptr, ResultNoMemory);
                std::memset(array, 0, sizeof(u64) * count);

                /* Set state */
                m_array = array;
                m_max_count = count;
                m_offset = 0;
                m_count = 0;

                RESULT_RETURN_SUCCESS;
            }

            void Finalize() {
                if (m_array != nullptr) {
                    delete [] m_array;
                    m_array = nullptr;
                    m_max_count = 0;
                    m_count     = 0;
                    m_offset    = 0;
                }
            }

            constexpr void SetMemory(T *array, u32 count) {
                m_array = array;
                m_max_count = count;
                m_offset = 0;
                m_count = 0;
            }
        public:
            constexpr Iterator begin() {
                return Iterator(m_offset,this);
            }
            constexpr Iterator end() {
                return Iterator(0,this);
            }

            bool PushBack(T value) {

                /* Integrity checks */
                VP_ASSERT(value != 0);
                VP_ASSERT(!IsCancellable || reinterpret_cast<u64>(value) != cCancelValue);

                /* Reserve space for push back loop */
                const u32 max = m_max_count;
                const u32 max_mask = (max - 1);
                u32 prev_count = vp::util::InterlockedLoadAcquire(std::addressof(m_count));
                for (;;) {
                    /* Fail if out of space */
                    const u32 offset = vp::util::InterlockedLoadAcquire(std::addressof(m_offset));
                    if (static_cast<s32>(max) <= static_cast<s32>(prev_count - offset)) { return false; }

                    /* Reserve space */
                    const u32 next_count = prev_count + 1;
                    const bool result = vp::util::InterlockedCompareExchange(std::addressof(prev_count), std::addressof(m_count), next_count, prev_count);
                    if (result) { break; }
                }

                /* Insert element into our reserved space */
                vp::util::MemoryBarrierInnerReadWrite();
                m_array[prev_count & max_mask] = reinterpret_cast<u64>(value);
                return true;
            }

            T RemoveFront() {

                /* Remove front loop */
                const u32 max = m_max_count;
                const u32 max_mask = (max - 1);
                u32 prev_offset = m_offset;
                u64 value;
                for (;;) {
                    const s32 count = static_cast<s32>(m_count - prev_offset);
                    if (count < 1) { return static_cast<T>(0); }

                    /* Ensure value */
                    value = m_array[prev_offset & max_mask];
                    if (value == 0) {
                        /* Try again, the value has not arrived, or maybe someone else took our value */
                        vp::util::MemoryBarrierInnerReadWrite();
                        u32 reload_offset = m_offset;
                        if (reload_offset == prev_offset) { return static_cast<T>(0); }
                        prev_offset = reload_offset;
                        continue;
                    }

                    /* Reserve offset and value */
                    const u32 next_offset = prev_offset + 1;
                    const bool result = vp::util::InterlockedCompareExchangeRelaxed(std::addressof(prev_offset), std::addressof(m_offset), next_offset, prev_offset);
                    if (!result) { continue; }
                    vp::util::MemoryBarrierInnerReadWrite();
                    m_array[prev_offset & max_mask] = 0;

                    /* Succeed if not cancelled */
                    if (!IsCancellable || value != cCancelValue) { return reinterpret_cast<T>(value); }
                    /* Try next offset if cancelled */
                    ++prev_offset;
                }
            }

            T RemoveFrontWeak() {

                /* Read loop */
                const u32 max_mask = (m_max_count - 1);
                u32 offset = m_offset;
                for (;;) {
                    /* Ensure we have an element */
                    const s32 count = static_cast<s32>(m_count - offset);
                    if (count < 1) { return static_cast<T>(0); }

                    /* Pop next value */
                    const u32 index = offset & max_mask;
                    u64 value = m_array[index];
                    /* Fail on zero if someone reserved space in PushBack, but their value has not yet been written */
                    if (value == 0) { return static_cast<T>(0); }
                    m_array[index] = 0;
                    ++offset;

                    /* Increment offset */
                    vp::util::MemoryBarrierInnerReadWrite();
                    m_offset = offset;
                    if (!IsCancellable || value != cCancelValue) { return reinterpret_cast<T>(value); }
                    /* Try next offset if cancelled */
                }
            }

            constexpr void Cancel(T value) requires (IsCancellable) {

                /* Integrity checks */
                VP_ASSERT(value != 0);
                VP_ASSERT(!IsCancellable || reinterpret_cast<u64>(value) != cCancelValue);

                const u32 max_mask = (m_max_count - 1);
                u32 offset = m_offset;
                u32 count = m_count;
                const u64 value_t = reinterpret_cast<u64>(value);
                while (offset < count) {
                    const u32 index = offset & max_mask;
                    if (m_array[index] == value_t) {
                        m_array[index] = cCancelValue;
                    }
                    ++offset;
                    count = m_count;
                }
                return;
            }

            constexpr void Clear() {
                if (!this->IsEmpty()) {
                    m_offset = 0;
                    m_count = 0;
                    for (u32 i = 0; i < m_max_count; ++i) {
                        m_array[i] = 0;
                    }
                }
                return;
            }

            constexpr T GetValue(u32 id) const {
                const u32 max = m_max_count;
                const u32 max_mask = (max - 1);
                return reinterpret_cast<T>(m_array[id & max_mask]);
            }

            constexpr bool IsValid() const { return m_array != nullptr; }
            constexpr bool IsEmpty() const { return (this->GetUsedCount() < 1); }
            constexpr bool IsFull() const { return this->GetUsedCount() == this->GetMaxCount(); }
            constexpr u32 GetUsedCount() const { return m_count - m_offset; }
            constexpr u32 GetCounter() const { return m_count; }
            constexpr u32 GetOffset() const { return m_offset; }
            constexpr u32 GetMaxCount() const { return m_max_count; }
	};

	template<typename T, u32 Count, bool IsCancellable = true>
        requires (sizeof(T) == sizeof(u64) && vp::util::IsAlignedPow2(Count))
	class AtomicFixedRingBuffer {
		public:
            static constexpr const u64 cCancelValue = 0xffff'ffff'ffff'ffff;
		public:
            class Iterator {
                public:
                    u32 m_offset;
                    T m_value;
                    const AtomicFixedRingBuffer *m_rb;
                public:
                    constexpr Iterator(u32 offset, AtomicFixedRingBuffer *rb) : m_offset(offset), m_rb(rb) {/*...*/}
                    constexpr ~Iterator() {/*...*/}

                    constexpr bool operator!=(const Iterator &rhs) const {
                        return m_offset < rhs.m_rb->GetCounter();
                    }
                    constexpr T operator*() const {
                        return m_value;
                    }
                    constexpr Iterator &operator++() {
                         while (m_offset < m_rb->GetCounter()) {
                            m_value = m_rb->GetValue(m_offset++);
                            if (m_value != 0 && (IsCancellable && m_value != cCancelValue)) { break; }
                        }
                        return *this;
                    }
            };
		private:
			u32 m_offset;
			u32 m_count;
			u64 m_array[Count];
        public:
            constexpr AtomicFixedRingBuffer() : m_offset(), m_count(), m_array{} {/*...*/}
            constexpr ~AtomicFixedRingBuffer() {/*...*/}

            constexpr Iterator begin() {
                return Iterator(m_offset,this);
            }
            constexpr Iterator end() {
                return Iterator(0,this);
            }

            bool PushBack(T value) {

                /* Integrity checks */
                VP_ASSERT(value != 0);
                VP_ASSERT(!IsCancellable || reinterpret_cast<u64>(value) != cCancelValue);

                /* Reserve space for push back loop */
                const u32 max = Count;
                const u32 max_mask = (max - 1);
                u32 prev_count = vp::util::InterlockedLoadAcquire(std::addressof(m_count));
                for (;;) {
                    /* Fail if out of space */
                    const u32 offset = vp::util::InterlockedLoadAcquire(std::addressof(m_offset));
                    if (max <= (prev_count - offset)) { return false; }

                    /* Reserve space */
                    const u32 next_count = prev_count + 1;
                    const bool result = vp::util::InterlockedCompareExchange(std::addressof(prev_count), std::addressof(m_count), next_count, prev_count);
                    if (result) { break; }
                }

                /* Insert element into our reserved space */
                vp::util::MemoryBarrierInnerReadWrite();
                m_array[prev_count & max_mask] = reinterpret_cast<u64>(value);
                return true;
            }

            T RemoveFront() {

                /* Remove front loop */
                const u32 max = Count;
                const u32 max_mask = (max - 1);
                u32 prev_offset = m_offset;
                u64 value;
                for (;;) {
                    const s32 count = static_cast<s32>(m_count - prev_offset);
                    if (count < 1) { return static_cast<T>(0); }

                    /* Ensure value */
                    value = m_array[prev_offset & max_mask];
                    if (value == 0) {
                        /* Try again, the value has not arrived, or maybe someone else took our value */
                        vp::util::MemoryBarrierInnerReadWrite();
                        u32 reload_offset = m_offset;
                        if (reload_offset == prev_offset) { return static_cast<T>(0); }
                        prev_offset = reload_offset;
                        continue;
                    }

                    /* Reserve offset and value */
                    const u32 next_offset = prev_offset + 1;
                    const bool result = vp::util::InterlockedCompareExchangeRelaxed(std::addressof(prev_offset), std::addressof(m_offset), next_offset, prev_offset);
                    if (!result) { continue; }
                    vp::util::MemoryBarrierInnerReadWrite();
                    m_array[prev_offset & max_mask] = 0;

                    /* Succeed if not cancelled */
                    if (!IsCancellable || value != cCancelValue) { return reinterpret_cast<T>(value); }
                    /* Try next offset if cancelled */
                    ++prev_offset;
                }
            }

            T RemoveFrontWeak() {

                /* Read loop */
                const u32 max_mask = (Count - 1);
                u32 offset = m_offset;
                for (;;) {
                    /* Ensure we have an element */
                    const s32 count = static_cast<s32>(m_count - offset);
                    if (count < 1) { return static_cast<T>(0); }

                    /* Pop next value */
                    const u32 index = offset & max_mask;
                    u64 value = m_array[index];
                    /* Fail on zero (is only meant to happen when space has been reserved in PushBack, but the value has not yet been written) */
                    if (value == 0) { return static_cast<T>(0); }
                    m_array[index] = 0;
                    ++offset;

                    /* Increment offset */
                    vp::util::MemoryBarrierInnerReadWrite();
                    m_offset = offset;
                    if (!IsCancellable || value != cCancelValue) { return reinterpret_cast<T>(value); }
                    /* Try next offset if cancelled */
                }
            }

            constexpr void Cancel(T value) requires (IsCancellable) {

                /* Integrity checks */
                VP_ASSERT(value != 0);
                VP_ASSERT(!IsCancellable || reinterpret_cast<u64>(value) != cCancelValue);

                const u32 max_mask = (Count - 1);
                u32 offset = m_offset;
                u32 count = m_count;
                const u64 value_t = reinterpret_cast<u64>(value);
                while (offset < count) {
                    const u32 index = offset & max_mask;
                    if (m_array[index] == value_t) {
                        m_array[index] = cCancelValue;
                    }
                    ++offset;
                    count = m_count;
                }
                return;
            }

            constexpr void Clear() {
                if (!this->IsEmpty()) {
                    m_offset = 0;
                    m_count = 0;
                    for (u32 i = 0; i < Count; ++i) {
                        m_array[i] = 0;
                    }
                }
                return;
            }

            constexpr T GetValue(u32 id) const {
                const u32 max = Count;
                const u32 max_mask = (max - 1);
                return reinterpret_cast<T>(m_array[id & max_mask]);
            }

            constexpr bool IsEmpty() const { return (static_cast<s32>(this->GetUsedCount()) < 1); }
            constexpr bool IsFull() const { return this->GetUsedCount() == this->GetMaxCount(); }
            constexpr u32 GetUsedCount() const { return m_count - m_offset; }
            constexpr u32 GetCounter() const { return m_count; }
            constexpr u32 GetOffset() const { return m_offset; }
            constexpr u32 GetMaxCount() const { return Count; }
	};
}
