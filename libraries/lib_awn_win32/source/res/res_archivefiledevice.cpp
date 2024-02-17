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

    Result ArchiveFileDevice::OpenFileImpl(FileHandle *out_file_handle, const char *path, OpenMode open_mode) {

        /* Integrity checks */
        RESULT_RETURN_UNLESS(out_file_handle != nullptr,                     ResultNullFileHandle);
        RESULT_RETURN_UNLESS(path != nullptr,                                ResultNullPath);
        RESULT_RETURN_UNLESS((open_mode & OpenMode::Read) != OpenMode::Read, ResultInvalidOpenMode);

        /* Get entry index */
        out_file_handle->archive_entry_index = m_archive_resource->TryGetEntryIndex(path);
        RESULT_RETURN_IF(out_file_handle->archive_entry_index == cInvalidEntryIndex, ResultPathNotFound);

        /* Set file handle and size */
        ArchiveFileReturn archive_file_return = {};
        const bool result = m_archive_resource->TryGetFileByIndex(std::addressof(archive_file_return), out_file_handle->archive_entry_index);
        RESULT_RETURN_IF(result == false, ResultFailedToOpenFile);

        out_file_handle->handle    = archive_file_return.file;
        out_file_handle->file_size = archive_file_return.file_size;

        RESULT_RETURN_SUCCESS;
    }
    Result ArchiveFileDevice::CloseFileImpl(FileHandle *file_handle) {

        /* Integrity checks */
        RESULT_RETURN_UNLESS(file_handle != nullptr,           ResultNullFileHandle);
        RESULT_RETURN_UNLESS(file_handle->handle != nullptr,   ResultInvalidFileHandle);
        RESULT_RETURN_UNLESS(file_handle->file_device != this, ResultInvalidFileHandle);

        /* Clear handle state */
        file_handle->handle              = nullptr;
        file_handle->archive_entry_index = cInvalidEntryIndex;

        RESULT_RETURN_SUCCESS;
    }
    Result ArchiveFileDevice::ReadFileImpl(void *read_buffer, size_t *out_read_size, FileHandle *file_handle, size_t read_size, size_t file_offset) {

        /* Integrity checks */
        RESULT_RETURN_UNLESS(file_handle != nullptr,           ResultNullFileHandle);
        RESULT_RETURN_UNLESS(file_handle->handle != nullptr,   ResultInvalidFileHandle);
        RESULT_RETURN_UNLESS(file_handle->file_device != this, ResultInvalidFileHandle);

        /* Copy file data */
        ::memcpy(read_buffer, reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(file_handle->handle) + file_offset), read_size);
        if (out_read_size != nullptr) {
            *out_read_size = read_size;
        }

        RESULT_RETURN_SUCCESS;
    }

    Result ArchiveFileDevice::OpenDirectoryImpl(DirectoryHandle *out_directory_handle, const char *path) {

        /* Integrity checks */
        RESULT_RETURN_UNLESS(path != nullptr,                  ResultNullPath);
        RESULT_RETURN_UNLESS(out_directory_handle != nullptr,  ResultNullDirectoryHandle);

        const char character = *path;
        RESULT_RETURN_IF(character != '\0' && character != '/', ResultDirectoryNotFound);

        /* Open directory */
        out_directory_handle->entry_index = 0;

        RESULT_RETURN_SUCCESS;
    }
    Result ArchiveFileDevice::CloseDirectoryImpl(DirectoryHandle *directory_handle) {
        RESULT_RETURN_UNLESS(directory_handle != nullptr, ResultNullDirectoryHandle);
        directory_handle->entry_index = 0;
        RESULT_RETURN_SUCCESS;
    }
    Result ArchiveFileDevice::ReadDirectoryImpl(DirectoryHandle *directory_handle, DirectoryEntry *entry_array, u32 entry_count) {
        
        /* Integrity checks */
        RESULT_RETURN_UNLESS(directory_handle != nullptr, ResultNullDirectoryHandle);

        /* Calculate directory count to read */
        const u32 file_count = m_archive_resource->GetFileCount();
        u32 end_offset = directory_handle->entry_index + entry_count;
        if (file_count <= end_offset) {
            end_offset = file_count;
        }

        /* Read directory entries */
        u32 iter_index = directory_handle->entry_index;
        for (u32 i = 0; iter_index < end_offset; ++iter_index) {
            const bool result = m_archive_resource->TryReadDirectoryEntryByIndex(std::addressof(entry_array[i]), iter_index);
            RESULT_RETURN_IF(result == false, ResultDirectoryExhausted);
            ++i;
        }

        RESULT_RETURN_IF(iter_index == file_count, ResultDirectoryExhausted);
        RESULT_RETURN_SUCCESS;
    }

    Result ArchiveFileDevice::CheckDirectoryExistsImpl(const char *path) {
        const char character = *path;
        RESULT_RETURN_IF(character != '\0' && character != '/', ResultDirectoryNotFound);
        RESULT_RETURN_SUCCESS;
    }

    Result ArchiveFileDevice::GetFileSizeImpl(size_t *out_size, FileHandle *file_handle) {

        /* Integrity checks */
        RESULT_RETURN_UNLESS(file_handle != nullptr, ResultNullFileHandle);

        /* Find file */
        ArchiveFileReturn file_return = {};
        const bool result = m_archive_resource->TryGetFileByIndex(std::addressof(file_return), file_handle->archive_entry_index); 
        RESULT_RETURN_IF(result == false, ResultFileNotFound);

        /* Set output size */
        if (out_size != nullptr) {
            *out_size = file_return.file_size;
        }

        RESULT_RETURN_SUCCESS;
    }
    Result ArchiveFileDevice::GetFileSizeImpl(size_t *out_size, const char *path) {

        /* Integrity checks */
        RESULT_RETURN_UNLESS(path != nullptr, ResultNullFileHandle);

        /* Find file */
        ArchiveFileReturn file_return = {};
        const bool result = m_archive_resource->TryGetFileByPath(std::addressof(file_return), path); 
        RESULT_RETURN_IF(result == false, ResultFileNotFound);

        /* Set output size */
        if (out_size != nullptr) {
            *out_size = file_return.file_size;
        }

        RESULT_RETURN_SUCCESS;
    }

    Result ArchiveFileDevice::CheckFileExistsImpl(const char *path) {
        RESULT_RETURN_IF(m_archive_resource->TryGetEntryIndex(path) == cInvalidEntryIndex, ResultFileNotFound);
        RESULT_RETURN_SUCCESS;
    }

    Result ArchiveFileDevice::FormatPath(vp::util::FixedString<vp::util::cMaxPath> *out_formatted_path, const char *path) {
        *out_formatted_path = path;
        RESULT_RETURN_SUCCESS;
    }

    Result ArchiveFileDevice::GetFileReferenceImpl(void **out_file, size_t *out_size, const char *file_path) {

        /* Integrity checks */
        RESULT_RETURN_UNLESS(file_path != nullptr, ResultNullPath);

        ArchiveFileReturn file_return = {};
        const bool result = m_archive_resource->TryGetFileByPath(std::addressof(file_return), file_path);
        RESULT_RETURN_IF(result == false, ResultFileNotFound);

        if (out_file != nullptr) {
            *out_file = file_return.file;
        }
        if (out_size != nullptr) {
            *out_size = file_return.file_size;
        }

        RESULT_RETURN_SUCCESS;
    }

    Result ArchiveFileDevice::GetFileReference(void **out_file, size_t *out_size, const char *file_path) {
        return this->GetFileReferenceImpl(out_file, out_size, file_path);
    }
}
