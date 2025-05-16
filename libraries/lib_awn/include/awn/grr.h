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

    #include <d3d11_4.h>
    #include <dcomp.h>
    #include <awn/grr/impl/grr_dxgiinterfaceext.win32.h>
    #include <awn/grr/impl/grr_dxgicontext11.win32.h>

    #include <awn/grr/impl/grr_dragdrophandler.win32.h>
    #include <awn/grr/grr_nativewindow.win32.h>
    #include <awn/grr/grr_display.win32.h>
    #include <awn/grr/impl/grr_displaymanager.win32.h>
    #include <awn/grr/grr_displaymanagerapi.win32.h>

#elif VP_TARGET_PLATFORM_nx
#endif
