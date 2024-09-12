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

namespace awn::res {

    using MaxExtensionString = vp::util::MaxExtensionString;

    enum class CompressionType : u16 {
        None        = (0 << 0),
        Auto        = (1 << 0),
        Szs         = (1 << 1),
        Zstandard   = (1 << 2),
    };
    VP_ENUM_FLAG_TRAITS(CompressionType);

    constexpr CompressionType GetCompressionType(MaxExtensionString *extension) {
        const MaxExtensionString zs_ext(".zs");
        if (*extension == zs_ext) {
            return CompressionType::Zstandard;
        }
        const MaxExtensionString szs_ext(".szs");
        if (*extension == szs_ext) {
            return CompressionType::Szs;
        }
        return CompressionType::None;
    }

    using DecompressorHandle = u32;

	DecompressorHandle AllocateDecompressorHandle();
	void FreeDecompressorHandle(DecompressorHandle handle);

	IDecompressor *GetDecompressor(DecompressorHandle handle, CompressionType decompressor_type, s32 priority, sys::CoreMask core_mask);
}
