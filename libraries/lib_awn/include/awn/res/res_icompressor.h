#pragma once

namespace awn::res {

    struct FileSaveInfo;

    class ICompressor {
        public:
            VP_RTTI_BASE(ICompressor);
        public:
            constexpr ICompressor() {/*...*/}
            constexpr virtual ~ICompressor() {/*...*/}

            virtual Result SaveFileWithCompression(const char *path, FileSaveInfo *save_info, FileDeviceBase *file_device) { VP_UNUSED(path, save_info, file_device); RESULT_RETURN_SUCCESS; }
    };

    class ZstdCompressor : public ICompressor {
        private:
            void *m_work_memory;
            void *m_out_memory;
            size_t m_out_size;
            size_t m_cctx_size;
            u32 m_max_compression_level;
            u32 m_compression_level;
        public:
            VP_RTTI_DERIVED(ZstdCompressor);
        public:
            constexpr ZstdCompressor() : ICompressor(), m_work_memory(), m_out_memory(), m_out_size(), m_cctx_size(), m_max_compression_level(), m_compression_level() {/*...*/}
            constexpr ~ZstdCompressor() {/*...*/}

            void Initialize(mem::Heap *heap, u32 max_compression_level);
            void Finalize();

            virtual Result SaveFileWithCompression(const char *path, FileSaveInfo *save_info, FileDeviceBase *file_device) override;

            constexpr u32 GetMaxCompressionLevel() const {
                return m_max_compression_level;
            }
            constexpr u32 GetCompressionLevel() const {
                return m_compression_level;
            }
            constexpr void SetCompressionLevel(u32 compression_level) {
                m_compression_level = vp::util::Min(compression_level, m_max_compression_level);
            }
    };
}
