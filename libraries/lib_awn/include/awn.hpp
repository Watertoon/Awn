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

#include <vp.hpp>

namespace awn {
    using Result                   = vp::Result;
    using TimeSpan                 = vp::TimeSpan;
    using TickSpan                 = vp::TickSpan;
    constexpr Result ResultSuccess = vp::ResultSuccess;

    template <typename T>
    using VectorConstant = vp::util::VectorConstant<T>;

    using Vector4h = vp::util::Vector<_Float16,4>;
    using Vector8h = vp::util::Vector<_Float16,8>;
    using Vector16h = vp::util::Vector<_Float16,16>;

    using Vector2f = vp::util::Vector<float,2>;
    using Vector3f = vp::util::Vector<float,3>;
    using Vector4f = vp::util::Vector<float,4>;
    using Vector8f = vp::util::Vector<float,8>;
    using Vector16f = vp::util::Vector<float,16>;
    using Float2 = vp::util::Plain<float,2>;
    using Float3 = vp::util::Plain<float,3>;
    using Float4 = vp::util::Plain<float,4>;
    using Float8 = vp::util::Plain<float,8>;
    using Float16 = vp::util::Plain<float,16>;

    using Vector2d = vp::util::Vector<double,2>;
    using Vector3d = vp::util::Vector<double,3>;
    using Vector4d = vp::util::Vector<double,4>;
    using Vector8d = vp::util::Vector<double,8>;
    using Double2 = vp::util::Plain<double,2>;
    using Double3 = vp::util::Plain<double,3>;
    using Double4 = vp::util::Plain<double,4>;
    using Double8 = vp::util::Plain<double,8>;

    using Vector16cc = vp::util::Vector<char,16>;
    using Vector32cc = vp::util::Vector<char,32>;
    using Vector64cc = vp::util::Vector<char,64>;

    using Vector16sc = vp::util::Vector<s8,16>;
    using Vector32sc = vp::util::Vector<s8,32>;
    using Vector64sc = vp::util::Vector<s8,64>;

    using Vector16uc = vp::util::Vector<u8,16>;
    using Vector32uc = vp::util::Vector<u8,32>;
    using Vector64uc = vp::util::Vector<u8,64>;

    using Vector2ss = vp::util::Vector<s16,2>;
    using Vector3ss = vp::util::Vector<s16,3>;
    using Vector4ss = vp::util::Vector<s16,4>;
    using Vector8ss = vp::util::Vector<s16,8>;
    using Vector16ss = vp::util::Vector<s16,16>;
    using Vector32ss = vp::util::Vector<s16,32>;
    using Short2 = vp::util::Plain<s16,2>;
    using Short3 = vp::util::Plain<s16,3>;
    using Short4 = vp::util::Plain<s16,4>;
    using Short8 = vp::util::Plain<s16,8>;
    using Short16 = vp::util::Plain<s16,16>;
    using Short32 = vp::util::Plain<s16,32>;

    using Vector2us = vp::util::Vector<u16,2>;
    using Vector3us = vp::util::Vector<u16,3>;
    using Vector4us = vp::util::Vector<u16,4>;
    using Vector8us = vp::util::Vector<u16,8>;
    using Vector16us = vp::util::Vector<u16,16>;
    using Vector32us = vp::util::Vector<u16,32>;
    using UShort2 = vp::util::Plain<u16,2>;
    using UShort3 = vp::util::Plain<u16,3>;
    using UShort4 = vp::util::Plain<u16,4>;
    using UShort8 = vp::util::Plain<u16,8>;
    using UShort16 = vp::util::Plain<u16,16>;
    using UShort32 = vp::util::Plain<u16,32>;

    using Vector2si = vp::util::Vector<s32,2>;
    using Vector3si = vp::util::Vector<s32,3>;
    using Vector4si = vp::util::Vector<s32,4>;
    using Vector8si = vp::util::Vector<s32,8>;
    using Vector16si = vp::util::Vector<s32,16>;
    using Int2 = vp::util::Plain<s32,2>;
    using Int3 = vp::util::Plain<s32,3>;
    using Int4 = vp::util::Plain<s32,4>;
    using Int8 = vp::util::Plain<s32,8>;
    using Int16 = vp::util::Plain<s32,16>;

