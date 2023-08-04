#pragma once

namespace awn::gfx {

    constexpr void CalculatePrimitiveShapeInfoQuad(PrimitiveShapeInfo *out_primitive_shape_info, ShapeFormat shape_format) {

        /* Get stride */
        const u32 stride = GetPrimitiveShapeStride(shape_format);

        /* Calculate shape info */
        out_primitive_shape_info->vertex_count       = 4;
        out_primitive_shape_info->vertex_buffer_size = stride * 4;
        out_primitive_shape_info->index_count        = 6;
        out_primitive_shape_info->index_buffer_size  = 6;

        /* Set formats */
        out_primitive_shape_info->shape_format = shape_format;
        out_primitive_shape_info->index_format = IndexFormat::U8;

        return;
    }

    void CalculateVertexBufferPrimitiveShapeQuad(void *vertex_buffer, size_t vertex_buffer_size, PrimitiveShapeInfo *primitive_shape_info);

    template <typename T> 
    constexpr void CalculateIndexBufferPrimitiveShapeQuad(T *index_buffer, size_t index_buffer_size, PrimitiveShapeInfo *primitive_shape_info);

    void CalculatePrimitiveShapeQuad(void *vertex_buffer, size_t vertex_buffer_size, void *index_buffer, size_t index_buffer_size, PrimitiveShapeInfo *primitive_shape_info);

    void CreatePrimitiveShapeQuad(mem::GpuMemoryAddress *out_vertex_buffer_address, mem::GpuMemoryAddress *out_index_buffer_address, mem::Heap *gpu_heap, PrimitiveShapeInfo *primitive_shape_info);
}
