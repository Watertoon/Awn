#pragma once

namespace vp::res {

    enum ByteOrder {
        ByteOrder_Big    = 0xFFFE,
        ByteOrder_Little = 0xFEFF
    };

    struct ResSarc {
        u32 magic;
        u16 header_size;
        u16 endianess;
        u32 file_size;
        u32 file_array_offset;
        u16 version;
        u16 reserve0;

        static constexpr u32 cMagic         = util::TCharCode32("SARC");
        static constexpr u32 cTargetVersion = 0x100;
    };
    static_assert(sizeof(ResSarc) == 0x14);

    struct ResSfat {
        u32 magic;
        u16 header_size;
        u16 file_count;
        u32 hash_seed;

        static constexpr u32 cMagic = util::TCharCode32("SFAT");
    };
    static_assert(sizeof(ResSfat) == 0xC);

    struct ResSfatEntry {
        u32 file_name_hash;
        u32 file_name_offset     : 24;
        u32 hash_collision_count : 8;
        u32 file_array_start_offset;
        u32 file_array_end_offset;
    };
    static_assert(sizeof(ResSfatEntry) == 0x10);

    struct ResSfnt {
        u32 magic;
        u16 header_size;
        u16 padding;

        static constexpr u32 cMagic = util::TCharCode32("SFNT");
    };
    static_assert(sizeof(ResSfnt) == 0x8);

    class SarcExtractor {
        public:
            static constexpr u32 cInvalidEntryId = 0xFFFF'FFFF;
        private:
            ResSarc      *m_sarc;
            ResSfat      *m_sfat;
            ResSfatEntry *m_sfat_entry_array;
            void         *m_file_array;
            char         *m_path_table;
        public:
            constexpr SarcExtractor() {/*...*/}
            SarcExtractor(void *sarc_file) {
                this->Initialize(sarc_file);
            }

            bool Initialize(void *sarc_file) {
                if (sarc_file == nullptr) { return false; }

                m_sarc = reinterpret_cast<ResSarc*>(sarc_file);

                /* ResSarc integrity checks */
                if (m_sarc->magic       != ResSarc::cMagic)         { return false; }
                if (m_sarc->endianess   != ByteOrder_Little)        { return false; }
                if (m_sarc->header_size != sizeof(ResSarc))         { return false; }
                if (m_sarc->version     != ResSarc::cTargetVersion) { return false; }

                m_sfat = reinterpret_cast<ResSfat*>(reinterpret_cast<uintptr_t>(sarc_file) + sizeof(ResSarc));

                /* ResSfat integrity checks */
                if (m_sfat->magic != ResSfat::cMagic)       { return false; }
                if (m_sfat->header_size != sizeof(ResSfat)) { return false; }
                if ((m_sfat->file_count >> 14) != 0)        { return false; }

                m_sfat_entry_array = reinterpret_cast<ResSfatEntry*>(reinterpret_cast<uintptr_t>(sarc_file) + sizeof(ResSarc) + sizeof(ResSfat));

                ResSfnt *sfnt = reinterpret_cast<ResSfnt*>(reinterpret_cast<uintptr_t>(sarc_file) + sizeof(ResSarc) + sizeof(ResSfat) + sizeof(ResSfatEntry) * m_sfat->file_count);

                /* ResSfnt integrity checks*/
                if (sfnt->magic != ResSfnt::cMagic)       { return false; }
                if (sfnt->header_size != sizeof(ResSfnt)) { return false; }

                m_path_table = reinterpret_cast<char*>(reinterpret_cast<uintptr_t>(sfnt) + sfnt->header_size);

                m_file_array = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(sarc_file) + m_sarc->file_array_offset);
                
                return true;
            }

            const char *ConvertEntryIdToPath(u32 entry_id) const {
                return m_path_table + (m_sfat_entry_array[entry_id].file_name_offset * 4);
            }

            u32 ConvertPathToEntryId(const char *string) const {

                /* Calculate hash */
                const u32 hash_seed = m_sfat->hash_seed;
                u32 hash = 0;
                while (*string != '\0') {
                    hash = hash * hash_seed + static_cast<int>(*string);
                    string = string + 1;
                }

                /* Binary search for file by hash */
                size_t entry_id  = m_sfat->file_count / 2;
                u32 high_iter = m_sfat->file_count;
                u32 low_iter  = 0;
                u32 i = 0;
                u32 current_hash = m_sfat_entry_array[entry_id].file_name_hash;
                while (hash != current_hash) {

                    u32 result = entry_id;
                    i = low_iter;
                    if (hash < current_hash) {
                        i = high_iter;
                        high_iter = entry_id;
                        result = low_iter;
                    }

                    low_iter = result;
                    if (i == entry_id) {
                        return cInvalidEntryId;
                    }

                    i = high_iter + low_iter;

                    entry_id = i / 2;
                    current_hash = m_sfat_entry_array[entry_id].file_name_hash;
                }

                /* TODO Handle collisions */
                if (m_sfat_entry_array[entry_id].hash_collision_count != 1) {
                    return cInvalidEntryId;
                }

                return entry_id;
            }

            void *GetFileFast(u32 *out_file_size, u32 entry_id) {
                if (m_sfat->file_count < entry_id) { return nullptr; }

                const u32 start_offset = m_sfat_entry_array[entry_id].file_array_start_offset;

                if (out_file_size != nullptr) { *out_file_size = m_sfat_entry_array[entry_id].file_array_end_offset - start_offset; }

                return reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(m_file_array) + start_offset);
            }
            
            constexpr u32 GetFileCount() const { return m_sfat->file_count; }
    };
}
