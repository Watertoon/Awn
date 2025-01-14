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

    FileDeviceBase::~FileDeviceBase() {
        impl::FileDeviceManager *mgr = impl::FileDeviceManager::GetInstance();
        if (mgr == nullptr) { return; }
        mgr->RemoveFileDevice(this);
    }

    Result FileDeviceBase::LoadFileImpl(const char *path, FileLoadContext *file_load_context) {

        /* Integrity checks */
        RESULT_RETURN_IF(file_load_context->file != nullptr && file_load_context->file_size == 0, ResultInvalidFileBufferSize);
        RESULT_RETURN_IF((file_load_context->read_div & 0x1f) != 0, ResultInvalidReadDivAlignment);

        /* Declare a file handle closed on exit */
        FileHandle handle = {};
        ON_SCOPE_EXIT {
            RESULT_ABORT_UNLESS(handle.Close());
        };

        /* Open file handle */
        RESULT_TRY(this->OpenFile(std::addressof(handle), path, OpenMode::Read));
        /* Get file size */
        size_t       file_size   = 0;
        RESULT_TRY(this->GetFileSize(std::addressof(file_size), std::addressof(handle)));

        /* Align alignment */
        size_t output_alignment = (file_load_context->file_alignment < cMinimumFileAlignment) ? cMinimumFileAlignment : file_load_context->file_alignment;

        /* Check whether the otuput memory needs to be allocated */
        if (file_load_context->file == nullptr) {

            /* Allocate file memory */
            file_load_context->file = ::operator new(file_size, file_load_context->heap, output_alignment);
            RESULT_RETURN_IF(file_load_context->file == nullptr, ResultFailedToAllocateFileMemory);

            /* Set output file sizes */
            file_load_context->file_size                    = file_size;
            file_load_context->file_alignment               = output_alignment;
            file_load_context->out_is_file_memory_allocated = true;
        }

        /* Handle file allocation error */
        auto error_after_alloc_guard = SCOPE_GUARD {
            if (file_load_context->out_is_file_memory_allocated) {
                ::operator delete(file_load_context->file);
                file_load_context->file = nullptr;
            }
        };

        /* Enforce file size */
        RESULT_RETURN_UNLESS(file_size <= file_load_context->file_size, ResultInvalidFileBufferSize);

        /* Calculate read div */
        const u32 read_clamp = (file_load_context->read_div == 0) ? file_size : file_load_context->read_div;

        /* First read */
        size_t file_offset = 0;
        RESULT_TRY(this->ReadFile(file_load_context->file, std::addressof(file_offset), std::addressof(handle), read_clamp, file_offset));

        /* Stream read */
        while (file_offset < file_size) {

            /* Read next chunk */
            size_t size_read = 0;
            RESULT_TRY(this->ReadFile(reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(file_load_context->file) + file_offset), std::addressof(size_read), std::addressof(handle), read_clamp, file_offset));
            file_offset += size_read;
        }

        /* Cancel alloc error for success */
        error_after_alloc_guard.Cancel();

        RESULT_RETURN_SUCCESS;
    }

    Result FileDeviceBase::SaveFileImpl(const char *path, FileSaveInfo *file_save_info) {

        /* Integrity checks */
        RESULT_RETURN_IF(file_save_info->file != nullptr && file_save_info->file_size == 0, ResultInvalidFileBufferSize);

        /* Open write dest */
        FileHandle handle = {};
        ON_SCOPE_EXIT {
            RESULT_ABORT_UNLESS(handle.Close());
        };
        RESULT_TRY(this->OpenFile(std::addressof(handle), path, OpenMode::Write));

        /* Write file */
        size_t written_size       = 0;
        RESULT_TRY(this->WriteFile(std::addressof(written_size), std::addressof(handle), file_save_info->file, file_save_info->file_size, 0));

        RESULT_RETURN_SUCCESS;
    }

    Result FileDeviceBase::CopyFileImpl(const char *dest_path, const char *source_path, void *copy_buffer, size_t copy_size) {

        /* Integrity checks */
        RESULT_RETURN_IF(copy_buffer == nullptr || copy_size == 0, ResultNullOutBuffer);

        /* Open source */
        FileHandle src_handle = {};
        ON_SCOPE_EXIT {
            src_handle.Close();
        };
        RESULT_TRY(this->OpenFile(std::addressof(src_handle), source_path, OpenMode::Read));

        /* Open dest */
        FileHandle dst_handle = {};
        ON_SCOPE_EXIT {
            dst_handle.Close();
        };
        RESULT_TRY(this->OpenFile(std::addressof(dst_handle), dest_path, OpenMode::Write));

        /* Read source */
        size_t read_size = 0;
        RESULT_TRY(this->ReadFile(copy_buffer, std::addressof(read_size), std::addressof(src_handle), copy_size, 0));

        /* Write to dest */
        size_t write_size = 0;
        RESULT_TRY(this->WriteFile(std::addressof(write_size), std::addressof(dst_handle), copy_buffer, read_size, 0));

        RESULT_RETURN_SUCCESS;
    }
}
