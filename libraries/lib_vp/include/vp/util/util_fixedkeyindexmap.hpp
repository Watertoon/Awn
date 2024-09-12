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

    template <u32 Size>
	class FixedKeyIndexMap {
        public:
            static constexpr u32 cInvalidEntryIndex = 0xffff'ffff;
        public:
            struct Index {
                u32 hash;
                u32 index;
            };
        private:
            FixedHashMap<&Index::hash, Size> m_hash_map;
        public:
            constexpr ALWAYS_INLINE  FixedKeyIndexMap() : m_hash_map() {/*...*/}
            constexpr ALWAYS_INLINE ~FixedKeyIndexMap() {/*...*/}

            constexpr ALWAYS_INLINE void AddEntry(u32 hash, u32 index) {
                Index *entry = m_hash_map.AddEntry(hash);
                entry->index = index;
                return;
            }

            constexpr ALWAYS_INLINE u32 TryGetIndexByKey(u32 hash) const {
                const Index *entry = m_hash_map.TryGetParentByHash(hash);
                return (entry != nullptr) ? entry->index : cInvalidEntryIndex;
            }

            constexpr ALWAYS_INLINE void Clear() { m_hash_map.Clear(); }
    };
}
