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

    void LoadFile(const char *path, void **out_file, u32 *out_file_size);

    void LoadTextFile(const char *path, char **out_file, u32 *out_file_size);

    void LoadStbImage(const char *path, s32 desired_channels, unsigned char **out_image_data, s32 *out_width, s32 *out_height, s32 *out_channels);

    void FreeStbImage(unsigned char *image_data);
}
