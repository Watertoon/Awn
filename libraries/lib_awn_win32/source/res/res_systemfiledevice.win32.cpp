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

    Result ConvertWin32ErrorToResult() {
        const u32 last_error = ::GetLastError();
        switch(last_error) {
            case ERROR_FILE_NOT_FOUND:
                return ResultFileNotFound;
            case ERROR_PATH_NOT_FOUND:
                return ResultPathNotFound;
            case ERROR_SHARING_VIOLATION:
                return ResultFileSharingViolation;
            case ERROR_LOCK_VIOLATION:
                return ResultFileLockViolation;
            case ERROR_TOO_MANY_OPEN_FILES:
                return ResultOpenFileExhaustion;
            case ERROR_NO_MORE_FILES:
                return ResultDirectoryExhausted;
            default:
                break;
        }
        return ResultUnknownOSError;
    }

    Result SystemFileDevice::OpenFileImpl(FileHandle *out_file_handle, const char *path, OpenMode open_mode) {

        /* Integrity checks */
        const u32 format_open_mode = static_cast<u32>(open_mode) & static_cast<u32>(OpenMode::ReadWriteAppend);
        RESULT_RETURN_UNLESS(out_file_handle != nullptr, ResultNullFileHandle);
        RESULT_RETURN_UNLESS(path != nullptr,            ResultNullPath);
        RESULT_RETURN_UNLESS(format_open_mode != 0,      ResultInvalidOpenMode);

        /* Create access rights mask */
        const u32 read_rights   = ((format_open_mode & static_cast<u32>(OpenMode::Read)) == static_cast<u32>(OpenMode::Read))   ? GENERIC_READ  : 0;
        const u32 write_rights  = ((format_open_mode & static_cast<u32>(OpenMode::Write)) == static_cast<u32>(OpenMode::Write)) ? GENERIC_WRITE : 0;
        const u32 access_rights = read_rights | write_rights;

        /* Format path */
        MaxPathString formatted_path;
        const Result format_result = this->FormatPath(std::addressof(formatted_path), path);
        RESULT_RETURN_UNLESS(format_result == ResultSuccess, format_result);

        /* Open file */
        out_file_handle->handle = ::CreateFile(formatted_path.GetString(), access_rights, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
        if (out_file_handle->handle == INVALID_HANDLE_VALUE) {
            return ConvertWin32ErrorToResult();
        }
        out_file_handle->open_mode = static_cast<u32>(open_mode);

        /* Get file size */
        RESULT_RETURN_UNLESS(::GetFileSizeEx(out_file_handle->handle, reinterpret_cast<LARGE_INTEGER*>(std::addressof(out_file_handle->file_size))) == true, ResultFileSizeRetrievalFailed);

        RESULT_RETURN_SUCCESS;
    }

    Result SystemFileDevice::CloseFileImpl(FileHandle *file_handle) {

        /* Integrity checks */
        RESULT_RETURN_UNLESS(file_handle != nullptr,                                                        ResultNullFileHandle);
        RESULT_RETURN_UNLESS(file_handle->handle != nullptr && file_handle->handle != INVALID_HANDLE_VALUE, ResultInvalidFileHandle);

        /* Clear handle state on exit */
        ON_SCOPE_EXIT {
            file_handle->handle    = nullptr;
            file_handle->file_size = 0;
        };

        /* Close win32 handle */
        const bool close_result = ::CloseHandle(file_handle->handle);
        if (close_result == false) {
            return ConvertWin32ErrorToResult();
        }

        RESULT_RETURN_SUCCESS;
    }

    Result SystemFileDevice::ReadFileImpl(void *read_buffer, size_t *out_read_size, FileHandle *file_handle, size_t read_size, size_t file_offset) {

        /* Integrity checks */
        RESULT_RETURN_UNLESS(file_handle != nullptr,                                                             ResultNullFileHandle);
        RESULT_RETURN_UNLESS(file_handle->handle != nullptr && file_handle->handle != INVALID_HANDLE_VALUE,      ResultInvalidFileHandle);
        RESULT_RETURN_UNLESS(file_handle->file_device != this,                                                   ResultInvalidFileHandle);
        RESULT_RETURN_UNLESS((static_cast<u32>(file_handle->open_mode) & static_cast<u32>(OpenMode::Read)) != 0, ResultInvalidOpenMode);
        RESULT_RETURN_UNLESS(file_offset < file_handle->file_size,                                               ResultInvalidFileOffset);

        /* Set file location */
        LARGE_INTEGER offset = {
            .QuadPart = static_cast<s64>(file_offset),
        };
        u32 set_result = ::SetFilePointerEx(file_handle->handle, offset, nullptr, FILE_BEGIN);
        if (set_result == 0) {
            return ConvertWin32ErrorToResult();
        }

        /* Read File */
        u32       read_clamp = (0xffff'ffff < read_size) ? 0xffff'ffff : static_cast<u32>(read_size);
        u32       size_read  = 0;
        size_t    read_iter  = 0;
        ON_SCOPE_EXIT {
            if (out_read_size != nullptr) {
                *out_read_size = read_iter;
            }
        };
        do {
            const bool read_result = ::ReadFile(file_handle->handle, read_buffer, read_clamp, reinterpret_cast<long unsigned int*>(std::addressof(size_read)), nullptr);
            read_iter += size_read;
            if (read_result == false) {
                return ConvertWin32ErrorToResult();
            }
            if ((read_size - read_iter) < read_clamp) {
                read_clamp = (read_size - read_iter);
            }
        } while (read_iter != read_size && size_read == read_clamp);

        RESULT_RETURN_SUCCESS;
    }

    Result SystemFileDevice::WriteFileImpl(size_t *out_written_size, FileHandle *file_handle, void *write_buffer, size_t write_size, size_t file_offset) {

        /* Integrity checks */
        RESULT_RETURN_UNLESS(file_handle != nullptr,                                                              ResultNullFileHandle);
        RESULT_RETURN_UNLESS(file_handle->handle != nullptr && file_handle->handle != INVALID_HANDLE_VALUE,       ResultInvalidFileHandle);
        RESULT_RETURN_UNLESS(write_buffer != nullptr,                                                             ResultNullOutBuffer);
        RESULT_RETURN_UNLESS(write_size != 0,                                                                     ResultInvalidSize);
        RESULT_RETURN_UNLESS((static_cast<u32>(file_handle->open_mode) & static_cast<u32>(OpenMode::Write)) != 0, ResultInvalidOpenMode);
        RESULT_RETURN_UNLESS(file_handle->file_size < file_offset,                                                ResultInvalidFileOffset);

        /* Set file location */
        LARGE_INTEGER offset = {
            .QuadPart = static_cast<s64>(file_offset)
        };
        u32 set_result = ::SetFilePointerEx(file_handle->handle, offset, nullptr, FILE_BEGIN);
        if (set_result == 0) {
            return ConvertWin32ErrorToResult();
        }

        /* Write File */
        u32    write_clamp  = (0xffff'ffff < write_size) ? 0xffff'ffff : static_cast<u32>(write_size);
        u32    size_written = 0;
        size_t written_iter = 0;
        ON_SCOPE_EXIT {
            if (out_written_size != nullptr) {
                *out_written_size = written_iter;
            }
        };
        do {
            const bool write_result = ::WriteFile(file_handle->handle, write_buffer, write_clamp, reinterpret_cast<long unsigned int*>(std::addressof(size_written)), nullptr);
            written_iter += size_written;
            if (write_result == false) {
                return ConvertWin32ErrorToResult();
            }
            if ((write_size - written_iter) < write_clamp) {
                write_clamp = (write_size - written_iter);
            }
        } while (written_iter != write_size);

        RESULT_RETURN_SUCCESS;
    }

    Result SystemFileDevice::FlushFileImpl(FileHandle *file_handle) {

        /* Integrity checks */
        RESULT_RETURN_UNLESS(file_handle != nullptr,                                                        ResultNullFileHandle);
        RESULT_RETURN_UNLESS(file_handle->handle != nullptr && file_handle->handle != INVALID_HANDLE_VALUE, ResultInvalidFileHandle);

        /* Flush file buffers */
        const bool flush_result = ::FlushFileBuffers(file_handle->handle);
        if (flush_result == false) {
            return ConvertWin32ErrorToResult();
        }

        RESULT_RETURN_SUCCESS;
    }

    Result SystemFileDevice::GetFileSizeImpl(size_t *out_size, FileHandle *file_handle) {

        /* Integrity checks */
        RESULT_RETURN_UNLESS(file_handle != nullptr,                                                        ResultNullFileHandle);
        RESULT_RETURN_UNLESS(file_handle->handle != nullptr && file_handle->handle != INVALID_HANDLE_VALUE, ResultInvalidFileHandle);

        *out_size = file_handle->file_size;

        RESULT_RETURN_SUCCESS;
    }
    
    Result SystemFileDevice::GetFileSizeImpl(size_t *out_size, const char *path) {

        /* Integrity checks */
        RESULT_RETURN_UNLESS(path != nullptr, ResultNullPath);

        /* Format path */
        MaxPathString formatted_path;
        const Result format_result = this->FormatPath(std::addressof(formatted_path), path);
        RESULT_RETURN_UNLESS(format_result == ResultSuccess, format_result);

        /* Open file */
        FileHandle handle = {};
        const Result open_result = this->OpenFile(std::addressof(handle), formatted_path.GetString(), OpenMode::Read);
        RESULT_RETURN_UNLESS(open_result == ResultSuccess, open_result);

        /* Copy size */
        *out_size = handle.file_size;

        /* Close file */
        const Result close_result = this->CloseFile(std::addressof(handle));
        RESULT_RETURN_UNLESS(close_result == ResultSuccess, close_result);

        RESULT_RETURN_SUCCESS;
    }

    Result SystemFileDevice::CheckFileExistsImpl(const char *path) {

        /* Integrity checks */
        RESULT_RETURN_UNLESS(path != nullptr, ResultNullPath);

        /* Format path */
        MaxPathString formatted_path;
        const Result format_result = this->FormatPath(std::addressof(formatted_path), path);
        RESULT_RETURN_UNLESS(format_result == ResultSuccess, format_result);

        /* Check file path */
        const u32 check_result = ::GetFileAttributesA(formatted_path.GetString());
        if (check_result == INVALID_FILE_ATTRIBUTES) {
            return ConvertWin32ErrorToResult();
        }

        RESULT_RETURN_SUCCESS;
    }

    Result SystemFileDevice::CreateDirectoryImpl(const char *directory_path) {

        /* Integrity check */
        RESULT_RETURN_UNLESS(directory_path != nullptr, ResultNullPath);

        /* Format path */
        MaxPathString formatted_path;
        const Result format_result = this->FormatPath(std::addressof(formatted_path), directory_path);
        RESULT_RETURN_UNLESS(format_result == ResultSuccess, format_result);

        /* Create directory */
        const bool is_create_dir = ::CreateDirectoryA(formatted_path.GetString(), nullptr);
        if (is_create_dir == false) { return ConvertWin32ErrorToResult(); }

        RESULT_RETURN_SUCCESS;
    }

    Result SystemFileDevice::OpenDirectoryImpl(DirectoryHandle *out_directory_handle, const char *directory_path) {

        /* Integrity checks */
        RESULT_RETURN_UNLESS(out_directory_handle != nullptr, ResultNullDirectoryHandle);
        RESULT_RETURN_UNLESS(directory_path != nullptr,       ResultNullPath);

        /* Format directory path */
        const Result format_result = this->FormatPath(std::addressof(out_directory_handle->directory_path_array[0]), directory_path);
        RESULT_RETURN_UNLESS(format_result == ResultSuccess,                                                        format_result);
        RESULT_RETURN_UNLESS(::PathIsDirectoryA(out_directory_handle->directory_path_array[0].GetString()) == true, ResultDirectoryNotFound);

        /* Setup handle */
        out_directory_handle->entry_index             = 0;
        out_directory_handle->directory_depth         = 0;
        out_directory_handle->search_handle_array[0]  = INVALID_HANDLE_VALUE;

        RESULT_RETURN_SUCCESS;
    }

    Result SystemFileDevice::CloseDirectoryImpl(DirectoryHandle *directory_handle) {

        /* Integrity checks */
        RESULT_RETURN_UNLESS(directory_handle != nullptr, ResultNullDirectoryHandle);

        /* Close all handles */
        for (u32 i = 0; i < DirectoryHandle::cMaxDirectoryDepth; ++i) {
            if (directory_handle->search_handle_array[i] != INVALID_HANDLE_VALUE) {
                ::CloseHandle(directory_handle->search_handle_array[i]);
            }
            directory_handle->search_handle_array[i] = nullptr;
        }

        RESULT_RETURN_SUCCESS;
    }

    Result SystemFileDevice::ReadDirectoryImpl(DirectoryHandle *directory_handle, DirectoryEntry *entry_array, u32 entry_count) {

        /* Integrity checks */
        RESULT_RETURN_UNLESS(directory_handle != nullptr, ResultNullDirectoryHandle);

        /* Read next files */
        WIN32_FIND_DATAA  find_data      = {};
        HANDLE            search_handle  = directory_handle->search_handle_array[directory_handle->directory_depth];
        MaxPathString    *directory_path = std::addressof(directory_handle->directory_path_array[directory_handle->directory_depth]);

        u32 i = 0;
        while (i < entry_count) {

            /* Find a file */
            if (search_handle == INVALID_HANDLE_VALUE) {
                search_handle = ::FindFirstFileExA(directory_path->GetString(), FindExInfoBasic, std::addressof(find_data), FindExSearchNameMatch, nullptr, FIND_FIRST_EX_LARGE_FETCH);
                if (search_handle == INVALID_HANDLE_VALUE) { return ConvertWin32ErrorToResult(); }
                directory_handle->search_handle_array[directory_handle->directory_depth] = search_handle;
            } else {
                bool result = ::FindNextFileA(search_handle, std::addressof(find_data));
                if (result == false) { return ConvertWin32ErrorToResult(); }
            }

            /* Handle normal file */
            if ((find_data.dwFileAttributes & FILE_ATTRIBUTE_NORMAL) == FILE_ATTRIBUTE_NORMAL) {

                /* Append file path without drive */
                entry_array[i].file_path = find_data.cFileName + this->GetDriveSize();
                entry_array[i].file_size = static_cast<size_t>(find_data.nFileSizeLow) | (static_cast<size_t>(find_data.nFileSizeHigh) << 0x20);

                ++i;
                directory_handle->entry_index = directory_handle->entry_index + 1;
                continue;
            }

            /* TODO; handle more then normal files and directories */
            VP_ASSERT((find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY);

            /* Handle sub directory */
            directory_handle->directory_depth = directory_handle->directory_depth + 1;
            RESULT_RETURN_IF(DirectoryHandle::cMaxDirectoryDepth <= directory_handle->directory_depth, ResultExhaustedDirectoryDepth);

            /* Set sub directory path */
            directory_handle->directory_path_array[directory_handle->directory_depth] = find_data.cFileName;

            /* Save entry index */
            const u32       entry_index     = directory_handle->entry_index;
            const u32       sub_entry_count = directory_handle->entry_index - i;
            DirectoryEntry *sub_base_entry  = std::addressof(entry_array[i]);

            /* Process sub directory */
            const Result result = this->ReadDirectoryImpl(directory_handle, sub_base_entry, sub_entry_count);

            /* Add all parsed files */
            i = i + (directory_handle->entry_index - entry_index);

            /* Assert directory succeeded */
            RESULT_RETURN_UNLESS(result == ResultSuccess || result == ResultDirectoryExhausted, result);
        }

        RESULT_RETURN_SUCCESS;
    }

    Result SystemFileDevice::CheckDirectoryExistsImpl(const char *directory_path) { 

        /* Integrity check */
        RESULT_RETURN_UNLESS(directory_path != nullptr, ResultNullPath);

        /* Format path */
        MaxPathString formatted_path;
        const Result format_result = this->FormatPath(std::addressof(formatted_path), directory_path);
        RESULT_RETURN_UNLESS(format_result == ResultSuccess, format_result);

        RESULT_RETURN_IF(::PathIsDirectoryA(formatted_path.GetString()) == false, ResultDirectoryNotFound);

        RESULT_RETURN_SUCCESS;
    }

    Result ContentFileDevice::FormatPath(MaxPathString *out_formatted_path, const char *path_no_drive) {

        /* Format path (expects no drive) */
        RESULT_RETURN_UNLESS(out_formatted_path->Format("content/%s", path_no_drive) != vp::util::cMaxPath, ResultPathTooLong);

        RESULT_RETURN_SUCCESS;
    }

    u32 ContentFileDevice::GetDriveSize() const { return m_drive_size + 9; }

    Result SaveFileDevice::FormatPath(MaxPathString *out_formatted_path, const char *path_no_drive) {

        /* Format path (expects no drive) */
        RESULT_RETURN_UNLESS(out_formatted_path->Format("save/%s", path_no_drive) != vp::util::cMaxPath, ResultPathTooLong);

        RESULT_RETURN_SUCCESS;
    }

    u32 SaveFileDevice::GetDriveSize() const { return m_drive_size + 6; }
}
