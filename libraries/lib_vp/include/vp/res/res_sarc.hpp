#pragma once

namespace vp::res {

    struct ResSarcSfatEntry {
        u32 file_name_hash;
        union {
            u32 name_data;
            struct {
                u32 file_name_offset     : 24;
                u32 hash_collision_index : 8;
            };
        };
        u32 file_array_start_offset;
        u32 file_array_end_offset;
    };
    static_assert(sizeof(ResSarcSfatEntry) == 0x10);

    struct ResSarcSfat {
        u32              magic;
        u16              header_size;
        u16              file_count;
        u32              hash_seed;
        ResSarcSfatEntry entry_array[];

        static constexpr u32 cMagic = util::TCharCode32("SFAT");

        constexpr bool IsValid(bool is_reverse_endian) const {

            const u32 r_magic       = (is_reverse_endian == false) ? magic       : vp::util::SwapEndian(magic);
            const u16 r_header_size = (is_reverse_endian == false) ? header_size : vp::util::SwapEndian(header_size);
            const u16 r_file_count  = (is_reverse_endian == false) ? file_count  : vp::util::SwapEndian(file_count);
            if (r_magic       != cMagic)              { return false; }
            if (r_header_size != sizeof(ResSarcSfat)) { return false; }
            if ((r_file_count >> 14) != 0)            { return false; }

            return true;
        }
    };
    static_assert(sizeof(ResSarcSfat) == 0xc);

    struct ResSarcSfnt {
        u32 magic;
        u16 header_size;
        u16 reserve0;

        static constexpr u32 cMagic = util::TCharCode32("SFNT");

        constexpr bool IsValid(bool is_reverse_endian) const {

            const u32 r_magic       = (is_reverse_endian == false) ? magic       : vp::util::SwapEndian(magic);
            const u16 r_header_size = (is_reverse_endian == false) ? header_size : vp::util::SwapEndian(header_size);
            if (r_magic       != cMagic)              { return false; }
            if (r_header_size != sizeof(ResSarcSfnt)) { return false; }

            return true;
        }
    };
    static_assert(sizeof(ResSarcSfnt) == 0x8);

    enum ByteOrder {
        ByteOrder_Reverse = 0xFFFE,
        ByteOrder_Native  = 0xFEFF
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

        constexpr bool IsReverseEndian() const { return endianess == ByteOrder_Reverse; }

        constexpr bool IsValid() const {

            const bool is_reverse_endian = this->IsReverseEndian();
            const u32 r_magic            = (is_reverse_endian == false) ? magic       : vp::util::SwapEndian(magic);
            const u16 r_header_size      = (is_reverse_endian == false) ? header_size : vp::util::SwapEndian(header_size);
            const u16 r_version          = (is_reverse_endian == false) ? version     : vp::util::SwapEndian(version);
            if (r_magic       != cMagic)          { return false; }
            if (r_header_size != sizeof(ResSarc)) { return false; }
            if (r_version     != cTargetVersion)  { return false; }

            return true;
        }

        ALWAYS_INLINE ResSarcSfat *GetSfat() {
            return reinterpret_cast<ResSarcSfat*>(reinterpret_cast<uintptr_t>(this) + sizeof(ResSarc));
        }

        void SwapEndian() {

            /* Swap header */
            magic               = vp::util::SwapEndian(magic);
            header_size         = vp::util::SwapEndian(header_size);
            endianess           = vp::util::SwapEndian(endianess);
            file_size           = vp::util::SwapEndian(file_size);
            file_array_offset   = vp::util::SwapEndian(file_array_offset);
            version             = vp::util::SwapEndian(version);

            /* Swap sfat */
            ResSarcSfat *sfat = this->GetSfat();

            sfat->magic        = vp::util::SwapEndian(sfat->magic);
            sfat->header_size  = vp::util::SwapEndian(sfat->header_size);
            sfat->file_count   = vp::util::SwapEndian(sfat->file_count);
            sfat->hash_seed    = vp::util::SwapEndian(sfat->hash_seed);

            for (u32 i = 0; i < sfat->file_count; ++i) {
                sfat->entry_array[i].file_name_hash          = vp::util::SwapEndian(sfat->entry_array[i].file_name_hash);
                sfat->entry_array[i].name_data               = vp::util::SwapEndian(sfat->entry_array[i].name_data);
                sfat->entry_array[i].file_array_start_offset = vp::util::SwapEndian(sfat->entry_array[i].file_array_start_offset);
                sfat->entry_array[i].file_array_end_offset   = vp::util::SwapEndian(sfat->entry_array[i].file_array_end_offset);
            }

            /* Swap sfnt */
            ResSarcSfnt *sfnt = reinterpret_cast<ResSarcSfnt*>(reinterpret_cast<uintptr_t>(this) + sizeof(ResSarc) + sizeof(ResSarcSfat) + sizeof(ResSarcSfatEntry) * sfat->file_count);
            
            sfnt->magic        = vp::util::SwapEndian(sfat->magic);
            sfnt->header_size  = vp::util::SwapEndian(sfat->header_size);

            return;
        }
    };
    static_assert(sizeof(ResSarc) == 0x14);

