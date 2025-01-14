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
#include <awn.hpp>

namespace awn::gfx {

    namespace {
        static constexpr const vp::util::Float3 cPositionArray[] = {
            { -1.0f,  1.0f, 0.0f, },
            {  1.0f,  1.0f, 0.0f, },
            { -1.0f, -1.0f, 0.0f, },
            {  1.0f, -1.0f, 0.0f, },
        };
        static constexpr const u32 cPositionCount = sizeof(cPositionArray) / sizeof(vp::util::Float3);

        static constexpr const vp::util::Float3 cNormal = { 0.0f, 0.0f, 1.0f, };

        static constexpr const vp::util::Float2 cTextureCoordinateArray[] = {
            { 0.0f, 0.0f, },
            { 1.0f, 0.0f, },
            { 0.0f, 1.0f, },
            { 1.0f, 1.0f, },
        };
        static constexpr const u32 cTextureCoordinateCount = sizeof(cTextureCoordinateArray) / sizeof(vp::util::Float2);
        static_assert(cPositionCount == cTextureCoordinateCount);

        static constexpr const u8 cTrianglesIndexArray[] = {
            0,
            2,
            1,
            1,
            2,
            3,
        };
        static constexpr const u32 cTrianglesIndexCount = sizeof(cTrianglesIndexArray) / sizeof(u8);
        static constexpr const u8 cLineStripIndexArray[] = {
            0,
            1,
            3,
            2,
            0,
        };
        static constexpr const u32 cLineStripIndexCount = sizeof(cLineStripIndexArray) / sizeof(u8);

        template <typename T> 
        constexpr void CalculateIndexBufferPrimitiveShapeQuadImpl(T *index_buffer, size_t index_buffer_size, const PrimitiveShapeInfo *primitive_shape_info) {

            /* Integrity check */
            VP_ASSERT(primitive_shape_info->index_buffer_size <= index_buffer_size);

            /* Initialize index buffer */
            switch (primitive_shape_info->primitive_topology) {
                case PrimitiveTopology::Triangles:
                {
                    VP_ASSERT(primitive_shape_info->index_count <= cTrianglesIndexCount);
                    for (u32 i = 0; i < primitive_shape_info->index_count; ++i) {
                        index_buffer[i] = static_cast<T>(cTrianglesIndexArray[i]);
                    }
                    break;
                }
                case PrimitiveTopology::LineStrip:
                {
                    VP_ASSERT(primitive_shape_info->index_count <= cLineStripIndexCount);
                    for (u32 i = 0; i < primitive_shape_info->index_count; ++i) {
                        index_buffer[i] = static_cast<T>(cLineStripIndexArray[i]);
                    }
                    break;
                }
                default:
                    break;
            };

            return;
        }
    }

    void CalculateVertexBufferPrimitiveShapeQuad(void *vertex_buffer, size_t vertex_buffer_size, const PrimitiveShapeInfo *primitive_shape_info) {

        /* Integrity check */
        VP_ASSERT(primitive_shape_info->vertex_buffer_size <= vertex_buffer_size);
        VP_ASSERT(primitive_shape_info->vertex_count <= cPositionCount);

        if (primitive_shape_info->primitive_topology == PrimitiveTopology::LineStrip) {
            vp::util::Float3 *pos_array = reinterpret_cast<vp::util::Float3*>(vertex_buffer);
            for (u32 i = 0; i < primitive_shape_info->vertex_count; ++i) {
                pos_array[i] = cPositionArray[i];
            }
            return;
        }

        /* Initialize vertex buffer */
        const bool is_position = ((primitive_shape_info->shape_format & ShapeFormat::Position) == ShapeFormat::Position);
        const bool is_normal = ((primitive_shape_info->shape_format & ShapeFormat::Normal) == ShapeFormat::Normal);
        const bool is_tex_coord = ((primitive_shape_info->shape_format & ShapeFormat::TextureCoordinate) == ShapeFormat::TextureCoordinate);
        uintptr_t vertex_buffer_iter = reinterpret_cast<uintptr_t>(vertex_buffer);
        for (u32 i = 0; i < primitive_shape_info->vertex_count; ++i) {
            if (is_position) {
                *reinterpret_cast<vp::util::Float3*>(vertex_buffer_iter) = cPositionArray[i];
                vertex_buffer_iter += sizeof(vp::util::Float3);
            }
            if (is_normal) {
                *reinterpret_cast<vp::util::Float3*>(vertex_buffer_iter) = cNormal;
                vertex_buffer_iter += sizeof(vp::util::Float3);
            }
            if (is_tex_coord) {
                *reinterpret_cast<vp::util::Float2*>(vertex_buffer_iter) = cTextureCoordinateArray[i];
                vertex_buffer_iter += sizeof(vp::util::Float2);
            }
        }

        return;
    }

    void CalculateIndexBufferPrimitiveShapeQuad(void *index_buffer, size_t index_buffer_size, const PrimitiveShapeInfo *primitive_shape_info) {

        if (primitive_shape_info->index_format == IndexFormat::U8) {
            CalculateIndexBufferPrimitiveShapeQuadImpl(reinterpret_cast<u8*>(index_buffer), index_buffer_size, primitive_shape_info);
        } else if (primitive_shape_info->index_format == IndexFormat::U16) {
            CalculateIndexBufferPrimitiveShapeQuadImpl(reinterpret_cast<u16*>(index_buffer), index_buffer_size, primitive_shape_info);
        } else if (primitive_shape_info->index_format == IndexFormat::U32) {
            CalculateIndexBufferPrimitiveShapeQuadImpl(reinterpret_cast<u32*>(index_buffer), index_buffer_size, primitive_shape_info);
        } else {
            VP_UNEXPECTED_DEFAULT();
        }

        return;
    }

    void CalculatePrimitiveShapeQuad(void *vertex_buffer, size_t vertex_buffer_size, void *index_buffer, size_t index_buffer_size, const PrimitiveShapeInfo *primitive_shape_info) {

        /* Calculate triangle vertex buffer */
        CalculateVertexBufferPrimitiveShapeQuad(vertex_buffer, vertex_buffer_size, primitive_shape_info);

        /* Calculate index buffer coresponding to index format */
        CalculateIndexBufferPrimitiveShapeQuad(index_buffer, index_buffer_size, primitive_shape_info);

        return;
    }
}
