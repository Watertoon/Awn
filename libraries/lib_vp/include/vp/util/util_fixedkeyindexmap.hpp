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

    template <class KeyType, class IndexType, u32 Size>
	class FixedKeyIndexMap {
        public:
            static constexpr u32 cInvalidEntryIndex = 0xffff'ffff;
        public:
            struct Key {
                KeyType hash;
            };
        private:
            FixedHashMap<Key, &Key::hash, Size> m_hash_map;
            IndexType m_index_array[Size];
        public:
            constexpr  FixedKeyIndexMap() : m_hash_map(), m_index_array() {/*...*/}
            constexpr ~FixedKeyIndexMap() {/*...*/}

            constexpr void AddEntry(KeyType hash, IndexType index) {
                Key *entry = m_hash_map.AddEntry(hash);
                const u32 id = m_hash_map.IndexOf(entry);
                m_index_array[id] = index;
                return;
            }

            constexpr IndexType TryGetIndexByKey(KeyType hash) const {
                const u32 id = m_hash_map.TryGetIndexByHash(hash);
                return (id != FixedHashMap<Key, &Key::hash, Size>::cInvalidEntryIndex) ? m_index_array[id] : cInvalidEntryIndex;
            }

            constexpr void Clear() { m_hash_map.Clear(); }

            constexpr u32 GetUsedCount() const { return m_hash_map.GetUsedCount(); }
        public:
            constexpr u32 CountMaxIterationsPerKey() const {
                return m_hash_map.CountMaxIterationsPerKey();
            }
    };
}
