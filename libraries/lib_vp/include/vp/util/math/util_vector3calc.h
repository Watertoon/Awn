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
    
    /* Rotates a vector by "angle" along the specified axis */
    void RotateVectorAxisX(Vector3f *out_vector, const Vector3f& rot_vector, float angle);

    void RotateVectorAxisY(Vector3f *out_vector, const Vector3f& rot_vector, float angle);

    void RotateVectorAxisZ(Vector3f *out_vector, const Vector3f& rot_vector, float angle);

    /* Makes the vector "to_parallelize" parallel to "base"  */
    void Parallelize(Vector3f *out_vector, const Vector3f& base, const Vector3f& to_parallelize);
}
