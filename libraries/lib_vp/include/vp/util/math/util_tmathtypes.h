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

namespace vp::util {

    /* Vector */
    template <typename T>
    struct TVector2Helper {
        static constexpr bool cIsDummy = true;
    };

    template <>
    struct TVector2Helper<float> {
        static constexpr bool cIsDummy = false;
        using Type = Vector2f;
    };
    template <>
    struct TVector2Helper<double> {
        static constexpr bool cIsDummy = false;
        using Type = Vector2d;
    };
    template <>
    struct TVector2Helper<s32> {
        static constexpr bool cIsDummy = false;
        using Type = Vector2i;
    };

	template <typename T>
        requires (!TVector2Helper<T>::cIsDummy)
	using TVector2 = TVector2Helper<T>::Type;

    template <typename T>
    struct TVector3Helper {
        static constexpr bool cIsDummy = true;
    };

    template <>
    struct TVector3Helper<float> {
        static constexpr bool cIsDummy = false;
        using Type = Vector3f;
    };
    template <>
    struct TVector3Helper<double> {
        static constexpr bool cIsDummy = false;
        using Type = Vector3d;
    };
    template <>
    struct TVector3Helper<s32> {
        static constexpr bool cIsDummy = false;
        using Type = Vector3i;
    };

	template <typename T>
        requires (!TVector3Helper<T>::cIsDummy)
	using TVector3 = TVector3Helper<T>::Type;

    template <typename T>
    struct TVector4Helper {
        static constexpr bool cIsDummy = true;
    };

    template <>
    struct TVector4Helper<float> {
        static constexpr bool cIsDummy = false;
        using Type = Vector4f;
    };
    template <>
    struct TVector4Helper<double> {
        static constexpr bool cIsDummy = false;
        using Type = Vector4d;
    };
    template <>
    struct TVector4Helper<s32> {
        static constexpr bool cIsDummy = false;
        using Type = Vector4i;
    };

	template <typename T>
        requires (!TVector4Helper<T>::cIsDummy)
	using TVector4 = TVector4Helper<T>::Type;

	template <typename T, u32 Count>
    struct TVectorHelper {
        static constexpr bool cIsDummy = true;
    };

	template <typename T>
    struct TVectorHelper<T, 2> {
        static constexpr bool cIsDummy = false;
        using Type = TVector2<T>::Type;
    };

	template <typename T>
    struct TVectorHelper<T, 3> {
        static constexpr bool cIsDummy = false;
        using Type = TVector3<T>::Type;
    };

	template <typename T>
    struct TVectorHelper<T, 4> {
        static constexpr bool cIsDummy = false;
        using Type = TVector4<T>::Type;
    };

    template <typename T, u32 Count>
        requires (!TVectorHelper<T, Count>::cIsDummy)
    using TVector = TVectorHelper<T, Count>;

    /* Matrix */
    template <typename T>
    struct TMatrixRowMajor4x3Helper {
        static constexpr bool cIsDummy = true;
    };

    template <>
    struct TMatrixRowMajor4x3Helper<float> {
        static constexpr bool cIsDummy = false;
        using Type = MatrixRowMajor4x3f;
    };
    template <>
    struct TMatrixRowMajor4x3Helper<double> {
        static constexpr bool cIsDummy = false;
        using Type = MatrixRowMajor4x3d;
    };
    template <>
    struct TMatrixRowMajor4x3Helper<s32> {
        static constexpr bool cIsDummy = false;
        using Type = MatrixRowMajor4x3i;
    };

	template <typename T>
	using TMatrixRowMajor4x3 = TMatrixRowMajor4x3Helper<T>;

    template <typename T>
    struct TMatrixRowMajor4x4Helper {
        static constexpr bool cIsDummy = true;
    };

    template <>
    struct TMatrixRowMajor4x4Helper<float> {
        static constexpr bool cIsDummy = false;
        using Type = MatrixRowMajor4x4f;
    };
    template <>
    struct TMatrixRowMajor4x4Helper<double> {
        static constexpr bool cIsDummy = false;
        using Type = MatrixRowMajor4x4d;
    };
    template <>
    struct TMatrixRowMajor4x4Helper<s32> {
        static constexpr bool cIsDummy = false;
        using Type = MatrixRowMajor4x4i;
    };
    
	template <typename T>
	using TMatrixRowMajor4x4 = TMatrixRowMajor4x4Helper<T>;

    template <typename T, u32 ColumnCount, u32 RowCount>
    struct TMatrixRowMajorHelper {
        static constexpr bool cIsDummy = true;
    };

    template <typename T>
    struct TMatrixRowMajorHelper<T, 4,3> {
        static constexpr bool cIsDummy = false;
        using Type = TMatrixRowMajor4x3<T>::Type;
    };
    template <typename T>
    struct TMatrixRowMajorHelper<T, 4,4> {
        static constexpr bool cIsDummy = false;
        using Type = TMatrixRowMajor4x4<T>::Type;
    };
    
