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

#ifdef VP_IMAGE_ENABLE_PNG

namespace vp::image {

    void PngDecoder::ReleaseReadStruct() {
        ::png_destroy_read_struct(std::addressof(m_meta_data->struct_p), std::addressof(m_meta_data->info_p), nullptr);
        m_stream_reader.Finalize();
    }

    void PngDecoder::Initialize(PngMemoryAllocate allocate, void *allocate_user_data, PngMemoryFree free, void *free_user_data) {
        VP_ASSERT(m_status == Status::Uninitialized || m_status == Status::Decoded);
        m_allocator.Initialize(allocate, allocate_user_data, free, free_user_data);
        m_status = Status::Initialized;
    }

    void PngDecoder::SetImageData(const void *data, size_t data_size) {
        VP_ASSERT(m_status == Status::Initialized || m_status == Status::Analyzed);
        m_image_data = data;
        m_image_size = data_size;
        if (m_status == Status::Analyzed) {
            this->ReleaseReadStruct();
        }
        m_status = Status::SetImageData;
    }

    Result PngDecoder::Analyze() {

        VP_ASSERT(m_status == Status::SetImageData);

        /* Allocate meta data */
        if (m_meta_data == nullptr) {
            MetaData *data = reinterpret_cast<MetaData*>(m_allocator.Allocate(sizeof(MetaData)));
            RESULT_RETURN_IF(data == nullptr, ResultOutOfMemory);
            m_meta_data = data;
        }

        /* Read image data */
        png_structp structp = ::png_create_read_struct_2(png_libpng_ver, std::addressof(m_error_result), impl::PngErrorCallback, impl::PngWarningCallback, std::addressof(m_allocator), impl::PngMemoryAllocateCallback, impl::PngMemoryFreeCallback);
        RESULT_RETURN_IF(structp == nullptr, ResultFailedToAnalyzeImage);
        m_meta_data->struct_p = structp;

        /* Create info struct */
        png_infop infop = ::png_create_info_struct(structp);
        if (infop == nullptr) {
            ::png_destroy_read_struct(std::addressof(m_meta_data->struct_p), nullptr, nullptr);
            RESULT_RETURN(ResultFailedToAnalyzeImage);
        }
        m_meta_data->info_p = infop;

        /* Stream image */
        m_stream_reader.Initialize(m_image_data, m_image_size);
        ::png_set_read_fn(structp, std::addressof(m_stream_reader), impl::PngReadDataCallback);
        const Result result = impl::PngAnalyzeImpl(std::addressof(m_dimensions), std::addressof(m_input_pixel_format), m_output_pixel_format, structp, infop);
        if (RESULT_FAILED(result)) {
            ::png_destroy_read_struct(std::addressof(m_meta_data->struct_p), std::addressof(m_meta_data->info_p), nullptr);
            m_stream_reader.Finalize();
            RESULT_RETURN(result);
        }

        /* Get row stride */
        const PixelFormat format = (m_output_pixel_format != PixelFormat::Invalid) ? m_output_pixel_format : m_input_pixel_format;
        m_row_stride_min = impl::PngGetRowStride(format, m_dimensions[0]);

        m_status = Status::Analyzed;
        
        RESULT_RETURN_SUCCESS;
    }

    Result PngDecoder::Decode(void *output, size_t output_size, size_t row_stride) {
        VP_UNUSED(output_size);
        VP_ASSERT(m_status == Status::Analyzed);

        /* Allocate row meta data */
        void **row_array = reinterpret_cast<void**>(m_allocator.Allocate(sizeof(size_t) * m_dimensions[1]));
        
        Result result;
        {
            ON_SCOPE_EXIT {
                this->ReleaseReadStruct();
            };
            if (row_array == nullptr) {
                RESULT_RETURN(ResultOutOfMemory);
            }

            /* Decode png */
            result = impl::PngDecodeImpl(output, row_array, m_dimensions, row_stride, m_meta_data->struct_p);
        }
        m_allocator.Free(row_array);

        m_status = Status::Decoded;

        return result;
    }
}

#endif
