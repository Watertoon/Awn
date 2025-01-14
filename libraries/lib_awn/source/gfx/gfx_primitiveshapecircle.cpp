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

        template <typename T>
        constexpr void CalculateIndexBufferPrimitiveShapeCircleImpl(T *index_buffer, size_t index_buffer_size, const PrimitiveShapeInfo *primitive_shape_info) {

            /* Integrity check */
            VP_ASSERT(primitive_shape_info->index_buffer_size <= index_buffer_size);

            /* Setup triangle indices */
            T *index_buffer_iter = index_buffer;
            for (u32 i = 0; i < primitive_shape_info->segment_count; ++i) {
                index_buffer_iter[0] = i;
                ++i;
                index_buffer_iter[1] = (i < primitive_shape_info->segment_count) ? i : 0;
                index_buffer_iter[2] = primitive_shape_info->segment_count;
                index_buffer_iter += 3;
            }

            return;
        }
    }

    void CalculateVertexBufferPrimitiveShapeCircle(void *vertex_buffer, size_t vertex_buffer_size, const PrimitiveShapeInfo *primitive_shape_info) {

        /* Integrity check */
        VP_ASSERT(primitive_shape_info->vertex_buffer_size <= vertex_buffer_size);

        /* Initialize vertex buffer */
        float *vertex_buffer_iter = reinterpret_cast<float*>(vertex_buffer);

        constexpr float half_round      = vp::util::AngleHalfRound(1.0f);
        const float     segment_count_f = static_cast<float>(primitive_shape_info->segment_count);
        for (u32 i = 0; i < primitive_shape_info->segment_count; ++i) {

            /* Calculate location of vertex on circle */
            const float circle_location = half_round * (vp::util::ToRadians(i * 360.0f) / segment_count_f);
            const vp::util::Float2 sin_cos = vp::util::SampleSinCos(circle_location);

            if ((primitive_shape_info->shape_format & ShapeFormat::Position) == ShapeFormat::Position) {
                vertex_buffer_iter[0] = sin_cos.y;
                vertex_buffer_iter[1] = sin_cos.x;
                vertex_buffer_iter[2] = 0.0f;
                vertex_buffer_iter = reinterpret_cast<float*>(reinterpret_cast<uintptr_t>(vertex_buffer_iter) + sizeof(vp::util::Float3));
            }
            if ((primitive_shape_info->shape_format & ShapeFormat::Normal) == ShapeFormat::Normal) {
                vertex_buffer_iter[0] = 0.0f;
                vertex_buffer_iter[1] = 0.0f;
                vertex_buffer_iter[2] = 1.0f;
                vertex_buffer_iter = reinterpret_cast<float*>(reinterpret_cast<uintptr_t>(vertex_buffer_iter) + sizeof(vp::util::Float3));
            }
            if ((primitive_shape_info->shape_format & ShapeFormat::TextureCoordinate) == ShapeFormat::TextureCoordinate) {
                vertex_buffer_iter[0] = sin_cos.y * 0.5f + 0.5f;
                vertex_buffer_iter[1] = 1.0f - (sin_cos.x * 0.5f + 0.5f);
                vertex_buffer_iter = reinterpret_cast<float*>(reinterpret_cast<uintptr_t>(vertex_buffer_iter) + sizeof(vp::util::Float2));
            }
        }

        if ((primitive_shape_info->shape_format & ShapeFormat::Position) == ShapeFormat::Position) {
            vertex_buffer_iter[0] = 0.0f;
            vertex_buffer_iter[1] = 0.0f;
            vertex_buffer_iter[2] = 0.0f;
            vertex_buffer_iter = reinterpret_cast<float*>(reinterpret_cast<uintptr_t>(vertex_buffer_iter) + sizeof(vp::util::Float3));
        }
        if ((primitive_shape_info->shape_format & ShapeFormat::Normal) == ShapeFormat::Normal) {
            vertex_buffer_iter[0] = 0.0f;
            vertex_buffer_iter[1] = 0.0f;
            vertex_buffer_iter[2] = 1.0f;
            vertex_buffer_iter = reinterpret_cast<float*>(reinterpret_cast<uintptr_t>(vertex_buffer_iter) + sizeof(vp::util::Float3));
        }
        if ((primitive_shape_info->shape_format & ShapeFormat::TextureCoordinate) == ShapeFormat::TextureCoordinate) {
            vertex_buffer_iter[0] = 1.0f;
            vertex_buffer_iter[1] = 1.0f;
        }

        return;
    }

    void CalculateIndexBufferPrimitiveShapeCircle(void *index_buffer, size_t index_buffer_size, const PrimitiveShapeInfo *primitive_shape_info) {

        if (primitive_shape_info->index_format == IndexFormat::U8) {
            CalculateIndexBufferPrimitiveShapeCircleImpl(reinterpret_cast<u8*>(index_buffer), index_buffer_size, primitive_shape_info);
        } else if (primitive_shape_info->index_format == IndexFormat::U16) {
            CalculateIndexBufferPrimitiveShapeCircleImpl(reinterpret_cast<u16*>(index_buffer), index_buffer_size, primitive_shape_info);
        } else if (primitive_shape_info->index_format == IndexFormat::U32) {
            CalculateIndexBufferPrimitiveShapeCircleImpl(reinterpret_cast<u32*>(index_buffer), index_buffer_size, primitive_shape_info);
        } else {
            VP_UNEXPECTED_DEFAULT();
        }

        return;
    }

    void CalculatePrimitiveShapeCircle(void *vertex_buffer, size_t vertex_buffer_size, void *index_buffer, size_t index_buffer_size, const PrimitiveShapeInfo *primitive_shape_info) {

        /* Calculate vertex buffer */
        CalculateVertexBufferPrimitiveShapeCircle(vertex_buffer, vertex_buffer_size, primitive_shape_info);

        /* Calculate index buffer */
        CalculateIndexBufferPrimitiveShapeCircle(index_buffer, index_buffer_size, primitive_shape_info);

        return;
    }
}
