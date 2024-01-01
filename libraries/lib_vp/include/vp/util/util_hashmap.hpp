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

	template <auto HashMemberPtr>
        requires (std::is_integral<vp::util::MemberType<HashMemberPtr>>::value == true) && (std::is_standard_layout<vp::util::ParentType<HashMemberPtr>>::value == true)
	class HashMap {
        public:
            static constexpr u32 cInvalidEntryIndex = 0xffff'ffff;
        public:
            using Parent   = vp::util::ParentType<HashMemberPtr>;
            using HashType = vp::util::MemberType<HashMemberPtr>;
        private:
            u32     m_count;
            u32     m_max;
            Parent *m_array;
        private:
            constexpr ALWAYS_INLINE GetBaseIndex(HashType hash) const {

                /* Calculate base index */
                const u32 max_count  = m_max;
                const u32 base_div   = (max_count != 0) ? hash / max_count : 0;
                const u32 base_index = hash - (base_div * max_count);

                return base_index;
            }
		public:
            constexpr ALWAYS_INLINE  HashMap() : m_count(0), m_max(0), m_array(nullptr) {/*...*/}
            constexpr ALWAYS_INLINE ~HashMap() {/*...*/}

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

            constexpr ALWAYS_INLINE Parent *AddEntry(HashType hash) {

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

            constexpr ALWAYS_INLINE Parent *TryGetParentByHash(HashType hash) {

                /* Find actual index linearly from base */
                const u32 base_index = this->GetBaseIndex(hash);
                u32 i = base_index;
                do {
                    if (hash == m_array[i].*HashMemberPtr) { return std::addressof(m_array[i]); }
                    i = ((i + 1) < m_max) ? i + 1 : 0;
                } while (i != base_index);

                return nullptr;
            }

            constexpr ALWAYS_INLINE void Clear() {

                /* Destroy all */
                for (u32 i = 0; i < m_max; ++i) {
                    if (m_array[i].*HashMemberPtr == 0) { continue; }
                    m_array[i].*HashMemberPtr = 0;
                    std::destroy_at(std::addressof(m_array[i]));
                }

                m_count = 0;

                return;
            }
	};

	template <auto HashMemberPtr, u32 Size>
        requires (std::is_integral<vp::util::MemberType<HashMemberPtr>>::value == true) && (std::is_standard_layout<vp::util::ParentType<HashMemberPtr>>::value == true)
	class FixedHashMap {
        public:
            static constexpr u32 cInvalidEntryIndex = 0xffff'ffff;
        public:
            using Parent   = vp::util::ParentType<HashMemberPtr>;
            using HashType = vp::util::MemberType<HashMemberPtr>;
        private:
            u32    m_count;
            Parent m_array[Size];
        private:
            constexpr ALWAYS_INLINE GetBaseIndex(HashType hash) const {

                /* Calculate base index */
                const u32 max_count  = Size;
                const u32 base_div   = (max_count != 0) ? hash / max_count : 0;
                const u32 base_index = hash - (base_div * max_count);

                return base_index;
            }
		public:
            constexpr ALWAYS_INLINE  FixedHashMap() : m_count(0), m_array() {/*...*/}
            constexpr ALWAYS_INLINE ~FixedHashMap() {/*...*/}

            constexpr ALWAYS_INLINE Parent *AddEntry(HashType hash) {

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

            constexpr ALWAYS_INLINE Parent *TryGetParentByHash(HashType hash) {

                /* Find actual index linearly from base */
                const u32 base_index = this->GetBaseIndex(hash);
                u32 i = base_index;
                do {
                    if (hash == m_array[i].*HashMemberPtr) { return std::addressof(m_array[i]); }
                    i = ((i + 1) < Size) ? i + 1 : 0;
                } while (i != base_index);

                return nullptr;
            }

            constexpr ALWAYS_INLINE void Clear() {

                /* Destroy all */
                for (u32 i = 0; i < Size; ++i) {
                    if (m_array[i].*HashMemberPtr == 0) { continue; }
                    m_array[i].*HashMemberPtr = 0;
                    std::destroy_at(std::addressof(m_array[i]));
                }

                m_count = 0;

                return;
            }
	};
}
