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
    