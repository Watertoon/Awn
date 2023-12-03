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
        private:
            vp::res::SarcExtractor m_sarc_extractor;
        public:
            constexpr SarcArchiveResource() : ArchiveResource(), m_sarc_extractor() {/*...*/}
            constexpr virtual ~SarcArchiveResource() override {/*...*/}

            virtual bool Initialize(mem::Heap *heap, void *file, u32 file_size) override {
                VP_UNUSED(heap, file_size);
                return m_sarc_extractor.Initialize(file);
            }

            virtual u32 TryGetEntryIndex(const char *path) const override {
                return m_sarc_extractor.TryGetEntryIndexByPath(path);
            }

            virtual bool TryGetFileByIndex(ArchiveFileReturn *out_file_return, u32 entry_index) override {
                u32 out_size = 0;
                out_file_return->file                = m_sarc_extractor.TryGetFileByIndex(std::addressof(out_size), entry_index);
                out_file_return->file_size           = out_size;
                out_file_return->decompressed_size   = cInvalidSize;
                out_file_return->alignment           = cInvalidAlignment;
                out_file_return->compression_type    = CompressionType::None;
                return out_file_return->file != nullptr;
            }
            virtual bool TryGetFileByPath(ArchiveFileReturn *out_file_return, const char *path) override {
                u32 out_size = 0;
                out_file_return->file                = m_sarc_extractor.TryGetFileByPath(std::addressof(out_size), path);
                out_file_return->file_size           = out_size;
                out_file_return->decompressed_size   = cInvalidSize;
                out_file_return->alignment           = cInvalidAlignment;
                out_file_return->compression_type    = CompressionType::None;
                return out_file_return->file != nullptr;
            }

            virtual bool TryReadDirectoryEntryByIndex(DirectoryEntry *out_directory_entry, u32 index) override {

                if (out_directory_entry == nullptr) { return false; }
                if (m_sarc_extractor.GetFileCount() <= index)     { return false; }

                out_directory_entry->archive_entry_index = index;
                out_directory_entry->file_path           = m_sarc_extractor.TryGetPathByEntryIndex(index);

                return true;
            }

            virtual constexpr u32 GetFileCount() const override { return m_sarc_extractor.GetFileCount(); }
    };
}
