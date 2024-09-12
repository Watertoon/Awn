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
        public:
            class Iterator {
                private:
                    const T *m_mask_array;
                    u32      m_index;
                public:
                    constexpr Iterator(u32 base_index) : m_mask_array(), m_index(base_index) {/*...*/}
                    constexpr Iterator(const FixedConnection *connection, u32 base_index) : m_mask_array(connection->m_mask_array), m_index(base_index) {/*...*/}
                    constexpr ~Iterator() {/*...*/}

                    constexpr ALWAYS_INLINE u32 operator*() {
                        return m_index;
                    }
                    constexpr ALWAYS_INLINE u32 operator*() const {
                        return m_index;
                    }

                    constexpr ALWAYS_INLINE bool operator==(const Iterator &rhs) const {
                        return m_index == rhs.m_index;
                    }

                    constexpr ALWAYS_INLINE bool operator!=(const Iterator &rhs) const {
                        return m_index != rhs.m_index;
                    }

                    constexpr ALWAYS_INLINE Iterator &operator++() {

                        /* Linearly iterate bits until next one bit for small T sizes */
                        if constexpr (sizeof(T) < sizeof(u32)) {

                            u32 bit_i = m_index + 1;
                            u32 arr_i = bit_i / BITSIZEOF(T);
                            T mask = m_mask_array[arr_i];
                            u32 shift = (bit_i & (BITSIZEOF(T) - 1));
                            while (bit_i < BitCount && !((mask >> shift) & 1)) {
                                ++bit_i;
                                shift = (bit_i & (BITSIZEOF(T) - 1));
                                if (shift == 0) { 
                                    const u32 next_arr = arr_i + 1; 
                                    mask = m_mask_array[next_arr];
                                    arr_i = next_arr;
                                }
                            }
                            m_index = bit_i;
            
                            return *this;
                        }

                        /* Scan bits until next one bit for valid T sizes */
                        const u32 iter_i      = m_index + 1;
                        const T   base_offset = (iter_i & (BITSIZEOF(T) - 1));
                        const u32 arr_index   = iter_i / BITSIZEOF(T);
                        const T   mask        = (m_mask_array[arr_index] >> base_offset);
                        const u32 zero_bits   = (mask) ? vp::util::CountRightZeroBits(mask) : BITSIZEOF(T) - base_offset;
                        const u32 next_index  = iter_i + zero_bits;

                        if constexpr (BITSIZEOF(T) < BitCount) {
                            u32 arr_iter = arr_index + 1;
                            T   mask_i   = mask;
                            u32 next_i   = next_index;
                            while (mask_i == 0 && arr_iter < cArrayCount) {
                                mask_i  = m_mask_array[arr_iter];
                                next_i += vp::util::CountRightZeroBits(mask_i);
                                ++arr_iter;
                            }
                            m_index = vp::util::Min(next_i, BitCount);
                            return *this;
                        }

                        m_index = vp::util::Min(next_index, BitCount);
                        return *this;
                    }
            };
        private:
            T m_mask_array[cArrayCount];
        public:
            constexpr FixedConnection() : m_mask_array{} {/*...*/}
            constexpr ~FixedConnection() {/*...*/}

            constexpr Iterator begin() const {

                if constexpr (sizeof(T) < sizeof(u32)) {

                    u32 bit_i = 0;
                    u32 arr_i = 0;
                    T mask = m_mask_array[arr_i];
                    u32 shift = 0;
                    while (bit_i < BitCount && !((mask >> shift) & 1)) {
                        ++bit_i;
                        shift = (bit_i & (BITSIZEOF(T) - 1));
                        if (shift == 0) { 
                            const u32 next_arr = arr_i + 1; 
                            mask = m_mask_array[next_arr];
                            arr_i = next_arr;
                        }
                    }
    
                    return Iterator(this, bit_i);
                }

                /* Get base index */
                const T   mask = m_mask_array[0];
                const u32 base_index = vp::util::CountRightZeroBits(mask);

                if constexpr (BITSIZEOF(T) < BitCount) {
                    u32 arr_iter = 1;
                    T   mask_i = mask;
                    u32 base_i = base_index;
                    while (mask_i == 0 && arr_iter < cArrayCount) {
                        mask_i  = m_mask_array[arr_iter];
                        base_i += vp::util::CountRightZeroBits(mask_i);
                        ++arr_iter;
                    }
                    return Iterator(this, vp::util::Min(base_i, BitCount));
                }

                return Iterator(this, vp::util::Min(base_index, BitCount));
            }
            constexpr Iterator end() const {
                return Iterator(BitCount);
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

            constexpr u32 CountOneBits() const {
                u32 count = 0;
                for (u32 i = 0; i < cArrayCount; ++i) {
                    count += vp::util::CountOneBits(m_mask_array[i]);
                }
                return count;
            }
    };
}
