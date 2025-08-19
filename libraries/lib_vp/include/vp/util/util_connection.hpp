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
            static constexpr size_t GetWorkMemorySize(u32 bit_count) {
                const u32 array_count = (bit_count / BITSIZEOF(T)) + ((bit_count % BITSIZEOF(T)) != 0);
                return array_count * sizeof(T);
            }
        public:
            constexpr Connection() : m_array_count(), m_bit_count(), m_mask_array{} {/*...*/}
            constexpr ~Connection() {/*...*/}

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

            void Initialize(vp::imem::IHeap *heap, u32 bit_count) {

                /* Allocate mask array */
                const u32 array_count = (bit_count / BITSIZEOF(T)) + ((bit_count % BITSIZEOF(T)) != 0);
                m_mask_array = new (heap, alignof(T)) T[bit_count];
                VP_ASSERT(m_mask_array != nullptr);
                ::memset(m_mask_array, 0, array_count * sizeof(T));

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
            
            constexpr void SetMemory(T *memory, u32 bit_count) {
                const u32 array_count = (bit_count / BITSIZEOF(T)) + ((bit_count % BITSIZEOF(T)) != 0);
                m_mask_array = memory;
                m_array_count = array_count;
                m_bit_count = bit_count;
                return;
            }

            constexpr void Clear() {
                ::memset(m_mask_array, 0, sizeof(T) * m_array_count);
                return;
            }

            constexpr bool GetBit(u32 bit_index) const {
                return (m_mask_array[bit_index / BITSIZEOF(T)] >> (bit_index & (BITSIZEOF(T) - 1))) & 1;
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

            constexpr T GetBitMask(u32 mask_index) const {
                return m_mask_array[mask_index];
            }

            constexpr void SetBitMask(u32 mask_index, T mask) {
                m_mask_array[mask_index] = mask;
            }

            constexpr u32 CountOneBits() const {
                u32 count = 0;
                for (u32 i = 0; i < m_array_count; ++i) {
                    count += vp::util::CountOneBits(m_mask_array[i]);
                }
                return count;
            }
            
            constexpr u32 GetBitCount() const { return m_bit_count; }
            constexpr u32 GetArrayCount() const { return m_array_count; }
    };
}
