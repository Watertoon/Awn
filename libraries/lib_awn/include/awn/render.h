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

#ifdef VP_TARGET_PLATFORM_win32
    #include <dxgi1_6.h>
#ifdef VP_DEBUG
    #include <dxgidebug.h>
#endif
    #include <d3d12.h>
    #include <dcomp.h>
    #include <awn/render/impl/render_dxgiinterfaceext.win32.h>
    #include <awn/render/impl/render_dxgicontext.win32.h>
    #include <awn/render/impl/render_swapchain.win32.hpp>
    #include <awn/render/render_nativelayer.win32.h>
    #include <awn/render/render_nativedisplay.win32.h>
    #include <awn/render/impl/render_nativevideomanager.win32.hpp>
#elif VP_TARGET_PLATFORM_nx

#endif
#include <awn/render/render_nativevideoapi.h>
