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
#include <awn.hpp>

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

	Result BeaArchiveResource::OnFileLoad(mem::Heap *heap, void *file, size_t file_size) {
		VP_UNUSED(heap, file_size);
		m_bea = vp::res::ResBea::ResCast(file);
		RESULT_RETURN_UNLESS(m_bea != nullptr, ResultInvalidFile);
		RESULT_RETURN_SUCCESS;
	}

	u32 BeaArchiveResource::TryGetEntryIndex(const char *path) const {
		return m_bea->file_dictionary->TryGetEntryIndexByKey(path);
	}

	bool BeaArchiveResource::TryGetFileByIndex(ArchiveFileReturn *out_file_return, u32 entry_index) {
		vp::res::ResBeaFileEntry *file_entry = m_bea->TryGetFileEntryByIndex(entry_index);
		if (file_entry == nullptr) { return false; }
		out_file_return->file                = m_bea->GetFileByEntry(file_entry);
		out_file_return->file_size           = file_entry->compressed_size;
		out_file_return->decompressed_size   = file_entry->uncompressed_size;
		out_file_return->alignment           = file_entry->alignment;
		out_file_return->compression_type    = BeaCompressionTypeToAwnCompressionType(vp::res::BeaCompressionType{file_entry->compression_type});
		return true;
	}
	bool BeaArchiveResource::TryGetFileByPath(ArchiveFileReturn *out_file_return, const char *path) {
		vp::res::ResBeaFileEntry *file_entry = m_bea->TryGetFileEntryByPath(path);
		if (file_entry == nullptr) { return false; }
		out_file_return->file              = m_bea->GetFileByEntry(file_entry);
		out_file_return->file_size         = file_entry->compressed_size;
		out_file_return->decompressed_size = file_entry->uncompressed_size;
		out_file_return->alignment         = file_entry->alignment;
		out_file_return->compression_type  = BeaCompressionTypeToAwnCompressionType(vp::res::BeaCompressionType{file_entry->compression_type});
		return true;
	}

	bool BeaArchiveResource::TryReadDirectoryEntryByIndex(DirectoryEntry *out_directory_entry, u32 index) {

		if (out_directory_entry == nullptr) { return false; }
		if (m_bea->file_count <= index)     { return false; }

		out_directory_entry->archive_entry_index = index;
		out_directory_entry->file_path           = m_bea->GetFilePathByIndex(index)->GetString();

		return true;
	}
}
