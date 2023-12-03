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

    void CalculateVertexBufferPrimitiveShapeCircle(void *vertex_buffer, size_t vertex_buffer_size, PrimitiveShapeInfo *primitive_shape_info) {

        /* Integrity check */
        VP_ASSERT(primitive_shape_info->vertex_buffer_size <= vertex_buffer_size);

        /* Initialize vertex buffer */
        float *vertex_buffer_iter = reinterpret_cast<float*>(vertex_buffer);

        constexpr float half_round      = vp::util::AngleHalfRound(1.0f);
        const float     segment_count_f = static_cast<float>(primitive_shape_info->segment_count);
        for (u32 i = 0; i < primitive_shape_info->segment_count; ++i) {

            /* Calculate location of vertex on circle */
            const float              circle_location = half_round * (vp::util::ToDegrees(i * 360.0f) / segment_count_f);
            const vp::util::Vector2f sin_cos = vp::util::SampleSinCos(circle_location);

            if ((primitive_shape_info->shape_format & ShapeFormat::Position) == ShapeFormat::Position) {
                vertex_buffer_iter[0] = sin_cos[1];
                vertex_buffer_iter[1] = sin_cos[0];
                vertex_buffer_iter[2] = 0.0f;
                vertex_buffer_iter = reinterpret_cast<float*>(reinterpret_cast<uintptr_t>(vertex_buffer_iter) + sizeof(vp::util::Vector3f));
            }
            if ((primitive_shape_info->shape_format & ShapeFormat::Normal) == ShapeFormat::Normal) {
                vertex_buffer_iter[0] = 0.0f;
                vertex_buffer_iter[1] = 0.0f;
                vertex_buffer_iter[2] = 1.0f;
                vertex_buffer_iter = reinterpret_cast<float*>(reinterpret_cast<uintptr_t>(vertex_buffer_iter) + sizeof(vp::util::Vector3f));
            }
            if ((primitive_shape_info->shape_format & ShapeFormat::TextureCoordinate) == ShapeFormat::TextureCoordinate) {
                vertex_buffer_iter[0] = sin_cos[1] * 0.5f + 0.5f;
                vertex_buffer_iter[1] = 1.0f - (sin_cos[0] * 0.5f + 0.5f);
                vertex_buffer_iter = reinterpret_cast<float*>(reinterpret_cast<uintptr_t>(vertex_buffer_iter) + sizeof(vp::util::Vector2f));
            }
        }

        if ((primitive_shape_info->shape_format & ShapeFormat::Position) == ShapeFormat::Position) {
            vertex_buffer_iter[0] = 0.0f;
            vertex_buffer_iter[1] = 0.0f;
            vertex_buffer_iter[2] = 0.0f;
            vertex_buffer_iter = reinterpret_cast<float*>(reinterpret_cast<uintptr_t>(vertex_buffer_iter) + sizeof(vp::util::Vector3f));
        }
        if ((primitive_shape_info->shape_format & ShapeFormat::Normal) == ShapeFormat::Normal) {
            vertex_buffer_iter[0] = 0.0f;
            vertex_buffer_iter[1] = 0.0f;
            vertex_buffer_iter[2] = 1.0f;
            vertex_buffer_iter = reinterpret_cast<float*>(reinterpret_cast<uintptr_t>(vertex_buffer_iter) + sizeof(vp::util::Vector3f));
        }
        if ((primitive_shape_info->shape_format & ShapeFormat::TextureCoordinate) == ShapeFormat::TextureCoordinate) {
            vertex_buffer_iter[0] = 1.0f;
            vertex_buffer_iter[1] = 1.0f;
        }

        return;
    }

    template <typename T>
    void CalculateIndexBufferPrimitiveShapeCircle(T *index_buffer, size_t index_buffer_size, PrimitiveShapeInfo *primitive_shape_info) {

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

    void CalculatePrimitiveShapeCircle(void *vertex_buffer, size_t vertex_buffer_size, void *index_buffer, size_t index_buffer_size, PrimitiveShapeInfo *primitive_shape_info) {

        /* Calculate Triangle vertex buffer */
        CalculateVertexBufferPrimitiveShapeCircle(vertex_buffer, vertex_buffer_size, primitive_shape_info);

        /* Calculate index buffer coresponding to index format */
        if (primitive_shape_info->index_format == IndexFormat::U8) {
            CalculateIndexBufferPrimitiveShapeCircle(reinterpret_cast<u8*>(index_buffer), index_buffer_size, primitive_shape_info);
        } else if (primitive_shape_info->index_format == IndexFormat::U16) {
            CalculateIndexBufferPrimitiveShapeCircle(reinterpret_cast<u16*>(index_buffer), index_buffer_size, primitive_shape_info);
        } else if (primitive_shape_info->index_format == IndexFormat::U32) {
            CalculateIndexBufferPrimitiveShapeCircle(reinterpret_cast<u32*>(index_buffer), index_buffer_size, primitive_shape_info);
        } else {
            VP_ASSERT(false);
        }

        return;
    }

    void CreatePrimitiveShapeCircle(void **out_vertex_buffer_address, void **out_index_buffer_address, mem::Heap *gpu_heap, PrimitiveShapeInfo *primitive_shape_info) {

        /* Allocate gpu memory */
        void *vbo_address = ::operator new(primitive_shape_info->vertex_buffer_size, gpu_heap, Context::cTargetVertexBufferAlignment);
        void *ibo_address = ::operator new(primitive_shape_info->index_buffer_size, gpu_heap, Context::cTargetIndexBufferAlignment);

        /* Calculate shape */
        CalculatePrimitiveShapeCircle(vbo_address, primitive_shape_info->vertex_buffer_size, ibo_address, primitive_shape_info->index_buffer_size, primitive_shape_info);

        /* Set memory addresses */
        *out_vertex_buffer_address = vbo_address;
        *out_index_buffer_address  = ibo_address;

        return;
    }
}
