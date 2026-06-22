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
#include <awn.hpp>

namespace awn::mem {

    Result AllocateAddressRegion(void **out_address_region, size_t memory_size) {
        void *reserve = ::VirtualAlloc2(nullptr, nullptr, memory_size, MEM_RESERVE, PAGE_READWRITE, nullptr, 0);
        if (reserve == nullptr) { return res::ConvertWin32ErrorToResult(); }
        *out_address_region = reserve;
        RESULT_RETURN_SUCCESS;
    }

    Result AllocateMemoryPages(void *address_region, size_t memory_size) {
        void *commit = ::VirtualAlloc2(nullptr, address_region, memory_size, MEM_COMMIT, PAGE_READWRITE, nullptr, 0);
        if (commit == nullptr) { return res::ConvertWin32ErrorToResult(); }
        RESULT_RETURN_SUCCESS;
    }

    Result AllocateLargeAddressRegionWithMemoryPages(void **out_large_address_region, size_t large_memory_size) {
        void *large_commit = ::VirtualAlloc2(nullptr, nullptr, large_memory_size, MEM_RESERVE | MEM_COMMIT | MEM_LARGE_PAGES, PAGE_READWRITE, nullptr, 0);
        if (large_commit == nullptr) { return res::ConvertWin32ErrorToResult(); }
        *out_large_address_region = large_commit;
        RESULT_RETURN_SUCCESS;
    }

    void FreeMemoryPages(void *address_region, size_t free_size) {
        const bool result = ::VirtualFree(address_region, free_size, MEM_DECOMMIT);
        VP_ASSERT(result);
    }

    void FreeAddressRegion(void *address_region, size_t size) {
        VP_UNUSED(size);
        const bool result = ::VirtualFree(address_region, 0, MEM_RELEASE);
        VP_ASSERT(result);
    }

    void FreeLargeAddressRegionWithMemoryPages(void *large_address_region, size_t size) {
        FreeAddressRegion(large_address_region, size);
    }

    Result GetFreeVirtualMemory(size_t *out_memory_size) {
        MEMORYSTATUSEX memory_status = { .dwLength = sizeof(MEMORYSTATUSEX) };
        const bool result = ::GlobalMemoryStatusEx(std::addressof(memory_status));
        if (!result) { return res::ConvertWin32ErrorToResult(); }
        *out_memory_size = memory_status.ullAvailPhys;
        RESULT_RETURN_SUCCESS;
    }
}
