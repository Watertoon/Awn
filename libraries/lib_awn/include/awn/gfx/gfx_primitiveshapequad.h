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

    void CreatePrimitiveShapeQuad(void **out_vertex_buffer_address, void **out_index_buffer_address, mem::Heap *gpu_heap, PrimitiveShapeInfo *primitive_shape_info);
}
