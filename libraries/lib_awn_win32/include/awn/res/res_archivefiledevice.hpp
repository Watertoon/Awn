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

    class ArchiveFileDevice : public FileDeviceBase {
        private:
            ArchiveResource *m_archive_resource;
        public:
            VP_RTTI_DERIVED(ArchiveFileDevice, FileDeviceBase);
        protected:
            virtual Result OpenFileImpl(FileHandle *out_file_handle, const char *path, OpenMode open_mode) override;
            virtual Result CloseFileImpl(FileHandle *file_handle) override;
            virtual Result ReadFileImpl(void *read_buffer, size_t *out_read_size, FileHandle *file_handle, size_t read_size, size_t file_offset) override;
            virtual Result GetFileSizeImpl(size_t *out_size, FileHandle *file_handle) override;
            virtual Result GetFileSizeImpl(size_t *out_size, const char *path) override;
            virtual Result CheckFileExistsImpl(const char *path) override;

            virtual Result OpenDirectoryImpl(DirectoryHandle *out_directory_handle, const char *path) override;
            virtual Result CloseDirectoryImpl(DirectoryHandle *directory_handle) override;
            virtual Result ReadDirectoryImpl(DirectoryHandle *directory_handle, DirectoryEntry *entry_array, u32 entry_count) override;
            virtual Result CheckDirectoryExistsImpl(const char *path) override;

            virtual Result FormatPath(vp::util::FixedString<vp::util::cMaxPath> *out_formatted_path, const char *path) override;

            virtual Result GetFileReferenceImpl(void **out_file, size_t *out_size, const char *file_path);
        public:
            constexpr ArchiveFileDevice(ArchiveResource *archive_resource) : FileDeviceBase(), m_archive_resource(archive_resource) {/*...*/}
            constexpr virtual ~ArchiveFileDevice() override {/*...*/}

            Result GetFileReference(void **out_file, size_t *out_size, const char *file_path);
    };
}
