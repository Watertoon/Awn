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

        constexpr bool IsValid() const {

            if (magic       != cMagic)           { return false; }
            if (header_size != sizeof(ResSarc))  { return false; }
            if (endianess   != ByteOrder_Little) { return false; }
            if (version     != cTargetVersion)   { return false; }
            return true;
        }
    };
    static_assert(sizeof(ResSarc) == 0x14);

    struct ResSarcSfat {
        u32 magic;
        u16 header_size;
        u16 file_count;
        u32 hash_seed;

        static constexpr u32 cMagic = util::TCharCode32("SFAT");

        constexpr bool IsValid() const {

            if (magic       != cMagic)          { return false; }
            if (header_size != sizeof(ResSarcSfat)) { return false; }
            if ((file_count >> 14) != 0)        { return false; }
            return true;
        }
    };
    static_assert(sizeof(ResSarcSfat) == 0xc);

    struct ResSarcSfatEntry {
        u32 file_name_hash;
        u32 file_name_offset     : 24;
        u32 hash_collision_count : 8;
        u32 file_array_start_offset;
        u32 file_array_end_offset;
    };
    static_assert(sizeof(ResSarcSfatEntry) == 0x10);

    struct ResSarcSfnt {
        u32 magic;
        u16 header_size;
        u16 padding;

        static constexpr u32 cMagic = util::TCharCode32("SFNT");

        constexpr bool IsValid() const {

            if (magic       != cMagic)              { return false; }
            if (header_size != sizeof(ResSarcSfnt)) { return false; }
            return true;
        }
    };
    static_assert(sizeof(ResSarcSfnt) == 0x8);

    class SarcExtractor {
        public:
            static constexpr u32 cInvalidEntryIndex = 0xFFFF'FFFF;
        private:
            ResSarc          *m_sarc;
            ResSarcSfat      *m_sfat;
            ResSarcSfatEntry *m_sfat_entry_array;
            void             *m_file_region;
            char             *m_path_table;
        public:
            constexpr SarcExtractor() : m_sarc(nullptr), m_sfat(nullptr), m_sfat_entry_array(nullptr), m_file_region(nullptr), m_path_table(nullptr) {/*...*/}
            constexpr ~SarcExtractor() {/*...*/}

            bool Initialize(void *sarc_file) {

                /* Integrity check pointer */
                if (sarc_file == nullptr) { return false; }

                /* Cast sarc */
                m_sarc = reinterpret_cast<ResSarc*>(sarc_file);

                /* Validate sarc */
                if (m_sarc->IsValid() == false) { return false; }

                /* Get sfat */
                m_sfat = reinterpret_cast<ResSarcSfat*>(reinterpret_cast<uintptr_t>(sarc_file) + sizeof(ResSarc));

                /* Validate sfat */
                if (m_sfat->IsValid() == false) { return false; }

                /* Get sfat file entry array */
                m_sfat_entry_array = reinterpret_cast<ResSarcSfatEntry*>(reinterpret_cast<uintptr_t>(sarc_file) + sizeof(ResSarc) + sizeof(ResSarcSfat));

                /* Get sfnt */
                ResSarcSfnt *sfnt = reinterpret_cast<ResSarcSfnt*>(reinterpret_cast<uintptr_t>(sarc_file) + sizeof(ResSarc) + sizeof(ResSarcSfat) + sizeof(ResSarcSfatEntry) * m_sfat->file_count);

                /* Validate sfnt */
                if (sfnt->IsValid() == false) { return false; }

                /* Get file path table */
                m_path_table = reinterpret_cast<char*>(reinterpret_cast<uintptr_t>(sfnt) + sfnt->header_size);

                /* Get file region */
                m_file_region = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(sarc_file) + m_sarc->file_array_offset);
                
                return true;
            }

            constexpr const char *TryGetPathByEntryIndex(u32 entry_id) const {
                return m_path_table + (m_sfat_entry_array[entry_id].file_name_offset * 4);
            }

            constexpr u32 TryGetEntryIndexByPath(const char *path) const {

                /* Calculate file path hash */
                const u32 hash_seed = m_sfat->hash_seed;
                u32 hash = 0;
                while (*path != '\0') {
                    hash = hash * hash_seed + static_cast<int>(*path);
                    path = path + 1;
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
                        return cInvalidEntryIndex;
                    }

                    i = high_iter + low_iter;

                    entry_id = i / 2;
                    current_hash = m_sfat_entry_array[entry_id].file_name_hash;
                }

                /* TODO Handle collisions */
                if (m_sfat_entry_array[entry_id].hash_collision_count != 1) {
                    return cInvalidEntryIndex;
                }

                return entry_id;
            }

            void *TryGetFileByIndex(u32 *out_file_size, u32 entry_id) {

                /* Integrity check bounds */
                if (m_sfat->file_count < entry_id) { return nullptr; }

                /* Get offset into file region */
                const u32 start_offset = m_sfat_entry_array[entry_id].file_array_start_offset;

                /* Return file size if necessary */
                if (out_file_size != nullptr) { *out_file_size = m_sfat_entry_array[entry_id].file_array_end_offset - start_offset; }

                /* Return file pointer */
                return reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(m_file_region) + start_offset);
            }

            void *TryGetFileByPath(u32 *out_file_size, const char *path) {

                /* Get entry index */
                const u32 entry_index = this->TryGetEntryIndexByPath(path);

                /* Try get file and file size */
                return this->TryGetFileByIndex(out_file_size, entry_index);
            }
            
            constexpr ALWAYS_INLINE u32 GetFileCount() const { return m_sfat->file_count; }
    };
}
