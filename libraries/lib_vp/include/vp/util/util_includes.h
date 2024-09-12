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

/* Libc */
#include <cstddef>
#include <climits>
#include <cstring>
#include <cstdarg>
#include <cmath>
#include <cfloat>

/* STD */
#include <memory>
#include <type_traits>
#include <mutex>
#include <array>
#include <algorithm>

/* Platform includes */
#ifdef VP_TARGET_PLATFORM_win32
    #define NTDDI_VERSION NTDDI_WIN10_19H1
    #define WINVER 0x0A00
    
    #ifdef _WIN32_WINNT
        #undef _WIN32_WINNT
    #endif
    #define _WIN32_WINNT _WIN32_WINNT_WIN10
    #define WIN32_LEAN_AND_MEAN
    #include <windows.h>
    #include <shellapi.h>
    #include <shlwapi.h>
    #include <shellscalingapi.h>

    /* Undef macros that ruin the api */
    #ifdef near
        #undef near
    #endif
    #ifdef far
        #undef far
    #endif
    #ifdef InterlockedIncrementAcquire
        #undef InterlockedIncrementAcquire
    #endif
    #ifdef InterlockedIncrementRelease
        #undef InterlockedIncrementRelease
    #endif
    #ifdef InterlockedDecrementAcquire
        #undef InterlockedDecrementAcquire
    #endif
    #ifdef InterlockedDecrementRelease
        #undef InterlockedDecrementRelease
    #endif
    #ifdef InterlockedIncrementRelease
        #undef InterlockedIncrementRelease
    #endif
    #ifdef InterlockedCompareExchangeAcquire
        #undef InterlockedCompareExchangeAcquire
    #endif
    #ifdef InterlockedCompareExchangeRelease
        #undef InterlockedCompareExchangeRelease
    #endif
    #ifdef CreateWindow
        #undef CreateWindow
    #endif
    #ifdef GetClassName
        #undef GetClassName
    #endif
#endif

/* zstd */
#define ZSTD_STATIC_LINKING_ONLY
#include <zstd.h>

/* Graphics api includes */
#ifdef VP_TARGET_GRAPHICS_API_vk
    #define VK_USE_PLATFORM_WIN32_KHR
    #define VK_ENABLE_BETA_EXTENSIONS
    #include <vulkan/vulkan.h>
#endif

#include "util_defines.h"
