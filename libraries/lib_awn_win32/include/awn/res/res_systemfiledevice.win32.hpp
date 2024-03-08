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

    Result ConvertWin32ErrorToResult();

    class SystemFileDevice : public FileDeviceBase {
        protected:
            virtual Result OpenFileImpl(FileHandle *out_file_handle, const char *path, OpenMode open_mode) override;
            virtual Result CloseFileImpl(FileHandle *file_handle) override;
            virtual Result ReadFileImpl(void *read_buffer, size_t *out_read_size, FileHandle *file_handle, size_t read_size, size_t file_offset) override;
            virtual Result WriteFileImpl(size_t *out_written_size, FileHandle *file_handle, void *write_buffer, size_t write_size, size_t file_offset) override;
            virtual Result FlushFileImpl(FileHandle *file_handle) override;
            virtual Result GetFileSizeImpl(size_t *out_size, FileHandle *file_handle) override;
            virtual Result GetFileSizeImpl(size_t *out_size, const char *path) override;
            virtual Result CheckFileExistsImpl(const char *path) override;

            virtual Result CreateDirectoryImpl(const char *directory_path);
            virtual Result OpenDirectoryImpl(DirectoryHandle *out_directory_handle, const char *directory_path) override;
            virtual Result CloseDirectoryImpl(DirectoryHandle *directory_handle) override;
            virtual Result ReadDirectoryImpl(DirectoryHandle *directory_handle, DirectoryEntry *entry_array, u32 entry_count);
            virtual Result CheckDirectoryExistsImpl(const char *directory_path) override;
        public:
            explicit constexpr ALWAYS_INLINE SystemFileDevice(const char *device_name) : FileDeviceBase(device_name) {/*...*/}
            constexpr virtual ALWAYS_INLINE ~SystemFileDevice() override {/*...*/}
    };

    class ContentFileDevice : public SystemFileDevice {
        private:
            u32 m_drive_size;
        protected:
            virtual Result FormatPath(MaxPathString *out_formatted_path, const char *path_no_drive) override;
            virtual u32 GetDriveSize() const override;
        public:
            ALWAYS_INLINE ContentFileDevice() : SystemFileDevice("content"), m_drive_size() {

                /* Obtain size of process working dir */
                m_drive_size = ::GetCurrentDirectory(0, nullptr);
                VP_ASSERT(m_drive_size != 0);

                return;
            }
            virtual constexpr ALWAYS_INLINE ~ContentFileDevice() override {/*...*/}
    };

    class SaveFileDevice : public SystemFileDevice {
        private:
            u32 m_drive_size;
        protected:
            virtual Result FormatPath(MaxPathString *out_formatted_path, const char *path_no_drive) override;
            virtual u32 GetDriveSize() const override;
        public:
            ALWAYS_INLINE SaveFileDevice() : SystemFileDevice("save"), m_drive_size() {

                /* Obtain size of process working dir */
                m_drive_size = ::GetCurrentDirectory(0, nullptr);
                VP_ASSERT(m_drive_size != 0);

                return;
            }
            virtual constexpr ALWAYS_INLINE ~SaveFileDevice() override {/*...*/}
    };
}
