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

	template <typename RP, auto HashMemberPtr, u32 Size>
        requires (std::is_integral<vp::util::MemberType<HashMemberPtr>>::value) && (std::is_standard_layout<RP>::value)
	class FixedHashMap {
        public:
            static constexpr u32 cInvalidEntryIndex = 0xffff'ffff;
        public:
            using Parent   = RP;
            using HashType = vp::util::MemberType<HashMemberPtr>;
        private:
            u32    m_count;
            Parent m_array[Size];
        private:
            constexpr ALWAYS_INLINE u32 GetBaseIndex(HashType hash) const {

                /* Calculate base index */
                const u32 max_count  = Size;
                const u32 base_div   = (max_count != 0) ? hash / max_count : 0;
                const u32 base_index = hash - (base_div * max_count);

                return base_index;
            }
		public:
            constexpr ALWAYS_INLINE  FixedHashMap() : m_count(), m_array() {/*...*/}
            constexpr ALWAYS_INLINE ~FixedHashMap() {/*...*/}

            constexpr Parent *AddEntry(HashType hash) {

                /* Integrity check */
                VP_ASSERT(m_count < Size);
                VP_ASSERT(hash != 0);

                /* Insert entry */
                const u32 base_index = this->GetBaseIndex(hash);
                u32 i = base_index;
                do {
                    if (m_array[i].*HashMemberPtr == 0) {
                        ++m_count;
                        m_array[i].*HashMemberPtr = hash;
                        return std::addressof(m_array[i]);
                    }
                    i = ((i + 1) < Size) ? i + 1 : 0;
                } while (i != base_index);

                VP_ASSERT(false);

                return nullptr;
            }

            constexpr Parent *TryGetParentByHash(HashType hash) {
        
                /* Find actual index linearly from base */
                const u32 base_index = this->GetBaseIndex(hash);
                u32 i = base_index;
                do {
                    if (hash == m_array[i].*HashMemberPtr) { return std::addressof(m_array[i]); }
                    i = ((i + 1) < Size) ? i + 1 : 0;
                } while (i != base_index);

                return nullptr;
            }

            constexpr const Parent *TryGetParentByHash(HashType hash) const {
        
                /* Find actual index linearly from base */
                const u32 base_index = this->GetBaseIndex(hash);
                u32 i = base_index;
                do {
                    if (hash == m_array[i].*HashMemberPtr) { return std::addressof(m_array[i]); }
                    i = ((i + 1) < Size) ? i + 1 : 0;
                } while (i != base_index);

                return nullptr;
            }

            constexpr void Clear() {

                /* Destroy all */
                for (u32 i = 0; i < Size; ++i) {
                    if (m_array[i].*HashMemberPtr == 0) { continue; }
                    m_array[i].*HashMemberPtr = 0;
                    std::destroy_at(std::addressof(m_array[i]));
                }

                m_count = 0;

                return;
            }
        public:
            constexpr u32 CountMaxIterationsPerKey() const {

                /* Iterate hash map */
                u32 count = 0;
                HashType current_hash;
                for (u32 i = 0; i < Size; ++i) {

                    /* Check valid slot */
                    current_hash = m_array[i].*HashMemberPtr;
                    if (current_hash == 0) { continue; }
                    
                    /* Find offset from hash's base index */
                    const u32 base_index = this->GetBaseIndex(current_hash);
                    u32 sub_count = 0;
                    u32 iter = base_index;
                    do {
                        if (current_hash == m_array[iter].*HashMemberPtr) { break; }
                        ++sub_count;
                        iter = ((iter + 1) < Size) ? iter + 1 : 0;
                    } while (iter != base_index);

                    /* Set max iteration count */
                    count = vp::util::Max(count, sub_count);
                }

                return count;
            }
	};
}
