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

    class ZstdDecompressor : public IDecompressor {
        public:
            static constexpr size_t cMaxDictionaryCount = 8;
            static constexpr size_t cReadSize           = 0xd'0000;
            static constexpr size_t cLeftoverSize       = vp::util::c128KB;
        public:
            using DictionaryMap       = vp::util::FixedKeyIndexMap<cMaxDictionaryCount << 1>;
            using DictionaryAllocator = vp::util::FixedIndexAllocator<u8, cMaxDictionaryCount>;
        public:
            class DecompressorThread : public sys::ServiceThread {
                public:
                    friend class ZstdDecompressor;
                private:
                    u32                           m_stream_count;
                    u32                           m_is_error;
                    void                         *m_stream_memory;
                    vp::codec::ZstdStreamContext *m_current_stream_context;
                    sys::ServiceEvent             m_decompress_finish_event;
                    sys::ServiceEvent             m_stream_finish_event;
                public:
                    DecompressorThread(mem::Heap *heap) : ServiceThread("ZstdDecompressorThread", heap, sys::ThreadRunMode::WaitForMessage, 0x7fff'ffff'ffff'ffff, 0x20, 0x4000, sys::cPriorityNormal), m_current_stream_context(), m_decompress_finish_event(), m_stream_finish_event() {
                        m_decompress_finish_event.Initialize(sys::SignalState::Signaled, sys::ResetMode::Auto);
                        m_stream_finish_event.Initialize(sys::SignalState::Cleared, sys::ResetMode::Manual);
                    }
                    virtual ~DecompressorThread() override {
                        m_decompress_finish_event.Finalize();
                        m_stream_finish_event.Finalize();
                    }

                    virtual void ThreadMain(size_t stream_size) override {

                        /* Integrity checks */
                        VP_ASSERT(m_current_stream_context != nullptr);

                        /* Flip output */
                        void *zstd_stream = ((m_stream_count & 1) == 0) ? m_stream_memory : reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(m_stream_memory) + cReadSize);

                        /* Stream decompress zstd*/
                        size_t expected = vp::codec::StreamDecompressZstd(zstd_stream, stream_size, m_current_stream_context);

                        /* Increment stream count */
                        ++m_stream_count;

                        /* Update error state */
                        m_is_error = (expected == vp::codec::ZstdStreamContext::cInvalidStreamState);

                        /* Signal events */
                        if (m_current_stream_context->state == vp::codec::ZstdStreamContext::State::Finished || m_current_stream_context->state == vp::codec::ZstdStreamContext::State::Error) {
                            m_decompress_finish_event.Signal();
                        }
                        m_stream_finish_event.Signal();

                        return;
                    }
            };
        private:
            DecompressorThread   *m_decompressor_thread;
            void                 *m_work_memory;
            DictionaryMap         m_dic_map;
            ZSTD_DDict           *m_dic_array[cMaxDictionaryCount];
            DictionaryAllocator   m_dic_index_allocator;
        public:
            VP_RTTI_DERIVED(ZstdDecompressor, IDecompressor);
        public:
            constexpr ZstdDecompressor() : m_decompressor_thread(), m_work_memory(), m_dic_map(), m_dic_array{}, m_dic_index_allocator() {/*...*/}
            constexpr virtual ~ZstdDecompressor() {/*...*/}

            void Initialize(mem::Heap *heap) {

                /* Allocate decomp system memory */
                const size_t dctx_size  = ::ZSTD_estimateDCtxSize();
                m_work_memory           = reinterpret_cast<ZSTD_DCtx*>(::operator new(cReadSize * 2 + cLeftoverSize + dctx_size, heap, 0x20));
                VP_ASSERT(m_work_memory != nullptr);

                /* Create thread */
                m_decompressor_thread = new DecompressorThread(heap);
                m_decompressor_thread->StartThread();

                /* Clear ZsDic */
                this->ClearZsDic();

                return;
            }

            void Finalize() {
                if (m_decompressor_thread != nullptr) {
                    delete m_decompressor_thread;
                    m_decompressor_thread = nullptr;
                }
                if (m_work_memory != nullptr) {
                    ::operator delete(reinterpret_cast<void*>(m_work_memory));
                    m_work_memory = nullptr;
                }
            }

            virtual void SetPriority(u32 priority) override {
                m_decompressor_thread->SetPriority(priority);
            }
            virtual void SetCoreMask(sys::CoreMask core_mask) override {
                m_decompressor_thread->SetCoreMask(core_mask);
            }

            virtual Result LoadDecompressFile(size_t *out_size, s32 *out_alignment, const char *path, FileLoadContext *file_load_context, FileDeviceBase *file_device) override {

                /* Integrity checks */
                RESULT_RETURN_IF(file_load_context->file_buffer != nullptr && file_load_context->file_size == 0, ResultInvalidFileBufferSize);
                RESULT_RETURN_IF(file_load_context->read_div != 0,                                               ResultInvalidReadDivAlignment);

                /* Declare a file handle closed on exit */
                FileHandle handle = {};
                ON_SCOPE_EXIT {
                    RESULT_ABORT_UNLESS(handle.Close());
                };

                /* Open file handle */
                if (file_device == nullptr) {

                    /* Lookup file device */
                    MaxDriveString drive;
                    vp::util::GetDrive(std::addressof(drive), path);
                    RESULT_RETURN_UNLESS(0 < drive.GetLength(), ResultNullFileDevice);

                    file_device = FileDeviceManager::GetInstance()->GetFileDeviceByDrive(drive.GetString());
                    RESULT_RETURN_UNLESS(file_device != nullptr, ResultNullFileDevice);

                    /* Open file */
                    MaxPathString path_no_drive;
                    vp::util::GetPathWithoutDrive(std::addressof(path_no_drive), path);

                    const Result open_result = file_device->OpenFile(std::addressof(handle), path_no_drive.GetString(), OpenMode::Read);
                    RESULT_RETURN_UNLESS(open_result == ResultSuccess, open_result);

                } else {
                    /* Open file */
                    const Result open_result = file_device->OpenFile(std::addressof(handle), path, OpenMode::Read);
                    RESULT_RETURN_UNLESS(open_result == ResultSuccess, open_result);
                }

                /* Get file size */
                size_t       file_size   = 0;
                const Result size_result = file_device->GetFileSize(std::addressof(file_size), std::addressof(handle));
                RESULT_RETURN_UNLESS(size_result == ResultSuccess, size_result);

                /* Align alignment */
                s32 output_alignment = (0x20 < file_load_context->file_alignment) ? file_load_context->file_alignment : FileDeviceBase::cMinimumFileAlignment;

                /* Get stream base */
                const size_t  dctx_size   = ::ZSTD_estimateDCtxSize();
                void *stream = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(m_work_memory) + dctx_size + cLeftoverSize);

                /* First temp read */
                size_t       file_offset = 0;
                const Result read_result = file_device->ReadFile(stream, std::addressof(file_offset), std::addressof(handle), cReadSize, file_offset);
                RESULT_RETURN_UNLESS(read_result == ResultSuccess, read_result);                

                /* Get frame header */
                ZSTD_frameHeader frame_header = {};
                const size_t frame_header_result = ::ZSTD_getFrameHeader_advanced(std::addressof(frame_header), stream, file_offset, ZSTD_f_zstd1);
                RESULT_RETURN_IF(::ZSTD_isError(frame_header_result) == true, ResultZstdError);

                /* Get decompressed size */
                const size_t decomp_size = frame_header.frameContentSize;
                RESULT_RETURN_IF(decomp_size == ZSTD_CONTENTSIZE_UNKNOWN || decomp_size == 0, ResultInvalidZstdFrameContentSize);

                /* Check whether the otuput memory needs to be allocated */
                if (file_load_context->file_buffer == nullptr) {

                    /* Allocate file memory */
                    file_load_context->file_buffer = ::operator new(decomp_size, file_load_context->file_heap, output_alignment);
                    RESULT_RETURN_IF(file_load_context->file_buffer == nullptr, ResultFailedToAllocateFileMemory);

                    /* Set output file sizes */
                    file_load_context->file_size                    = decomp_size;
                    file_load_context->file_alignment               = output_alignment;
                    file_load_context->out_is_file_memory_allocated = true;
                } else {
                    RESULT_RETURN_UNLESS(decomp_size < file_load_context->file_size, ResultOutputBufferTooSmall);
                }

                /* Handle file allocation error */
                auto error_after_alloc_guard = SCOPE_GUARD {
                    if (file_load_context->out_is_file_memory_allocated == true) {
                        ::operator delete(file_load_context->file_buffer);
                        file_load_context->file_buffer = nullptr;
                    }
                };

                /* Try lookup zsdic */
                const u32   index      = m_dic_map.TryGetIndexByKey(frame_header.dictID);
                ZSTD_DDict *dictionary = (index != DictionaryMap::cInvalidEntryIndex) ? m_dic_array[index] : nullptr;

                /* Initialize decompression context */
                void         *work_memory = m_work_memory;
                ZSTD_DCtx    *dctx        = ::ZSTD_initStaticDCtx(work_memory, dctx_size);
                VP_ASSERT(dctx != nullptr);

                /* Single load */
                if (file_offset < cReadSize) {

                    /* Decompress */
                    size_t size_decomped = 0;
                    RESULT_RETURN_UNLESS(vp::codec::DecompressZstdWithContext(std::addressof(size_decomped), dctx, file_load_context->file_buffer, decomp_size, stream, file_offset, dictionary) == ResultSuccess, ResultZstdDecompressionFailed);
                    RESULT_RETURN_UNLESS(size_decomped == decomp_size, ResultZstdDecompressionFailed);

                    error_after_alloc_guard.Cancel();

                    if (out_size != nullptr) {
                        *out_size = decomp_size;
                    }
                    if (out_alignment != nullptr) {
                        *out_alignment = output_alignment;
                    }

                    RESULT_RETURN_SUCCESS;
                }

                /* Setup stream context */
                vp::codec::ZstdStreamContext stream_context = {
                    .state           = vp::codec::ZstdStreamContext::State::Begin,
                    .output          = file_load_context->file_buffer,
                    .output_size     = decomp_size,
                    .leftover_stream = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(work_memory) + dctx_size),
                    .leftover_size   = cLeftoverSize,
                    .zstd_dctx       = dctx,
                    .zstd_ddict      = dictionary,
                };

                /* Make sure the thread has completed any past file operations */
                m_decompressor_thread->m_decompress_finish_event.Wait();

                /* Set thread stream context */
                m_decompressor_thread->m_stream_memory          = stream;
                m_decompressor_thread->m_current_stream_context = std::addressof(stream_context);
                ON_SCOPE_EXIT {
                    m_decompressor_thread->m_current_stream_context = nullptr;
                };

                /* Streaming decompression */
                bool   temp_select   = true;
                size_t size_read     = cReadSize;
                do {
                    
                    /* Signal decompressor */
                    m_decompressor_thread->m_stream_finish_event.Clear();
                    m_decompressor_thread->SendMessage(size_read);

                    /* Select output buffer */
                    void *select_stream  = (temp_select == true) ? reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(stream) + cReadSize) : stream;
                    temp_select  ^= 1;

                    /* Stream next block */
                    size_read = 0;
                    const Result read_result = file_device->ReadFile(select_stream, std::addressof(size_read), std::addressof(handle), cReadSize, file_offset);
                    file_offset += size_read;

                    /* Wait for decompression */
                    m_decompressor_thread->m_stream_finish_event.Wait();

                    /* Ensure errors did not occur */
                    RESULT_RETURN_UNLESS(read_result == ResultSuccess, read_result);
                    RESULT_RETURN_IF(m_decompressor_thread->m_is_error == true, ResultStreamDecompressionError);

                } while (file_offset < file_size);

                /* Final decomp */
                if (0 < stream_context.expected_left) {
                    m_decompressor_thread->m_stream_finish_event.Clear();
                    m_decompressor_thread->SendMessage(size_read);
                    m_decompressor_thread->m_stream_finish_event.Wait();
                    RESULT_RETURN_IF(vp::codec::ZstdStreamContext::cInvalidStreamState == stream_context.expected_left, ResultStreamDecompressionError);
                }

                error_after_alloc_guard.Cancel();

                if (out_size != nullptr) {
                    *out_size = decomp_size;
                }
                if (out_alignment != nullptr) {
                    *out_alignment = output_alignment;
                }

                RESULT_RETURN_SUCCESS;
            }
        public:
            void RegisterZsDic(ZSTD_DDict *zstd_ddict) {

                /* Get dic id */
                u32 dic_id = 0;
                RESULT_ABORT_UNLESS(vp::codec::GetZstdDDictIdFromDDict(std::addressof(dic_id), zstd_ddict));

                /* Allocate index */
                const u32 index = static_cast<u32>(m_dic_index_allocator.Allocate());
                VP_ASSERT(index != DictionaryAllocator::cInvalidHandle);

                /* Set ddict and add to map */
                m_dic_array[index] = zstd_ddict;
                m_dic_map.AddEntry(dic_id, index);

                return;
            }

            void UnregisterZsDic(ZSTD_DDict *zstd_ddict) {

                /* Clear map */
                m_dic_map.Clear();

                /* Rebuild without removed */
                for (u32 i = 0; i < cMaxDictionaryCount; ++i) {
                    if (m_dic_array[i] == zstd_ddict) {
                        m_dic_index_allocator.Free(i);
                        m_dic_array[i] = nullptr;
                        continue;
                    }
                    if (m_dic_array == nullptr) { continue; }

                    /* Get dic id */
                    u32 dic_id = 0;
                    RESULT_ABORT_UNLESS(vp::codec::GetZstdDDictIdFromDDict(std::addressof(dic_id), m_dic_array[i]));

                    /* Add to map */
                    m_dic_map.AddEntry(dic_id, i);
                }

                return;
            }

            constexpr ALWAYS_INLINE void ClearZsDic() {

                /* Clear map */
                m_dic_map.Clear();
                m_dic_index_allocator.Clear();

                /* Rebuild without */
                for (u32 i = 0; i < cMaxDictionaryCount; ++i) {
                    m_dic_array[i] = nullptr;
                }

                return;
            }
    };
}
