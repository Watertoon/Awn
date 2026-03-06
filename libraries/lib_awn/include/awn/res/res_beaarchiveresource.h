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

    class BeaArchiveResource : public ArchiveResource {
        public:
            VP_RTTI_DERIVED(BeaArchiveResource, ArchiveResource);
        private:
            vp::res::ResBea *m_bea;
        public:
            constexpr BeaArchiveResource() : ArchiveResource(), m_bea(nullptr) {/*...*/}
            virtual ~BeaArchiveResource() override {/*...*/}

            virtual Result OnFileLoad(mem::Heap *heap, void *file, size_t file_size) override;

            virtual u32 TryGetEntryIndex(const char *path) const override;

            virtual bool TryGetFileByIndex(ArchiveFileReturn *out_file_return, u32 entry_index) override;
            virtual bool TryGetFileByPath(ArchiveFileReturn *out_file_return, const char *path) override;
            
            virtual bool TryReadDirectoryEntryByIndex(DirectoryEntry *out_directory_entry, u32 index) override;

            virtual constexpr u32 GetFileCount() const override { return m_bea->file_count; }
    };
}
