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
            constexpr ArchiveResource() : Resource() {/*...*/}
            constexpr virtual ~ArchiveResource() override {/*...*/}

            virtual u32 TryGetEntryIndex(const char *path) const { VP_UNUSED(path); return cInvalidEntryIndex; }

            virtual bool TryGetFileByIndex(ArchiveFileReturn *out_file_return, u32 index)       { VP_UNUSED(out_file_return, index); return false; }
            virtual bool TryGetFileByPath(ArchiveFileReturn *out_file_return, const char *path) { VP_UNUSED(out_file_return, path); return false; }

            virtual bool TryReadDirectoryEntryByIndex(DirectoryEntry *out_directory_entry, u32 index) { VP_UNUSED(out_directory_entry, index); return false; }

            virtual constexpr u32 GetFileCount() const { return 0; }
    };
}
