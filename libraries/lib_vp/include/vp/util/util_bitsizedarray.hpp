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

	template <typename IndexType, size_t BitSize>
		requires ((vp::util::CountOneBits(BitSize) == 1) && (BitSize & ((8 * sizeof(IndexType)) - 1)) != 0)
	class BitSizedArray {
		public:
			static constexpr u32 cElementsPerIndex = (8 * sizeof(IndexType)) / BitSize;
			static constexpr u32 cElementMask      = (1 << BitSize) - 1;
		private:
			u32		   m_element_count;
			IndexType *m_bit_array;
		public:
			constexpr  BitSizedArray() : m_element_count(), m_bit_array() {/*...*/}
			constexpr ~BitSizedArray() {/*...*/}

            Result Initialize(vp::imem::IHeap *heap, u32 element_count) {

                /* Get count */
                const u32 count = element_count / cElementsPerIndex + ((element_count % cElementsPerIndex) != 0);

                /* Set counts */
                IndexType *id_type = new (heap, alignof(IndexType)) IndexType[count];
                RESULT_RETURN_UNLESS(id_type != nullptr, ResultNoMemory);
                m_bit_array = id_type;
                m_element_count = element_count;

                RESULT_RETURN_SUCCESS;
            }

            void Finalize() {

                if (m_bit_array != nullptr) {
                    delete [] m_bit_array;
                    m_bit_array = nullptr;
                }
                m_element_count = 0;

                return;
            }
		public:
			constexpr IndexType GetElement(u32 index) const {
                VP_ASSERT(index < m_element_count);
				return (m_bit_array[index / cElementsPerIndex] >> (index % cElementsPerIndex)) & cElementMask;
			}

            constexpr void SetElement(u32 index, IndexType value) {

                /* Set new value while preserving old values */
                VP_ASSERT(index < m_element_count);
                const u32       array_index    = index / cElementsPerIndex;
                const u32       element_offset = index % cElementsPerIndex;
                const IndexType last_value     = m_bit_array[array_index] & ~(cElementMask << element_offset);
                const IndexType new_value      = last_value | ((value & cElementMask) << element_offset);
                m_bit_array[array_index]       = new_value;

                return;
            }
	};
    
    
	template <typename IndexType, size_t BitSize, size_t Count>
		requires ((vp::util::CountOneBits(BitSize) == true) && (BitSize & ((8 * sizeof(IndexType)) - 1)) != 0)
	class FixedBitSizedArray {
		public:
			static constexpr u32 cElementsPerIndex = (8 * sizeof(IndexType)) / BitSize;
			static constexpr u32 cElementMask      = (1 << BitSize) - 1;
			static constexpr u32 cArrayCount	   = Count / cElementsPerIndex + ((Count % cElementsPerIndex) != 0);
		private:
			IndexType m_bit_array[cArrayCount];
		public:
			constexpr  FixedBitSizedArray() : m_bit_array() {/*...*/}
			constexpr ~FixedBitSizedArray() {/*...*/}

			constexpr IndexType GetElement(u32 index) const {
                VP_ASSERT(index < Count);
				return (m_bit_array[index / cElementsPerIndex] >> (index % cElementsPerIndex)) & cElementMask;
			}

            constexpr void SetElement(u32 index, IndexType value) {

                /* Set new value while preserving old values */
                VP_ASSERT(index < Count);
                const u32       array_index    = index / cElementsPerIndex;
                const u32       element_offset = index % cElementsPerIndex;
                const IndexType last_value     = m_bit_array[array_index] & ~(cElementMask << element_offset);
                const IndexType new_value      = last_value | ((value & cElementMask) << element_offset);
                m_bit_array[array_index]       = new_value;

                return;
            }
	};
}
