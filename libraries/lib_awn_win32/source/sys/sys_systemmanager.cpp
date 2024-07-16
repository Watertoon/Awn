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

    VP_ENUM_TRAITS_IMPL(awn::sys, RegionType);
    VP_ENUM_TRAITS_IMPL(awn::sys, LanguageType);
    VP_ENUM_TRAITS_IMPL(awn::sys, ProductType);

    namespace {
        constinit vp::util::TypedStorage<SystemManager> sSystemManagerStorage   = {};
        bool                                            sSystemManagerInitGuard = false;
    }

    void InitializeSystemManager() {

        /* Check init guard */
        if (sSystemManagerInitGuard == true) { return; }
        sSystemManagerInitGuard = true;

        /* Construct system manager */
        vp::util::ConstructAt(sSystemManagerStorage);

        SystemManager *sys_mgr = vp::util::GetPointer(sSystemManagerStorage);

        /* Get real process handle */
        const bool result1 = ::DuplicateHandle(::GetCurrentProcess(), ::GetCurrentProcess(), ::GetCurrentProcess(), std::addressof(sys_mgr->process_handle), 0, false, DUPLICATE_SAME_ACCESS);
        VP_ASSERT(result1);

        /* Get system info */
        ::GetSystemInfo(std::addressof(sys_mgr->system_info));
        VP_ASSERT(sys_mgr->system_info.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64);
        VP_ASSERT(sys_mgr->system_info.dwPageSize == 0x1000);

        /* Get current process affinity mask */
        const bool result = ::GetProcessAffinityMask(sys_mgr->process_handle, std::addressof(sys_mgr->process_core_mask), std::addressof(sys_mgr->system_core_mask));
        VP_ASSERT(result);
        VP_ASSERT(sys_mgr->process_core_mask != 0 && sys_mgr->system_core_mask != 0);

        sys_mgr->system_core_count = vp::util::CountOneBits64(sys_mgr->system_core_mask);
        sys_mgr->process_core_count = vp::util::CountOneBits64(sys_mgr->process_core_mask);

        return;
    }

    bool IsSystemManagerInitialized() { return sSystemManagerInitGuard; }

    SystemManager *GetSystemManager() { return vp::util::GetPointer(sSystemManagerStorage); }

    u32 GetProcessCoreCount() {
        return GetSystemManager()->process_core_count;
    }

    u32 GetSystemCoreCount() {
        return GetSystemManager()->system_core_count;
    }

    CoreMask GetProcessCoreMask() {
        return GetSystemManager()->process_core_mask;
    }

    CoreMask GetSystemCoreMask() {
        return GetSystemManager()->system_core_mask;
    }

    Result SetProcessCoreMask(CoreMask core_mask) {
        if (core_mask == GetProcessCoreMask()) { RESULT_RETURN_SUCCESS; }
        RESULT_RETURN_IF((core_mask & ~GetSystemCoreMask()) != 0, ResultInvalidCoreMask);
        SystemManager *sys_mgr = vp::util::GetPointer(sSystemManagerStorage);
        const bool result0 = ::SetProcessAffinityMask(sys_mgr->process_handle, core_mask);
        const bool result1 = ::GetProcessAffinityMask(sys_mgr->process_handle, std::addressof(sys_mgr->process_core_mask), std::addressof(sys_mgr->system_core_mask));
        VP_ASSERT(result1);
        sys_mgr->system_core_count = vp::util::CountOneBits64(sys_mgr->system_core_mask);
        sys_mgr->process_core_count = vp::util::CountOneBits64(sys_mgr->process_core_mask);
        if (!result0) { return res::ConvertWin32ErrorToResult(); }
        RESULT_RETURN_SUCCESS;
    }

    u32 GetCurrentCoreNumber() {
        return ::GetCurrentProcessorNumber();
    }

    SYSTEM_INFO *GetSystemInfo() {
        return std::addressof(GetSystemManager()->system_info);
    }

    vp::util::Rectanglei GetVirtualScreenRectangle() {
        const s32 x      = ::GetSystemMetrics(SM_XVIRTUALSCREEN);
        const s32 y      = ::GetSystemMetrics(SM_YVIRTUALSCREEN);
        const s32 width  = ::GetSystemMetrics(SM_CXVIRTUALSCREEN);
        const s32 height = ::GetSystemMetrics(SM_CYVIRTUALSCREEN);
        return { .left = x, .top = y, .right = x + width, .bottom = y + height, };
    }

    u32 GetMonitorAndMirrorCount() {

        u32  i      = 0;
        bool result = ::EnumDisplayDevicesA(nullptr, i, nullptr, 0);
        while(result == true) {
            ++i;
            result = ::EnumDisplayDevicesA(nullptr, i, nullptr, 0);
        }

        return i;
    }

    u32 GetMonitorCount() {

        DISPLAY_DEVICEA display_device = {
            .cb = sizeof(DISPLAY_DEVICEA),
        };
        u32  count  = 0;
        u32  i      = 0;
        bool result = ::EnumDisplayDevicesA(nullptr, i, std::addressof(display_device), 0);
        while(result == true) {
            if ((display_device.StateFlags & DISPLAY_DEVICE_MIRRORING_DRIVER) == 0) { ++count; }
            ++i;
            result = ::EnumDisplayDevicesA(nullptr, i, std::addressof(display_device), 0);
        }

        return i;
    }

    bool GetMonitorIndexArray(u32 *out_index_array, u32 max_indice_count) {

        DISPLAY_DEVICEA display_device = {
            .cb = sizeof(DISPLAY_DEVICEA),
        };
        u32  count  = 0;
        u32  i      = 0;
        bool result = ::EnumDisplayDevicesA(nullptr, i, std::addressof(display_device), 0);
        while(result == true) {
            if ((display_device.StateFlags & DISPLAY_DEVICE_MIRRORING_DRIVER) != 0) { ++i; continue; }
            if (max_indice_count <= count) { return false; }
            out_index_array[count] = i;
            ++count; 
            ++i;
            result = ::EnumDisplayDevicesA(nullptr, i, std::addressof(display_device), 0);
        }

        return true;
    }

    bool IsProcessInFocus() {
        HWND focus_window = ::GetForegroundWindow();
        if (focus_window == nullptr) { return false; }
        long unsigned int window_process_id = 0;
        const u32 result = ::GetWindowThreadProcessId(focus_window, std::addressof(window_process_id));
        return result != 0 && ::GetCurrentProcessId() == window_process_id;
    }
}
