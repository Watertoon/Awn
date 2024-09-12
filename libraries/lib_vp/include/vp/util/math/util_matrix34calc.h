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

    /* WARNING: Avoid continous rotations with these unless you know what you're doing */
    void RotateLocalX(Matrix34f *out_rot_matrix, float theta);

    void RotateLocalY(Matrix34f *out_rot_matrix, float theta);

    void RotateLocalZ(Matrix34f *out_rot_matrix, float theta);
}
