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

    typedef u32 UKernHandle;
    typedef u64 UKernCoreMask;

    using ThreadFunction = void (*)(void *);

    constexpr inline UKernHandle InvalidHandle                = 0xFFFFFFFF;
    constexpr inline size_t      MaxFiberNameLength           = 32;
    constexpr inline s64         WindowsToUKernPriorityOffset = 2;
    constexpr inline size_t      MainThreadHandle             = 1;
    constexpr inline size_t      MaxCoreCount                 = 32;
    constexpr inline size_t      MaxThreadCount               = 256;

    static_assert(2 == (THREAD_PRIORITY_NORMAL + WindowsToUKernPriorityOffset));

    enum FiberState : u32 {
        FiberState_Scheduled,
        FiberState_Running,
        FiberState_Exiting,
        FiberState_Waiting,
        FiberState_Suspended
    };

    enum ActivityLevel : u16 {
        ActivityLevel_Suspended,
        ActivityLevel_Schedulable
    };

    namespace impl {
        class WaitableObject;
    }

    struct FiberLocalStorage {
        s32                          priority;
        u32                          current_core;
        UKernCoreMask                core_mask;
        size_t                       stack_size;
        void*                        user_arg;
        ThreadFunction               user_function;
        bool                         is_suspended;
        UKernHandle                  ukern_fiber_handle;
        void                        *win32_fiber_handle;
        vp::util::IntrusiveListNode  scheduler_list_node;
        vp::util::IntrusiveListNode  wait_list_node;

        using WaitList = vp::util::IntrusiveListTraits<FiberLocalStorage, &FiberLocalStorage::wait_list_node>::List;
        WaitList                     wait_list;
        u16                          was_locked;
        ActivityLevel                activity_level;
        u32                          wait_tag;
        u32                         *lock_address;
        u32                         *wait_address;
        impl::WaitableObject        *waitable_object;
        u64                          timeout;
        u32                          last_result;
        u32                          fiber_state;
        const char                  *fiber_name;
        char                         fiber_name_storage[MaxFiberNameLength];

        static constexpr u32 HasChildWaitersBit = 0x4000'0000;

        constexpr ALWAYS_INLINE FiberLocalStorage() {/*...*/}

        bool IsSchedulable(u32 core_number, u64 time);
        void ReleaseLockWaitListUnsafe();
    };

    constexpr inline size_t UserFiberStorageSize = MaxThreadCount * sizeof(FiberLocalStorage);

    typedef FiberLocalStorage ThreadType;
}
