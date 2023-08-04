#pragma once

namespace awn::gfx {

    enum class ShapeFormat : u32 {
        Position          = (1 << 0),
        Normal            = (1 << 1),
        TextureCoordinate = (1 << 2),
    };
    VP_ENUM_FLAG_TRAITS(ShapeFormat);

    constexpr ALWAYS_INLINE u32 GetPrimitiveShapeStride(ShapeFormat shape_format) {

        u32 stride = 0;
        if ((shape_format & ShapeFormat::Position) == ShapeFormat::Position) {
            stride += sizeof(vp::util::Vector3f);
        }
        if ((shape_format & ShapeFormat::Normal) == ShapeFormat::Normal) {
            stride += sizeof(vp::util::Vector3f);
        }
        if ((shape_format & ShapeFormat::TextureCoordinate) == ShapeFormat::TextureCoordinate) {
            stride += sizeof(vp::util::Vector2f);
        }

        return stride;
    }

    struct PrimitiveShapeInfo {
        ShapeFormat  shape_format;
        IndexFormat  index_format;
        u32          vertex_count;
        u32          index_count;
        size_t       vertex_buffer_size;
        size_t       index_buffer_size;
        u32          segment_count;
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
