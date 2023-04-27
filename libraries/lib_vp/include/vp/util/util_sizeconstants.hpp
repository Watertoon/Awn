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

namespace vp::util {

    consteval size_t KiloBytesToBytes(size_t kb) { return (kb * 1024); }
    consteval size_t MegaBytesToBytes(size_t mb) { return (mb * 1024 * 1024); }
    consteval size_t GigaBytesToBytes(size_t gb) { return (gb * 1024 * 1024 * 1024); }
    consteval size_t TeraBytesToBytes(size_t tb) { return (tb * 1024 * 1024 * 1024 * 1024); }

    consteval size_t BytesToKiloBytes(size_t b)      { return (b / 1024); }
    consteval size_t MegaBytesToKiloBytes(size_t mb) { return (mb * 1024); }
    consteval size_t GigaBytesToKiloBytes(size_t gb) { return (gb * 1024 * 1024); }
    consteval size_t TeraBytesToKiloBytes(size_t tb) { return (tb * 1024 * 1024 * 1024); }

    consteval size_t BytesToMegaBytes(size_t b)      { return (b / 1024 / 1024); }
    consteval size_t KiloBytesToMegaBytes(size_t kb) { return (kb / 1024); }
    consteval size_t GigaBytesToMegaBytes(size_t gb) { return (gb * 1024); }
    consteval size_t TeraBytesToMegaBytes(size_t tb) { return (tb * 1024 * 1024); }

    consteval size_t BytesToGigaBytes(size_t b)      { return (b / 1024 / 1024 / 1024); }
    consteval size_t KiloBytesToGigaBytes(size_t kb) { return (kb / 1024 / 1024); }
    consteval size_t MegaBytesToGigaBytes(size_t mb) { return (mb / 1024); }
    consteval size_t TeraBytesToGigaBytes(size_t tb) { return (tb * 1024); }

    consteval size_t BytesToTeraBytes(size_t b)      { return (b / 1024 / 1024 / 1024 / 1024); }
    consteval size_t KiloBytesToTeraBytes(size_t kb) { return (kb / 1024 / 1024 / 1024); }
    consteval size_t MegaBytesToTeraBytes(size_t tb) { return (tb / 1024 / 1024); }
    consteval size_t GigaBytesToTeraBytes(size_t gb) { return (gb / 1024); }

    constexpr inline size_t c1B    = 0x1;
    constexpr inline size_t c2B    = 0x2;
    constexpr inline size_t c4B    = 0x4;
    constexpr inline size_t c8B    = 0x8;
    constexpr inline size_t c16B   = 0x10;
    constexpr inline size_t c32B   = 0x20;
    constexpr inline size_t c64B   = 0x40;
    constexpr inline size_t c96B   = 0x60;
    constexpr inline size_t c128B  = 0x80;
    constexpr inline size_t c256B  = 0x100;
    constexpr inline size_t c384B  = 0x180;
    constexpr inline size_t c512B  = 0x200;

    constexpr inline size_t c1KB   = 0x400;
    constexpr inline size_t c2KB   = 0x800;
    constexpr inline size_t c4KB   = 0x1000;
    constexpr inline size_t c8KB   = 0x2000;
    constexpr inline size_t c16KB  = 0x4000;
    constexpr inline size_t c32KB  = 0x8000;
    constexpr inline size_t c64KB  = 0x1'0000;
    constexpr inline size_t c96KB  = 0x1'8000;
    constexpr inline size_t c128KB = 0x2'0000;
    constexpr inline size_t c256KB = 0x4'0000;
    constexpr inline size_t c384KB = 0x6'0000;
    constexpr inline size_t c512KB = 0x8'0000;

    constexpr inline size_t c1MB   = 0x10'0000;
    constexpr inline size_t c2MB   = 0x20'0000;
    constexpr inline size_t c4MB   = 0x40'0000;
    constexpr inline size_t c8MB   = 0x80'0000;
    constexpr inline size_t c16MB  = 0x100'0000;
    constexpr inline size_t c32MB  = 0x200'0000;
    constexpr inline size_t c64MB  = 0x400'0000;
    constexpr inline size_t c96MB  = 0x600'0000;
    constexpr inline size_t c128MB = 0x800'0000;
    constexpr inline size_t c256MB = 0x1000'0000;
    constexpr inline size_t c384MB = 0x1800'0000;
    constexpr inline size_t c512MB = 0x2000'0000;

    constexpr inline size_t c1GB    = 0x4000'0000;
    constexpr inline size_t c2GB    = 0x8000'0000;
    constexpr inline size_t c4GB    = 0x1'0000'0000;
    constexpr inline size_t c8GB    = 0x2'0000'0000;
    constexpr inline size_t c16GB   = 0x4'0000'0000;
    constexpr inline size_t c32GB   = 0x8'0000'0000;
    constexpr inline size_t c64GB   = 0x10'0000'0000;
    constexpr inline size_t c96GB   = 0x18'0000'0000;
    constexpr inline size_t c128GB  = 0x20'0000'0000;
    constexpr inline size_t c256GB  = 0x40'0000'0000;
    constexpr inline size_t c384GB  = 0x60'0000'0000;
    constexpr inline size_t c512GB  = 0x80'0000'0000;

    constexpr inline size_t c1TB    = 0x100'0000'0000;
    constexpr inline size_t c2TB    = 0x200'0000'0000;
    constexpr inline size_t c4TB    = 0x400'0000'0000;
    constexpr inline size_t c8TB    = 0x800'0000'0000;
    constexpr inline size_t c16TB   = 0x1000'0000'0000;
    constexpr inline size_t c32TB   = 0x2000'0000'0000;
    constexpr inline size_t c64TB   = 0x4000'0000'0000;
    constexpr inline size_t c96TB   = 0x6000'0000'0000;
    constexpr inline size_t c128TB  = 0x8000'0000'0000;
    constexpr inline size_t c256TB  = 0x1'0000'0000'0000;
    constexpr inline size_t c384TB  = 0x1'8000'0000'0000;
    constexpr inline size_t c512TB  = 0x2'0000'0000'0000;
}
