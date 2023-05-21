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
#include <cmath>

/* STD */
#include <memory>
#include <type_traits>
#include <mutex>
#include <array>

/* Windows */
#ifdef VP_TARGET_PLATFORM_win32
    #define WIN32_LEAN_AND_MEAN
    #include <windows.h>
    #include <shellapi.h>
#endif

/* Undef erroneous near and far from windef.h */
#ifdef near
    #undef near
#endif
#ifdef far
    #undef far
#endif

/* Vulkan */
#ifdef VP_TARGET_GRAPHICS_API_vk
    #define VK_USE_PLATFORM_WIN32_KHR
    #define VK_ENABLE_BETA_EXTENSIONS
    #include <vulkan/vulkan.h>
#endif

/* DD */
#include "util_defines.h"
