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

    class ParallelZstdDecompressor : public IParallelDecompressor {
        public:
            static constexpr size_t cMaxDictionaryCount = 8;
            static constexpr size_t cReadSize           = 0xd'0000;
            static constexpr size_t cLeftoverSize       = vp::util::c128KB;
        public:
            using DictionaryMap       = vp::util::FixedKeyIndexMap<u32, u8, cMaxDictionaryCount * 2>;
            using DictionaryAllocator = vp::util::FixedIndexAllocator<u8, cMaxDictionaryCount>;
        public:
            class DecompressorThread : public sys::Thread {
                public:
                    friend class ParallelZstdDecompressor;
                private:
                    u32                           m_stream_count;
                    u32                           m_is_error;
                    void                         *m_stream_memory;
                    vp::codec::ZstdStreamContext *m_current_stream_context;
                    sys::Event                    m_decompress_finish_event;
                    sys::Event                    m_stream_finish_event;
                public:
                    constexpr DecompressorThread() : Thread(), m_current_stream_context(), m_decompress_finish_event(sys::SignalState::Signaled, sys::ResetMode::Auto), m_stream_finish_event(sys::SignalState::Cleared, sys::ResetMode::Manual) {/*...*/}
                    virtual ~DecompressorThread() override {/*...*/}

                    virtual void ThreadMain(size_t stream_size) override;
            };
        private:
            DecompressorThread   *m_decompressor_thread;
            void                 *m_work_memory;
            DictionaryMap         m_dic_map;
            ZSTD_DDict           *m_dic_array[cMaxDictionaryCount];
            DictionaryAllocator   m_dic_index_allocator;
        public:
            VP_RTTI_DERIVED(ParallelZstdDecompressor);
        public:
            constexpr ParallelZstdDecompressor() : m_decompressor_thread(), m_work_memory(), m_dic_map(), m_dic_array{}, m_dic_index_allocator() {/*...*/}
            constexpr virtual ~ParallelZstdDecompressor() {/*...*/}

            void Initialize(mem::Heap *heap);
            void Finalize();

            virtual void SetPriority(u32 priority) override;
            virtual void SetCoreMask(sys::CoreMask core_mask) override;
            
            virtual u32           GetPriority() override { return m_decompressor_thread->GetPriority(); }
            virtual sys::CoreMask GetCoreMask() override { return m_decompressor_thread->GetCoreMask(); }

            virtual Result LoadFileWithDecompression(size_t *out_size, s32 *out_alignment, const char *path, FileLoadContext *file_load_context, FileDeviceBase *file_device) override;
        public:
            void RegisterZsDic(ZSTD_DDict *zstd_ddict);
            void UnregisterZsDic(ZSTD_DDict *zstd_ddict);

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
