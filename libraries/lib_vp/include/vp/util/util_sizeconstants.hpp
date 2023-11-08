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

    constexpr inline u64 c1B    = 0x1;
    constexpr inline u64 c2B    = 0x2;
    constexpr inline u64 c4B    = 0x4;
    constexpr inline u64 c8B    = 0x8;
    constexpr inline u64 c16B   = 0x10;
    constexpr inline u64 c32B   = 0x20;
    constexpr inline u64 c64B   = 0x40;
    constexpr inline u64 c96B   = 0x60;
    constexpr inline u64 c128B  = 0x80;
    constexpr inline u64 c256B  = 0x100;
    constexpr inline u64 c384B  = 0x180;
    constexpr inline u64 c512B  = 0x200;

    constexpr inline u64 c1KB   = 0x400;
    constexpr inline u64 c2KB   = 0x800;
    constexpr inline u64 c4KB   = 0x1000;
    constexpr inline u64 c8KB   = 0x2000;
    constexpr inline u64 c16KB  = 0x4000;
    constexpr inline u64 c32KB  = 0x8000;
    constexpr inline u64 c64KB  = 0x1'0000;
    constexpr inline u64 c96KB  = 0x1'8000;
    constexpr inline u64 c128KB = 0x2'0000;
    constexpr inline u64 c256KB = 0x4'0000;
    constexpr inline u64 c384KB = 0x6'0000;
    constexpr inline u64 c512KB = 0x8'0000;

    constexpr inline u64 c1MB   = 0x10'0000;
    constexpr inline u64 c2MB   = 0x20'0000;
    constexpr inline u64 c4MB   = 0x40'0000;
    constexpr inline u64 c8MB   = 0x80'0000;
    constexpr inline u64 c16MB  = 0x100'0000;
    constexpr inline u64 c32MB  = 0x200'0000;
    constexpr inline u64 c64MB  = 0x400'0000;
    constexpr inline u64 c96MB  = 0x600'0000;
    constexpr inline u64 c128MB = 0x800'0000;
    constexpr inline u64 c256MB = 0x1000'0000;
    constexpr inline u64 c384MB = 0x1800'0000;
    constexpr inline u64 c512MB = 0x2000'0000;

    constexpr inline u64 c1GB    = 0x4000'0000;
    constexpr inline u64 c2GB    = 0x8000'0000;
    constexpr inline u64 c4GB    = 0x1'0000'0000;
    constexpr inline u64 c8GB    = 0x2'0000'0000;
    constexpr inline u64 c16GB   = 0x4'0000'0000;
    constexpr inline u64 c32GB   = 0x8'0000'0000;
    constexpr inline u64 c64GB   = 0x10'0000'0000;
    constexpr inline u64 c96GB   = 0x18'0000'0000;
    constexpr inline u64 c128GB  = 0x20'0000'0000;
    constexpr inline u64 c256GB  = 0x40'0000'0000;
    constexpr inline u64 c384GB  = 0x60'0000'0000;
    constexpr inline u64 c512GB  = 0x80'0000'0000;

    constexpr inline u64 c1TB    = 0x100'0000'0000;
    constexpr inline u64 c2TB    = 0x200'0000'0000;
    constexpr inline u64 c4TB    = 0x400'0000'0000;
    constexpr inline u64 c8TB    = 0x800'0000'0000;
    constexpr inline u64 c16TB   = 0x1000'0000'0000;
    constexpr inline u64 c32TB   = 0x2000'0000'0000;
    constexpr inline u64 c64TB   = 0x4000'0000'0000;
    constexpr inline u64 c96TB   = 0x6000'0000'0000;
    constexpr inline u64 c128TB  = 0x8000'0000'0000;
    constexpr inline u64 c256TB  = 0x1'0000'0000'0000;
    constexpr inline u64 c384TB  = 0x1'8000'0000'0000;
    constexpr inline u64 c512TB  = 0x2'0000'0000'0000;
}
