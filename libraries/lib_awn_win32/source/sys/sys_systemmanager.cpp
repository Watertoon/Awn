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

namespace awn::sys {

    namespace {
        constinit vp::util::TypeStorage<SystemManager> sSystemManagerStorage = {};
    }

    void InitializeSystemManager() {

        /* Construct system manager */
        vp::util::ConstructAt(sSystemManagerStorage);

        SystemManager *instance = vp::util::GetPointer(sSystemManagerStorage);

        /* Get system info */
        ::GetSystemInfo(std::addressof(instance->system_info));
        VP_ASSERT(instance->system_info.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64);
        VP_ASSERT(instance->system_info.dwPageSize == 0x1000);

        /* Get active processor count from group 0 */
        instance->active_processor_count = ::GetActiveProcessorCount(0);
        VP_ASSERT(instance->active_processor_count != 0);

        /* Get current process affinity mask */
        const bool result = ::GetProcessAffinityMask(::GetCurrentProcess(), std::addressof(instance->process_core_mask), std::addressof(instance->system_core_mask));
        VP_ASSERT(result != false);
        VP_ASSERT(instance->process_core_mask != 0 && instance->system_core_mask != 0);

        instance->process_core_count = vp::util::CountOneBits64(instance->process_core_mask);
    }

    SystemManager *GetSystemManager() { return vp::util::GetPointer(sSystemManagerStorage); }

    u32 GetServiceCoreCount() {
        return GetSystemManager()->process_core_count;
    }
    u32 GetCoreCount() {
        return ukern::GetCoreCount();
    }

    u32 GetCurrentCoreNumber() {
        return ::GetCurrentProcessorNumber();
    }

    SYSTEM_INFO *GetSystemInfo() {
        return std::addressof(GetSystemManager()->system_info);
    }
}
