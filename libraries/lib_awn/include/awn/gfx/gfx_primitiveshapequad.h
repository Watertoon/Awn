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

    constexpr void CalculatePrimitiveShapeInfoQuad(PrimitiveShapeInfo *primitive_shape_info) {

        VP_ASSERT(primitive_shape_info->shape_format == ShapeFormat::Position || primitive_shape_info->primitive_topology != PrimitiveTopology::LineStrip);

        /* Calculate shape info */
        const u32 vertex_stride = GetPrimitiveShapeVertexStride(primitive_shape_info->shape_format);
        primitive_shape_info->vertex_count = 4;
        primitive_shape_info->vertex_buffer_size = vertex_stride * 4;

        /* Find index count */
        u32 index_count0 = (primitive_shape_info->primitive_topology == PrimitiveTopology::Triangles) ? 6 : 0;
        u32 index_count1 = (primitive_shape_info->primitive_topology != PrimitiveTopology::LineStrip) ? index_count0 : 5;
        primitive_shape_info->index_count = index_count1;

        /* Calculate index size */
        const u32 index_stride = GetPrimitiveShapeIndexStride(primitive_shape_info->index_format);
        primitive_shape_info->index_buffer_size  = index_stride * index_count1;

        return;
    }

    void CalculateVertexBufferPrimitiveShapeQuad(void *vertex_buffer, size_t vertex_buffer_size, const PrimitiveShapeInfo *primitive_shape_info);
    void CalculateIndexBufferPrimitiveShapeQuad(void *index_buffer, size_t index_buffer_size, const PrimitiveShapeInfo *primitive_shape_info);

    void CalculatePrimitiveShapeQuad(void *vertex_buffer, size_t vertex_buffer_size, void *index_buffer, size_t index_buffer_size, const PrimitiveShapeInfo *primitive_shape_info);
}
