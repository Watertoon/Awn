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

namespace vp::codec {

    constexpr const char *cNisasystKeyGenString = "e413645fa69cafe34a76192843e48cbd691d1f9fba87e8a23d40e02ce13b0d534d10301576f31bc70b763a60cf07149cfca50e2a6b3955b98f26ca84a5844a8aeca7318f8d7dba406af4e45c4806fa4d7b736d51cceaaf0e96f657bb3a8af9b175d51b9bddc1ed475677260f33c41ddbc1ee30b46c4df1b24a25cf7cb6019794";
    constexpr u64         cNisasystMagic        = vp::util::TCharCode64("nisasyst");

	struct NisasystKey {
		u8 key_array[0x10];
	};

	void GenerateNisasystKey(NisasystKey *out_key, vp::res::NintendoWareRandom *random);

    bool IsNisasystEncrypted(void *file, u32 file_size);

	void DecryptNisasyst(void *in_out_file, u32 file_size, const char *file_path);
}
