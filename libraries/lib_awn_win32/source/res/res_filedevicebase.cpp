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

    Result FileDeviceBase::LoadFileImpl(const char *path, FileLoadContext *file_load_context) {

        /* Integrity checks */
        RESULT_RETURN_IF(file_load_context->file_buffer != nullptr && file_load_context->file_size == 0, ResultInvalidFileBufferSize);
        RESULT_RETURN_IF((file_load_context->read_div & 0x1f) != 0, ResultInvalidReadDivAlignment);
        
        /* Declare a file handle closed on exit */
        FileHandle handle = {};
        ON_SCOPE_EXIT {
            RESULT_ABORT_UNLESS(handle.Close());
        };

        /* Open file handle */
        const Result open_result = this->OpenFile(std::addressof(handle), path, OpenMode::Read);
        RESULT_RETURN_UNLESS(open_result == ResultSuccess, open_result);

        /* Get file size */
        size_t       file_size   = 0;
        const Result size_result = this->GetFileSize(std::addressof(file_size), std::addressof(handle));
        RESULT_RETURN_UNLESS(size_result == ResultSuccess, size_result);

        /* Align alignment */
        size_t output_alignment = (file_load_context->file_alignment < cMinimumFileAlignment) ? cMinimumFileAlignment : file_load_context->file_alignment;

        /* Check whether the otuput memory needs to be allocated */
        if (file_load_context->file_buffer == nullptr) {

            /* Allocate file memory */
            file_load_context->file_buffer = ::operator new(file_size, file_load_context->file_heap, output_alignment);
            RESULT_RETURN_IF(file_load_context->file_buffer == nullptr, ResultFailedToAllocateFileMemory);

            /* Set output file sizes */
            file_load_context->file_size                    = file_size;
            file_load_context->file_alignment               = output_alignment;
            file_load_context->out_is_file_memory_allocated = true;
        }

        /* Handle file allocation error */
        auto error_after_alloc_guard = SCOPE_GUARD {
            if (file_load_context->out_is_file_memory_allocated == true) {
                ::operator delete(file_load_context->file_buffer);
                file_load_context->file_buffer = nullptr;
            }
        };

        /* Enforce file size */
        RESULT_RETURN_UNLESS(file_size <= file_load_context->file_size, ResultInvalidFileBufferSize);

        /* Calculate read div */
        const u32    read_clamp     = (file_load_context->read_div == 0) ? file_size : file_load_context->read_div;

        /* First read */
        size_t file_offset = 0;
        const Result read_result = this->ReadFile(file_load_context->file_buffer, std::addressof(file_offset), std::addressof(handle), read_clamp, file_offset);
        RESULT_RETURN_UNLESS(read_result == ResultSuccess, read_result);

        /* Stream read */
        while (file_offset < file_size) {

            /* Read next chunk */
            size_t size_read = 0;
            const Result read_loop_result = this->ReadFile(reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(file_load_context->file_buffer) + file_offset), std::addressof(size_read), std::addressof(handle), read_clamp, file_offset);
            file_offset += size_read;
            RESULT_RETURN_UNLESS(read_loop_result == ResultSuccess, read_loop_result);
        }

        /* Cancel alloc error for success */
        error_after_alloc_guard.Cancel();

        RESULT_RETURN_SUCCESS;
    }
}
