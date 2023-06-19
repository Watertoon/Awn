#pragma once

namespace awn::res {

    constexpr inline const u32 cInvalidAlignment = 0xffff'ffff;
    constexpr inline const u32 cInvalidSize      = 0xffff'ffff;
    struct ArchiveFileReturn {
        void           *file;
        u32             file_size;
        u32             decompressed_size;
        s32             alignment;
        CompressionType compression_type;
    };

    class ArchiveResource : public Resource {
        private:
        public:
            constexpr ArchiveResource() {/*...*/}

            virtual u32 TryGetEntryIndex(const char *path) const {/*...*/}

            virtual bool TryGetFileByIndex(ArchiveFileReturn *out_file_return, u32 index)       { return false; }
            virtual bool TryGetFileByPath(ArchiveFileReturn *out_file_return, const char *path) { return false; }

            virtual bool TryReadDirectoryEntryByIndex(DirectoryEntry *out_directory_entry, u32 index) { return false; }

            virtual constexpr u32 GetFileCount() const { return 0; }
    };
}
