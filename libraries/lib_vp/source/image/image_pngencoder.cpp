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

    void PngEncoder::Initialize(PngMemoryAllocate allocate, void *allocate_user_data, PngMemoryFree free, void *free_user_data) {
        VP_ASSERT(m_status == Status::Uninitialized);
        m_allocator.Initialize(allocate, allocate_user_data, free, free_user_data);
        m_status = Status::Initialized;
    }

    void PngEncoder::SetPixelData(const void *data, size_t data_size, PixelFormat pixel_format, vp::util::Int2 dimensions, size_t row_stride) {
        VP_UNUSED(data_size);
        VP_ASSERT(m_status == Status::Initialized || m_status == Status::SetPixelData);
        m_pixel_data = data;
        m_encode_format = pixel_format;
        m_dimensions = dimensions;
        m_row_stride = row_stride;
        m_status = Status::SetPixelData;
    }

    Result PngEncoder::Encode(size_t *out_write_size, void *output_memory, size_t output_size) {

        /* Create write struct */
        png_structp structp = ::png_create_write_struct_2(png_libpng_ver, std::addressof(m_error_result), impl::PngErrorCallback, impl::PngWarningCallback, std::addressof(m_allocator), impl::PngMemoryAllocateCallback, impl::PngMemoryFreeCallback);
        RESULT_RETURN_IF(structp == nullptr, ResultOutOfMemory);

        /* Create info */
        png_infop infop = ::png_create_info_struct(structp);
        if (infop == nullptr) {
            ::png_destroy_write_struct(std::addressof(structp), nullptr);
            RESULT_RETURN(ResultOutOfMemory);
        }

        /* Allocate row array */
        void **row_array = reinterpret_cast<void**>(m_allocator.Allocate(m_dimensions[1] * sizeof(size_t)));
        if (row_array == nullptr) {
            ::png_destroy_write_struct(std::addressof(structp), std::addressof(infop));
            RESULT_RETURN(ResultOutOfMemory);
        }

        /* Set writer */
        impl::PngStreamWriter writer;
        writer.Initialize(output_memory, output_size);
        ::png_set_write_fn(structp, std::addressof(writer), impl::PngWriteDataCallback, impl::PngFlushDataCallback);

        /* Encode png */
        const Result result = impl::PngEncodeImpl(m_pixel_data, row_array, m_dimensions, m_encode_format, m_row_stride, structp, infop);
        *out_write_size = writer.GetOffset();

        ::png_destroy_write_struct(std::addressof(structp), std::addressof(infop));
        writer.Finalize();
        m_allocator.Free(row_array);

        RESULT_RETURN(result);
    }
}

#endif
