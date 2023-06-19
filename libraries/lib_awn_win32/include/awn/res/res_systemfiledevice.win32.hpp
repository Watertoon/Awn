#pragma once

namespace awn::res {

    ALWAYS_INLINE Result ConvertWin32ErrorToResult() {
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

    class SystemFileDevice : public FileDeviceBase {
        protected:
            virtual Result OpenFileImpl(FileHandle *out_file_handle, const char *path, OpenMode open_mode) override {

                /* Integrity checks */
                RESULT_RETURN_UNLESS(out_file_handle != nullptr, ResultNullHandle);
                RESULT_RETURN_UNLESS(path != nullptr, ResultNullPath);
                RESULT_RETURN_UNLESS((open_mode & OpenMode_ReadWriteAppend) != 0, ResultInvalidOpenMode);

                /* Format path */
                MaxPathString formatted_path;
                const Result format_result = this->FormatPath(std::addressof(formatted_path), path);
                RESULT_RETURN_UNLESS(format_result == ResultSuccess, format_result);

                /* Open file */
                out_file_handle->handle = ::CreateFile(path, GENERIC_READ, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
                if (out_file_handle->handle == INVALID_HANDLE_VALUE) {
                    return ConvertWin32ErrorToResult();
                }

                /* Get file size */
                RESULT_RETURN_IF(::GetFileSizeEx(out_file_handle->handle, reinterpret_cast<LARGE_INTEGER*>(std::addressof(out_file_handle->file_size))), ResultFileSizeRetrievalFailed);

                RESULT_RETURN_SUCCESS;
            }

            virtual Result CloseFileImpl(FileHandle *file_handle) override {

                /* Integrity checks */
                RESULT_RETURN_UNLESS(file_handle != nullptr, ResultNullHandle);
                RESULT_RETURN_UNLESS(file_handle->handle != nullptr && file_handle->handle != INVALID_HANDLE_VALUE, ResultInvalidHandle);

                /* Close win32 handle */
                const bool close_result = ::CloseHandle(file_handle->handle);
                if (close_result == false) {
                    return ConvertWin32ErrorToResult();
                }

                /* Null res::FileHandle */
                file_handle->handle = nullptr;
                file_handle->file_offset  = 0;
                file_handle->file_size    = 0;

                RESULT_RETURN_SUCCESS;
            }

            virtual Result ReadFileImpl(void *out_read_buffer, FileHandle *file_handle, u32 read_size) override {

                /* Integrity checks */
                RESULT_RETURN_UNLESS(file_handle != nullptr, ResultNullHandle);
                RESULT_RETURN_UNLESS(file_handle->handle != nullptr && file_handle->handle != INVALID_HANDLE_VALUE, ResultInvalidHandle);

                /* Set file location */
                LARGE_INTEGER offset = {
                    .QuadPart = static_cast<s64>(file_handle->file_offset)
                };
                u32 set_result = ::SetFilePointerEx(file_handle->handle, offset, nullptr, FILE_BEGIN);
                if (set_result != 0) {
                    return ConvertWin32ErrorToResult();
                }

                /* Read File */
                u32 out_read_size = 0;
                const bool read_result = ::ReadFile(file_handle->handle, std::addressof(out_read_buffer), read_size, reinterpret_cast<long unsigned int*>(std::addressof(out_read_size)), nullptr);
                if (read_result == false) {
                    return ConvertWin32ErrorToResult();
                }

                RESULT_RETURN_SUCCESS;
            }
            
            virtual Result WriteFileImpl(FileHandle *file_handle, void *write_buffer, u32 write_size) override {

                /* Integrity checks */
                RESULT_RETURN_UNLESS(file_handle != nullptr, ResultNullHandle);
                RESULT_RETURN_UNLESS(file_handle->handle != nullptr && file_handle->handle != INVALID_HANDLE_VALUE, ResultInvalidHandle);
                RESULT_RETURN_UNLESS(write_buffer != nullptr, ResultNullOutBuffer);
                RESULT_RETURN_UNLESS(write_size != 0, ResultInvalidSize);

                /* Write file */
                u32 out_write_size = 0;
                [[maybe_unused]] const bool write_result = ::WriteFile(file_handle->handle, write_buffer, write_size, reinterpret_cast<long unsigned int*>(std::addressof(out_write_size)), nullptr);
                //if () {
                    
                //}
                //RESULT_RETURN_IF()
                    
                RESULT_RETURN_SUCCESS;
            }

            virtual Result FlushFileImpl(FileHandle *file_handle) override {

                /* Integrity checks */
                RESULT_RETURN_UNLESS(file_handle != nullptr, ResultNullHandle);
                RESULT_RETURN_UNLESS(file_handle->handle != nullptr && file_handle->handle != INVALID_HANDLE_VALUE, ResultInvalidHandle);

                /* Flush file buffers */
                const bool flush_result = ::FlushFileBuffers(file_handle->handle);
                if (flush_result == false) {
                    return ConvertWin32ErrorToResult();
                }

                RESULT_RETURN_SUCCESS;
            }

            virtual Result GetFileSizeImpl(size_t *out_size, FileHandle *file_handle) override {

                /* Integrity checks */
                RESULT_RETURN_UNLESS(file_handle != nullptr, ResultNullHandle);
                RESULT_RETURN_UNLESS(file_handle->handle != nullptr && file_handle->handle != INVALID_HANDLE_VALUE, ResultInvalidHandle);

                *out_size = file_handle->file_size;

                RESULT_RETURN_SUCCESS;
            }
            
            virtual Result GetFileSizeImpl(size_t *out_size, const char *path) override {

                /* Integrity checks */
                RESULT_RETURN_UNLESS(path != nullptr, ResultNullPath);

                /* Format path */
                MaxPathString formatted_path;
                const Result format_result = this->FormatPath(std::addressof(formatted_path), path);
                RESULT_RETURN_UNLESS(format_result == ResultSuccess, format_result);

                /* Open file */
                FileHandle handle = {};
                const Result open_result = this->TryOpenFile(std::addressof(handle), formatted_path.GetString(), OpenMode_Read);
                RESULT_RETURN_UNLESS(open_result == ResultSuccess, open_result);

                /* Copy size */
                *out_size = handle.file_size;

                /* Close file */
                const Result close_result = this->TryCloseFile(std::addressof(handle));
                RESULT_RETURN_UNLESS(close_result == ResultSuccess, close_result);

                RESULT_RETURN_SUCCESS;
            }

            virtual Result CheckFileExistsImpl(const char *path) override {

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

            virtual Result OpenDirectoryImpl(DirectoryHandle *out_directory_handle, const char *directory_path) override {

                /* Integrity checks */
                RESULT_RETURN_UNLESS(out_directory_handle != nullptr,            ResultNullHandle);
                RESULT_RETURN_UNLESS(directory_path != nullptr,                  ResultNullPath);
                RESULT_RETURN_UNLESS(::PathIsDirectoryA(directory_path) == true, ResultDirectoryNotFound);

                /* Setup handle */
                out_directory_handle->search_handle  = INVALID_HANDLE_VALUE;
                out_directory_handle->directory_path = directory_path;

                RESULT_RETURN_SUCCESS;
            }
            virtual Result CloseDirectoryImpl(DirectoryHandle *directory_handle) override {
                ::CloseHandle(directory_handle->search_handle);
            }
            virtual Result ReadDirectoryImpl(DirectoryHandle *directory_handle, DirectoryEntry *entry_array, u32 entry_count) {

                /* Read next file */
                WIN32_FIND_DATAA  find_data = {};
                for (u32 i = 0; i < entry_count; ++i) {

                    /* Find a file */
                    if (directory_handle->search_handle == INVALID_HANDLE_VALUE) {
                        directory_handle->search_handle = ::FindFirstFileExA(directory_handle->directory_path.GetString(), FindExInfoBasic, std::addressof(find_data), FindExSearchNameMatch, nullptr, FIND_FIRST_EX_LARGE_FETCH);
                        if (directory_handle->search_handle == INVALID_HANDLE_VALUE) { return ConvertWin32ErrorToResult(); }
                    } else {
                        bool result = ::FindNextFileA(directory_handle->search_handle, std::addressof(find_data));
                        if (result == false) { return ConvertWin32ErrorToResult(); }
                    }
                }

                RESULT_RETURN_SUCCESS;
            }

            virtual Result CheckDirectoryExistsImpl(const char *directory_path) override { return ::PathIsDirectoryA(directory_path);}
        public:
            explicit constexpr ALWAYS_INLINE SystemFileDevice(const char *device_name) : FileDeviceBase(device_name) {/*...*/}
            constexpr virtual ALWAYS_INLINE ~SystemFileDevice() override {/*...*/}
    };

    class ContentFileDevice : public SystemFileDevice {
        protected:
            virtual Result FormatPath(MaxPathString *out_formatted_path, const char *path) override {

                MaxPathString path_no_drive;

                vp::util::GetPathWithoutDrive(std::addressof(path_no_drive), path);

                RESULT_RETURN_UNLESS(out_formatted_path->Format("content/%s", path_no_drive) != vp::util::cMaxPath, ResultPathTooLong);

                RESULT_RETURN_SUCCESS;
            }
        public:
            constexpr ALWAYS_INLINE ContentFileDevice() : SystemFileDevice("content") {/*...*/}
            constexpr virtual ALWAYS_INLINE ~ContentFileDevice() override {/*...*/}
    };

    class SaveFileDevice : public SystemFileDevice {
        protected:
            virtual Result FormatPath(MaxPathString *out_formatted_path, const char *path) override {

                MaxPathString path_no_drive;

                vp::util::GetPathWithoutDrive(std::addressof(path_no_drive), path);

                RESULT_RETURN_UNLESS(out_formatted_path->Format("save/%s", path_no_drive) != vp::util::cMaxPath, ResultPathTooLong);

                RESULT_RETURN_SUCCESS;
            }
        public:
            constexpr ALWAYS_INLINE SaveFileDevice() : SystemFileDevice("save") {/*...*/};
            constexpr virtual ALWAYS_INLINE ~SaveFileDevice() override {/*...*/}
    };
}
