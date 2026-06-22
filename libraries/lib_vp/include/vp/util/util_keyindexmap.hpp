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
            struct KeyIndex {
                KeyType hash;
                IndexType index;
            };
        public:
            using MapType = HashMap<Key, &Key::hash>;
        public:
            class Iterator {
                private:
                    const KeyIndexMap *m_map;
                    const Key *m_iter;
                public:
                    constexpr Iterator(const KeyIndexMap *map, const Key *iter) : m_map(map), m_iter(iter) {/*...*/}
                    constexpr ~Iterator() {/*...*/}

                    constexpr KeyIndex operator*() {
                        const u32 hash_id = m_map->m_hash_map.IndexOf(m_iter);
                        const u32 id = m_map->GetIndexByHashIndex(hash_id);
                        return { m_iter->hash, id, };
                    }
                    constexpr KeyIndex operator*() const {
                        const u32 hash_id = m_map->m_hash_map.IndexOf(m_iter);
                        const u32 id = m_map->GetIndexByHashIndex(hash_id);
                        return { m_iter->hash, id, };
                    }
                    constexpr bool operator==(const Iterator &rhs) const {
                        return m_iter == rhs.m_iter;
                    }

                    constexpr bool operator!=(const Iterator &rhs) const {
                        return m_iter != rhs.m_iter;
                    }

                    constexpr Iterator &operator++() {
                        const Key *iter = m_map->m_hash_map.GetNextEntry(m_iter);
                        m_iter = iter;
                        return *this;
                    }
            };
        private:
            MapType m_hash_map;
            IndexType *m_index_array;
        public:
            constexpr  KeyIndexMap() : m_hash_map(), m_index_array() {/*...*/}
            constexpr ~KeyIndexMap() {/*...*/}

            Result Initialize(vp::imem::IHeap *heap, u32 max_count) {
                RESULT_TRY(m_hash_map.Initialize(heap, max_count));
                auto fail_guard0 = SCOPE_GUARD { m_hash_map.Finalize(); };
                IndexType *id_array = new (heap, alignof(IndexType)) IndexType[max_count];
                RESULT_RETURN_UNLESS(id_array != nullptr, ResultNoMemory);
                fail_guard0.Cancel();
                m_index_array = id_array;
                RESULT_RETURN_SUCCESS;
            }

            void Finalize() {
                m_hash_map.Finalize();
                if (m_index_array != nullptr) {
                    delete [] m_index_array;
                    m_index_array = nullptr;
                }
                return;
            }

            static constexpr size_t GetRequiredMemorySize(u32 count) {
                return sizeof(KeyIndex) * count;
            }
            constexpr void SetMemory(Key *key_array, IndexType *index_array, u32 count) {
                m_hash_map.SetMemory(key_array, count);
                m_index_array = index_array;
                return;
            }
        public:
            constexpr Iterator begin() const {
                const Key *key = m_hash_map.GetFirstEntry();
                return Iterator(this, key);
            }
            constexpr Iterator end() const {
                const Key *key = m_hash_map.GetLastEntry();
                return Iterator(this, key);
            }

            constexpr void AddEntry(KeyType hash, IndexType index) {
                Key *entry = m_hash_map.AddEntry(hash);
                const u32 id = m_hash_map.IndexOf(entry);
                m_index_array[id] = index;
                return;
            }
            constexpr void ReplaceEntry(KeyType hash, IndexType index) {
                const u32 id = m_hash_map.TryGetIndexByHash(hash);
                VP_ASSERT(id != MapType::cInvalidEntryIndex);
                m_index_array[id] = index;
                return;
            }

            constexpr IndexType TryGetIndexByKey(KeyType hash) const {
                const u32 id = m_hash_map.TryGetIndexByHash(hash);
                return (id != MapType::cInvalidEntryIndex) ? this->GetIndexByHashIndex(id) : cInvalidEntryIndex;
            }

            constexpr void Clear() { m_hash_map.Clear(); }

            constexpr u32 GetUsedCount() const { return m_hash_map.GetUsedCount(); }
            constexpr u32 GetMaxCount() const { return m_hash_map.GetMaxCount(); }
        public:
            constexpr u32 GetIndexByHashIndex(u32 id) const {
                return m_index_array[id];
            }

            constexpr u32 CountMaxIterationsPerKey() const {
                return m_hash_map.CountMaxIterationsPerKey();
            }
    };

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