    class SarcExtractor {
        public:
            static constexpr u32 cInvalidEntryIndex = 0xFFFF'FFFF;
        private:
            ResSarc          *m_sarc;
            ResSarcSfat      *m_sfat;
            void             *m_file_region;
            char             *m_path_table;
        public:
            constexpr  SarcExtractor() : m_sarc(nullptr), m_sfat(nullptr), m_file_region(nullptr), m_path_table(nullptr) {/*...*/}
            constexpr ~SarcExtractor() {/*...*/}

            bool Initialize(void *sarc_file) {

                /* Integrity check pointer */
                if (sarc_file == nullptr) { return false; }

                /* Cast sarc */
                m_sarc = reinterpret_cast<ResSarc*>(sarc_file);

                /* Validate sarc */
                if (m_sarc->IsValid() == false) { return false; }

                /* Endianess check */
                const bool is_reverse_endian = m_sarc->IsReverseEndian();

                /* Get sfat */
                m_sfat = reinterpret_cast<ResSarcSfat*>(reinterpret_cast<uintptr_t>(sarc_file) + sizeof(ResSarc));

                /* Validate sfat */
                if (m_sfat->IsValid(is_reverse_endian) == false) { return false; }

                /* Get sfnt */
                const u32 file_count = (is_reverse_endian == false) ? m_sfat->file_count : vp::util::SwapEndian(m_sfat->file_count);
                ResSarcSfnt *sfnt = reinterpret_cast<ResSarcSfnt*>(reinterpret_cast<uintptr_t>(sarc_file) + sizeof(ResSarc) + sizeof(ResSarcSfat) + sizeof(ResSarcSfatEntry) * file_count);

                /* Validate sfnt */
                if (sfnt->IsValid(is_reverse_endian) == false) { return false; }

                /* Get file path table */
                const u16 sfnt_header_size = (is_reverse_endian == false) ? sfnt->header_size : vp::util::SwapEndian(sfnt->header_size);
                m_path_table = reinterpret_cast<char*>(reinterpret_cast<uintptr_t>(sfnt) + sfnt_header_size);

                /* Get file region */
                const u32 file_array_offset = (is_reverse_endian == false) ? m_sarc->file_array_offset : vp::util::SwapEndian(m_sarc->file_array_offset);
                m_file_region = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(sarc_file) + file_array_offset);

                return true;
            }

            constexpr const char *TryGetPathByEntryIndex(u32 entry_index) const {

                /* Endianess check */
                const bool is_reverse_endian = m_sarc->IsReverseEndian();

                /* Integrity check file count */
                const u32 file_count = (is_reverse_endian == false) ? m_sfat->file_count : vp::util::SwapEndian(m_sfat->file_count);
                if (file_count <= entry_index) { return nullptr; }

                /* Integrity check name offset */
                const u32 file_name_offset = ((is_reverse_endian == false) ? m_sfat->entry_array[entry_index].name_data : vp::util::SwapEndian(m_sfat->entry_array[entry_index].name_data)) & 0xff'ffff;
                if (m_file_region < m_path_table + file_name_offset) { return nullptr; }

                /* Get file path */
                return m_path_table + (file_name_offset << 2);
            }

