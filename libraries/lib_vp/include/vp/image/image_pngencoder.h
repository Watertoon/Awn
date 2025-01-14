#pragma once

namespace vp::image {

    class PngEncoder {
        public:
            enum class Status : u32 {
                Uninitialized,
                Initialized,
                SetPixelData,
            };
        private:
            Status m_status;
            const void *m_pixel_data;
            size_t m_pixel_size;
            PixelFormat m_encode_format;
            vp::util::Int2 m_dimensions;
            size_t m_row_stride;
            impl::PngMemoryAllocator m_allocator;
            Result m_error_result;
        public:
            constexpr PngEncoder() : m_status(Status::Uninitialized), m_allocator() {/*...*/}
            constexpr ~PngEncoder() {/*...*/}

            void Initialize(PngMemoryAllocate allocate, void *allocate_user_data, PngMemoryFree free, void *free_user_data);

            void SetPixelData(const void *data, size_t data_size, PixelFormat pixel_format, vp::util::Int2 dimensions, size_t row_stride);

            Result Encode(size_t *out_write_size, void *output_memory, size_t output_size);

            constexpr size_t GetMaxPngSize() const {
                const size_t dim_size = m_dimensions[1] + m_dimensions[1] * m_row_stride;
                const size_t mip_size0 = dim_size + ((dim_size + 7) >> 3) + ((dim_size + 0x3f) >> 6) + 0xb;
                const size_t mip_size1 = mip_size0 + (mip_size0 >> 0xd) * 0xc + 0x39;
                return mip_size1;
            }
    };
}
