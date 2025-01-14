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

namespace vp::image::impl {

    void *PngMemoryAllocator::Allocate(size_t size) {
        VP_ASSERT(m_is_initialized);
        return (m_allocate)(size, m_allocate_user_data);
    }

    void PngMemoryAllocator::Free(void *address) {
        VP_ASSERT(m_is_initialized);
        return (m_free)(address, m_free_user_data);
    }

    bool PngStreamReader::Read(void *out, size_t read_size) {
        VP_ASSERT(m_is_initialized);
        const size_t size = m_size;
        const size_t offset = m_offset;
        const size_t next_offset = offset + read_size;
        if (size < next_offset) { return false; }
        void *read_pos = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(m_input) + offset);
        ::memcpy(out, read_pos, read_size);
        m_offset = next_offset;
        return true;
    }

    bool PngStreamWriter::Write(void *in, size_t write_size) {
        VP_ASSERT(m_is_initialized);
        const size_t size = m_size;
        const size_t offset = m_offset;
        const size_t next_offset = offset + write_size;
        if (size < next_offset) { return false; }
        void *write_pos = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(m_output) + offset);
        ::memcpy(write_pos, in, write_size);
        m_offset = next_offset;
        return true;
    }

    void PngErrorCallback(png_structp structp, const char *error) {
        Result *result = reinterpret_cast<Result*>(::png_get_error_ptr(structp));
        *result = ResultUnknownPngError;
        if (vp::util::StringCompare(error, "Out of Memory.\n")) {
            *result = ResultOutOfMemory;
        }
        if (vp::util::StringCompare(error, "Invalid read position for png data.\n")) {
            *result = ResultInvalidReadPosition;
        }
        if (vp::util::StringCompare(error, "Invalid write position for png data.\n")) {
            *result = ResultInvalidWritePosition;
        }
        ::png_longjmp(structp, 1);
    }

    void PngWarningCallback(png_structp structp, const char *warn) { VP_UNUSED(structp, warn); }

    void *PngMemoryAllocateCallback(png_structp structp, png_alloc_size_t size) {
        PngMemoryAllocator *allocator = reinterpret_cast<PngMemoryAllocator*>(::png_get_mem_ptr(structp));
        void *memory = allocator->Allocate(size);
        if (memory != nullptr) { return memory; }
        ::png_error(structp, "Out of Memory.\n");
    }

    void PngMemoryFreeCallback(png_structp structp, png_voidp address) {
        PngMemoryAllocator *allocator = reinterpret_cast<PngMemoryAllocator*>(::png_get_mem_ptr(structp));
        allocator->Free(address);
    }

    void PngReadDataCallback(png_structp structp, png_bytep output, size_t read_size) {
        PngStreamReader *reader = reinterpret_cast<PngStreamReader*>(::png_get_io_ptr(structp));
        const bool result = reader->Read(output, read_size);
        if (result) { return; }
        ::png_error(structp, "Invalid read position for png data.\n");
    }

    void PngWriteDataCallback(png_structp structp, png_bytep input, size_t write_size) {
        PngStreamWriter *writer = reinterpret_cast<PngStreamWriter*>(::png_get_io_ptr(structp));
        const bool result = writer->Write(input, write_size);
        if (result) { return; }
        ::png_error(structp, "Invalid read position for png data.\n");
    }

    void PngFlushDataCallback(png_structp structp) { VP_UNUSED(structp); }

    void PngTransform(png_structp structp, png_infop infop, PixelFormat format) {

        /* Get depth and color type */
        const u32 bit_depth = ::png_get_bit_depth(structp, infop);
        const u32 color_type = ::png_get_color_type(structp, infop);
        
        /* Ensure valid output format */
        if (format == PixelFormat::Invalid || 0xf < static_cast<u32>(format)) {
            VP_UNEXPECTED_DEFAULT();
        }
        const u32 format_depth = GetBitDepth(format);

        /* Set pallete to rgb conversion */
        if (color_type == PNG_COLOR_TYPE_PALETTE) {
            ::png_set_palette_to_rgb(structp);
        }

        /* Set gray to color or color to gray */
        if (IsColorFormat(format)) {
            if ((color_type & PNG_COLOR_MASK_COLOR) == 0) {
                ::png_set_gray_to_rgb(structp);                    
            }
        } else if ((color_type & PNG_COLOR_MASK_COLOR) != 0) {
            ::png_set_rgb_to_gray(structp, 1, PNG_RGB_TO_GRAY_DEFAULT, PNG_RGB_TO_GRAY_DEFAULT);
        }

        const u32 flag = ::png_get_valid(structp, infop, PNG_INFO_tRNS);

        /* Set add alpha for alpha output format, or strip alpha for non-alpha output format */
        if (IsAlphaFormat(format)) {
            if ((color_type & PNG_COLOR_MASK_ALPHA) == 0) {
                if (flag == 0) {                        
                    ::png_set_add_alpha(structp, 0xffff, PNG_FILLER_AFTER);
                } else {
                    ::png_set_tRNS_to_alpha(structp);
                }
            }
        } else if ((color_type & PNG_COLOR_MASK_ALPHA) != 0 || flag != 0) {                
            ::png_set_strip_alpha(structp);
        }

        /* Set image scale */
        if (format_depth == 8) {
            if (bit_depth == 0x10) {
                ::png_set_scale_16(structp);
            } else if (bit_depth < 8) {
                ::png_set_expand(structp);
            }
        } else if (format_depth == 0x10) {
            if (bit_depth < 0x10) {
                ::png_set_expand_16(structp);
            }
            ::png_set_swap(structp);
        }

        return;
    }

    Result PngAnalyzeImpl(vp::util::Int2 *out_dimensions, PixelFormat *out_format, PixelFormat output_format, png_structp structp, png_infop infop) {
        
        /* Set longjmp */
        auto env = png_jmpbuf(structp);
        const s32 is_jmp = ::setjmp(env);
        if (is_jmp != 0) {
            Result *error_ptr = reinterpret_cast<Result*>(::png_get_error_ptr(structp));
            return *error_ptr;
        }

        /* Read info */
        ::png_read_info(structp, infop);

        /* Get depth and color type */
        const u32 bit_depth = ::png_get_bit_depth(structp, infop);
        const u32 color_type = ::png_get_color_type(structp, infop);

        /* Force convert pallete to R8G8B8 */
        const u32 conv_color_type = (color_type == PNG_COLOR_TYPE_PALETTE) ? PNG_COLOR_TYPE_RGB : color_type;
        const u32 conv_depth = (color_type == PNG_COLOR_TYPE_PALETTE) ? 8 : bit_depth;

        /* Adjust if tRNS */
        const u32 flag = ::png_get_valid(structp, infop, PNG_INFO_tRNS);
        const u32 flag_depth = (conv_depth < 8) ? 8 : conv_depth;
        const u32 adj_depth = (flag != 0) ? flag_depth : conv_depth;
        const u32 adj_color_type = (flag != 0) ? conv_color_type | PNG_COLOR_MASK_ALPHA : conv_color_type;

        /* Create output format */
        const PixelFormat input_format = PngMakePixelFormat(adj_depth, adj_color_type);
        *out_format = input_format;

        /* Transform image data */
        const PixelFormat transform_format = (output_format != PixelFormat::Invalid) ? output_format : input_format;
        PngTransform(structp, infop, transform_format);

        /* Get image dimensions */
        (*out_dimensions)[0] = ::png_get_image_width(structp, infop);
        (*out_dimensions)[1] = ::png_get_image_height(structp, infop);

        /* Read update info */
        ::png_read_update_info(structp, infop);

        RESULT_RETURN_SUCCESS;
    }

    Result PngDecodeImpl(void *output, void **output_row_array, vp::util::Int2 dimensions, size_t row_size, png_structp structp) {

        /* Set longjmp */
        auto env = png_jmpbuf(structp);
        const s32 is_jmp = ::setjmp(env);
        if (is_jmp != 0) {
            Result *error_ptr = reinterpret_cast<Result*>(::png_get_error_ptr(structp));
            return *error_ptr;
        }

        /* Partition rows */
        uintptr_t output_i = reinterpret_cast<uintptr_t>(output);
        const s32 height = dimensions[1];
        for (s32 i = 0; i < height; ++i) {
            output_row_array[i] = reinterpret_cast<void*>(output_i);
            output_i += row_size;
        }

        /* Read image */
        ::png_read_image(structp, reinterpret_cast<png_bytepp>(output_row_array));

        RESULT_RETURN_SUCCESS;
    }

    Result PngEncodeImpl(const void *input, void **input_row_array, vp::util::Int2 dimensions, PixelFormat encode_format, size_t row_size, png_structp structp, png_infop infop) {

        /* Set longjmp */
        auto env = png_jmpbuf(structp);
        const s32 is_jmp = ::setjmp(env);
        if (is_jmp != 0) {
            Result *error_ptr = reinterpret_cast<Result*>(::png_get_error_ptr(structp));
            return *error_ptr;
        }

        /* Partition rows */
        uintptr_t input_i = reinterpret_cast<uintptr_t>(input);
        const s32 height = dimensions[1];
        for (s32 i = 0; i < height; ++i) {
            input_row_array[i] = reinterpret_cast<void*>(input_i);
            input_i += row_size;
        }
        ::png_set_rows(structp, infop, reinterpret_cast<png_bytepp>(input_row_array));

        /* Setup meta data  */
        const u32 bit_depth = GetBitDepth(encode_format);
        const u32 color_type = PngGetColorType(encode_format);
        ::png_set_filter(structp, 0, PNG_FILTER_NONE);
        ::png_set_IHDR(structp, infop, dimensions[0], dimensions[1], bit_depth, color_type, 0, 0, 0);

        /* Write png */
        const u32 write_flags = (bit_depth == 0x10) ? PNG_TRANSFORM_SWAP_ENDIAN : 0;
        ::png_write_png(structp, infop, write_flags, nullptr);

        RESULT_RETURN_SUCCESS;
    }
}

#endif
