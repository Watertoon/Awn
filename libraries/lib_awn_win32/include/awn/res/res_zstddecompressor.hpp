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
                private:
                    u32                           m_stream_count;
                    u32                           m_is_error;
                    void                         *m_stream_memory;
                    vp::codec::ZstdStreamContext *m_current_stream_context;
                    sys::ServiceEvent             m_decompress_finish_event;
                    sys::ServiceEvent             m_stream_finish_event;
                public:
                    DecompressorThread() : ServiceThread(), m_current_stream_context(), m_decompress_finish_event(), m_stream_finish_event() {
                        m_decompress_finish_event.Initialize();
                        m_stream_finish_event.Initialize();
                    }
                    virtual ~DecompressorThread() override {
                        m_decompress_finish_event.Finalize();
                        m_stream_finish_event.Finalize();
                    }

                    virtual void ThreadCalc(size_t stream_size) override {

                        /* Flip output */
                        zstd_stream = ((m_stream_count & 1) == 0) ? m_stream_memory : reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(m_stream_memory) + stream_size);

                        /* Stream decompress zstd*/
                        size_t expected = vp::codec::StreamDecompressZstd(zstd_stream, stream_size, m_current_stream_context);

                        /* Increment stream count */
                        ++m_stream_count;

                        /* Update error state */
                        m_is_error = (expected == vp::codec::ZstdStreamContext::cInvalidStreamState);

                        /* Signal events */
                        if (m_current_stream_context->status == vp::codec::ZstdStreamContext::Status::Finished || m_current_stream_context->status == vp::codec::ZstdStreamContext::Status::Error) {
                            m_decompress_finish_event.Signal();
                        }
                        m_stream_finish_event.Signal();

                        return;
                    }
            };
        private:
            DecompressorThread   *m_decompressor_thread;
            ZSTD_DCtx            *m_dctx;
            void                 *m_leftover;
            void                 *m_temp_buffer;
            DictionaryMap         m_dic_map;
            Zstd_DDict           *m_dic_array[cMaxDictionaryCount];
            DictionaryAllocator   m_dic_index_allocator;
        public:
            VP_RTTI_DERIVED(ZstdDecompressor, IDecompressor);
        public:
            constexpr ZstdDecompressor() : m_decompressor_thread(), m_dic_map(), m_dic_array{} {/*...*/}
            constexpr virtual ~ZstdDecompressor() {/*...*/}

            void Initialize(mem::Heap *heap) {

                /* Allocate decomp system memory */
                m_dctx        = reinterpret_cast<ZSTD_DCtx*>(::operator new(cReadSize * 2 + cLeftoverSize + sizeof(ZSTD_DCtx), heap, 0x20));
                m_leftover    = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(m_dctx) + sizeof(ZSTD_DCtx));
                m_temp_buffer = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(m_dctx) + sizeof(ZSTD_DCtx) + cLeftoverSize);

                /* Create thread */
                m_decompressor_thread = new DecompressorThread();

                /* Clear ZsDic */
                this->ClearZsDic();

                return;
            }

            void Finalize() {
                if (m_decompressor_thread != nullptr) {
                    delete m_decompressor_thread;
                    m_decompressor_thread = nullptr;
                }
                if (m_dctx != nullptr) {
                    ::operator delete(reinterpret_cast<void*>(m_dctx));
                    m_dctx        = nullptr;
                    m_leftover    = nullptr;
                    m_temp_buffer = nullptr;
                }
            }

            virtual void SetPriority(u32 priority) override {
                m_decompressor_thread->SetPriority(priority);
            }
            virtual void SetCoreMask(sys::CoreMask core_mask) override {
                m_decompressor_thread->SetCoreMask(core_mask);
            }

            virtual Result TryLoadDecompress(size_t *out_size, u32 *out_alignment, const char *path, ResourceLoadContext *resource_load_context) override {

                /* Integrity checks */
                RESULT_RETURN_IF(resource_load_context->file != nullptr && resource_load_context->file_size == 0, ResultInvalidFileBufferSize);
                RESULT_RETURN_IF(resource_load_context->read_div != 0,                                            ResultInvalidReadDivAlignment);

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
                RESULT_RETURN_UNLESS(file_size < 0x18, size_result);

                /* Align alignment */
                size_t output_alignment = vp::util::AlignUp(resource_load_context->file_alignment, alignof(u64));

                /* First temp read */
                size_t       file_offset = 0;
                const Result read_result = this->ReadFile(m_temp_buffer, std::addressof(file_offset), std::addressof(handle), cReadSize, file_offset);
                RESULT_RETURN_UNLESS(read_result == ResultSuccess, read_result);

                /* Get frame header */
                ZSTD_frameHeader frame_header = {};
                const size_t decomp_size = ::ZSTD_getFrameHeader_advanced(std::addressof(frame_header), m_temp_buffer, offset);
                RESULT_RETURN_IF(::ZSTD_isError(decomp_size) == true, ResultZstdError);

                /* Check whether the otuput memory needs to be allocated */
                if (resource_load_context->file == nullptr) {

                    /* Allocate file memory */
                    resource_load_context->file = ::operator new(decomp_size, resource_load_context->heap, output_alignment);
                    RESULT_RETURN_IF(resource_load_context->file == nullptr, ResultFailedToAllocateFileMemory);

                    /* Set output file sizes */
                    resource_load_context->file_size                    = decomp_size;
                    resource_load_context->file_alignment               = output_alignment;
                    resource_load_context->out_is_file_memory_allocated = true;
                } else {
                    RESULT_RETURN_UNLESS(decomp_size < resource_load_context->file_size, ResultOutputBufferTooSmall);
                }

                /* Handle file allocation error */
                auto error_after_alloc_guard = SCOPE_GUARD {
                    if (resource_load_context->out_is_file_memory_allocated == true) {
                        ::operator delete(resource_load_context->file);
                        resource_load_context->file = nullptr;
                    }
                };

                /* Try lookup zsdic */
                const u32   index      = m_dic_map.TryGetIndexByHash(frame_header.dictID);
                ZSTD_DDict *dictionary = (index != DictionaryMap::cInvalidHandle) ? m_dic_array[index] : nullptr;

                /* Single load */
                if (file_offset < cRealSize) {

                    /* Decompress */
                    RESULT_RETURN_UNLESS(vp::codec::DecompressZstdWithDictionary(resource_load_context->file, , m_temp_buffer, dictionary), ResultZstdDecompressionFailed);

                    error_after_alloc_guard.Cancel();

                    *out_size      = file_offset;
                    *out_alignment = output_alignment;

                    RESULT_RETURN_SUCCESS;
                }

                /* Setup stream context */
                vp::codec::ZstdStreamContext stream_context = {
                    .state           = vp::codex::ZstdStreamContext::State::Begin,
                    .output          = resource_load_context->file,
                    .output_size     = resource_load_context->file_size,
                    .leftover_stream = m_leftover,
                    .leftover_size   = cLeftoverSize,
                    .zstd_dctx       = m_dctx,
                    .zstd_ddict      = dictionary,
                };

                /* Make sure the thread has completed any past file operations */
                m_decompressor_thread->m_decompress_finish_event.Wait();
                m_decompressor_thread->m_decompress_finish_event.Clear();

                /* Set new stream context */
                m_decompressor_thread->m_current_stream_context = std::addressof(stream_context);

                /* Streaming decompression */
                bool   temp_select = true;
                size_t size_read = cReadSize;
                do {
                    
                    /* Signal decompressor */
                    m_decompressor_thread->m_decompress_finish_event.Clear();
                    m_decompressor_thread->SendMessage(size_read);

                    /* Select output buffer */
                    void *stream = (temp_select == true) ? reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(m_temp_buffer) + cReadSize) : m_temp_buffer;

                    /* Stream next block */
                    size_read = 0;
                    const Result read_result = this->TryReadFile(stream, std::addressof(size_read), std::addressof(handle), cReadSize, file_offset);
                    file_offset += size_read;
                    RESULT_RETURN_UNLESS(read_result == ResultSuccess, read_result);

                    /* Wait for decompression */
                    m_decompressor_thread->m_decompress_finish_event.Wait();

                    /* Ensure stream did not error */
                    RESULT_RETURN_IF(vp::codec::ZstdStreamContext::cInvalidStreamState == stream_context.expected_left, ResultStreamDecompressionError);

                } while (size_read < cReadSize);

                /* Final decomp */
                if (0 < stream_context.expected_left) {
                    m_decompressor_thread->m_decompress_finish_event.Clear();
                    m_decompressor_thread->SendMessage(size_read);
                    m_decompressor_thread->m_decompress_finish_event.Wait();
                    RESULT_RETURN_IF(vp::codec::ZstdStreamContext::cInvalidStreamState == stream_context.expected_left, ResultStreamDecompressionError);
                }

                error_after_alloc_guard.Cancel();

                *out_size      = file_offset;
                *out_alignment = output_alignment;

                RESULT_RETURN_SUCCESS;
            }
        public:
            void RegisterZsDic(Zstd_DDict *zstd_ddict) {

                /* Get dic id */
                u32 dic_id = 0;
                RESULT_ABORT_UNLESS(vp::codec::GetZstdDDictId(std::addressof(), zstd_ddict));

                /* Allocate index */
                const u32 index = static_cast<u32>(m_dic_index_allocator.Allocate());
                VP_ASSERT(index != DictionaryAllocator::cInvalidIndex);

                /* Set ddict and add to map */
                m_dic_array[index] = zstd_ddict;
                m_dic_map.AddEntry(dic_id, index);

                return;
            }

            void UnregisterZsDic(Zstd_DDict *zstd_ddict) {

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
                    RESULT_ABORT_UNLESS(vp::codec::GetZstdDDictId(m_dic_array[i], zstd_ddict));

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
