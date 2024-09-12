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

    constexpr void CalculatePrimitiveShapeInfoCircle(PrimitiveShapeInfo *out_primitive_shape_info, ShapeFormat shape_format, u32 segment_count) {

        /* Get stride */
        const u32 stride = GetPrimitiveShapeStride(shape_format);

        /* Calculate shape info */
        out_primitive_shape_info->vertex_count       = segment_count + 1;
        out_primitive_shape_info->vertex_buffer_size = stride * (segment_count + 1);
        out_primitive_shape_info->index_count        = segment_count * 3;

        /* Set shape format */
        out_primitive_shape_info->shape_format = shape_format;

        /* Set index format */
        u32 index_size = sizeof(u8);
        if (out_primitive_shape_info->index_count <= 0xff) {
            out_primitive_shape_info->index_format = IndexFormat::U8;
        } else if (out_primitive_shape_info->index_count <= 0xffff) {
            out_primitive_shape_info->index_format = IndexFormat::U16;
            index_size = sizeof(u16);
        } else {
            out_primitive_shape_info->index_format = IndexFormat::U32;
            index_size = sizeof(u32);
        }

        /* Calculate index buffer size */
        out_primitive_shape_info->index_buffer_size  = out_primitive_shape_info->index_count * index_size;

        return;
    }

    void CalculateVertexBufferPrimitiveShapeCircle(void *vertex_buffer, size_t vertex_buffer_size, PrimitiveShapeInfo *primitive_shape_info);

    template <typename T>
    void CalculateIndexBufferPrimitiveShapeCircle(T *index_buffer, size_t index_buffer_size, PrimitiveShapeInfo *primitive_shape_info);

    void CalculatePrimitiveShapeCircle(void *vertex_buffer, size_t vertex_buffer_size, void *index_buffer, size_t index_buffer_size, PrimitiveShapeInfo *primitive_shape_info);

    void CreatePrimitiveShapeCircle(void **out_vertex_buffer_address, void **out_index_buffer_address, mem::Heap *gpu_heap, PrimitiveShapeInfo *primitive_shape_info);
}
