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

    Result FileDeviceBase::LoadFileImpl(FileLoadContext *file_load_context) {

        /* Integrity checks */
        RESULT_RETURN_UNLESS(file_load_context->file != nullptr && file_load_context->file_size == 0, ResultInvalidFileBufferSize);
        RESULT_RETURN_UNLESS((file_load_context->read_div & 0x1f) != 0, ResultInvalidReadDivAlignment);
        
        /* Declare a file handle closed on exit */
        FileHandle handle = {};
        ON_SCOPE_EXIT {
            RESULT_ABORT_UNLESS(handle.Close());
        };

        /* Open file handle */
        const Result open_result = this->TryOpenFile(std::addressof(handle), file_load_context->file_path, OpenMode::Read);
        RESULT_RETURN_UNLESS(open_result == ResultSuccess, open_result);

        /* Get file size */
        size_t       file_size   = 0;
        const Result size_result = this->GetFileSize(std::addressof(file_size), std::addressof(handle));
        RESULT_RETURN_UNLESS(size_result == ResultSuccess, size_result);

        /* Align alignment */
        size_t output_alignment = vp::util::AlignUp(file_load_context->file_alignment, alignof(u64));

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
            if (file_load_context->out_is_file_memory_allocated == true) {
                ::operator delete(file_load_context->file);
                file_load_context->file = nullptr;
            }
        };

        /* Enforce file size */
        RESULT_RETURN_UNLESS(file_size <= file_load_context->file_size, ResultInvalidFileBufferSize);

        /* Calculate read div */
        const u32    div_size          = (file_load_context->read_div == 0) ? file_size : file_load_context->read_div;
        const size_t div_total_size    = vp::util::AlignDown(file_size, div_size);

        /* First read */
        const Result read_result = this->TryReadFile(file_load_context->file, std::addressof(handle), div_size);
        RESULT_RETURN_UNLESS(read_result == ResultSuccess, read_result);

        /* Stream read */
        size_t offset = div_size;
        while (offset != div_total_size) {

            /* Read next chunk */
            const Result read_loop_result = this->TryReadFile(reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(file_load_context->file) + offset), std::addressof(handle), div_size);
            RESULT_RETURN_UNLESS(read_loop_result == ResultSuccess, read_loop_result);

            /* Advance file read offset */
            offset = ((div_total_size - offset) < div_size) ? div_total_size - offset : offset + div_size;
        }

        /* Cancel alloc error for success */
        error_after_alloc_guard.Cancel();

        RESULT_RETURN_SUCCESS;
    }
}
