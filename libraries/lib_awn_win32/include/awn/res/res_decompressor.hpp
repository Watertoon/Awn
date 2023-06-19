#pragma once

namespace awn::res {

    enum class CompressionType : u16 {
        None        = 0,
        Szs         = 1,
        Zstandard   = 2,
        MeshCodec   = 3,
        Zlib        = 4,
    };

    class DecompressorBase {
        private:
            vp::util::IntrusiveListNode m_mgr_node;
        public:
            constexpr DecompressorBase() {/*...*/}

            //virtual Result TryLoadDecompressedByResource(size_t *out_decompressed_size, FileLoadContext *load_context, Resource *resource) {
            //    RESULT_RETURN_SUCCESS;
            //}
            //virtual Result TryLoadDecompressedByArchive(size_t *out_decompressed_file, FileLoadContext *load_context, ArchiveFileReturn *archive_file_return) {
            //
            //    /* Decompress file */
            //    
            //    RESULT_RETURN_SUCCESS;
            //}
    };
}
