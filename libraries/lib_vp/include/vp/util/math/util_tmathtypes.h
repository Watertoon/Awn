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

    template <typename T>
    struct TVector2Helper {};

    template <>
    struct TVector2Helper<float> {
        using Type = Vector2f;
    };
    template <>
    struct TVector2Helper<double> {
        using Type = Vector2d;
    };
    template <>
    struct TVector2Helper<s32> {
        using Type = Vector2i;
    };

	template <typename T>
	using Vector2 = TVector2Helper<T>::Type;

    template <typename T>
    struct TVector3Helper {};

    template <>
    struct TVector3Helper<float> {
        using Type = Vector3f;
    };
    template <>
    struct TVector3Helper<double> {
        using Type = Vector3d;
    };
    template <>
    struct TVector3Helper<s32> {
        using Type = Vector3i;
    };

	template <typename T>
	using Vector3 = TVector3Helper<T>::Type;

    template <typename T>
    struct TVector4Helper {};

    template <>
    struct TVector4Helper<float> {
        using Type = Vector4f;
    };
    template <>
    struct TVector4Helper<double> {
        using Type = Vector4d;
    };
    template <>
    struct TVector4Helper<s32> {
        using Type = Vector4i;
    };

	template <typename T>
	using Vector4 = TVector4Helper<T>::Type;

    template <typename T>
    struct TMatrixRowMajor4x3Helper {};

    template <>
    struct TMatrixRowMajor4x3Helper<float> {
        using Type = MatrixRowMajor4x3f;
    };
    template <>
    struct TMatrixRowMajor4x3Helper<double> {
        using Type = MatrixRowMajor4x3d;
    };
    template <>
    struct TMatrixRowMajor4x3Helper<s32> {
        using Type = MatrixRowMajor4x3i;
    };

	template <typename T>
	using MatrixRowMajor4x3 = TMatrixRowMajor4x3Helper<T>;

    template <typename T>
    struct TMatrixRowMajor4x4Helper {};

    template <>
    struct TMatrixRowMajor4x4Helper<float> {
        using Type = MatrixRowMajor4x4f;
    };
    template <>
    struct TMatrixRowMajor4x4Helper<double> {
        using Type = MatrixRowMajor4x4d;
    };
    template <>
    struct TMatrixRowMajor4x4Helper<s32> {
        using Type = MatrixRowMajor4x4i;
    };
    
	template <typename T>
	using MatrixRowMajor4x4 = TMatrixRowMajor4x4Helper<T>;

    template <typename T>
    struct TMatrixColumnMajor4x3Helper {};

    template <>
    struct TMatrixColumnMajor4x3Helper<float> {
        using Type = MatrixColumnMajor4x3f;
    };
    template <>
    struct TMatrixColumnMajor4x3Helper<double> {
        using Type = MatrixColumnMajor4x3d;
    };
    template <>
    struct TMatrixColumnMajor4x3Helper<s32> {
        using Type = MatrixColumnMajor4x3i;
    };

	template <typename T>
	using MatrixColumnMajor4x3 = TMatrixColumnMajor4x3Helper<T>;

    template <typename T>
    struct TMatrixColumnMajor4x4Helper {};

    template <>
    struct TMatrixColumnMajor4x4Helper<float> {
        using Type = MatrixColumnMajor4x4f;
    };
    template <>
    struct TMatrixColumnMajor4x4Helper<double> {
        using Type = MatrixColumnMajor4x4d;
    };
    template <>
    struct TMatrixColumnMajor4x4Helper<s32> {
        using Type = MatrixColumnMajor4x4i;
    };
    
	template <typename T>
	using MatrixColumnMajor4x4 = TMatrixColumnMajor4x4Helper<T>;

    using Real  = double;
    using Real2 = Double2;
    using Real3 = Double3;
    using Real4 = Double4;
    
    using RealRowMajor33 = DoubleRowMajor3x3;
    using RealRowMajor34 = DoubleRowMajor4x3;
    using RealRowMajor44 = DoubleRowMajor4x4;

    using RealColumnMajor33 = DoubleColumnMajor3x3;
    using RealColumnMajor34 = DoubleColumnMajor4x3;
    using RealColumnMajor44 = DoubleColumnMajor4x4;

    using Vector2r = Vector2<Real>;
    using Vector3r = Vector3<Real>;
    using Vector4r = Vector4<Real>;

    using MatrixRowMajor4x3r = MatrixRowMajor4x3<Real>;
    using MatrixRowMajor4x4r = MatrixRowMajor4x4<Real>;
    using MatrixColumnMajor4x3r = MatrixColumnMajor4x3<Real>;
    using MatrixColumnMajor4x4r = MatrixColumnMajor4x4<Real>;
}
