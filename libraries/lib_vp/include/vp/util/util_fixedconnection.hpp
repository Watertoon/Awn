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

            constexpr void Clear() { ::memset(m_mask_array, 0, sizeof(m_mask_array)); }

            constexpr bool GetBit(u32 bit_index) const { return (m_mask_array[bit_index / BITSIZEOF(T)] >> (bit_index & (BITSIZEOF(T) - 1))) & 1; }
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
        public:
            constexpr bool SetBitAtomic(u32 bit_index) {
                const u32 index = bit_index / BITSIZEOF(T);
                const u32 shift = (bit_index & (BITSIZEOF(T) - 1));
                const T bit = (static_cast<T>(1) << shift);
                const T prev = vp::util::InterlockedFetchOr(std::addressof(m_mask_array[index]), bit);
                const bool ret = (prev >> shift) & 1;
                return ret;
            }
            constexpr bool ClearBitAtomic(u32 bit_index) {
                const u32 index = bit_index / BITSIZEOF(T);
                const u32 shift = (bit_index & (BITSIZEOF(T) - 1));
                const T mask = (~(static_cast<T>(1) << shift));
                const T prev = vp::util::InterlockedFetchAnd(std::addressof(m_mask_array[index]), mask);
                const bool ret = (prev >> shift) & 1;
                return ret;
            }
            constexpr bool XorBitAtomic(u32 bit_index) {
                const u32 index = bit_index / BITSIZEOF(T);
                const u32 shift = (bit_index & (BITSIZEOF(T) - 1));
                const T bit = (static_cast<T>(1) << shift);
                const T prev = vp::util::InterlockedFetchXor(std::addressof(m_mask_array[index]), bit);
                const bool ret = (prev >> shift) & 1;
                return ret;
            }
            constexpr T AndBitMaskAtomic(u32 mask_index, T mask) {
                const T prev = vp::util::InterlockedFetchAnd(std::addressof(m_mask_array[mask_index]), mask);
                return prev;
            }
            constexpr T OrBitMaskAtomic(u32 mask_index, T mask) {
                const T prev = vp::util::InterlockedFetchOr(std::addressof(m_mask_array[mask_index]), mask);
                return prev;
            }
            constexpr T XorBitMaskAtomic(u32 mask_index, T mask) {
                const T prev = vp::util::InterlockedFetchXor(std::addressof(m_mask_array[mask_index]), mask);
                return prev;
            }
            constexpr void ExchangeBitMaskAtomic(u32 mask_index, T clear_mask, T set_mask) {
                T *loc = std::addressof(m_mask_array[mask_index]);
                T prev = *loc;
                for (;;) {
                    const T next = (prev & (~clear_mask)) | set_mask;
                    if (vp::util::InterlockedCompareExchange(std::addressof(prev), loc, next, prev)) { return; }
                }
            }
        public:
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
