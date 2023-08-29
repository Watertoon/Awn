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

namespace awn::ukern {

    namespace impl {
        class UserScheduler;

        UserScheduler *GetScheduler();
    }

    Result CreateThread(UKernHandle *out_handle, ThreadFunction thread_func, uintptr_t arg, size_t stack_size, u32 priority, u32 core_id);
    void   ExitThread(UKernHandle handle);

    Result StartThread(UKernHandle handle);
    Result ResumeThread (UKernHandle handle);
    Result SuspendThread(UKernHandle handle);

    void   SetThreadName(UKernHandle handle, const char *name);
    void   SetThreadNamePointer(UKernHandle handle, const char *name);
    Result SetThreadPriority(UKernHandle handle, u32 priority);
    Result SetThreadCoreMask(UKernHandle handle, UKernCoreMask core_mask);

    Result GetThreadPriority(u32 *out_priority, UKernHandle handle);
    Result GetThreadCoreMask(UKernCoreMask *out_core_mask, UKernHandle handle);

    void Sleep(TimeSpan timeout_span);
    void YieldThread();

    ThreadType *GetCurrentThread();
    u32 GetCoreCount();
}
