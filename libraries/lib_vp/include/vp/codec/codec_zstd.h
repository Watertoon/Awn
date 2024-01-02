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

namespace vp::codec {

    Result GetZstdDDictIdFromStream(u32 *out_id, const void *zstd_stream, size_t stream_size);
    Result GetZstdDDictIdFromDDict(u32 *out_id, const ZSTD_DDict *zstd_ddict);

    Result DecompressZstd(size_t *out_read_size, void *output, size_t output_size, void *zstd_stream, size_t zstd_stream_size);
    Result DecompressZstdWithContext(size_t *out_read_size, ZSTD_DCtx *context, void *output, size_t output_size, void *zstd_stream, size_t zstd_stream_size);
    Result DecompressZstdWithContext(size_t *out_read_size, ZSTD_DCtx *context, void *output, size_t output_size, void *zstd_stream, size_t zstd_stream_size, ZSTD_DDict *zstd_ddict);

    Result GetDecompressedSizeZstd(size_t *out_size, void *zstd_stream, size_t zstd_stream_size);

    struct ZstdStreamContext {
        enum class State : u32 {
            Begin,
            Streaming,
            Finished,
            Error,
        };
        static constexpr size_t cInvalidStreamState = 0xffff'ffff'ffff'ffff;

        State       state;
        void       *output;
        size_t      output_size;
        size_t      output_used;
        void       *leftover_stream;
        size_t      leftover_size;
        size_t      leftover_used;
        ZSTD_DCtx  *zstd_dctx;
        ZSTD_DDict *zstd_ddict;
        size_t      expected_left;
    };

    size_t StreamDecompressZstd(void *stream, size_t stream_size, ZstdStreamContext *stream_context);
}
