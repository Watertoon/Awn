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

    void ZstdCompressor::Initialize(mem::Heap *heap, u32 max_compression_level) {

        /* Allocate work memory */
        const size_t cctx_size = ::ZSTD_estimateCStreamSize(max_compression_level);
        const size_t out_size = ::ZSTD_CStreamOutSize();
        const size_t total_size = cctx_size + out_size;
        void *work_memory = ::operator new(total_size, heap, 0x20);
        m_work_memory = work_memory;
        m_out_memory = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(work_memory) + cctx_size);
        m_out_size = m_out_size;
        m_cctx_size = cctx_size;
        m_max_compression_level = max_compression_level;
        m_compression_level = 3;

        return;
    }

    void ZstdCompressor::Finalize() {
        if (m_work_memory != nullptr) {
            ::operator delete (m_work_memory);
            m_work_memory = nullptr;
            m_out_memory = nullptr;
        }
    }

    Result ZstdCompressor::SaveCompressFile(const char *path, FileSaveInfo *save_info, FileDeviceBase *file_device) { 

        /* Add compression extension */
        res::MaxPathString path_ext;
        path_ext.Format("%s.zs", path);

        /* Create file handle */
        FileHandle handle = {};
        ON_SCOPE_EXIT {
            RESULT_ABORT_UNLESS(handle.Close());
        };
        if (file_device == nullptr) {

            /* Lookup file device */
            MaxDriveString drive;
            vp::util::GetDrive(std::addressof(drive), std::addressof(path_ext));
            RESULT_RETURN_UNLESS(0 < drive.GetLength(), ResultNullFileDevice);

            file_device = impl::FileDeviceManager::GetInstance()->GetFileDeviceByDrive(drive.GetString());
            RESULT_RETURN_UNLESS(file_device != nullptr, ResultNullFileDevice);

            /* Create file */
            MaxPathString path_no_drive;
            vp::util::GetPathWithoutDrive(std::addressof(path_no_drive), std::addressof(path_ext));
            RESULT_TRY(file_device->CreateFile(std::addressof(handle), path_no_drive.GetString(), save_info->file_size));
        } else {
            /* Create file */
            RESULT_TRY(file_device->CreateFile(std::addressof(handle), path_ext.GetString(), save_info->file_size));
        }

        /* Init context */
        ZSTD_CCtx *cctx = ::ZSTD_initStaticCCtx(m_work_memory, m_cctx_size);
        RESULT_RETURN_IF(cctx == nullptr, ResultZstdError);
        const size_t is_set = ::ZSTD_CCtx_setParameter(cctx, ZSTD_c_compressionLevel, m_compression_level);
        RESULT_RETURN_IF(::ZSTD_isError(is_set), ResultZstdError);

        /* Stream out */
        ZSTD_inBuffer in = {
            .src = save_info->file,
            .size = save_info->file_size,
            .pos = 0,
        };
        ZSTD_outBuffer out = {
            .dst = m_out_memory,
            .size = m_out_size,
            .pos = 0,
        };
        size_t offset = 0;
        while (in.pos < in.size) {
            /* Compress a frame */
            const size_t left = ::ZSTD_compressStream2(cctx, std::addressof(out), std::addressof(in), ZSTD_e_flush);
            RESULT_RETURN_IF(::ZSTD_isError(left) || left != 0, ResultZstdError);
            /* Write frame */
            file_device->WriteFile(nullptr, std::addressof(handle), out.dst, out.pos, offset);
            /* Advance */
            offset += out.pos;
            out.pos = 0;
        }
        file_device->Commit();

        RESULT_RETURN_SUCCESS;
    }
}
