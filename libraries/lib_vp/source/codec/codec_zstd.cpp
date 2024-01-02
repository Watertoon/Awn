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
#include <vp.hpp>

namespace vp::codec {

    Result GetZstdDDictIdFromStream(u32 *out_id, const void *zstd_stream, size_t stream_size) {
        u32 id = ::ZSTD_getDictID_fromFrame(zstd_stream, stream_size);
        RESULT_RETURN_IF(id == 0, ResultInvalidZstdDictionary);
        if (out_id != nullptr) {
            *out_id = id;
        }
        RESULT_RETURN_SUCCESS;
    }

    Result GetZstdDDictIdFromDDict(u32 *out_id, const ZSTD_DDict *zstd_ddict) {
        u32 id = ::ZSTD_getDictID_fromDDict(zstd_ddict);
        RESULT_RETURN_IF(id == 0, ResultInvalidZstdDictionary);
        if (out_id != nullptr) {
            *out_id = id;
        }
        RESULT_RETURN_SUCCESS;
    }

    Result DecompressZstd(size_t *out_read_size, void *output, size_t output_size, void *zstd_stream, size_t zstd_stream_size) {
        const size_t result = ::ZSTD_decompress(output, output_size, zstd_stream, zstd_stream_size);
        RESULT_RETURN_IF(::ZSTD_isError(result) == true, ResultZstdError);
        if (out_read_size != nullptr) {
            *out_read_size = result;
        }
        RESULT_RETURN_SUCCESS;
    }

    Result DecompressZstdWithContext(size_t *out_read_size, ZSTD_DCtx *context, void *output, size_t output_size, void *zstd_stream, size_t zstd_stream_size) {
        const size_t result = ::ZSTD_decompressDCtx(context, output, output_size, zstd_stream, zstd_stream_size);
        RESULT_RETURN_IF(::ZSTD_isError(result) == true, ResultZstdError);
        if (out_read_size != nullptr) {
            *out_read_size = result;
        }
        RESULT_RETURN_SUCCESS;
    }

    Result DecompressZstdWithContext(size_t *out_read_size, ZSTD_DCtx *context, void *output, size_t output_size, void *zstd_stream, size_t zstd_stream_size, ZSTD_DDict *zstd_ddict) {
        const size_t result = ::ZSTD_decompress_usingDDict(context, output, output_size, zstd_stream, zstd_stream_size, zstd_ddict);
        RESULT_RETURN_IF(::ZSTD_isError(result) == true, ResultZstdError);
        if (out_read_size != nullptr) {
            *out_read_size = result;
        }
        RESULT_RETURN_SUCCESS;
    }

    Result GetDecompressedSizeZstd(size_t *out_size, void *zstd_stream, size_t zstd_stream_size) {
        const size_t result = ::ZSTD_getFrameContentSize(zstd_stream, zstd_stream_size);
        RESULT_RETURN_IF(::ZSTD_isError(result) == true, ResultZstdError);
        if (out_size != nullptr) {
            *out_size = result;
        }
        RESULT_RETURN_SUCCESS;
    }

    size_t StreamDecompressZstd(void *stream, size_t stream_size, ZstdStreamContext *stream_context) {

        /* Integrity checks */
        VP_ASSERT(stream_context != nullptr);

        /* Parse header */
        if (stream_context->state == ZstdStreamContext::State::Begin) {

            /* Get decompressed output size */
            const size_t expected_size = ::ZSTD_getFrameContentSize(stream, stream_size);
            if (::ZSTD_isError(expected_size) != 0 || stream_context->output_size < expected_size) { stream_context->state = ZstdStreamContext::State::Error; return ZstdStreamContext::cInvalidStreamState; }
            stream_context->expected_left = expected_size;

            /* Start decompression with ddict if available */
            const size_t result0 = ::ZSTD_decompressBegin_usingDDict(stream_context->zstd_dctx, stream_context->zstd_ddict);
            RESULT_RETURN_IF(::ZSTD_isError(result0) != 0, ResultZstdError);

            stream_context->state = ZstdStreamContext::State::Streaming;
        }

        /* Update stream state on scope exit */
        size_t  expected_size_left = stream_context->expected_left;
        size_t  output_size_left   = stream_context->output_size - stream_context->output_used;
        size_t  leftover_used_iter = stream_context->leftover_used;
        void   *output_iter        = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(stream_context->output) + stream_context->output_used);
        void   *stream_iter        = stream;
        ON_SCOPE_EXIT {
            stream_context->leftover_used = leftover_used_iter;
            stream_context->expected_left = expected_size_left;
            stream_context->output_used   = stream_context->output_size - output_size_left;
        };

        /* Streaming decompression loop */
        size_t next_size          = 0;
        size_t stream_size_iter   = stream_size;
        while (0 < stream_size_iter) {

            /* Get next source size */
            next_size = ::ZSTD_nextSrcSizeToDecompress(stream_context->zstd_dctx);
            if (next_size == 0) { stream_context->state = ZstdStreamContext::State::Finished; return 0; }

            /* No leftover */
            if (leftover_used_iter == 0) {

                /* Setup leftovers if we do not have enough stream to decompress */
                if (stream_size_iter < next_size) {

                    /* Ensure enough leftover memory for source size */
                    if (stream_context->leftover_size < next_size) { stream_context->state = ZstdStreamContext::State::Error; return ZstdStreamContext::cInvalidStreamState; }

                    /* Copy leftover data */
                    ::memcpy(stream_context->leftover_stream, stream_iter, stream_size_iter);
                    leftover_used_iter += stream_size_iter;

                    break;
                }

                /* Stream decompress stream */
                const size_t size_read = ::ZSTD_decompressContinue(stream_context->zstd_dctx, output_iter, output_size_left, stream_iter, next_size);
                if (::ZSTD_isError(size_read) != 0) { stream_context->state = ZstdStreamContext::State::Error; return ZstdStreamContext::cInvalidStreamState; }

                /* Advance iters */
                output_iter         = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(output_iter) + size_read);
                stream_iter         = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(stream_iter) + next_size);
                stream_size_iter   -= next_size;
                output_size_left   -= size_read;
                expected_size_left -= size_read;

                continue;
            }

            /* Copy in new stream data to leftovers if under next size threshold */
            if (leftover_used_iter < next_size) {

                const size_t next_size_left = next_size - leftover_used_iter;
                const size_t append_size = (next_size_left < stream_size_iter) ? next_size_left : stream_size_iter;

                ::memcpy(reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(stream_context->leftover_stream) + leftover_used_iter), stream_iter, append_size);

                /* Advance iterators */
                leftover_used_iter += append_size;
                stream_iter         = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(stream_iter) + append_size);
                stream_size_iter   -= append_size;
            }

            /* Check if we have reached the next decompress threshold */
            if (leftover_used_iter < next_size) { continue; }

            /* Stream decompress leftover stream */
            const size_t size_read = ::ZSTD_decompressContinue(stream_context->zstd_dctx, output_iter, output_size_left, stream_context->leftover_stream, next_size);
            if (::ZSTD_isError(size_read) != 0) { stream_context->state = ZstdStreamContext::State::Error; return ZstdStreamContext::cInvalidStreamState; }

            /* Adjust iters */
            output_iter         = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(output_iter) + size_read);
            output_size_left   -= size_read;
            expected_size_left -= size_read;

            /* Setup next leftovers */
            const size_t next_left = leftover_used_iter - next_size;
            if (next_left != 0) {
                ::memcpy(stream_context->leftover_stream, reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(stream_context->leftover_stream) + next_size), next_left);
            }
            leftover_used_iter = next_left;
        }

        return expected_size_left;
    }
}
