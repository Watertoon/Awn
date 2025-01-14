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

namespace vp::image {

    enum class PixelFormat {
        Invalid = 0,
        Pallete1 = 1,
        Pallete2 = 2,
        Pallete4 = 3,
        Pallete8 = 4,
        Gray1 = 5,
        Gray2 = 6,
        Gray4 = 7,
        Gray8 = 8,
        Gray16 = 9,
        R8G8B8 = 10,
        R16G16B16 = 11,
        Gray8A8 = 12,
        Gray16A16 = 13,
        R8G8B8A8 = 14,
        R16G16B16A16 = 15,
    };

    constexpr bool IsAlphaFormat(PixelFormat format) {
        return (format == PixelFormat::Gray8A8) || (format == PixelFormat::Gray16A16) || (format == PixelFormat::R8G8B8A8) || (format == PixelFormat::R16G16B16A16);
    }

    constexpr bool IsColorFormat(PixelFormat format) {
        return (format == PixelFormat::R8G8B8) || (format == PixelFormat::R16G16B16) || (format == PixelFormat::R8G8B8A8) || (format == PixelFormat::R16G16B16A16) || (format == PixelFormat::Pallete1) || (format == PixelFormat::Pallete2) || (format == PixelFormat::Pallete4) || (format == PixelFormat::Pallete8);
    }

    constexpr u32 GetBitDepth(PixelFormat pixel_format) {
        static constexpr u32 cBitDepthArray[0x10] = {
            0, /* Invalid */
            1, /* Pallete1 */
            2, /* Pallete2 */
            4, /* Pallete4 */
            8, /* Pallete8 */
            1, /* Gray1 */
            2, /* Gray2 */
            4, /* Gray4 */
            8, /* Gray8 */
            16, /* Gray16 */
            8, /* R8G8B8 */
            16, /* R16G16B16 */
            8, /* Gray8A8 */
            16, /* Gray16A16 */
            8, /* R8G8B8A8 */
            16, /* R16G16B16A16 */
        };
        const u32 index = static_cast<u32>(pixel_format);
        if (0xf < index) { VP_UNEXPECTED_DEFAULT(); }
        return cBitDepthArray[index];
    }

    constexpr u32 GetChannelCount(PixelFormat pixel_format) {
        static constexpr u32 cChannelCountArray[0x10] = {
            0, /* Invalid */
            1, /* Pallete1 */
            1, /* Pallete2 */
            1, /* Pallete4 */
            1, /* Pallete8 */
            1, /* Gray1 */
            1, /* Gray2 */
            1, /* Gray4 */
            1, /* Gray8 */
            1, /* Gray16 */
            3, /* R8G8B8 */
            3, /* R16G16B16 */
            2, /* Gray8A8 */
            2, /* Gray16A16 */
            4, /* R8G8B8A8 */
            4, /* R16G16B16A16 */
        };
        const u32 index = static_cast<u32>(pixel_format);
        if (0xf < index) { VP_UNEXPECTED_DEFAULT(); }
        return cChannelCountArray[index];
    }

    using PngMemoryAllocate = void*(size_t,void*);
    using PngMemoryFree = void(void*,void*);
}
