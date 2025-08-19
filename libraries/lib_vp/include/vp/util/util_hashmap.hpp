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

	template <typename RP, auto HashMemberPtr>
        requires (std::is_integral<vp::util::MemberType<HashMemberPtr>>::value) && (std::is_standard_layout<RP>::value)
	class HashMap {
        public:
            static constexpr u32 cInvalidEntryIndex = 0xffff'ffff;
        public:
            using Parent   = RP;
            using HashType = vp::util::MemberType<HashMemberPtr>;
        private:
            u32     m_count;
            u32     m_max;
            Parent *m_array;
        private:
            constexpr u32 GetBaseIndex(HashType hash) const {

                /* Calculate base index */
                const u32 max_count  = m_max;
                const u32 base_div   = (max_count != 0) ? hash / max_count : 0;
                const u32 base_index = hash - (base_div * max_count);

                return base_index;
            }
		public:
            constexpr  HashMap() : m_count(), m_max(), m_array() {/*...*/}
            constexpr ~HashMap() {/*...*/}

            void Initialize(vp::imem::IHeap *heap, u32 max_count) {

                /* Integrity checks */
                VP_ASSERT(0 < max_count);

                /* Allocate array */
                m_array = new (heap, alignof(Parent)) Parent[max_count];
                VP_ASSERT(m_array != nullptr);

                /* Set counts */
                m_count = 0;
                m_max   = max_count;

                return;
            }

            void Finalize() {

                if (m_array != nullptr) {
                    delete [] m_array;
                }
                m_array = nullptr;
                m_count = 0;
                m_max   = 0;

                return;
            }

            constexpr void SetMemory(Parent *parent_array, u32 max_count) {
                m_array = parent_array;
                m_max = max_count;
            }

            constexpr Parent *AddEntry(HashType hash) {

                /* Integrity check */
                VP_ASSERT(m_count < m_max);
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
                    i = ((i + 1) < m_max) ? i + 1 : 0;
                } while (i != base_index);

                VP_ASSERT(false);

                return nullptr;
            }

            constexpr u32 TryGetIndexByHash(HashType hash) const {

                /* Ensure initialized */
                if (m_max < 1) { return cInvalidEntryIndex; }

                /* Find actual index linearly from base */
                const u32 base_index = this->GetBaseIndex(hash);
                u32 i = base_index;
                do {
                    if (hash == m_array[i].*HashMemberPtr) { return i; }
                    i = ((i + 1) < m_max) ? i + 1 : 0;
                } while (i != base_index);

                return cInvalidEntryIndex;
            }

            constexpr Parent *TryGetParentByHash(HashType hash) {
                const u32 index = this->TryGetIndexByHash(hash);
                Parent *parent = (index != cInvalidEntryIndex) ? std::addressof(m_array[index]) : nullptr;
                return parent;
            }

            constexpr const Parent *TryGetParentByHash(HashType hash) const {
                const u32 index = this->TryGetIndexByHash(hash);
                const Parent *parent = (index != cInvalidEntryIndex) ? std::addressof(m_array[index]) : nullptr;
                return parent;
            }

            constexpr void Clear() {

                /* Destroy all */
                for (u32 i = 0; i < m_max; ++i) {
                    if (m_array[i].*HashMemberPtr == 0) { continue; }
                    m_array[i].*HashMemberPtr = 0;
                    std::destroy_at(std::addressof(m_array[i]));
                }

                m_count = 0;

                return;
            }

            constexpr u32 GetUsedCount() const { return m_count; }
            constexpr u32 GetMaxCount() const { return m_max; }
        public:
            constexpr u32 IndexOf(Parent *parent) {
                return static_cast<u32>(parent - m_array);
            }

            constexpr u32 CountMaxIterationsPerKey() const {

                /* Iterate hash map */
                u32 count = 0;
                HashType current_hash;
                for (u32 i = 0; i < m_max; ++i) {

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
                        iter = ((iter + 1) < m_max) ? iter + 1 : 0;
                    } while (iter != base_index);

                    /* Set max iteration count */
                    count = vp::util::Max(count, sub_count);
                }

                return count;
            }
	};
}
