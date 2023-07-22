#pragma once

namespace awn::res {

    struct StreamState {};

    struct DecompSize {
        u32             size;
        u32             alignment;
    };

    class DecompressorBase {
        public:
            VP_RTTI_BASE(DecompressorBase);
        public:
            constexpr DecompressorBase() {/*...*/}
            constexpr virtual ~DecompressorBase() {/*...*/}

            virtual DecompSize GetDecompressedSize([[maybe_unused]] void *file, [[maybe_unused]] size_t file_size) {
                return DecompSize{};
            }

            virtual Result TryStreamDecompress([[maybe_unused]] StreamState *stream_state, [[maybe_unused]] void *stream, [[maybe_unused]] size_t stream_size) {
                RESULT_RETURN_SUCCESS;
            }
            
            virtual Result TryDecompress([[maybe_unused]] void *output, [[maybe_unused]] size_t output_size, [[maybe_unused]] void *stream, [[maybe_unused]] size_t stream_size) {
                RESULT_RETURN_SUCCESS;
            }
    };
}
    