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

	template <typename T, const u32 FlagBitCount>
	class PointerBitMask {
        public:
            static constexpr size_t cPointerMask =  (-1 << FlagBitCount);
            static constexpr size_t cValueMask   = ~(-1 << FlagBitCount);
        public:
            static_assert(FlagBitCount < 0x20);
            //static_assert(vp::util::CountRightZeroBits64(alignof(T)) <= FlagBitCount);
		private:
			uintptr_t m_pointer;
		public:
			constexpr PointerBitMask() = default;
			inline PointerBitMask(T *pointer) : m_pointer(reinterpret_cast<uintptr_t>(pointer)) {/*...*/}
			constexpr ~PointerBitMask() = default;

            constexpr inline bool GetBit(u32 bit_offset) const {
                return (m_pointer >> bit_offset) & 1;
            }
            constexpr inline void SetBit(u32 bit_index) {
                const size_t bit_insert = (1ull << bit_index);
                const size_t bit_mask   = ~bit_insert;
                m_pointer = ((m_pointer & bit_mask) | bit_insert);
            }
            constexpr inline void ClearBit(u32 bit_index) {
                const size_t bit_insert = (1ull << bit_index);
                const size_t bit_mask   = ~bit_insert;
                m_pointer = (m_pointer & bit_mask);
            }

            constexpr inline uintptr_t InterlockedExchangePointerAndClearBitMaskRelaxed(T *pointer) {
                return InterlockedExchangeRelaxed(std::addressof(m_pointer), reinterpret_cast<uintptr_t>(pointer));
            }

            constexpr inline uintptr_t GetBitMask() const { return (m_pointer & cValueMask); }
            constexpr inline void SetBitMask(uintptr_t bit_mask) { m_pointer = (m_pointer & cPointerMask) | (bit_mask & cValueMask); }
            constexpr inline void SetBitMaskAndClearPointer(uintptr_t bit_mask) { m_pointer = bit_mask; }

            inline T *GetPointer() const { return reinterpret_cast<T*>(m_pointer & cPointerMask); }
            inline void SetPointer(T *pointer) { m_pointer = reinterpret_cast<uintptr_t>(pointer) | (m_pointer & cValueMask); }
            inline void SetPointerAndClearBitMask(T *pointer) { m_pointer = reinterpret_cast<uintptr_t>(pointer); }
            inline void SetPointerAndBitMask(T *pointer, uintptr_t bit_mask) { m_pointer = reinterpret_cast<uintptr_t>(pointer) | bit_mask; }

            constexpr inline uintptr_t GetValue() const { return m_pointer; }
            constexpr inline void SetValue(uintptr_t value) { m_pointer = value; }
	};
    static_assert(std::is_trivially_constructible_v<PointerBitMask<void*,1>>);
    static_assert(std::is_trivially_destructible_v<PointerBitMask<void*,1>>);
}
