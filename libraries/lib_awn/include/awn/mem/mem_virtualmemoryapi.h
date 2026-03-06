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

namespace awn::mem {

    static constexpr size_t cPageSize = vp::util::c4KB;

    Result AllocateAddressRegion(void **out_address_region, size_t memory_size);
    void FreeAddressRegion(void *address_region, size_t memory_size);

    Result AllocateMemoryPages(void *address_region, size_t memory_size);
    void FreeMemoryPages(void *address_region, size_t free_size);

    Result AllocateLargeAddressRegionWithMemoryPages(void **out_large_address_region, size_t large_memory_size);
    void FreeLargeAddressRegionWithMemoryPages(void *large_address_region, size_t large_memory_size);

    Result GetFreeVirtualMemory(size_t *out_memory_size);
}
