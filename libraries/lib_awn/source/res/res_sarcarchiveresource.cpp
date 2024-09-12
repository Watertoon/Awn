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

	Result SarcArchiveResource::OnFileLoad(mem::Heap *heap, mem::Heap *gpu_heap, void *file, size_t file_size) {
		VP_UNUSED(heap, gpu_heap, file_size);
		RESULT_RETURN_IF(!m_sarc_extractor.Initialize(file), ResultInvalidFile);
		RESULT_RETURN_SUCCESS;
	}

	u32 SarcArchiveResource::TryGetEntryIndex(const char *path) const {
		return m_sarc_extractor.TryGetEntryIndexByPath(path);
	}

	bool SarcArchiveResource::TryGetFileByIndex(ArchiveFileReturn *out_file_return, u32 entry_index) {
		u32 out_size = 0;
		out_file_return->file                = m_sarc_extractor.TryGetFileByIndex(std::addressof(out_size), entry_index);
		out_file_return->file_size           = out_size;
		out_file_return->decompressed_size   = cInvalidSize;
		out_file_return->alignment           = cInvalidAlignment;
		out_file_return->compression_type    = CompressionType::None;
		return out_file_return->file != nullptr;
	}
	bool SarcArchiveResource::TryGetFileByPath(ArchiveFileReturn *out_file_return, const char *path) {
		u32 out_size = 0;
		out_file_return->file                = m_sarc_extractor.TryGetFileByPath(std::addressof(out_size), path);
		out_file_return->file_size           = out_size;
		out_file_return->decompressed_size   = cInvalidSize;
		out_file_return->alignment           = cInvalidAlignment;
		out_file_return->compression_type    = CompressionType::None;
		return out_file_return->file != nullptr;
	}

	bool SarcArchiveResource::TryReadDirectoryEntryByIndex(DirectoryEntry *out_directory_entry, u32 index) {

		if (out_directory_entry == nullptr) { return false; }
		if (m_sarc_extractor.GetFileCount() <= index)     { return false; }

		out_directory_entry->archive_entry_index = index;
		out_directory_entry->file_path           = m_sarc_extractor.TryGetPathByEntryIndex(index);

		return true;
	}
}
