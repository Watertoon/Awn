#pragma once

namespace vp::util {

	class KeyIndexMap {
        public:
            static constexpr u32 cInvalidEntryIndex = 0xffff'ffff;
        public:
            struct Index {
                u32 hash;
                u32 index;
            };
        private:
            HashMap<&Index::hash> m_hash_map;
        public:
            constexpr ALWAYS_INLINE  KeyIndexMap() : m_hash_map() {/*...*/}
            constexpr ALWAYS_INLINE ~KeyIndexMap() {/*...*/}

            ALWAYS_INLINE void Initialize(vp::imem::IHeap *heap, u32 max_count) {
                m_hash_map.Initialize(heap, max_count);
            }

            ALWAYS_INLINE void Finalize() {
                m_hash_map.Finalize();
            }

            constexpr ALWAYS_INLINE void AddEntry(u32 hash, u32 index) {
                Index *entry = m_hash_map.AddEntry(hash);
                entry->index = index;
                return;
            }

            constexpr ALWAYS_INLINE u32 TryGetIndexByKey(u32 hash) {
                Index *entry = m_hash_map.TryGetParentByHash(hash);
                return (entry != nullptr) ? entry->index : cInvalidEntryIndex;
            }
    };

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

            constexpr ALWAYS_INLINE u32 TryGetIndexByKey(u32 hash) {
                Index *entry = m_hash_map.TryGetParentByHash(hash);
                return (entry != nullptr) ? entry->index : cInvalidEntryIndex;
            }
    };
}
