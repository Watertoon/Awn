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

    enum class RegionType : u32 {
        UnitedStates = 0,
        Europe       = 1,
        Japan        = 2,
        Taiwan       = 3,
        Default      = 4,
    };
    VP_ENUM_TRAITSV(
        awn::sys, RegionType,
        UnitedStates, 0x0,
        Europe,       0x1,
        Japan,        0x2,
        Taiwan,       0x3,
        Default,      0x4
    );

    enum class LanguageType : u32 {
        English  = 0,
        Japanese = 1,
        Default  = 2,
    };
    VP_ENUM_TRAITSV(
        awn::sys, LanguageType,
        English,  0x0,
        Japanese, 0x1,
        Default,  0x2
    );

    enum class ProductType : u32 {
        Debug   = 0,
        Develop = 1,
        Product = 2,
        Pilot   = 3,
    };
    VP_ENUM_TRAITSV(
        awn::sys, ProductType,
        Debug          , 0x0,
        Develop        , 0x1,
        Product        , 0x2,
        Pilot          , 0x3
    );
}
VP_ENUM_TRAITS_AUTO(awn::sys::RegionType);
VP_ENUM_TRAITS_AUTO(awn::sys::LanguageType);
VP_ENUM_TRAITS_AUTO(awn::sys::ProductType);

namespace awn::sys {

    struct SystemManager {
        bool        init_guard;
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
    
    vp::util::Rectanglei GetVirtualScreenRectangle();
    u32                  GetMonitorAndMirrorCount();
    u32                  GetMonitorCount();
    bool                 GetMonitorIndexArray(u32 *out_index_array, u32 max_indice_count);

    bool IsProcessInFocus();
}
