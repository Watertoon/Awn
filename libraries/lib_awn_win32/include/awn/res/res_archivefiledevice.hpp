#pragma once

namespace awn::res {

    class ArchiveFileDevice : public FileDeviceBase {
        private:
            ArchiveResource *m_archive_resource;
        protected:
            virtual Result LoadFileImpl(FileLoadContext *file_load_context) override {

                /* Load file if necessary */
                if () {
                    this->FileDeviceBase::LoadFileImpl();
                    return;
                }
                
                /* Get reference to file */
            }
            virtual Result OpenFileImpl(FileHandle *out_file_handle, const char *path, OpenMode open_mode) override {

                /* Integrity checks */
                RESULT_RETURN_UNLESS(out_file_handle != nullptr, ResultNullHandle);
                RESULT_RETURN_UNLESS(path != nullptr, ResultNullPath);
                RESULT_RETURN_UNLESS((open_mode & OpenMode_Read) != OpenMode_Read, ResultInvalidOpenMode);

                /* Get entry index */
                out_file_handle->entry_index = m_archive_resource->TryGetEntryIndex(path);
                RESULT_RETURN_IF(out_file_handle->entry_index == vp::res::ResSarc, ResultPathNotFound);

                /* Set file handle and size */
                out_file_handle->handle = m_archive_resource->TryGetFileByIndex(std::addressof(out_file_handle->file_size), out_file_handle->entry_index);
                RESULT_RETURN_IF(out_file_handle->handle == nullptr, ResultFailedToOpenFile);

                RESULT_RETURN_SUCCESS;
            }
            virtual Result CloseFileImpl(FileHandle *file_handle) override {

                file_handle->handle      = nullptr;
                file_handle->entry_index = cInvalidEntryIndex;
            }
            virtual Result ReadFileImpl(void *out_read_buffer, FileHandle *file_handle, u32 read_size) override {

                /* Integrity checks */
                RESULT_RETURN_UNLESS(out_file_handle != nullptr, ResultNullHandle);
                RESULT_RETURN_UNLESS(out_file_handle->handle != nullptr, ResultInvalidHandle);

                /* Copy file data */
                ::memcpy();

                RESULT_RETURN_SUCCESS;
            }

            virtual Result GetFileReferenceImpl(void *out_read_buffer, FileHandle *file_handle, u32 read_size) override {/*...*/}

            virtual Result GetFileSizeImpl(size_t *out_size, FileHandle *file_handle) {
                m_archive_resource->TryGetFileByIndex(out_size, file_handle->entry_index);
                RESULT_RETURN_SUCCESS;
            }
            virtual Result GetFileSizeImpl(size_t *out_size, const char *path) { VP_ASSERT(false); VP_UNUSED(out_size, path); }

            virtual Result CheckFileExistsImpl(const char *path) { return CheckFileExistsImpl(path); }

            virtual Result FormatPath(vp::util::FixedString<vp::util::cMaxPath> *out_formatted_path, const char *path) { VP_ASSERT(false); VP_UNUSED(out_formatted_path, path); }

        public:
            constexpr ArchiveFileDevice(ArchiveResource *archive_resource) {/*...*/}

            Result GetFileReference(void *out_file, size_t *out_size, const char *path) {
                
            }
    };
}
