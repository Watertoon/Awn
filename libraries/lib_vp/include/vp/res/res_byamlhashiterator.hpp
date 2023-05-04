#pragma once

namespace vp::res {

    class ByamlHashArrayPair {
        private:
            void *m_hash_pair;
            u32   m_data_offset;
        public:
            constexpr ByamlHashArrayHelper(void *hash_pair, u32 data_offset) : m_hash_pair(hash_pair), m_data_offset(stride) {/*...*/}

            ALWAYS_INLINE u32 GetValue() const {
                return *reinterpret_cast<u32*>(reinterpret_cast<uintptr_t>(m_hash_pair) + m_data_offset);
            }

            ALWAYS_INLINE u64 GetHash() const {
                if (m_data_offset == 8) { return *reinterpret_cast<u64*>(m_hash_pair); }
                if (m_data_offset == 4) { return static_cast<u64>(*reinterpret_cast<u32*>(m_hash_pair)); }
                return 0;
            }
    };
    

}