    using Vector2ui = vp::util::Vector<u32,2>;
    using Vector3ui = vp::util::Vector<u32,3>;
    using Vector4ui = vp::util::Vector<u32,4>;
    using Vector8ui = vp::util::Vector<u32,8>;
    using Vector16ui = vp::util::Vector<u32,16>;
    using UInt2 = vp::util::Plain<u32,2>;
    using UInt3 = vp::util::Plain<u32,3>;
    using UInt4 = vp::util::Plain<u32,4>;
    using UInt8 = vp::util::Plain<u32,8>;
    using UInt16 = vp::util::Plain<u32,16>;

    using Vector2sll = vp::util::Vector<s64,2>;
    using Vector3sll = vp::util::Vector<s64,3>;
    using Vector4sll = vp::util::Vector<s64,4>;
    using Vector8sll = vp::util::Vector<s64,8>;
    using Long2 = vp::util::Plain<s64,2>;
    using Long3 = vp::util::Plain<s64,3>;
    using Long4 = vp::util::Plain<s64,4>;
    using Long8 = vp::util::Plain<s64,8>;

    using Vector2ull = vp::util::Vector<u64,2>;
    using Vector3ull = vp::util::Vector<u64,3>;
    using Vector4ull = vp::util::Vector<u64,4>;
    using Vector8ull = vp::util::Vector<u64,8>;
    using ULong2 = vp::util::Plain<u64,2>;
    using ULong3 = vp::util::Plain<u64,3>;
    using ULong4 = vp::util::Plain<u64,4>;
    using ULong8 = vp::util::Plain<u64,8>;

    template <typename T>
    using MatrixConstant = vp::util::MatrixConstant<T>;

    using FloatRowMajor3x3 = vp::util::MatrixPlain<float,3,3,true>;
    using FloatRowMajor4x3 = vp::util::MatrixPlain<float,4,3,true>;
    using FloatRowMajor4x4 = vp::util::MatrixPlain<float,4,4,true>;
    using MatrixRowMajor4x3f = vp::util::MatrixVector<float,4,3,true>;
    using MatrixRowMajor4x4f = vp::util::MatrixVector<float,4,4,true>;

    using FloatColumnMajor3x3 = vp::util::MatrixPlain<float,3,3,false>;
    using FloatColumnMajor4x3 = vp::util::MatrixPlain<float,4,3,false>;
    using FloatColumnMajor4x4 = vp::util::MatrixPlain<float,4,4,false>;
    using MatrixColumnMajor4x3f = vp::util::MatrixVector<float,4,3,false>;
    using MatrixColumnMajor4x4f = vp::util::MatrixVector<float,4,4,false>;

    using DoubleRowMajor3x3 = vp::util::MatrixPlain<double,3,3,true>;
    using DoubleRowMajor4x3 = vp::util::MatrixPlain<double,4,3,true>;
    using DoubleRowMajor4x4 = vp::util::MatrixPlain<double,4,4,true>;
    using MatrixRowMajor4x3d = vp::util::MatrixVector<double,4,3,true>;
    using MatrixRowMajor4x4d = vp::util::MatrixVector<double,4,4,true>;

    using DoubleColumnMajor3x3 = vp::util::MatrixPlain<double,3,3,false>;
    using DoubleColumnMajor4x3 = vp::util::MatrixPlain<double,4,3,false>;
    using DoubleColumnMajor4x4 = vp::util::MatrixPlain<double,4,4,false>;
    using MatrixColumnMajor4x3d = vp::util::MatrixVector<double,4,3,false>;
    using MatrixColumnMajor4x4d = vp::util::MatrixVector<double,4,4,false>;
}

#include <awn/result.h>

#include <awn/mem.h>
#include <awn/sys.h>
#include <awn/async.h>
#include <awn/res.h>
#include <awn/para.h>
#include <awn/gfx.h>
#include <awn/hid.h>
#include <awn/grr.h>
#include <awn/frm.h>
