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

namespace vp::res {

	struct ResSzs {
		u32 magic;
		u32 decompressed_size;
		u32 alignment;
		u32 reserve1;
		u8  data_array[];

		static constexpr u32 cMagic = util::TCharCode32("Yaz0");

        constexpr bool IsValid() const {
            return (magic == cMagic) & (decompressed_size != 0);
        }

        constexpr u32 GetDecompressedSize() const {
            return vp::util::SwapEndian(decompressed_size);
        }

        constexpr u32 GetAlignment() const {
            return vp::util::SwapEndian(alignment);
        }
	};
	static_assert(sizeof(ResSzs) == 0x10);
}
