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
        requires (std::is_integral<T>::value && std::is_unsigned<T>::value)
    class Connection {
        public:
            class Iterator {
                private:
                    const T *m_mask_array;
                    u32      m_index;
                    u32      m_bit_max;
                public:
                    constexpr Iterator(u32 base_index, u32 bit_max) : m_mask_array(), m_index(base_index), m_bit_max(bit_max) {/*...*/}
                    constexpr Iterator(const Connection *connection, u32 base_index, u32 bit_max) : m_mask_array(connection->m_mask_array), m_index(base_index), m_bit_max(bit_max) {/*...*/}
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
                        u32 bit_i = m_index + 1;
                        u32 arr_i = bit_i / BITSIZEOF(T);
                        T mask = m_mask_array[arr_i];
                        u32 shift = (bit_i & (BITSIZEOF(T) - 1));
                        while (bit_i < m_bit_max && !((mask >> shift) & 1)) {
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
            };
        private:
            u32 m_array_count;
            u32 m_bit_count;
            T *m_mask_array;
        public:
            constexpr Connection() : m_array_count(), m_bit_count(), m_mask_array{} {/*...*/}
            constexpr ~Connection() {/*...*/}

            constexpr Iterator begin() const {

                u32 bit_i = 0;
                u32 arr_i = 0;
                T mask = m_mask_array[arr_i];
                u32 shift = 0;
                while (bit_i < m_bit_count && !((mask >> shift) & 1)) {
                    ++bit_i;
                    shift = (bit_i & (BITSIZEOF(T) - 1));
                    if (shift == 0) { 
                        const u32 next_arr = arr_i + 1; 
                        mask = m_mask_array[next_arr];
                        arr_i = next_arr;
                    }
                }

                return Iterator(this, bit_i, m_bit_count);
            }
            constexpr Iterator end() const {
                return Iterator(m_bit_count, m_bit_count);
            }

            void Initialize(vp::imem::IHeap *heap, u32 bit_count) {

                /* Allocate mask array */
                const u32 array_count = (bit_count / BITSIZEOF(T)) + ((bit_count % BITSIZEOF(T)) != 0);
                m_mask_array = new (heap, alignof(T)) T[bit_count];
                VP_ASSERT(m_mask_array != nullptr);

                /* Set counts */
                m_array_count = array_count;
                m_bit_count = bit_count;
        
                return;
            }

            void Finalize() {
                if (m_mask_array != nullptr) {
                    delete m_mask_array;
                    m_array_count = 0;
                    m_bit_count = 0;
                    m_mask_array = nullptr;
                }
            }

            constexpr void Clear() { ::memset(m_mask_array, 0, sizeof(T) * m_array_count); }

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
                for (u32 i = 0; i < m_array_count; ++i) {
                    count += vp::util::CountOneBits(m_mask_array[i]);
                }
                return count;
            }
    };
}