	template <typename T, u32 ColumnCount, u32 RowCount>
        requires (!TMatrixRowMajorHelper<T, ColumnCount, RowCount>::cIsDummy)
	using TMatrixRowMajor = TMatrixRowMajorHelper<T, ColumnCount, RowCount>::Type;

    template <typename T>
    struct TMatrixColumnMajor4x3Helper {
        static constexpr bool cIsDummy = true;
    };

    template <>
    struct TMatrixColumnMajor4x3Helper<float> {
        static constexpr bool cIsDummy = false;
        using Type = MatrixColumnMajor4x3f;
    };
    template <>
    struct TMatrixColumnMajor4x3Helper<double> {
        static constexpr bool cIsDummy = false;
        using Type = MatrixColumnMajor4x3d;
    };
    template <>
    struct TMatrixColumnMajor4x3Helper<s32> {
        static constexpr bool cIsDummy = false;
        using Type = MatrixColumnMajor4x3i;
    };

	template <typename T>
	using TMatrixColumnMajor4x3 = TMatrixColumnMajor4x3Helper<T>;

    template <typename T>
    struct TMatrixColumnMajor4x4Helper {
        static constexpr bool cIsDummy = true;
    };

    template <>
    struct TMatrixColumnMajor4x4Helper<float> {
        static constexpr bool cIsDummy = false;
        using Type = MatrixColumnMajor4x4f;
    };
    template <>
    struct TMatrixColumnMajor4x4Helper<double> {
        static constexpr bool cIsDummy = false;
        using Type = MatrixColumnMajor4x4d;
    };
    template <>
    struct TMatrixColumnMajor4x4Helper<s32> {
        static constexpr bool cIsDummy = false;
        using Type = MatrixColumnMajor4x4i;
    };

	template <typename T>
	using TMatrixColumnMajor4x4 = TMatrixColumnMajor4x4Helper<T>::Type;

    template <typename T, u32 ColumnCount, u32 RowCount>
    struct TMatrixColumnMajorHelper {
        static constexpr bool cIsDummy = true;
    };

    template <typename T>
    struct TMatrixColumnMajorHelper<T, 4,3> {
        static constexpr bool cIsDummy = false;
        using Type = TMatrixColumnMajor4x3<T>::Type;
    };
    template <typename T>
    struct TMatrixColumnMajorHelper<T, 4,4> {
        static constexpr bool cIsDummy = false;
        using Type = TMatrixColumnMajor4x4<T>::Type;
    };
    
	template <typename T, u32 ColumnCount, u32 RowCount>
        requires (!TMatrixColumnMajorHelper<T, ColumnCount, RowCount>::cIsDummy)
	using TMatrixColumnMajor = TMatrixColumnMajorHelper<T, ColumnCount, RowCount>::Type;

    template <typename T, u32 ColumnCount, u32 RowCount, bool IsRowMajor>
    struct TMatrixHelper {
        static constexpr bool cIsDummy = true;
    };

    template <typename T, u32 ColumnCount, u32 RowCount>
    struct TMatrixHelper<T, ColumnCount, RowCount, true> {
        static constexpr bool cIsDummy = false;
        using Type = TMatrixRowMajor<T, ColumnCount, RowCount>::Type;
    };

    template <typename T, u32 ColumnCount, u32 RowCount>
    struct TMatrixHelper<T, ColumnCount, RowCount, false> {
        static constexpr bool cIsDummy = false;
        using Type = TMatrixColumnMajor<T, ColumnCount, RowCount>::Type;
    };

    template <typename T, u32 ColumnCount, u32 RowCount, bool IsRowMajor>
        requires (!TMatrixHelper<T, ColumnCount, RowCount, IsRowMajor>::cIsDummy)
	using TMatrix = TMatrixHelper<T, ColumnCount, RowCount, IsRowMajor>::Type;

    using Real  = double;
    using Real2 = Double2;
    using Real3 = Double3;
    using Real4 = Double4;
    
    using RealRowMajor33 = DoubleRowMajor3x3;
    using RealRowMajor43 = DoubleRowMajor4x3;
    using RealRowMajor44 = DoubleRowMajor4x4;

    using RealColumnMajor33 = DoubleColumnMajor3x3;
    using RealColumnMajor43 = DoubleColumnMajor4x3;
    using RealColumnMajor44 = DoubleColumnMajor4x4;

    using Vector2r = TVector2<Real>;
    using Vector3r = TVector3<Real>;
    using Vector4r = TVector4<Real>;

    using MatrixRowMajor4x3r = TMatrixRowMajor4x3<Real>;
    using MatrixRowMajor4x4r = TMatrixRowMajor4x4<Real>;
    using MatrixColumnMajor4x3r = TMatrixColumnMajor4x3<Real>;
    using MatrixColumnMajor4x4r = TMatrixColumnMajor4x4<Real>;
}
