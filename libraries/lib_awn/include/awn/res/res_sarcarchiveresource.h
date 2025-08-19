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

    class SarcArchiveResource : public ArchiveResource {
        public:
            VP_RTTI_DERIVED(SarcArchiveResource, ArchiveResource);
        private:
            vp::res::SarcExtractor m_sarc_extractor;
        public:
            constexpr SarcArchiveResource() : ArchiveResource(), m_sarc_extractor() {/*...*/}
            virtual ~SarcArchiveResource() override {/*...*/}

            virtual Result OnFileLoad(mem::Heap *heap, mem::Heap *gpu_heap, void *file, size_t file_size) override;

            virtual u32 TryGetEntryIndex(const char *path) const override;

            virtual bool TryGetFileByIndex(ArchiveFileReturn *out_file_return, u32 entry_index) override;
            virtual bool TryGetFileByPath(ArchiveFileReturn *out_file_return, const char *path) override;

            virtual bool TryReadDirectoryEntryByIndex(DirectoryEntry *out_directory_entry, u32 index) override;

            virtual constexpr u32 GetFileCount() const override { return m_sarc_extractor.GetFileCount(); }
    };
}
