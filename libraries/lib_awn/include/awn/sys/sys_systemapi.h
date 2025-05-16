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
        awn::sys::RegionType, RegionType,
        UnitedStates,
        Europe,
        Japan,
        Taiwan,
        Default
    );

    enum class LanguageType : u32 {
        English  = 0,
        Japanese = 1,
        Default  = 2,
    };
    VP_ENUM_TRAITSV(
        awn::sys::LanguageType, LanguageType,
        English,
        Japanese,
        Default
    );

    enum class ProductType : u32 {
        Debug   = 0,
        Develop = 1,
        Product = 2,
        Pilot   = 3,
    };
    VP_ENUM_TRAITSV(
        awn::sys::ProductType, ProductType,
        Debug,
        Develop,
        Product,
        Pilot
    );
}
VP_ENUM_TRAITS_AUTO(awn::sys::RegionType);
VP_ENUM_TRAITS_AUTO(awn::sys::LanguageType);
VP_ENUM_TRAITS_AUTO(awn::sys::ProductType);

namespace awn::sys {

    static constexpr size_t cMaxCoreCount = 64;
    using CoreMask = u64;

#ifdef VP_TARGET_PLATFORM_win32
    using NativeHandle = HANDLE;
#elif VP_TARGET_PLATFORM_nx
    using NativeHandle = nn::os::Handle;
#endif

    void InitializeSystem();
    void FinalizeSystem();
    bool IsSystemInitialized();

    NativeHandle GetProcessHandle();

    u32 GetProcessCoreCount();
    u32 GetSystemCoreCount();

    CoreMask GetProcessCoreMask();
    Result SetProcessCoreMask(CoreMask core_mask);

    u32 GetCurrentCoreNumber();

    size_t GetPageSize();
    
    bool IsEnableLargePages();
    size_t GetLargePageSize();

    u32 GetAllocationGranularity();

    vp::util::Rectanglei GetVirtualScreenRectangle();
    u32                  GetMonitorAndMirrorCount();
    u32                  GetMonitorCount();
    bool                 GetMonitorIndexArray(u32 *out_index_array, u32 max_indice_count);

    bool IsProcessInFocus();
}
