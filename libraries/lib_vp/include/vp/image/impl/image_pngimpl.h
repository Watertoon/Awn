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

#include <png.h>

namespace vp::image::impl {

    constexpr u32 PngGetColorType(PixelFormat format) {
        const u32 index = static_cast<u32>(format) - 1;
        if (format == PixelFormat::Invalid || 0xe < index) {
            VP_UNEXPECTED_DEFAULT();
        }
        static constexpr u32 cColorTypeArray[15] {
            PNG_COLOR_TYPE_PALETTE,
            PNG_COLOR_TYPE_PALETTE,
            PNG_COLOR_TYPE_PALETTE,
            PNG_COLOR_TYPE_PALETTE,
            PNG_COLOR_TYPE_GRAY,
            PNG_COLOR_TYPE_GRAY,
            PNG_COLOR_TYPE_GRAY,
            PNG_COLOR_TYPE_GRAY,
            PNG_COLOR_TYPE_GRAY,
            PNG_COLOR_TYPE_RGB,
            PNG_COLOR_TYPE_RGB,
            PNG_COLOR_TYPE_GRAY_ALPHA,
            PNG_COLOR_TYPE_GRAY_ALPHA,
            PNG_COLOR_TYPE_RGB_ALPHA,
            PNG_COLOR_TYPE_RGB_ALPHA,
        };
        return cColorTypeArray[index];
    }

    constexpr size_t PngGetRowStride(PixelFormat format, u32 width) {
        const u32 bit_depth = GetBitDepth(format);
        const u32 channel_count = GetChannelCount(format);
        return ((bit_depth * width * channel_count) + 7) / BITSIZEOF(u8);
    }

    constexpr PixelFormat PngMakePixelFormat(u32 bit_depth, u32 color_type) {
        switch (bit_depth) {
            case 1:
            {
                const PixelFormat format0 = (color_type == PNG_COLOR_TYPE_GRAY) ? PixelFormat::Gray1 : PixelFormat::Invalid;
                const PixelFormat format1 = (color_type == PNG_COLOR_TYPE_PALETTE) ? PixelFormat::Pallete1 : format0;
                return format1;
            }
            case 2:
            {
                const PixelFormat format0 = (color_type == PNG_COLOR_TYPE_GRAY) ? PixelFormat::Gray2 : PixelFormat::Invalid;
                const PixelFormat format1 = (color_type == PNG_COLOR_TYPE_PALETTE) ? PixelFormat::Pallete2 : format0;
                return format1;
            }
            case 4:
            {
                const PixelFormat format0 = (color_type == PNG_COLOR_TYPE_GRAY) ? PixelFormat::Gray4 : PixelFormat::Invalid;
                const PixelFormat format1 = (color_type == PNG_COLOR_TYPE_PALETTE) ? PixelFormat::Pallete4 : format0;
                return format1;
            }
            case 8:
            {
                static constexpr PixelFormat cFormat8Array[7] = {
                    PixelFormat::Gray8,
                    PixelFormat::Invalid,
                    PixelFormat::R8G8B8,
                    PixelFormat::Pallete8,
                    PixelFormat::Gray8A8,
                    PixelFormat::Invalid,
                    PixelFormat::R8G8B8A8,
                };
                if (6 < color_type) { return PixelFormat::Invalid; }
                return cFormat8Array[color_type];
            }
            case 16:
            {
                static constexpr PixelFormat cFormat16Array[7] = {
                    PixelFormat::Gray16,
                    PixelFormat::Invalid,
                    PixelFormat::R16G16B16,
                    PixelFormat::Invalid,
                    PixelFormat::Gray16A16,
                    PixelFormat::Invalid,
                    PixelFormat::R16G16B16A16,
                };
                if (6 < color_type) { return PixelFormat::Invalid; }
                return cFormat16Array[color_type];
            }
            default:
                VP_UNEXPECTED_DEFAULT();
        };
    }

    class PngMemoryAllocator {
        private:
            bool m_is_initialized;
            PngMemoryAllocate *m_allocate;
            void *m_allocate_user_data;
            PngMemoryFree *m_free;
            void *m_free_user_data;
        public:
            constexpr PngMemoryAllocator() : m_is_initialized() {/*...*/}
            constexpr ~PngMemoryAllocator() {/*...*/}

            constexpr void Initialize(PngMemoryAllocate *allocate, void *allocate_user_data, PngMemoryFree *free, void *free_user_data) {
                VP_ASSERT(!m_is_initialized);
                VP_ASSERT(allocate != nullptr);
                VP_ASSERT(free != nullptr);
                m_allocate = allocate;
                m_allocate_user_data = allocate_user_data;
                m_free = free;
                m_free_user_data = free_user_data;
                m_is_initialized = true;
            }

            constexpr void Finalize() {
                m_is_initialized = false;
            }

            void *Allocate(size_t size);
            void Free(void *address);
    };

    class PngStreamAccessor {
        protected:
            size_t m_size;
            size_t m_offset;
            bool m_is_initialized;
        public:
            constexpr PngStreamAccessor() : m_is_initialized() {/*...*/}
            constexpr ~PngStreamAccessor() {/*...*/}

            constexpr void Initialize(size_t size) {
                VP_ASSERT(!m_is_initialized);
                m_size = size;
                m_offset = 0;
                m_is_initialized = true;
            }

            constexpr void Finalize() {
                m_is_initialized = false;
            }

            constexpr size_t GetOffset() const {
                return m_offset;
            }
    };

    class PngStreamReader : public PngStreamAccessor {
        protected:
            const void *m_input;
        public:
            constexpr PngStreamReader() : PngStreamAccessor() {/*...*/}
            constexpr ~PngStreamReader() {/*...*/}

            constexpr void Initialize(const void *input, size_t input_size) {
                this->PngStreamAccessor::Initialize(input_size);
                m_input = input;
            }

            bool Read(void *out, size_t read_size);
    };

    class PngStreamWriter : public PngStreamAccessor {
        protected:
            void *m_output;
        public:
            constexpr PngStreamWriter() : PngStreamAccessor() {/*...*/}
            constexpr ~PngStreamWriter() {/*...*/}

            constexpr void Initialize(void *output, size_t output_size) {
                this->PngStreamAccessor::Initialize(output_size);
                m_output = output;
            }

            bool Write(void *in, size_t write_size);
    };

    void PngErrorCallback(png_structp structp, const char *error);
    void PngWarningCallback(png_structp structp, const char *warn);

    void *PngMemoryAllocateCallback(png_structp structp, png_alloc_size_t size);
    void PngMemoryFreeCallback(png_structp structp, png_voidp address);

    void PngReadDataCallback(png_structp structp, png_bytep output, size_t read_size);
    void PngWriteDataCallback(png_structp structp, png_bytep input, size_t write_size);
    void PngFlushDataCallback(png_structp structp);

    void PngTransform(png_structp structp, png_infop infop, PixelFormat format);

    Result PngAnalyzeImpl(vp::util::Int2 *out_dimensions, PixelFormat *out_format, PixelFormat output_format, png_structp structp, png_infop infop);

    Result PngDecodeImpl(void *output, void **output_row_array, vp::util::Int2 dimensions, size_t row_size, png_structp structp);

    Result PngEncodeImpl(const void *input, void **input_row_array,vp::util::Int2 dimensions, PixelFormat encode_format, size_t row_size, png_structp structp, png_infop infop);
}
