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

    enum class ShapeFormat : u32 {
        Position          = (1 << 0),
        Normal            = (1 << 1),
        TextureCoordinate = (1 << 2),
    };
    VP_ENUM_FLAG_TRAITS(ShapeFormat);

    constexpr ALWAYS_INLINE u32 GetPrimitiveShapeVertexStride(ShapeFormat shape_format) {

        u32 stride = 0;
        if ((shape_format & ShapeFormat::Position) == ShapeFormat::Position) {
            stride += sizeof(vp::util::Float3);
        }
        if ((shape_format & ShapeFormat::Normal) == ShapeFormat::Normal) {
            stride += sizeof(vp::util::Float3);
        }
        if ((shape_format & ShapeFormat::TextureCoordinate) == ShapeFormat::TextureCoordinate) {
            stride += sizeof(vp::util::Float2);
        }

        return stride;
    }

    constexpr ALWAYS_INLINE u32 GetPrimitiveShapeIndexStride(IndexFormat index_format) {

        if (index_format == IndexFormat::U8) {
            return sizeof(u8);
        }
        if (index_format == IndexFormat::U16) {
            return sizeof(u16);
        }
        if (index_format == IndexFormat::U32) {
            return sizeof(u32);
        }
        VP_UNEXPECTED_DEFAULT();
        return sizeof(u32);
    }

    struct PrimitiveShapeInfo {
        ShapeFormat       shape_format = ShapeFormat::Position;
        IndexFormat       index_format = IndexFormat::U16;
        PrimitiveTopology primitive_topology = PrimitiveTopology::Triangles;
        u32               vertex_count;
        u32               index_count;
        u32               vertex_buffer_size;
        u32               index_buffer_size;
        u32               segment_count;
    };

    void CalculatePrimitiveShapeInfoRoundRectangle(PrimitiveShapeInfo *out_primitive_shape_info);
    void CalculatePrimitiveShapeInfoCone(PrimitiveShapeInfo *out_primitive_shape_info);
    void CalculatePrimitiveShapeInfoCube(PrimitiveShapeInfo *out_primitive_shape_info);
    void CalculatePrimitiveShapeInfoSphere(PrimitiveShapeInfo *out_primitive_shape_info);
    void CalculatePrimitiveShapeInfoHemisphere(PrimitiveShapeInfo *out_primitive_shape_info);
    void CalculatePrimitiveShapeInfoCylinder(PrimitiveShapeInfo *out_primitive_shape_info);
    void CalculatePrimitiveShapeInfoPipe(PrimitiveShapeInfo *out_primitive_shape_info);

    void CalculatePrimitiveShapeRoundRectangle(void *vertex_buffer, size_t vertex_buffer_size, void *index_buffer, size_t index_buffer_size, PrimitiveShapeInfo *primitive_shape_info);
    void CalculatePrimitiveShapeCone(void *vertex_buffer, size_t vertex_buffer_size, void *index_buffer, size_t index_buffer_size, PrimitiveShapeInfo *primitive_shape_info);
    void CalculatePrimitiveShapeCube(void *vertex_buffer, size_t vertex_buffer_size, void *index_buffer, size_t index_buffer_size, PrimitiveShapeInfo *primitive_shape_info);
    void CalculatePrimitiveShapeSphere(void *vertex_buffer, size_t vertex_buffer_size, void *index_buffer, size_t index_buffer_size, PrimitiveShapeInfo *primitive_shape_info);
    void CalculatePrimitiveShapeHemisphere(void *vertex_buffer, size_t vertex_buffer_size, void *index_buffer, size_t index_buffer_size, PrimitiveShapeInfo *primitive_shape_info);
    void CalculatePrimitiveShapeCylinder(void *vertex_buffer, size_t vertex_buffer_size, void *index_buffer, size_t index_buffer_size, PrimitiveShapeInfo *primitive_shape_info);
    void CalculatePrimitiveShapePipe(void *vertex_buffer, size_t vertex_buffer_size, void *index_buffer, size_t index_buffer_size, PrimitiveShapeInfo *primitive_shape_info);

    void CreatePrimitiveShapeRoundRectangle(PrimitiveShapeInfo *primitive_shape_info);
    void CreatePrimitiveShapeCone(PrimitiveShapeInfo *primitive_shape_info);
    void CreatePrimitiveShapeCube(PrimitiveShapeInfo *primitive_shape_info);
    void CreatePrimitiveShapeSphere(PrimitiveShapeInfo *primitive_shape_info);
    void CreatePrimitiveShapeHemisphere(PrimitiveShapeInfo *primitive_shape_info);
    void CreatePrimitiveShapeCylinder(PrimitiveShapeInfo *primitive_shape_info);
    void CreatePrimitiveShapePipe(PrimitiveShapeInfo *primitive_shape_info);
}
