#pragma once

namespace vp::res {

    enum class BeaCompressionType : u16 {
        None      = 0,
        Zlib      = 1,
        Zstandard = 2,
    };

    struct ResBeaFileEntry : public ResNintendoWareSubHeader {
        u16     compression_type;
        u16     alignment;
        u32     compressed_size;
        u32     uncompressed_size;
        u32     reserve0;
        size_t  file_offset;
        char   *file_name;

        static constexpr inline u32 cMagic = util::TCharCode32("ASST");
    };
    static_assert(sizeof(ResBeaFileEntry) == 0x30);

    struct ResBea : public ResNintendoWareFileHeader {
        s64                         file_count;
        ResBeaFileEntry           **file_entry_array;
        ResNintendoWareDictionary  *file_dictionary;
        u64                         reserve0;
        char                       *archive_name;

        static ResBea *ResCast(void *file) {
            ResBea *bea = reinterpret_cast<ResBea*>(file);
            if (bea == nullptr || bea->ResNintendoWareFileHeader::IsValid(cMagic, 1, 1) == false) { return nullptr; }
            bea->Relocate();
            return bea;
        }

        static bool IsValid(void *file) {
            ResBea *bea = reinterpret_cast<ResBea*>(file);
            return bea->ResNintendoWareFileHeader::IsValid(cMagic, 1, 1, 0);
        }

        constexpr ALWAYS_INLINE ResBeaFileEntry *TryGetFileEntryByPath(const char *file_path) {
            const s32 entry_index = file_dictionary->FindEntryIndex(file_path);
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

        static constexpr inline u64 cMagic = util::TCharCode32("SCNE");
    };
    static_assert(sizeof(ResBea) == 0x48);
}

