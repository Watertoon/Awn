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
            //virtual Result LoadFileImpl(FileLoadContext *file_load_context) override {
            //
            //    /* Load file into user buffer if necessary */
            //    if (file_load_context->compression_type == CompressionType::None && file_load_context->out_file != nullptr) {
            //        return this->FileDeviceBase::LoadFileImpl(file_load_context);
            //    }
            //
            //    /* Get reference to file */
            //    ArchiveFileReturn file_return = {};
            //    const bool result = m_archive_resource->TryGetFileByPath(std::addressof(file_return), file_path);
            //    RESULT_RETURN_IF(result == false, ResultFileNotFound);
            //
            //    /* Handle compression */
            //    if (file_load_context->compression_type != CompressionType::None) {
            //
            //        file_load_context->out_file = ::operator new();
            //
            //        Decompressor *decompressor = DecompressorManager::GetInstance()->GetDecompressor(file_load_context->compression_type);
            //        decompresssor->TryDecompressFile();
            //
            //        file_load_context->out_file_size = ;
            //    }
            //
            //    return ;
            //}
            virtual Result OpenFileImpl(FileHandle *out_file_handle, const char *path, OpenMode open_mode) override {

                /* Integrity checks */
                RESULT_RETURN_UNLESS(out_file_handle != nullptr, ResultNullHandle);
                RESULT_RETURN_UNLESS(path != nullptr, ResultNullPath);
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
            virtual Result CloseFileImpl(FileHandle *file_handle) override {

                /* Integrity checks */
                RESULT_RETURN_UNLESS(file_handle != nullptr, ResultNullHandle);

                /* Clear handle state */
                file_handle->handle              = nullptr;
                file_handle->archive_entry_index = cInvalidEntryIndex;

                RESULT_RETURN_SUCCESS;
            }
            virtual Result ReadFileImpl(void *out_read_buffer, FileHandle *file_handle, u32 read_size) override {

                /* Integrity checks */
                RESULT_RETURN_UNLESS(file_handle != nullptr,         ResultNullHandle);
                RESULT_RETURN_UNLESS(file_handle->handle != nullptr, ResultInvalidHandle);

                /* Copy file data */
                ::memcpy(out_read_buffer, reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(file_handle->handle) + file_handle->file_offset), read_size);

                RESULT_RETURN_SUCCESS;
            }

            virtual Result OpenDirectoryImpl(DirectoryHandle *out_directory_handle, const char *path) override {
                RESULT_RETURN_UNLESS(out_directory_handle != nullptr, ResultNullHandle);
                RESULT_RETURN_UNLESS(path != nullptr, ResultNullPath);
                const char character = *path;
                RESULT_RETURN_IF(character != '\0' && character != '/', ResultDirectoryNotFound);
                out_directory_handle->entry_index = 0;
                RESULT_RETURN_SUCCESS;
            }
            virtual Result CloseDirectoryImpl(DirectoryHandle *directory_handle) override {
                RESULT_RETURN_UNLESS(directory_handle != nullptr, ResultNullHandle);
                directory_handle->entry_index = 0;
                RESULT_RETURN_SUCCESS;
            }
            virtual Result ReadDirectoryImpl(DirectoryHandle *directory_handle, DirectoryEntry *entry_array, u32 entry_count) override {
                
                /* Integrity checks */
                RESULT_RETURN_UNLESS(directory_handle != nullptr, ResultNullHandle);

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

            virtual bool CheckDirectoryExistsImpl(const char *path) override {
                const char character = *path;
                RESULT_RETURN_IF(character != '\0' && character != '/', ResultDirectoryNotFound);
                RESULT_RETURN_SUCCESS;
            }

            virtual Result GetFileSizeImpl(size_t *out_size, FileHandle *file_handle) override {

                /* Integrity checks */
                RESULT_RETURN_UNLESS(file_handle != nullptr, ResultNullHandle);

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
            virtual Result GetFileSizeImpl(size_t *out_size, const char *path) override {

                /* Integrity checks */
                RESULT_RETURN_UNLESS(path != nullptr, ResultNullPath);

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

            virtual Result CheckFileExistsImpl(const char *path) override {
                RESULT_RETURN_IF(m_archive_resource->TryGetEntryIndex(path) == cInvalidEntryIndex, ResultFileNotFound);
                RESULT_RETURN_SUCCESS;
            }

            virtual Result FormatPath(vp::util::FixedString<vp::util::cMaxPath> *out_formatted_path, const char *path) override {
                *out_formatted_path = path;
                RESULT_RETURN_SUCCESS;
            }

            virtual Result GetFileReferenceImpl(void **out_file, size_t *out_size, const char *file_path) {

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

        public:
            constexpr ArchiveFileDevice(ArchiveResource *archive_resource) : FileDeviceBase(), m_archive_resource(archive_resource) {/*...*/}
            constexpr virtual ~ArchiveFileDevice() override {/*...*/}

            Result GetFileReference(void **out_file, size_t *out_size, const char *file_path) {
                return this->GetFileReferenceImpl(out_file, out_size, file_path);
            }
    };
}
