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

#ifdef VP_TARGET_ARCHITECTURE_x86
    #include <vp/codec/codec_aes128cbc.x86.hpp>
#endif
#include <vp/codec/codec_nisasyst.hpp>

#ifdef VP_TARGET_PLATFORM_win32
    #include <vp/codec/codec_zstd.h>
#endif
