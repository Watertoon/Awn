#pragma once

namespace vp::res {

    enum class BeaCompressionType : u16 {
        None = 0,
        Zlib = 1,
        Zstd = 2,
    };

    struct ResBeaFileEntry : public ResNintendoWareSubHeader {
        u16     compression_type; /* No longer compression type? */
        u16     alignment_pow2;
        u32     compressed_size;
        u32     decompressed_size;
        u32     magic0;
        u32     magic1;
        u32     reserve0;
        size_t  file_offset;
        char   *file_name;

        static constexpr inline u32 cMagic = util::TCharCode32("ASST");
    };
    static_assert(sizeof(ResBeaFileEntry) == 0x38);

    struct ResBea : public ResNintendoWareFileHeader {
        u32                         file_count;
        u32                         reserve0;
        ResBeaFileEntry            *file_entry_head;
        ResBeaFileEntry           **file_entry_array;
        ResNintendoWareDictionary  *file_dictionary;
        const char                 *archive_name;
        const char                 *compression_name;
        const char                **reserve1;

        static ResBea *ResCast(void *file) {
            ResBea *bea = reinterpret_cast<ResBea*>(file);
            if (bea == nullptr || bea->ResNintendoWareFileHeader::IsValid(cMagic, cTargetMajorVersion, cTargetMinorVersion) == false) { return nullptr; }
            bea->Relocate();
            return bea;
        }

        static bool IsValid(void *file) {
            ResBea *bea = reinterpret_cast<ResBea*>(file);
            return bea->ResNintendoWareFileHeader::IsValid(cMagic, cTargetMajorVersion, cTargetMinorVersion, 0);
        }

        ALWAYS_INLINE ResBeaFileEntry *TryGetFileEntryByPath(const char *file_path) {
            const u32 entry_index = file_dictionary->TryGetEntryIndexByKey(file_path);
            if (entry_index == ResNintendoWareDictionary::cInvalidEntryIndex) { return nullptr; }

            return file_entry_array[entry_index];
        }
        constexpr ALWAYS_INLINE ResBeaFileEntry *TryGetFileEntryByIndex(u32 entry_index) {
            if (file_count <= entry_index) { return nullptr; }
            return file_entry_array[entry_index];
        }

        ALWAYS_INLINE void *GetFileByEntry(ResBeaFileEntry *file_entry) {
            return reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(this) + file_entry->file_offset);
        }

        ALWAYS_INLINE const char *GetFilePathByIndex(u32 entry_index) const {
            return file_dictionary->GetKeyByEntryIndex(entry_index);
        }

        static constexpr inline u64 cMagic              = util::TCharCode32("SCNE");
        static constexpr inline u32 cTargetMajorVersion = 5;
        static constexpr inline u32 cTargetMinorVersion = 0;
    };
    static_assert(sizeof(ResBea) == 0x58);
}

