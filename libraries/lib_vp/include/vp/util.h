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

#include <vp/util/util_includes.h>
#include <vp/util/util_typedefs.h>
#include <vp/util/util_defines.h>

#include <vp/util/util_new.h>

#include <vp/util/util_spinloopintrinsics.x86.hpp>
#include <vp/util/util_busymutex.x86.hpp>
#include <vp/util/util_memorybarrier.x86.hpp>

#include <vp/util/util_alignment.hpp>
#include <vp/util/util_endian.hpp>
#include <vp/util/util_sizeconstants.hpp>
#include <vp/util/util_countbits.hpp>
#include <vp/util/util_member.hpp>
#include <vp/util/util_intrusivelist.hpp>
#include <vp/util/util_typestorage.hpp>
#include <vp/util/util_delegate.hpp>
#include <vp/util/util_runtimetypeinfo.hpp>
#include <vp/util/util_fixedobjectallocator.hpp>
#include <vp/util/util_fixedhandletable.hpp>
#include <vp/util/util_fixedringbuffer.hpp>
#include <vp/util/util_heapobjectallocator.hpp>
#include <vp/util/util_pointerarray.hpp>
#include <vp/util/util_heaparray.hpp>
#include <vp/util/util_timestamp.h>
#include <vp/util/util_timespan.hpp>
#include <vp/util/util_timeconstants.hpp>
#include <vp/util/util_constevalfail.hpp>
#include <vp/util/util_tstring.hpp>
#include <vp/util/util_charactercode.hpp>
#include <vp/util/util_bufferstring.hpp>
#include <vp/util/util_pathutil.hpp>
#include <vp/util/util_intrusiveredblacktree.hpp>
#include <vp/util/util_murmur32.hpp>
#include <vp/util/util_pointerarrayallocator.hpp>
#include <vp/util/util_ringbuffer.hpp>
#include <vp/util/util_color4.hpp>
#include <vp/util/util_job.hpp>
#include <vp/util/util_enumtraits.h>
#include <vp/util/util_priorityqueue.hpp>

#include <vp/util/math/sse4/util_int128.sse4.hpp>
#include <vp/util/math/sse4/util_float128.sse4.hpp>
#include <vp/util/math/sse4/util_vector2.hpp>
#include <vp/util/math/sse4/util_vector3.hpp>
#include <vp/util/math/sse4/util_vector3calc.h>
#include <vp/util/math/sse4/util_vector4.hpp>
#include <vp/util/math/util_constants.hpp>
#include <vp/util/math/sse4/util_matrix33.hpp>
#include <vp/util/math/sse4/util_matrix34.hpp>
#include <vp/util/math/sse4/util_matrix34calc.h>
#include <vp/util/math/sse4/util_matrix44.hpp>
#include <vp/util/math/util_clamp.hpp>

#include <vp/util/util_crc32b.hpp>
#include <vp/util/util_logicalframebuffer.hpp>
#include <vp/util/util_viewport.hpp>
#include <vp/util/util_camera.hpp>
#include <vp/util/util_projection.hpp>

#include <vp/util/util_deltatime.h>
