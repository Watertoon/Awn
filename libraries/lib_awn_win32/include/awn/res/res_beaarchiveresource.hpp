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
    
    namespace {

        constexpr ALWAYS_INLINE CompressionType BeaCompressionTypeToAwnCompressionType(vp::res::BeaCompressionType compression_type) {
            switch (compression_type) {
                case vp::res::BeaCompressionType::None:
                    return CompressionType::None;
                case vp::res::BeaCompressionType::Zstandard:
                    return CompressionType::Zstandard;
                default:
                    VP_ASSERT(compression_type != vp::res::BeaCompressionType::Zlib);
                    break;
            }
            return CompressionType::Auto;
        }
    }

    class BeaArchiveResource : public ArchiveResource {
        private:
            vp::res::ResBea *m_bea;
        public:
            constexpr BeaArchiveResource() : ArchiveResource(), m_bea(nullptr) {/*...*/}
            constexpr virtual ~BeaArchiveResource() override {/*...*/}

            virtual Result OnFileLoad(mem::Heap *heap, void *file, size_t file_size) override {
                VP_UNUSED(heap, file_size);
                m_bea = vp::res::ResBea::ResCast(file);
                RESULT_RETURN_UNLESS(m_bea != nullptr, ResultInvalidFile);
                RESULT_RETURN_SUCCESS;
            }

            virtual u32 TryGetEntryIndex(const char *path) const override {
                return m_bea->file_dictionary->TryGetEntryIndexByKey(path);
            }

            virtual bool TryGetFileByIndex(ArchiveFileReturn *out_file_return, u32 entry_index) override {
                vp::res::ResBeaFileEntry *file_entry = m_bea->TryGetFileEntryByIndex(entry_index);
                if (file_entry == nullptr) { return false; }
                out_file_return->file                = m_bea->GetFileByEntry(file_entry);
                out_file_return->file_size           = file_entry->compressed_size;
                out_file_return->decompressed_size   = file_entry->uncompressed_size;
                out_file_return->alignment           = file_entry->alignment;
                out_file_return->compression_type    = BeaCompressionTypeToAwnCompressionType(vp::res::BeaCompressionType{file_entry->compression_type});
                return true;
            }
            virtual bool TryGetFileByPath(ArchiveFileReturn *out_file_return, const char *path) override {
                vp::res::ResBeaFileEntry *file_entry = m_bea->TryGetFileEntryByPath(path);
                if (file_entry == nullptr) { return false; }
                out_file_return->file              = m_bea->GetFileByEntry(file_entry);
                out_file_return->file_size         = file_entry->compressed_size;
                out_file_return->decompressed_size = file_entry->uncompressed_size;
                out_file_return->alignment         = file_entry->alignment;
                out_file_return->compression_type  = BeaCompressionTypeToAwnCompressionType(vp::res::BeaCompressionType{file_entry->compression_type});
                return true;
            }
            
            virtual bool TryReadDirectoryEntryByIndex(DirectoryEntry *out_directory_entry, u32 index) override {

                if (out_directory_entry == nullptr) { return false; }
                if (m_bea->file_count <= index)     { return false; }

                out_directory_entry->archive_entry_index = index;
                out_directory_entry->file_path           = m_bea->GetFilePathByIndex(index) + 2;

                return true;
            }

            virtual constexpr u32 GetFileCount() const override { return m_bea->file_count; }
    };
}
