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

namespace awn::sys {

    struct SystemManager {
        u32         active_processor_count;
        u32         process_core_count;
        u64         process_core_mask;
        u64         system_core_mask;
        SYSTEM_INFO system_info;

        static constexpr size_t cMaximumSupportedProcessorCount = 64;
    };

    void InitializeSystemManager();
    
    bool IsSystemManagerInitialized();

    SystemManager *GetSystemManager();

    u32 GetServiceCoreCount();
    u32 GetCoreCount();
    u32 GetCurrentCoreNumber();
    
    SYSTEM_INFO *GetSystemInfo();
}
