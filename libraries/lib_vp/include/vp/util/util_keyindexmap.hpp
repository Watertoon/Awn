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

    template <class KeyType, class IndexType>
	class KeyIndexMap {
        public:
            static constexpr IndexType cInvalidEntryIndex = static_cast<IndexType>(0xffff'ffff'ffff'ffff);
        public:
            struct Key {
                KeyType hash;
            };
        private:
            HashMap<Key, &Key::hash> m_hash_map;
            IndexType *m_index_array;
        public:
            constexpr  KeyIndexMap() : m_hash_map(), m_index_array() {/*...*/}
            constexpr ~KeyIndexMap() {/*...*/}

            void Initialize(vp::imem::IHeap *heap, u32 max_count) {
                m_hash_map.Initialize(heap, max_count);
                m_index_array = new (heap, alignof(IndexType)) IndexType[max_count];
                VP_ASSERT(m_index_array != nullptr);
                return;
            }

            void Finalize() {
                m_hash_map.Finalize();
                if (m_index_array != nullptr) {
                    delete [] m_index_array;
                    m_index_array = nullptr;
                }
                return;
            }

            constexpr void SetMemory(Key *key_array, IndexType *index_array, u32 count) {
                m_hash_map.SetMemory(key_array, count);
                m_index_array = index_array;
                return;
            }

            constexpr void AddEntry(KeyType hash, IndexType index) {
                Key *entry = m_hash_map.AddEntry(hash);
                const u32 id = m_hash_map.IndexOf(entry);
                m_index_array[id] = index;
                return;
            }

            constexpr IndexType TryGetIndexByKey(KeyType hash) const {
                const u32 id = m_hash_map.TryGetIndexByHash(hash);
                return (id != HashMap<Key, &Key::hash>::cInvalidEntryIndex) ? m_index_array[id] : cInvalidEntryIndex;
            }

            constexpr void Clear() { m_hash_map.Clear(); }

            constexpr u32 GetUsedCount() const { return m_hash_map.GetUsedCount(); }
            constexpr u32 GetMaxCount() const { return m_hash_map.GetMaxCount(); }
        public:
            constexpr u32 CountMaxIterationsPerKey() const {
                return m_hash_map.CountMaxIterationsPerKey();
            }
    };
}
