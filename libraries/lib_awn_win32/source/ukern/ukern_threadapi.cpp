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

namespace awn::ukern {

    Result CreateThread(UKernHandle *out_handle, ThreadFunction thread_func, uintptr_t arg, size_t stack_size, u32 priority, u32 core_id) {
        return impl::GetScheduler()->CreateThreadImpl(out_handle, thread_func, arg, stack_size, priority, core_id);
    }

    void ExitThread(UKernHandle handle) {
        impl::GetScheduler()->ExitThreadImpl(handle);
    }

    Result StartThread(UKernHandle handle) {
        return impl::GetScheduler()->SetActivityImpl(handle, ActivityLevel_Schedulable);
    }

    Result ResumeThread (UKernHandle handle) {
        return impl::GetScheduler()->SetActivityImpl(handle, ActivityLevel_Schedulable);
    }

    Result SuspendThread(UKernHandle handle) {
        return impl::GetScheduler()->SetActivityImpl(handle, ActivityLevel_Suspended);
    }

    void SetThreadName(UKernHandle handle, const char *name) {

        /* Get fiber from handle */
        FiberLocalStorage *fiber = impl::GetScheduler()->GetFiberByHandle(handle);
        VP_ASSERT(fiber != nullptr);

        /* Set name */
        if (name == nullptr) {
            impl::UserScheduler::SetInitialFiberNameUnsafe(fiber);
        } else {
            ::strncpy(fiber->fiber_name_storage, name, MaxFiberNameLength);
        }
    }

    void SetThreadNamePointer(UKernHandle handle, const char *name) {

        /* Get fiber from handle */
        FiberLocalStorage *fiber = impl::GetScheduler()->GetFiberByHandle(handle);
        VP_ASSERT(fiber != nullptr);

        /* Set name */
        if (name == nullptr) {
            impl::UserScheduler::SetInitialFiberNameUnsafe(fiber);
        } else {
            fiber->fiber_name = name;
        }
    }

    Result SetThreadPriority(UKernHandle handle, u32 priority) {
        return impl::GetScheduler()->SetPriorityImpl(handle, priority);
    }

    Result SetThreadCoreMask(UKernHandle handle, UKernCoreMask core_mask) {
        return impl::GetScheduler()->SetCoreMaskImpl(handle, core_mask);
    }

    void Sleep(TimeSpan timeout_span) {
        impl::GetScheduler()->SleepThreadImpl(impl::GetAbsoluteTimeToWakeup(timeout_span));
    }

    void YieldThread() {
        impl::GetScheduler()->SleepThreadImpl(0);
    }

    ThreadType *GetCurrentThread() { return impl::GetScheduler()->GetCurrentThreadImpl(); }
}
