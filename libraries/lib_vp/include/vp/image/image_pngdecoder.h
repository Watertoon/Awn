#pragma once

#include <setjmp.h>
#include <png.h>

namespace vp::image {

    class PngDecoder {
        public:
            struct MetaData {
                png_structp struct_p;
                png_infop info_p;
            };
        public:
            enum class Status : u32 {
                Uninitialized,
                Initialized,
                SetImageData,
                Analyzed,
                Decoded,
            };
        private:
            Status m_status;
            const void *m_image_data;
            size_t m_image_size;
            vp::util::Int2 m_dimensions;
            PixelFormat m_input_pixel_format;
            PixelFormat m_output_pixel_format;
            size_t m_row_stride_min;
            impl::PngMemoryAllocator m_allocator;
            impl::PngStreamReader m_stream_reader;
            MetaData *m_meta_data;
            Result m_error_result;
        private:
            void ReleaseReadStruct();
        public:
            constexpr PngDecoder() : m_status(Status::Uninitialized), m_image_data(), m_allocator(), m_meta_data() {/*...*/}
            constexpr ~PngDecoder() {/*...*/}

            void Initialize(PngMemoryAllocate allocate, void *allocate_user_data, PngMemoryFree free, void *free_user_data);

            void SetImageData(const void *data, size_t data_size);

            Result Analyze();

            Result Decode(void *output, size_t output_size, size_t row_stride);

            constexpr void SetOutputPixelFormat(PixelFormat format) { m_output_pixel_format = format; }
            constexpr PixelFormat GetOutputPixelFormat() const { return m_output_pixel_format; }
            constexpr vp::util::Int2 GetDimensions() const { return m_dimensions; }
            constexpr size_t GetRowStrideMin() const { return m_row_stride_min; }
    };
}