            constexpr u32 TryGetEntryIndexByPath(const char *path) const {

                /* Endianess check */
                const bool is_reverse_endian = m_sarc->IsReverseEndian();

                /* Error on null file count */
                const u32 file_count = (is_reverse_endian == false) ? m_sfat->file_count : vp::util::SwapEndian(m_sfat->file_count);
                if (file_count == 0) { return cInvalidEntryIndex; }

                /* Calculate file path hash */
                const u32 hash_seed = (is_reverse_endian == false) ? m_sfat->hash_seed : vp::util::SwapEndian(m_sfat->hash_seed);
                u32 hash = 0;
                while (*path != '\0') {
                    hash = hash * hash_seed + static_cast<int>(*path);
                    path = path + 1;
                }

                /* Binary search for file by hash */
                size_t entry_index  = file_count / 2;
                u32 high_iter    = file_count;
                u32 low_iter     = 0;
                u32 i            = 0;
                u32 current_hash = (is_reverse_endian == false) ? m_sfat->entry_array[entry_index].file_name_hash : vp::util::SwapEndian(m_sfat->entry_array[entry_index].file_name_hash);
                while (hash != current_hash) {

                    u32 result = entry_index;
                    i = low_iter;
                    if (hash < current_hash) {
                        i = high_iter;
                        high_iter = entry_index;
                        result = low_iter;
                    }

                    low_iter = result;
                    if (i == entry_index) { return cInvalidEntryIndex; }

                    i = high_iter + low_iter;

                    entry_index = i / 2;
                    current_hash = (is_reverse_endian == false) ? m_sfat->entry_array[entry_index].file_name_hash : vp::util::SwapEndian(m_sfat->entry_array[entry_index].file_name_hash);
                }

                /* Handle collisions */
                const u32 root_name_data = (is_reverse_endian == false) ? m_sfat->entry_array[entry_index].name_data : vp::util::SwapEndian(m_sfat->entry_array[entry_index].name_data);
                const u32 root_collision_index = (root_name_data >> 0x18);
                if (root_collision_index != 1) {

                    /* Walk back to first entry with hash */
                    entry_index = entry_index - root_collision_index + 1;

                    /* Linear search collisions */
                    while (entry_index < file_count) {

                        /* Check whether the hashes still match */
                        const u32 entry_hash = (is_reverse_endian == false) ? m_sfat->entry_array[entry_index].file_name_hash : vp::util::SwapEndian(m_sfat->entry_array[entry_index].file_name_hash);
                        if (entry_hash != hash) { return cInvalidEntryIndex; }

                        /* Check whether the path is valid */
                        const u32 collision_entry_data  = (is_reverse_endian == false) ? m_sfat->entry_array[entry_index].name_data : vp::util::SwapEndian(m_sfat->entry_array[entry_index].name_data);
                        const u32 collision_name_offset = collision_entry_data & 0xff'ffff;
                        const char *file_path = m_path_table + (collision_name_offset << 2);
                        if (m_file_region < file_path) { return cInvalidEntryIndex; }

                        /* Check whether the paths match */
                        if (::strcmp(path, file_path) == 0) { return entry_index; }

                        ++entry_index;
                    }
                }

                return entry_index;
            }

            void *TryGetFileByIndex(u32 *out_file_size, u32 entry_index) {

                /* Endianess check */
                const bool is_reverse_endian = m_sarc->IsReverseEndian();

                /* Integrity check bounds */
                const u32 file_count = (is_reverse_endian == false) ? m_sfat->file_count : vp::util::SwapEndian(m_sfat->file_count);
                if (file_count < entry_index) { return nullptr; }

                /* Get offset into file region */
                const u32 start_offset = (is_reverse_endian == false) ? m_sfat->entry_array[entry_index].file_array_start_offset : vp::util::SwapEndian(m_sfat->entry_array[entry_index].file_array_start_offset);

                /* Return file size if necessary */
                if (out_file_size != nullptr) {
                    const u32 file_array_end_offset = (is_reverse_endian == false) ? m_sfat->entry_array[entry_index].file_array_end_offset : vp::util::SwapEndian(m_sfat->entry_array[entry_index].file_array_end_offset);
                    *out_file_size = file_array_end_offset - start_offset; 
                }

                /* Return file pointer */
                return reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(m_file_region) + start_offset);
            }

            void *TryGetFileByPath(u32 *out_file_size, const char *path) {

                /* Get entry index */
                const u32 entry_index = this->TryGetEntryIndexByPath(path);

                /* Try get file and file size */
                return this->TryGetFileByIndex(out_file_size, entry_index);
            }
            
            constexpr ALWAYS_INLINE u32 GetFileCount() const {
                const bool is_reverse_endian = m_sarc->IsReverseEndian();
                return (is_reverse_endian == false) ? m_sfat->file_count : vp::util::SwapEndian(m_sfat->file_count); 
            }
    };
}
