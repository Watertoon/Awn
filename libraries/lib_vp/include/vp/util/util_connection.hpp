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

    template <typename T, u32 ConnectionCount>
        requires (std::is_integral<T>::value && std::is_unsigned<T>::value)
    class FixedConnection {
        public:
            static constexpr u32 cConnectionArrayCount = (ConnectionCount / BITSIZEOF(T)) + ((ConnectionCount % BITSIZEOF(T)) != 0);
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

                        /* Get next index */
                        const u32 iter_i      = m_index + 1;
                        const T   base_offset = (iter_i & (BITSIZEOF(T) - 1));
                        const u32 arr_index   = iter_i / BITSIZEOF(T);
                        const T   mask        = (m_mask_array[arr_index] >> base_offset);
                        const u32 zero_bits   = (mask) ? vp::util::CountRightZeroBits(mask) : BITSIZEOF(T) - base_offset;
                        const u32 next_index  = iter_i + zero_bits;

                        if constexpr (BITSIZEOF(T) < ConnectionCount) {
                            u32 arr_iter = arr_index + 1;
                            T   mask_i   = mask;
                            u32 next_i   = next_index;
                            while (mask_i == 0 && arr_iter < cConnectionArrayCount) {
                                mask_i  = m_mask_array[arr_iter];
                                next_i += vp::util::CountRightZeroBits(mask_i);
                                ++arr_iter;
                            }
                            m_index = next_i;
                            return *this;
                        }

                        m_index = next_index;
                        return *this;
                    }
            };
        private:
            T m_mask_array[cConnectionArrayCount];
        public:
            constexpr FixedConnection() : m_mask_array{} {/*...*/}
            constexpr ~FixedConnection() {/*...*/}

            constexpr Iterator begin() const {

                /* Get base index */
                const T   mask = m_mask_array[0];
                const u32 base_index = vp::util::CountRightZeroBits(mask);

                if constexpr (BITSIZEOF(T) < ConnectionCount) {
                    u32 arr_iter = 1;
                    T   mask_i = mask;
                    u32 base_i = base_index;
                    while (mask_i == 0 && arr_iter < cConnectionArrayCount) {
                        mask_i  = m_mask_array[arr_iter];
                        base_i += vp::util::CountRightZeroBits(mask_i);
                        ++arr_iter;
                    }
                    return Iterator(this, base_i);
                }

                return Iterator(this, base_index);
            }
            constexpr Iterator end() const {
                return Iterator(ConnectionCount);
            }

            constexpr void Clear() { ::memset(m_mask_array, 0, sizeof(m_mask_array)); }

            constexpr bool GetBit(u32 bit_index) const { return (m_mask_array[bit_index / BITSIZEOF(T)] >> (bit_index & (BITSIZEOF(T) - 1))) & 1; }
            constexpr void SetBit(u32 bit_index)       { m_mask_array[bit_index / BITSIZEOF(T)] |= (static_cast<T>(1) << (bit_index & (BITSIZEOF(T) - 1))); }
            constexpr void ClearBit(u32 bit_index)     { m_mask_array[bit_index / BITSIZEOF(T)] &= ~(static_cast<T>(1) << (bit_index & (BITSIZEOF(T) - 1))); }
            constexpr void XorBit(u32 bit_index)       { m_mask_array[bit_index / BITSIZEOF(T)] ^= (static_cast<T>(1) << (bit_index & (BITSIZEOF(T) - 1))); }

            constexpr T GetBitMask(u32 mask_index) { return m_mask_array[mask_index]; }
            constexpr void SetBitMask(u32 mask_index, T mask) { m_mask_array[mask_index] = mask; }
    };
}
