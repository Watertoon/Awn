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
    #include <awn/sys/sys_internalsynchronization.win32.h>
    #include <awn/sys/sys_mutex.win32.h>
#elifdef VP_TARGET_PLATFORM_nx
    #include <awn/sys/sys_mutex.nx.hpp>
#elifdef VP_TARGET_PLATFORM_linux
    #include <awn/sys/sys_internalsynchronization.linux.h>
    #include <awn/sys/sys_mutex.linux.h>
#endif

#include <awn/gfx/gfx_nngfxinteroperation.h>
#ifdef VP_TARGET_GRAPHICS_API_vk
    #include <awn/gfx/gfx_memorypool.vk.h>
#elif VP_TARGET_GRAPHICS_API_nvn
    #include <awn/gfx/gfx_memorypool.nvn.h>
#endif

#include <awn/mem/mem_heapmanager.h>
#include <awn/mem/mem_idisposer.h>
#include <awn/mem/mem_heap.hpp>
#include <awn/mem/mem_idisposer.hpp>
#include <awn/mem/mem_expheap.hpp>
#include <awn/mem/mem_separateheap.hpp>
#include <awn/mem/mem_unitheap.hpp>
#include <awn/mem/mem_virtualmemoryapi.h>
#include <awn/mem/mem_virtualaddressheap.h>

#include <awn/mem/mem_singleton.h>
#include <awn/mem/impl/mem_new.hpp>

#include <awn/mem/mem_frameheap.hpp>
//#include <awn/mem/mem_gpuexpheap.hpp>

//#include <awn/mem/mem_gpuheapmanager.hpp>
//#include <awn/mem/mem_gpuvirtualmemory.h>

