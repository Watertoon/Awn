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
    class Connection {
        public:
            static constexpr T cHeadBit = static_cast<T>(1) << (BITSIZEOF(T) - 1);
        public:
            class Iterator {
                private:
                    const T *m_mask_array;
                    T        m_mask;
                    u32      m_max_count;
                    u32      m_max_bit;
                    u32      m_iter;
                    u32      m_index;
                public:
                    constexpr Iterator(u32 max_array, u32 max_bit) : m_mask(), m_max_count(max_array), m_iter(max_array), m_index(max_bit) {/*...*/}
                    constexpr Iterator(const Connection *connection, T mask, u32 base_index) : m_mask_array(connection->m_mask_array), m_max_count(connection->m_array_count), m_max_bit(connection->m_bit_count), m_iter(base_index) {
                        const u32 lz = vp::util::CountRightZeroBits(mask);
                        const T clear = static_cast<T>(1) << lz;
                        m_index = (m_iter * BITSIZEOF(T)) + lz;
                        m_mask = clear ^ mask;
                    }
                    constexpr ~Iterator() {/*...*/}

                    constexpr ALWAYS_INLINE u32 operator*() {
                        return m_index;
                    }
                    constexpr ALWAYS_INLINE u32 operator*() const {
                        return m_index;
                    }

                    constexpr bool operator==(const Iterator &rhs) const {
                        return m_iter == rhs.m_iter && m_index == rhs.m_index;
                    }

                    constexpr bool operator!=(const Iterator &rhs) const {
                        return m_iter != rhs.m_iter || m_index != rhs.m_index;
                    }

                    constexpr Iterator &operator++() {
                        while (m_mask == 0) {
                            m_iter += 1;
                            if (m_max_count <= m_iter) {
                                m_iter = m_max_count;
                                m_index = m_max_bit;
                                return *this;
                            }
                            m_mask = m_mask_array[m_iter];
                        }
                        const T mask = m_mask;
                        const u32 lz = vp::util::CountRightZeroBits(mask);
                        const T clear = static_cast<T>(1) << lz;
                        m_index = (m_iter * BITSIZEOF(T)) + lz;
                        m_mask = clear ^ mask;
                        return *this;
                    }
            };
        private:
            u32 m_array_count;
            u32 m_bit_count;
            T *m_mask_array;
        public:
            constexpr Connection() : m_array_count(), m_bit_count(), m_mask_array{} {/*...*/}
            constexpr ~Connection() {/*...*/}

            Result Initialize(vp::imem::IHeap *heap, u32 bit_count) {

                /* Allocate mask array */
                const u32 array_count = (bit_count / BITSIZEOF(T)) + ((bit_count % BITSIZEOF(T)) != 0);
                T *array = new (heap, alignof(T)) T[bit_count];
                RESULT_RETURN_UNLESS(array != nullptr, ResultNoMemory);
                ::memset(array, 0, array_count * sizeof(T));

                /* Set counts */
                m_mask_array = array;
                m_array_count = array_count;
                m_bit_count = bit_count;

                RESULT_RETURN_SUCCESS;
            }

            void Finalize() {
                if (m_mask_array != nullptr) {
                    delete m_mask_array;
                    m_array_count = 0;
                    m_bit_count = 0;
                    m_mask_array = nullptr;
                }
            }

            static constexpr size_t GetRequiredMemorySize(u32 bit_count) {
                const u32 array_count = (bit_count / BITSIZEOF(T)) + ((bit_count % BITSIZEOF(T)) != 0);
                return array_count * sizeof(T);
            }

            constexpr void SetMemory(T *memory, u32 bit_count) {
                const u32 array_count = (bit_count / BITSIZEOF(T)) + ((bit_count % BITSIZEOF(T)) != 0);
                m_mask_array = memory;
                m_array_count = array_count;
                m_bit_count = bit_count;
                return;
            }
        public:
            constexpr Iterator begin() const {
                const u32 arr_count = m_array_count;
                u32 iter = 0;
                T mask = m_mask_array[iter];
                while (mask == 0) {
                    iter += 1;
                    if (arr_count <= iter) {
                        return Iterator(m_array_count, m_bit_count);
                    }
                    mask = m_mask_array[iter];
                }
                return Iterator(this, mask, iter);
            }
            constexpr Iterator end() const {
                return Iterator(m_array_count, m_bit_count);
            }

            constexpr void Fill(bool is_set) {
                const u32 value = (is_set) ? 0xffff'ffff : 0;
                ::memset(m_mask_array, value, sizeof(T) * m_array_count);
                return;
            }
            constexpr void Clear() {
                ::memset(m_mask_array, 0, sizeof(T) * m_array_count);
                return;
            }

            constexpr bool GetBit(u32 bit_index) const {
                const u32 index = bit_index / BITSIZEOF(T);
                const u32 shift = (bit_index & (BITSIZEOF(T) - 1));
                const T mask = m_mask_array[index];
                return (mask >> shift) & 1;
            }

            constexpr bool SetBit(u32 bit_index) {
                const u32 index = bit_index / BITSIZEOF(T);
                const u32 shift = (bit_index & (BITSIZEOF(T) - 1));
                const T mask = m_mask_array[index];
                m_mask_array[index] = mask | (static_cast<T>(1) << shift);
                const bool ret = (mask >> shift) & 1;
                return ret;
            }

            constexpr bool ClearBit(u32 bit_index) {
                const u32 index = bit_index / BITSIZEOF(T);
                const u32 shift = (bit_index & (BITSIZEOF(T) - 1));
                const T mask = m_mask_array[index];
                m_mask_array[index] = mask & (~(static_cast<T>(1) << shift));
                const bool ret = (mask >> shift) & 1;
                return ret;
            }

            constexpr bool XorBit(u32 bit_index) {
                const u32 index = bit_index / BITSIZEOF(T);
                const u32 shift = (bit_index & (BITSIZEOF(T) - 1));
                const T mask = m_mask_array[index];
                m_mask_array[index] = mask ^ (static_cast<T>(1) << shift);
                const bool ret = (mask >> shift) & 1;
                return ret;
            }

            constexpr bool ExchangeBit(u32 bit_index, bool is_set) {
                const u32 index = bit_index / BITSIZEOF(T);
                const u32 shift = (bit_index & (BITSIZEOF(T) - 1));
                const T mask = m_mask_array[index];
                const T set_mask = static_cast<T>(is_set) << shift;
                const T clear_mask = ~(static_cast<T>(1) << shift);
                m_mask_array[index] = (mask & (clear_mask)) | (set_mask);
                const bool ret = (mask >> shift) & 1;
                return ret;
            }

            constexpr T GetBitMask(u32 mask_index) const {
                return m_mask_array[mask_index];
            }

            constexpr void SetBitMask(u32 mask_index, T mask) {
                m_mask_array[mask_index] = mask;
            }

            constexpr T *GetBitMaskArray() { return m_mask_array; }

            constexpr void ExchangeBitMask(u32 mask_index, T clear_mask, T set_mask) {
                const T prev = m_mask_array[mask_index];
                const T next = (prev & (~clear_mask)) | set_mask;
                m_mask_array[mask_index] = next;
            }
    
            constexpr u32 CountOneBits() const {
                u32 count = 0;
                for (u32 i = 0; i < m_array_count; ++i) {
                    count += vp::util::CountOneBits(m_mask_array[i]);
                }
                return count;
            }

            constexpr u32 GetIndexOfFirstOne() const {
                const T *start = m_mask_array;
                const T *iter = start;
                const T *end = start + m_array_count;
                T mask;
                do {
                    mask = *iter;
                    if (mask != static_cast<T>(0)) { break; }
                    iter += 1;
                } while (iter != end);
                u32 index = (iter - start) * BITSIZEOF(T);
                index += vp::util::CountRightZeroBits(mask);
                return index;
            }
            constexpr u32 GetIndexOfFirstOne(u32 base_index) const {
                const T *start = m_mask_array;
                const T *iter = start + (base_index / BITSIZEOF(T));
                const T *end = start + m_array_count;
                const T bit_mask = ((~static_cast<T>(0)) << base_index);
                T mask = *iter & bit_mask;
                for (;;) {
                    if (mask != static_cast<T>(0)) { break; }
                    ++iter;
                    if (iter == end) { break; }
                    mask = *iter;
                }
                u32 index = (iter - start) * BITSIZEOF(T);
                index += vp::util::CountRightZeroBits(mask);
                return index;
            }

            constexpr u32 GetIndexOfFirstZero() const {
                const T *start = m_mask_array;
                const T *iter = start;
                const T *end = start + m_array_count;
                T mask;
                do {
                    mask = *iter;
                    if (mask != ~static_cast<T>(0)) { break; }
                    iter += 1;
                } while (iter != end);
                u32 index = (iter - start) * BITSIZEOF(T);
                index += vp::util::CountRightOneBits(mask);
                return index;
            }

            constexpr u32 GetBitCount() const { return m_bit_count; }
            constexpr u32 GetArrayCount() const { return m_array_count; }
            
            constexpr bool IsValid() const { return m_mask_array != nullptr; }
        public:
            constexpr bool GetBitAcquire(u32 bit_index) const {
                const u32 index = bit_index / BITSIZEOF(T);
                const u32 shift = (bit_index & (BITSIZEOF(T) - 1));
                const T mask = vp::util::InterlockedLoadAcquire(m_mask_array + index);
                return (mask >> shift) & 1;
            }

            constexpr bool InterlockedSetBit(u32 bit_index) {
                const u32 index = bit_index / BITSIZEOF(T);
                const u32 shift = (bit_index & (BITSIZEOF(T) - 1));
                const T bit = (static_cast<T>(1) << shift);
                const T prev = vp::util::InterlockedFetchOr(std::addressof(m_mask_array[index]), bit);
                const bool ret = (prev >> shift) & 1;
                return ret;
            }
            constexpr bool InterlockedClearBit(u32 bit_index) {
                const u32 index = bit_index / BITSIZEOF(T);
                const u32 shift = (bit_index & (BITSIZEOF(T) - 1));
                const T mask = (~(static_cast<T>(1) << shift));
                const T prev = vp::util::InterlockedFetchAnd(std::addressof(m_mask_array[index]), mask);
                const bool ret = (prev >> shift) & 1;
                return ret;
            }
            constexpr bool InterlockedXorBit(u32 bit_index) {
                const u32 index = bit_index / BITSIZEOF(T);
                const u32 shift = (bit_index & (BITSIZEOF(T) - 1));
                const T bit = (static_cast<T>(1) << shift);
                const T prev = vp::util::InterlockedFetchXor(std::addressof(m_mask_array[index]), bit);
                const bool ret = (prev >> shift) & 1;
                return ret;
            }
            constexpr bool InterlockedExchangeBit(u32 bit_index, bool is_set) {
                const u32 index = bit_index / BITSIZEOF(T);
                const u32 shift = (bit_index & (BITSIZEOF(T) - 1));
                const T set_mask = static_cast<T>(is_set) << shift;
                const T clear_mask = ~(static_cast<T>(1) << shift);
                T *loc = std::addressof(m_mask_array[index]);
                T prev = *loc;
                for (;;) {
                    const T next = (prev & (~clear_mask)) | set_mask;
                    if (vp::util::InterlockedCompareExchange(std::addressof(prev), loc, next, prev)) { break; }
                }
                const bool ret = (prev >> shift) & 1;
                return ret;
            }

            constexpr T InterlockedAndBitMask(u32 mask_index, T mask) {
                const T prev = vp::util::InterlockedFetchAnd(std::addressof(m_mask_array[mask_index]), mask);
                return prev;
            }
            constexpr T InterlockedOrBitMask(u32 mask_index, T mask) {
                const T prev = vp::util::InterlockedFetchOr(std::addressof(m_mask_array[mask_index]), mask);
                return prev;
            }
            constexpr T InterlockedXorBitMask(u32 mask_index, T mask) {
                const T prev = vp::util::InterlockedFetchXor(std::addressof(m_mask_array[mask_index]), mask);
                return prev;
            }
            constexpr void InterlockedExchangeBitMask(u32 mask_index, T clear_mask, T set_mask) {
                T *loc = std::addressof(m_mask_array[mask_index]);
                T prev = *loc;
                for (;;) {
                    const T next = (prev & (~clear_mask)) | set_mask;
                    if (vp::util::InterlockedCompareExchange(std::addressof(prev), loc, next, prev)) { return; }
                }
            }
        public:
            constexpr void CopyFrom(const Connection &rhs) {
                const u32 count = vp::util::Min(m_array_count, rhs.m_array_count);
                for (u32 i = 0; i < count; ++i) {
                    m_mask_array[i] = rhs.m_mask_array[i];
                }
                return;
            }
            constexpr void And(const Connection &rhs) {
                const u32 count = vp::util::Min(m_array_count, rhs.m_array_count);
                for (u32 i = 0; i < count; ++i) {
                    m_mask_array[i] &= rhs.m_mask_array[i];
                }
                return;
            }
            constexpr void Or(const Connection &rhs) {
                const u32 count = vp::util::Min(m_array_count, rhs.m_array_count);
                for (u32 i = 0; i < count; ++i) {
                    m_mask_array[i] |= rhs.m_mask_array[i];
                }
                return;
            }
            constexpr void Xor(const Connection &rhs) {
                const u32 count = vp::util::Min(m_array_count, rhs.m_array_count);
                for (u32 i = 0; i < count; ++i) {
                    m_mask_array[i] ^= rhs.m_mask_array[i];
                }
                return;
            }
            constexpr void Not(const Connection &rhs) {
                const u32 count = vp::util::Min(m_array_count, rhs.m_array_count);
                for (u32 i = 0; i < count; ++i) {
                    m_mask_array[i] = ~rhs.m_mask_array[i];
                }
                return;
            }
    };
    
    template <typename T, u32 BitCount>
        requires (std::is_integral<T>::value && std::is_unsigned<T>::value)
    class FixedConnection {
        public:
            static constexpr u32 cArrayCount = (BitCount / BITSIZEOF(T)) + ((BitCount % BITSIZEOF(T)) != 0);
            static constexpr T cHeadBit = static_cast<T>(1) << (BITSIZEOF(T) - 1);
        public:
            class Iterator {
                private:
                    const T *m_mask_array;
                    T        m_mask;
                    u32      m_iter;
                    u32      m_index;
                public:
                    constexpr Iterator() : m_iter(cArrayCount), m_index(BitCount) {/*...*/}
                    constexpr Iterator(const FixedConnection *connection, T mask, u32 base_index) : m_mask_array(connection->m_mask_array), m_iter(base_index) {
                        const u32 lz = vp::util::CountRightZeroBits(mask);
                        const T clear = static_cast<T>(1) << lz;
                        m_index = (m_iter * BITSIZEOF(T)) + lz;
                        m_mask = clear ^ mask;
                    }
                    constexpr ~Iterator() {/*...*/}

                    constexpr u32 operator*() {
                        return m_index;
                    }
                    constexpr u32 operator*() const {
                        return m_index;
                    }

                    constexpr bool operator==(const Iterator &rhs) const {
                        return m_iter == rhs.m_iter && m_index == rhs.m_index;
                    }

                    constexpr Iterator &operator++() {
                        while (m_mask == 0) {
                            m_iter += 1;
                            if (cArrayCount <= m_iter) {
                                m_iter = cArrayCount;
                                m_index = BitCount;
                                return *this;
                            }
                            m_mask = m_mask_array[m_iter];
                        }
                        const T mask = m_mask;
                        const u32 lz = vp::util::CountRightZeroBits(mask);
                        const T clear = static_cast<T>(1) << lz;
                        m_index = (m_iter * BITSIZEOF(T)) + lz;
                        m_mask = clear ^ mask;
                        return *this;
                    }
            };
        private:
            T m_mask_array[cArrayCount];
        public:
            constexpr FixedConnection() : m_mask_array{} {/*...*/}
            constexpr ~FixedConnection() {/*...*/}

            constexpr Iterator begin() const {
                u32 iter = 0;
                T mask = m_mask_array[iter];
                while (mask == 0) {
                    iter += 1;
                    if (cArrayCount <= iter) {
                        return Iterator();
                    }
                    mask = m_mask_array[iter];
                }
                return Iterator(this, mask, iter);
            }
            constexpr Iterator end() const {
                return Iterator();
            }

            constexpr void Fill(bool is_set) {
                const u32 value = (is_set) ? 0xffff'ffff : 0;
                ::memset(m_mask_array, value, sizeof(m_mask_array));
            }
            constexpr void Clear() { ::memset(m_mask_array, 0, sizeof(m_mask_array)); }

            constexpr bool GetBit(u32 bit_index) const {
                const u32 index = bit_index / BITSIZEOF(T);
                const u32 shift = (bit_index & (BITSIZEOF(T) - 1));
                const T mask = m_mask_array[index];
                return (mask >> shift) & 1;
            }

            constexpr bool SetBit(u32 bit_index) {
                const u32 index = bit_index / BITSIZEOF(T);
                const u32 shift = (bit_index & (BITSIZEOF(T) - 1));
                const T mask = m_mask_array[index];
                m_mask_array[index] = mask | (static_cast<T>(1) << shift);
                const bool ret = (mask >> shift) & 1;
                return ret;
            }
            constexpr bool ClearBit(u32 bit_index) {
                const u32 index = bit_index / BITSIZEOF(T);
                const u32 shift = (bit_index & (BITSIZEOF(T) - 1));
                const T mask = m_mask_array[index];
                m_mask_array[index] = mask & (~(static_cast<T>(1) << shift));
                const bool ret = (mask >> shift) & 1;
                return ret;
            }
            constexpr bool XorBit(u32 bit_index) {
                const u32 index = bit_index / BITSIZEOF(T);
                const u32 shift = (bit_index & (BITSIZEOF(T) - 1));
                const T mask = m_mask_array[index];
                m_mask_array[index] = mask ^ (static_cast<T>(1) << shift);
                const bool ret = (mask >> shift) & 1;
                return ret;
            }

            constexpr bool ExchangeBit(u32 bit_index, bool is_set) {
                const u32 index = bit_index / BITSIZEOF(T);
                const u32 shift = (bit_index & (BITSIZEOF(T) - 1));
                const T mask = m_mask_array[index];
                const T set_mask = static_cast<T>(is_set) << shift;
                const T clear_mask = ~(static_cast<T>(1) << shift);
                m_mask_array[index] = (mask & (clear_mask)) | (set_mask);
                const bool ret = (mask >> shift) & 1;
                return ret;
            }

            constexpr T GetBitMask(u32 mask_index) const { return m_mask_array[mask_index]; }
            constexpr void SetBitMask(u32 mask_index, T mask) { m_mask_array[mask_index] = mask; }
            constexpr T *GetBitMaskArray() { return m_mask_array; }

            constexpr void ExchangeBitMask(u32 mask_index, T clear_mask, T set_mask) {
                const T prev = m_mask_array[mask_index];
                const T next = (prev & (~clear_mask)) | set_mask;
                m_mask_array[mask_index] = next;
            }

            constexpr u32 CountOneBits() const {
                u32 count = 0;
                for (u32 i = 0; i < cArrayCount; ++i) {
                    count += vp::util::CountOneBits(m_mask_array[i]);
                }
                return count;
            }

            constexpr u32 GetIndexOfFirstOne() const {
                const T *start = m_mask_array;
                const T *iter = start;
                const T *end = start + cArrayCount;
                T mask;
                do {
                    mask = *iter;
                    if (mask != static_cast<T>(0)) { break; }
                    iter += 1;
                } while (iter != end);
                u32 index = (iter - start) * BITSIZEOF(T);
                index += vp::util::CountRightZeroBits(mask);
                return index;
            }
            constexpr u32 GetIndexOfFirstOne(u32 base_index) const {
                const T *start = m_mask_array;
                const T *iter = start + (base_index / BITSIZEOF(T));
                const T *end = start + cArrayCount;
                const T bit_mask = ((~static_cast<T>(0)) << base_index);
                T mask = *iter & bit_mask;
                for (;;) {
                    if (mask != static_cast<T>(0)) { break; }
                    ++iter;
                    if (iter == end) { break; }
                    mask = *iter;
                }
                u32 index = (iter - start) * BITSIZEOF(T);
                index += vp::util::CountRightZeroBits(mask);
                return index;
            }

            constexpr u32 GetIndexOfFirstZero() const {
                const T *start = m_mask_array;
                const T *iter = start;
                const T *end = start + cArrayCount;
                T mask;
                do {
                    mask = *iter;
                    if (mask != ~static_cast<T>(0)) { break; }
                    iter += 1;
                } while (iter != end);
                u32 index = (iter - start) * BITSIZEOF(T);
                index += vp::util::CountRightOneBits(mask);
                return index;
            }
        public:
            constexpr bool GetBitAcquire(u32 bit_index) const {
                const u32 index = bit_index / BITSIZEOF(T);
                const u32 shift = (bit_index & (BITSIZEOF(T) - 1));
                const T mask = vp::util::InterlockedLoadAcquire(m_mask_array + index);
                return (mask >> shift) & 1;
            }

            constexpr bool InterlockedSetBit(u32 bit_index) {
                const u32 index = bit_index / BITSIZEOF(T);
                const u32 shift = (bit_index & (BITSIZEOF(T) - 1));
                const T bit = (static_cast<T>(1) << shift);
                const T prev = vp::util::InterlockedFetchOr(std::addressof(m_mask_array[index]), bit);
                const bool ret = (prev >> shift) & 1;
                return ret;
            }
            constexpr bool InterlockedClearBit(u32 bit_index) {
                const u32 index = bit_index / BITSIZEOF(T);
                const u32 shift = (bit_index & (BITSIZEOF(T) - 1));
                const T mask = (~(static_cast<T>(1) << shift));
                const T prev = vp::util::InterlockedFetchAnd(std::addressof(m_mask_array[index]), mask);
                const bool ret = (prev >> shift) & 1;
                return ret;
            }
            constexpr bool InterlockedXorBit(u32 bit_index) {
                const u32 index = bit_index / BITSIZEOF(T);
                const u32 shift = (bit_index & (BITSIZEOF(T) - 1));
                const T bit = (static_cast<T>(1) << shift);
                const T prev = vp::util::InterlockedFetchXor(std::addressof(m_mask_array[index]), bit);
                const bool ret = (prev >> shift) & 1;
                return ret;
            }
            constexpr T InterlockedAndBitMask(u32 mask_index, T mask) {
                const T prev = vp::util::InterlockedFetchAnd(std::addressof(m_mask_array[mask_index]), mask);
                return prev;
            }
            constexpr T InterlockedOrBitMask(u32 mask_index, T mask) {
                const T prev = vp::util::InterlockedFetchOr(std::addressof(m_mask_array[mask_index]), mask);
                return prev;
            }
            constexpr T InterlockedXorBitMask(u32 mask_index, T mask) {
                const T prev = vp::util::InterlockedFetchXor(std::addressof(m_mask_array[mask_index]), mask);
                return prev;
            }
            constexpr void InterlockedExchangeBitMask(u32 mask_index, T clear_mask, T set_mask) {
                T *loc = std::addressof(m_mask_array[mask_index]);
                T prev = *loc;
                for (;;) {
                    const T next = (prev & (~clear_mask)) | set_mask;
                    if (vp::util::InterlockedCompareExchange(std::addressof(prev), loc, next, prev)) { return; }
                }
            }
        public:
            constexpr void CopyFrom(const FixedConnection &rhs) {
                for (u32 i = 0; i < cArrayCount; ++i) {
                    m_mask_array[i] = rhs.m_mask_array[i];
                }
                return;
            }
            constexpr void And(const FixedConnection &rhs) {
                for (u32 i = 0; i < cArrayCount; ++i) {
                    m_mask_array[i] &= rhs.m_mask_array[i];
                }
                return;
            }
            constexpr void Or(const FixedConnection &rhs) {
                for (u32 i = 0; i < cArrayCount; ++i) {
                    m_mask_array[i] |= rhs.m_mask_array[i];
                }
                return;
            }
            constexpr void Xor(const FixedConnection &rhs) {
                for (u32 i = 0; i < cArrayCount; ++i) {
                    m_mask_array[i] ^= rhs.m_mask_array[i];
                }
                return;
            }
            constexpr void Not(const FixedConnection &rhs) {
                for (u32 i = 0; i < cArrayCount; ++i) {
                    m_mask_array[i] = ~rhs.m_mask_array[i];
                }
                return;
            }
    };
}
