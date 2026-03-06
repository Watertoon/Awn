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
#include <meta>

/* Architecture includes */
#ifdef VP_TARGET_ARCHITECTURE_x86
    #include <immintrin.h>
#endif

/* Platform includes */
#ifdef VP_TARGET_PLATFORM_win32
    #define NTDDI_VERSION NTDDI_WIN11_GE
    #define WINVER _WIN32_WINNT_WIN10

    #ifdef _WIN32_WINNT
        #undef _WIN32_WINNT
    #endif
    #define _WIN32_WINNT _WIN32_WINNT_WIN10
    #define WIN32_LEAN_AND_MEAN
    #include <windows.h>
    //#include <windowsx.h>
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

#ifdef VP_TARGET_PLATFORM_linux
    #define _LARGEFILE64_SOURCE
    #include <linux/input.h>
    #include <linux/futex.h>
    #include <linux/prctl.h>
    #include <linux/sched.h>
    #include <linux/mman.h>
    #include <sys/types.h>
    #include <sys/eventfd.h>
    #include <sys/inotify.h>
    #include <sys/mman.h>
    #include <sys/poll.h>
    #include <sys/prctl.h>
    #include <sys/resource.h>
    #include <sys/syscall.h>
    #include <sys/stat.h>
    #include <sys/timerfd.h>
    #include <sys/wait.h>
    #include <systemd/sd-device.h>
    #include <dlfcn.h>
    #include <dirent.h>
    #include <fcntl.h>
    #include <signal.h>
    #include <unistd.h>
    #include <gbm.h>
#endif

/* zstd */
#define ZSTD_STATIC_LINKING_ONLY
#include <zstd.h>

/* Graphics api includes */
#ifdef VP_TARGET_GRAPHICS_API_vk

    #ifdef VP_TARGET_PLATFORM_win32
        #define VK_USE_PLATFORM_WIN32_KHR
    #elifdef VP_TARGET_PLATFORM_linux
        #define VK_USE_PLATFORM_WAYLAND_KHR
    #endif

    #define VK_ENABLE_BETA_EXTENSIONS
    #include <vulkan/vulkan.h>
#endif

#include "util_defines.h"
