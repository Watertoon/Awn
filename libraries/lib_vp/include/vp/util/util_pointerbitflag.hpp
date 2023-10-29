#pragma once

namespace vp::util {

	template <typename T, const u32 FlagBitCount>
	class PointerBitFlag {
        public:
            static constexpr size_t cPointerMask =  (-1 << FlagBitCount);
            static constexpr size_t cValueMask   = ~(-1 << FlagBitCount);
        public:
            static_assert(FlagBitCount < 0x20);
            //static_assert(vp::util::CountRightZeroBits64(alignof(T)) <= FlagBitCount);
		private:
			uintptr_t m_pointer;
		public:
			ALWAYS_INLINE  PointerBitFlag(T *pointer) : m_pointer(reinterpret_cast<uintptr_t>(pointer)) {/*...*/}
			ALWAYS_INLINE ~PointerBitFlag() {/*...*/}

            ALWAYS_INLINE T &operator->() {
                return *reinterpret_cast<T*>(m_pointer & cPointerMask);
            }

            constexpr ALWAYS_INLINE void SetBit(u32 bit_offset) {
                const size_t bit_insert = (1ull << bit_offset);
                const size_t bit_mask   = ~bit_insert;
                m_pointer = ((m_pointer & bit_mask) | bit_insert);
            }
            constexpr ALWAYS_INLINE void ClearBit(u32 bit_offset) {
                const size_t bit_insert = (1ull << bit_offset);
                const size_t bit_mask   = ~bit_insert;
                m_pointer = (m_pointer & bit_mask);
            }

            constexpr ALWAYS_INLINE u32 GetBit(u32 bit_offset) const { return (m_pointer >> bit_offset) & 1; }
            constexpr ALWAYS_INLINE u32 GetBitMask()           const { return (m_pointer & cValueMask); }

            ALWAYS_INLINE       T *GetPointer()       { return reinterpret_cast<T*>(m_pointer & cPointerMask); }
            ALWAYS_INLINE const T *GetPointer() const { return reinterpret_cast<const T*>(m_pointer & cPointerMask); }

            constexpr ALWAYS_INLINE void SetBitMask(u32 bit_mask)  { m_pointer = (m_pointer & cPointerMask) | (bit_mask & cValueMask); }
            ALWAYS_INLINE void SetPointer(T *pointer)              { m_pointer = reinterpret_cast<uintptr_t>(pointer) | (m_pointer & cValueMask); }
            ALWAYS_INLINE void SetPointerAndClearFlags(T *pointer) { m_pointer = reinterpret_cast<uintptr_t>(pointer); }
	};
}
