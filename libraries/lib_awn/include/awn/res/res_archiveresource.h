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
        public:
            static constexpr inline const u32 cInvalidEntryIndex = 0xffff'ffff;
        public:
            constexpr ArchiveResource() : Resource() {/*...*/}
            virtual ~ArchiveResource() override {/*...*/}

            virtual u32 TryGetEntryIndex(const char *path) const { VP_UNUSED(path); return cInvalidEntryIndex; }

            virtual bool TryGetFileByIndex(ArchiveFileReturn *out_file_return, u32 index)       { VP_UNUSED(out_file_return, index); return false; }
            virtual bool TryGetFileByPath(ArchiveFileReturn *out_file_return, const char *path) { VP_UNUSED(out_file_return, path); return false; }

            virtual bool TryReadDirectoryEntryByIndex(DirectoryEntry *out_directory_entry, u32 index) { VP_UNUSED(out_directory_entry, index); return false; }

            virtual constexpr u32 GetFileCount() const { return 0; }
    };
}
