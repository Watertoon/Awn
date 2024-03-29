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

    void CalculateVertexBufferPrimitiveShapeQuad(void *vertex_buffer, size_t vertex_buffer_size, PrimitiveShapeInfo *primitive_shape_info) {

        /* Integrity check */
        VP_ASSERT(primitive_shape_info->vertex_buffer_size <= vertex_buffer_size);

        /* Initialize vertex buffer */
        float *vertex_buffer_iter = reinterpret_cast<float*>(vertex_buffer);
        if ((primitive_shape_info->shape_format & ShapeFormat::Position) == ShapeFormat::Position) {
            vertex_buffer_iter[0] = -1.0f;
            vertex_buffer_iter[1] = 1.0f;
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
            vertex_buffer_iter[0] = 0.0f;
            vertex_buffer_iter[1] = 0.0f;
            vertex_buffer_iter = reinterpret_cast<float*>(reinterpret_cast<uintptr_t>(vertex_buffer_iter) + sizeof(vp::util::Vector2f));
        }

        if ((primitive_shape_info->shape_format & ShapeFormat::Position) == ShapeFormat::Position) {
            vertex_buffer_iter[0] = 1.0f;
            vertex_buffer_iter[1] = 1.0f;
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
            vertex_buffer_iter[1] = 0.0f;
            vertex_buffer_iter = reinterpret_cast<float*>(reinterpret_cast<uintptr_t>(vertex_buffer_iter) + sizeof(vp::util::Vector2f));
        }

        if ((primitive_shape_info->shape_format & ShapeFormat::Position) == ShapeFormat::Position) {
            vertex_buffer_iter[0] = -1.0f;
            vertex_buffer_iter[1] = -1.0f;
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
            vertex_buffer_iter[0] = 0.0f;
            vertex_buffer_iter[1] = 1.0f;
            vertex_buffer_iter = reinterpret_cast<float*>(reinterpret_cast<uintptr_t>(vertex_buffer_iter) + sizeof(vp::util::Vector2f));
        }

        if ((primitive_shape_info->shape_format & ShapeFormat::Position) == ShapeFormat::Position) {
            vertex_buffer_iter[0] = 1.0f;
            vertex_buffer_iter[1] = -1.0f;
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
            vertex_buffer_iter = reinterpret_cast<float*>(reinterpret_cast<uintptr_t>(vertex_buffer_iter) + sizeof(vp::util::Vector2f));
        }

        return;
    }

    template <typename T> 
    constexpr void CalculateIndexBufferPrimitiveShapeQuad(T *index_buffer, size_t index_buffer_size, PrimitiveShapeInfo *primitive_shape_info) {

        /* Integrity check */
        VP_ASSERT(primitive_shape_info->index_buffer_size  <= index_buffer_size);

        /* Initialize index buffer */
        index_buffer[0] = 0;
        index_buffer[1] = 2;
        index_buffer[2] = 1;
        index_buffer[3] = 1;
        index_buffer[4] = 2;
        index_buffer[5] = 3;
    }

    void CalculatePrimitiveShapeQuad(void *vertex_buffer, size_t vertex_buffer_size, void *index_buffer, size_t index_buffer_size, PrimitiveShapeInfo *primitive_shape_info) {

        /* Calculate Triangle vertex buffer */
        CalculateVertexBufferPrimitiveShapeQuad(vertex_buffer, vertex_buffer_size, primitive_shape_info);

        /* Calculate index buffer coresponding to index format */
        if (primitive_shape_info->index_format == IndexFormat::U8) {
            CalculateIndexBufferPrimitiveShapeQuad(reinterpret_cast<u8*>(index_buffer), index_buffer_size, primitive_shape_info);
        } else if (primitive_shape_info->index_format == IndexFormat::U16) {
            CalculateIndexBufferPrimitiveShapeQuad(reinterpret_cast<u16*>(index_buffer), index_buffer_size, primitive_shape_info);
        } else if (primitive_shape_info->index_format == IndexFormat::U32) {
            CalculateIndexBufferPrimitiveShapeQuad(reinterpret_cast<u32*>(index_buffer), index_buffer_size, primitive_shape_info);
        } else {
            VP_ASSERT(false);
        }

        return;
    }

    void CreatePrimitiveShapeQuad(void **out_vertex_buffer_address, void **out_index_buffer_address, mem::Heap *gpu_heap, PrimitiveShapeInfo *primitive_shape_info) {

        /* Allocate gpu memory */
        void *vbo_address = ::operator new(primitive_shape_info->vertex_buffer_size, gpu_heap, Context::cTargetVertexBufferAlignment);
        void *ibo_address = ::operator new(primitive_shape_info->index_buffer_size, gpu_heap, Context::cTargetIndexBufferAlignment);

        /* Calculate shape */
        CalculatePrimitiveShapeQuad(vbo_address, primitive_shape_info->vertex_buffer_size, ibo_address, primitive_shape_info->index_buffer_size, primitive_shape_info);

        /* Set memory addresses */
        *out_vertex_buffer_address = vbo_address;
        *out_index_buffer_address  = ibo_address;

        return;
    }
}
