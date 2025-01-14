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

#include <vp/image/image_pixelformat.h>

#define VP_IMAGE_ENABLE_PNG

#ifdef VP_IMAGE_ENABLE_PNG
    #include <vp/image/impl/image_pngimpl.h>
    #include <vp/image/image_pngdecoder.h>
    #include <vp/image/image_pngencoder.h>
#endif
