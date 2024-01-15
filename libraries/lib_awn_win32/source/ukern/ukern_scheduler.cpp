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

namespace awn::ukern::impl {

    constinit vp::util::FixedObjectAllocator<FiberLocalStorage, cMaxThreadCount> sUserFiberLocalAllocator = {};

    TickSpan GetAbsoluteTimeToWakeup(TimeSpan timeout_ns) {

        /* Convert to tick */
        const s64 timeout_tick = timeout_ns.GetTick();

        /* "Infinite" time on zero */
        if (0 >= timeout_tick) { return TimeSpan::cMaxTime; }

        /* Calculate absolute timeout */
        const s64 absolute_time = timeout_tick + vp::util::GetSystemTick();

        if (absolute_time + 1 <= 1) { return TimeSpan::cMaxTime; }

        return absolute_time + 2;
    }

    NO_RETURN void UserScheduler::SchedulerFiberMain(size_t core_num) {

        /* Alias core number */
        const u32 core_number = core_num;

        /* Label for post dispatch/rest restart */
        _ukern_scheduler_restart:

        /* Get current time for fibers on a timeout */
        const u64 tick = vp::util::GetSystemTick();

        /* Visit waiting thread list for timeouts */
        WaitList::iterator wait_iter = m_wait_list.begin();
        while (wait_iter != m_wait_list.end()) {

            /* Get local storage */
            FiberLocalStorage &waiting_fiber = *wait_iter;

            /* Advance wait list */
            ++wait_iter;

            /* Check if the last local storage has become schedulable */
            waiting_fiber.IsSchedulable(core_number, tick);
        }

        /* Try acquire local ring job */
        FiberLocalStorage *fiber = (m_scheduler_local_ring_table[core_number].GetUsedCount() == 0) ? nullptr : m_scheduler_local_ring_table[core_number].RemoveFront();

        /* Fallback to schedule from priority queue */
        while (fiber == nullptr && m_thread_queue.GetUsedCount() != 0) {
            fiber = m_thread_queue.RemoveFront();

            /* Setup core mask */
            UKernCoreMask core_mask             = fiber->core_mask;
            const u32     runnable_thread_count = vp::util::CountOneBits64(fiber->core_mask);

            /* Check for whether the current core is preferred */
            if (runnable_thread_count == 1 && ((core_mask & (1 << vp::util::CountRightZeroBits64(core_mask))) != 0)) { break; }
            if (fiber->current_core == core_number && (core_mask & (1 << core_number)) != 0) { break; }

            /* Rank threads */
            LocalThreadRingBuffer *ring_buffer    = nullptr;
            u32                    found_core     = 0;
            u32                    min_used_count = 0xffff'ffff;
            for (u32 i = 0; i < runnable_thread_count; ++i) {

                /* Try a core number */
                const u32 other_core = vp::util::CountRightZeroBits64(core_mask);

                /* Find min thread */
                const u32 other_used_count = m_scheduler_local_ring_table[other_core].GetUsedCount();
                if (other_used_count < min_used_count) {
                    min_used_count = other_used_count;
                    found_core     = other_core;
                    ring_buffer    = std::addressof(m_scheduler_local_ring_table[other_core]);
                }

                /* Clear bit */
                core_mask = core_mask & ((~1) << other_core);
            }
            VP_ASSERT(ring_buffer != nullptr);

            /* Insert into lowest ranked thread */
            fiber->fiber_state  = FiberState_ScheduledLocal;
            fiber->current_core = found_core;
            ring_buffer->Insert(fiber);

            /* Clear fiber for loop continue */
            fiber = nullptr;
        }

        /* Dispatch fiber */
        if (fiber != nullptr) {
            this->Dispatch(fiber, core_number);
            goto _ukern_scheduler_restart;
        }

        --m_active_cores;

        /* If we are the last core, and there are no runnable fibers, then we have deadlocked */
        if (m_runnable_fibers == 0 && m_active_cores == 0) { 
            /* Kill process */
            //VP_ASSERT(false);
        } else if (m_active_cores == 0) {
            /* Alert another waiting core to check if they can run a fiber */
            ::WakeByAddressSingle(std::addressof(m_runnable_fibers));
        }

        do {
            /* Find next wakeup time */
            u64 timeout_tick = 0x7fff'ffff'ffff'ffff;
            for (FiberLocalStorage &waiting_fiber : m_wait_list) {
                if ((waiting_fiber.core_mask & (1 << core_number)) != 0  && waiting_fiber.timeout < timeout_tick)  { timeout_tick = waiting_fiber.timeout; }
            }

            /* Release scheduler lock */
            u32 wait_value = m_runnable_fibers;
            ::ReleaseSRWLockExclusive(std::addressof(m_scheduler_lock));

            /* Rest core until a new fiber is schedulable, or another rester is looking to sleep */
            u32 time_left = TimeSpan::GetTimeLeftOnTarget(timeout_tick).GetMilliSeconds();
            if (time_left == 0) { ::AcquireSRWLockExclusive(std::addressof(m_scheduler_lock)); break; }

            ::WaitOnAddress(std::addressof(m_runnable_fibers), std::addressof(wait_value), sizeof(u32), time_left);

            /* Reacquire scheduler lock */
            ::AcquireSRWLockExclusive(std::addressof(m_scheduler_lock));
        } while (m_runnable_fibers == 0);

        ++m_active_cores;

        /* Attempt to schedule a fiber */
        goto _ukern_scheduler_restart;
    }

    void UserScheduler::Dispatch(FiberLocalStorage *fiber_local, u32 core_number) {

        /* Set fiber runtime args */
        fiber_local->fiber_state  = FiberState_Running;
        fiber_local->current_core = core_number;
        VP_ASSERT(m_core_count > fiber_local->current_core);

        /* Decrement scheduler fiber count */
        --m_runnable_fibers;

        /* Switch to user fiber */
        ::SwitchToFiber(fiber_local->win32_fiber_handle);

        VP_ASSERT(core_number == fiber_local->current_core);

        /* Handle previous fiber */
        switch (fiber_local->fiber_state) {
            case FiberState_Running:
                /* Readd the fiber to the scheduler */
                this->AddToSchedulerUnsafe(fiber_local);

                break;
            case FiberState_Exiting:
                /* Delete Win32 fiber */
                ::DeleteFiber(fiber_local->win32_fiber_handle);

                /* Unregister handle */
                m_handle_table.FreeHandle(fiber_local->ukern_fiber_handle);

                /* Free fiber local */
                sUserFiberLocalAllocator.Free(fiber_local);

                break;
            case FiberState_Waiting:
                break;
            default:
                VP_ASSERT(false);
                break;
        }

        return;
    }

	void UserScheduler::Initialize(u32 core_count) {

		/* Get and set initial core count */
		m_active_cores = core_count;
		m_core_count   = core_count;
        m_core_mask    = ~((~1) << core_count);

		/* Set main thread core mask to core 0 */
		const u64 main_thread_mask = 1;
		::SetThreadAffinityMask(::GetCurrentThread(), main_thread_mask);

        /* Initialize handle table */
        m_handle_table.Initialize();

		/* Set main thread handle */
		HANDLE main_thread_handle = nullptr;
		const bool result0 = ::DuplicateHandle(::GetCurrentProcess(), ::GetCurrentThread(), ::GetCurrentProcess(), std::addressof(main_thread_handle), 0, false, DUPLICATE_SAME_ACCESS);
        VP_ASSERT(result0 == true);
		m_scheduler_thread_table[0] = main_thread_handle;

		/* Setup main thread fiber local */
		FiberLocalStorage *main_fiber_local = sUserFiberLocalAllocator.Allocate();

		main_fiber_local->priority           = cPriorityNormal;
		main_fiber_local->current_core       = 0;
		main_fiber_local->core_mask          = 1;
		main_fiber_local->fiber_state        = FiberState_Running;
		main_fiber_local->activity_level     = ActivityLevel_Schedulable;
        main_fiber_local->win32_fiber_handle = ::ConvertThreadToFiber(main_fiber_local);
        VP_ASSERT(main_fiber_local->win32_fiber_handle != nullptr);

        /* Create main thread scheduler fiber */
		m_scheduler_fiber_table[0] = ::CreateFiber(0x2000, InternalSchedulerMainThreadFiberMain, main_fiber_local);
        VP_ASSERT(m_scheduler_fiber_table[0] != nullptr);

        /* Reserve main thread */
		const bool result1 = m_handle_table.ReserveHandle(std::addressof(main_fiber_local->ukern_fiber_handle), main_fiber_local);
        VP_ASSERT(result1 == true);

		this->SetInitialFiberNameUnsafe(main_fiber_local);

		/* Allocate scheduler worker fibers */
		for (u32 i = 1; i < core_count; ++i) {
			m_scheduler_thread_table[i] = ::CreateThread(nullptr, 0x1000, InternalSchedulerFiberMain, reinterpret_cast<void*>(i), CREATE_SUSPENDED, nullptr);
			VP_ASSERT(m_scheduler_thread_table[i] != INVALID_HANDLE_VALUE);

            u64 secondary_mask = (1 << i);
			::SetThreadAffinityMask(m_scheduler_thread_table[i], secondary_mask);
			::ResumeThread(m_scheduler_thread_table[i]);
		}

		return;
	}

    FiberLocalStorage *UserScheduler::GetFiberByHandle(UKernHandle handle) {
        return reinterpret_cast<FiberLocalStorage*>(m_handle_table.GetObjectByHandle(handle));
    }

    /* Service api */
    Result UserScheduler::CreateThreadImpl(UKernHandle *out_handle, ThreadFunction thread_func, uintptr_t arg, size_t stack_size, s32 priority, u32 core_id) {

        /* Integrity checks */
        RESULT_RETURN_UNLESS(thread_func != nullptr,              ResultInvalidThreadFunctionPointer);
        RESULT_RETURN_UNLESS(stack_size  != 0,                    ResultInvalidStackSize);
        RESULT_RETURN_UNLESS(-2 <= priority && priority <= 2,     ResultInvalidPriority);
        RESULT_RETURN_UNLESS(core_id == cDefaultCoreId || ((1 << core_id) & m_core_mask) != 0, ResultInvalidCoreId);

        /* Lock the scheduler */
        ScopedSchedulerLock lock(this);

        /* Try to acquire a freed fiber slot from the free list */
        FiberLocalStorage *fiber_local = sUserFiberLocalAllocator.Allocate();
        RESULT_RETURN_IF(fiber_local == nullptr, ResultThreadStorageExhaustion);
        ::InterlockedAdd(reinterpret_cast<long int*>(std::addressof(m_allocated_user_threads)), 1);

        /* Try to reserve a ukern handle */
        const bool result = m_handle_table.ReserveHandle(std::addressof(fiber_local->ukern_fiber_handle), fiber_local);
        RESULT_RETURN_IF(result == false, ResultHandleExhaustion);

        /* Set fiber args */
        fiber_local->priority       = priority + cWindowsToUKernPriorityOffset;
        fiber_local->stack_size     = stack_size;
        fiber_local->core_mask      = (core_id == cDefaultCoreId) ? cDefaultCoreIdMask : (1 << core_id);
        fiber_local->user_arg       = reinterpret_cast<void*>(arg);
        fiber_local->user_function  = thread_func;
        fiber_local->fiber_state    = FiberState_Suspended;
        fiber_local->activity_level = ActivityLevel_Suspended;

        this->SetInitialFiberNameUnsafe(fiber_local);

        /* Create win32 fiber */
        fiber_local->win32_fiber_handle = ::CreateFiber(stack_size, UserFiberMain, fiber_local);
        VP_ASSERT(fiber_local->win32_fiber_handle != nullptr);

        *out_handle = fiber_local->ukern_fiber_handle;

        RESULT_RETURN_SUCCESS;
    }

    void UserScheduler::ExitFiberImpl() {

        /* Get current fiber */
        FiberLocalStorage *fiber_local = this->GetCurrentThreadImpl();

        /* Acquire scheduler lock */
        ::AcquireSRWLockExclusive(std::addressof(m_scheduler_lock));

        /* Set state */
        fiber_local->fiber_state = FiberState_Exiting;

        /* Swap to scheduler */
        ::SwitchToFiber(this->GetSchedulerFiber(fiber_local));
    }

    void UserScheduler::ExitThreadImpl(UKernHandle handle) {

        /* Get fiber by handle */
        FiberLocalStorage *exit_fiber = this->GetFiberByHandle(handle);

        /* Sleep loop while handle is valid */
        while (exit_fiber != nullptr) {
            this->SleepThreadImpl(0);
            exit_fiber = this->GetFiberByHandle(handle);
        }

        return;
    }

    Result UserScheduler::SetPriorityImpl(UKernHandle handle, s32 priority) {

        /* Verify input */
        RESULT_RETURN_IF(-2 <= priority && priority <= 2, ResultInvalidPriority);

        /* Get fiber by handle  */
        FiberLocalStorage *fiber_local = this->GetFiberByHandle(handle);
        RESULT_RETURN_UNLESS(fiber_local != nullptr, ResultInvalidHandle)

        /* Lock scheduler */
        ScopedSchedulerLock lock(this);

        /* Same value check */
        if (fiber_local->priority == priority) { return ResultSamePriority; }

        /* Change priority */
        fiber_local->priority = priority;

        /* Reschedule if necessary */
        this->AddToSchedulerUnsafe(fiber_local);

        RESULT_RETURN_SUCCESS;
    }

    Result UserScheduler::SetCoreMaskImpl(UKernHandle handle, UKernCoreMask core_mask) {

        /* Integrity check */
        RESULT_RETURN_IF((core_mask & (~m_core_mask)) != 0, ResultInvalidCoreMask);

        /* Get fiber by handle  */
        FiberLocalStorage *fiber_local = this->GetFiberByHandle(handle);
        RESULT_RETURN_UNLESS(fiber_local != nullptr, ResultInvalidHandle);

        /* Lock scheduler */
        ScopedSchedulerLock lock(this);

        /* Same value check */
        if (fiber_local->core_mask == core_mask) { return ResultSameCoreMask; }

        /* Change core mask */
        fiber_local->core_mask = core_mask;

        /* Reschedule if necessary */
        this->AddToSchedulerUnsafe(fiber_local);

        RESULT_RETURN_SUCCESS;
    }

    Result UserScheduler::SetActivityImpl(UKernHandle handle, u16 activity_level) {

        /* Get fiber by handle  */
        FiberLocalStorage *fiber_local = this->GetFiberByHandle(handle);

        /* Integrity check */
        RESULT_RETURN_IF(fiber_local == nullptr, ResultInvalidHandle);

        /* Lock scheduler */
        ScopedSchedulerLock lock(this);
        
        /* Same value check */
        RESULT_RETURN_IF(fiber_local->activity_level == activity_level, ResultSameActivityLevel);

        /* Set new activity level */
        fiber_local->activity_level = static_cast<ActivityLevel>(activity_level);

        this->AddToSchedulerUnsafe(fiber_local);

        RESULT_RETURN_SUCCESS;
    }

    void UserScheduler::SleepThreadImpl(u64 absolute_timeout) {

        /* Get current fiber */
        FiberLocalStorage *current_fiber = this->GetCurrentThreadImpl();

        ScopedSchedulerLock lock(this);

        /* Only set timeout if required */
        TimeWaiter time_waiter;
        if (absolute_timeout != 0) {

            /* Set timeout state */
            current_fiber->timeout         = absolute_timeout;
            current_fiber->fiber_state     = FiberState_Waiting;
            current_fiber->waitable_object = std::addressof(time_waiter);
            m_wait_list.PushBack(*current_fiber);
        }

        /* Switch to scheduler */
        ::SwitchToFiber(this->GetSchedulerFiber(current_fiber));

        return;
    }

    Result UserScheduler::ArbitrateLockImpl(UKernHandle handle, u32 *lock_address, u32 tag) {

        /* Integrity checks */
        RESULT_RETURN_UNLESS(lock_address != nullptr, ResultInvalidAddress);

        /* Get current fiber */
        FiberLocalStorage *current_fiber = this->GetCurrentThreadImpl();

        /* Lock scheduler */
        ScopedSchedulerLock lock(this);

        /* Get fiber from handle table */
        FiberLocalStorage *handle_fiber = this->GetFiberByHandle(handle);
        RESULT_RETURN_IF(handle_fiber == nullptr || handle_fiber == current_fiber, ResultInvalidHandle);

        /* Load value from address */
        const u32 address_tag = *lock_address;
        RESULT_RETURN_IF(address_tag != (handle | FiberLocalStorage::HasChildWaitersBit), ResultInvalidLockAddressValue);

        /* Set lock state */
        LockArbiter lock_arbiter = {};
        current_fiber->waitable_object = std::addressof(lock_arbiter);
        current_fiber->lock_address    = lock_address;
        current_fiber->wait_tag        = tag;
        current_fiber->fiber_state     = FiberState_Waiting;
        current_fiber->timeout         = 0x7fff'ffff'ffff'ffff;

        /* Push back thread waiter */
        handle_fiber->wait_list.PushBack(*current_fiber);

        /* Swap to scheduler */
        ::SwitchToFiber(this->GetSchedulerFiber(current_fiber));

        return current_fiber->last_result;
    }

    Result UserScheduler::ArbitrateUnlockImpl(u32 *lock_address) {

        /* Integrity checks */
        RESULT_RETURN_UNLESS(lock_address != nullptr, ResultInvalidAddress);

        /* Get current fiber */
        FiberLocalStorage *current_fiber = this->GetCurrentThreadImpl();

        /* Lock scheduler */
        ScopedSchedulerLock lock(this);

        /* Integrity checks */
        RESULT_RETURN_UNLESS(current_fiber->wait_list.IsEmpty() == false,                                                  ResultRequiresLock);
        RESULT_RETURN_UNLESS((current_fiber->ukern_fiber_handle | FiberLocalStorage::HasChildWaitersBit) == *lock_address, ResultInvalidLockAddressValue);

        /* Release lock */
        current_fiber->ReleaseLockWaitListUnsafe();

        RESULT_RETURN_SUCCESS;
    }

    Result UserScheduler::WaitKeyImpl(u32 *lock_address, u32 *cv_key, u32 tag, s64 absolute_timeout) {

        /* Integrity checks */
        RESULT_RETURN_UNLESS(lock_address != nullptr, ResultInvalidAddress);
        RESULT_RETURN_UNLESS(cv_key != nullptr,  ResultInvalidAddress);

        /* Get current fiber */
        FiberLocalStorage *current_fiber = this->GetCurrentThreadImpl();

        /* Lock scheduler */
        ScopedSchedulerLock lock(this);

        /* Integrity checks */
        RESULT_RETURN_UNLESS(current_fiber->ukern_fiber_handle == ((*lock_address) & (~FiberLocalStorage::HasChildWaitersBit)), ResultInvalidLockAddressValue);

        /* Release lock */
        if (current_fiber->wait_list.IsEmpty() == false) {
            current_fiber->ReleaseLockWaitListUnsafe();
        } else {
            *lock_address = 0;
        }

        /* Set cv key to 1 */
        *cv_key = 1;

        /* Check if timed out */
        KeyArbiter key_arbiter = {};
        u32 result = ResultSuccess;
        if (0 == absolute_timeout) {
            result = ResultTimeout;
        } else {
            /* Set wait state */
            current_fiber->waitable_object = std::addressof(key_arbiter);
            current_fiber->wait_address    = cv_key;
            current_fiber->lock_address    = lock_address;
            current_fiber->wait_tag        = tag;
            current_fiber->fiber_state     = FiberState_Waiting;
            current_fiber->timeout         = absolute_timeout;
            
            /* Find a parent cv waiter */
            for (FiberLocalStorage &waiting_fiber : m_wait_list) {
                if (waiting_fiber.wait_address == cv_key) {
                    waiting_fiber.wait_list.PushBack(*current_fiber);
                    break;
                }
            }

            /* If no parent, become the parent */
            if (current_fiber->wait_list_node.IsLinked() == false) {
                m_wait_list.PushBack(*current_fiber);
            }
        }

        /* Swap to scheduler */
        ::SwitchToFiber(this->GetSchedulerFiber(current_fiber));

        if (result == ResultSuccess) {
            result = current_fiber->last_result;
        }

        return result;
    }

    bool UserScheduler::TransferFiberForSignalKey(FiberLocalStorage *waiting_fiber) {

        /* Check lock is set */
        const u32 prev_tag = *waiting_fiber->lock_address;

        if (prev_tag != 0) {

            *waiting_fiber->lock_address |= FiberLocalStorage::HasChildWaitersBit;

            /* Get fiber by handle */
            FiberLocalStorage *lock_fiber = this->GetFiberByHandle(prev_tag & (~FiberLocalStorage::HasChildWaitersBit));

            /* Push back fiber waiter */
            lock_fiber->wait_list.PushBack(*waiting_fiber);

            return false;
        }

        /* Take the lock back */
        u32 tag                      = waiting_fiber->wait_tag;
        *waiting_fiber->lock_address = tag;

        return true;
    }

    Result UserScheduler::SignalKeyImpl(u32 *cv_key, u32 signal_count) {

        /* Integrity checks */
        RESULT_RETURN_IF(cv_key == nullptr, ResultInvalidAddress);

        /* Lock scheduler */
        ScopedSchedulerLock lock(this);

        /* Clear cv key if signal count of 0 */
        if (signal_count == 0) {

            *cv_key = 0;

            RESULT_RETURN_SUCCESS;
        }

        /* Find parent waiter */
        FiberLocalStorage *cv_fiber = nullptr;
        for (FiberLocalStorage &fiber_local : m_wait_list) {
            if (fiber_local.wait_address != cv_key) { continue; }
            cv_fiber = std::addressof(fiber_local);
            break;
        }

        /* Check we found a waiter */
        RESULT_RETURN_IF(cv_fiber == nullptr, ResultInvalidAddress);

        /* Reacquire the lock or join the cs wait list */
        cv_fiber->wait_list_node.Unlink();
        if (this->TransferFiberForSignalKey(cv_fiber) == true) {
            cv_fiber->waitable_object->EndWait(cv_fiber, ResultSuccess);
            cv_fiber->waitable_object = nullptr;
        }

        /* Transfer cv waiters */
        u32 i = 1;
        for (FiberLocalStorage &waiting_fiber : cv_fiber->wait_list) {

            /* Break if signal count is reached */
            if (signal_count <= i) { break; }

            /* Detach from child list */
            waiting_fiber.wait_list_node.Unlink();

            /* Handle reacquisition of lock */
            this->TransferFiberForSignalKey(std::addressof(waiting_fiber));

            /* Loop count */
            ++i;
        }

        /* Set cv key to 0 */
        *cv_key = 0;

        RESULT_RETURN_SUCCESS;
    }

    Result UserScheduler::WaitForAddressIfEqualImpl(u32 *wait_address, u32 value, s64 absolute_timeout) {

        /* Integrity checks */
        RESULT_RETURN_IF(wait_address == nullptr, ResultInvalidAddress);

        /* Get current fiber */
        FiberLocalStorage *current_fiber = this->GetCurrentThreadImpl();

        /* Lock scheduler */
        ScopedSchedulerLock lock(this);

        /* Check address */
        u32 result = ResultSuccess;
        if (*wait_address != value) {
            result = ResultInvalidWaitAddressValue;
        } else if (absolute_timeout <= 0) {
            result = ResultTimeout;
        } else {

            /* Set wait address state */
            WaitAddressArbiter wait_address_arbiter = {};
            current_fiber->waitable_object = std::addressof(wait_address_arbiter);
            current_fiber->wait_address    = wait_address;
            current_fiber->fiber_state     = FiberState_Waiting;
            current_fiber->timeout         = absolute_timeout;

            /* Try to find if the address is already in the wait list */
            FiberLocalStorage *address_fiber = nullptr;
            for (FiberLocalStorage &waiting_fiber : m_wait_list) {
                if (wait_address == waiting_fiber.wait_address) {
                    address_fiber = std::addressof(waiting_fiber);
                    break;
                }
            }

            /* Push back to a wait list */
            if (address_fiber == nullptr) {
                m_wait_list.PushBack(*current_fiber);
            } else {
                address_fiber->wait_list.PushBack(*current_fiber);
            }
        }

        ::SwitchToFiber(this->GetSchedulerFiber(current_fiber));

        if (result == ResultSuccess) {
            result = current_fiber->last_result;
        }

        return result;
    }

    Result UserScheduler::WaitForAddressIfLessThanImpl(u32 *wait_address, u32 value, s64 absolute_timeout, bool do_decrement) {

        /* Integrity checks */
        RESULT_RETURN_IF(wait_address == nullptr, ResultInvalidAddress);

        /* Get current fiber */
        FiberLocalStorage *current_fiber = this->GetCurrentThreadImpl();

        /* Lock scheduler */
        ScopedSchedulerLock lock(this);

        /* Perform decrement */
        u32 wait_value = *wait_address;
        if (do_decrement == true) {
            ::InterlockedDecrement(wait_address);
        }

        /* Check address */
        u32 result = ResultSuccess;
        if (wait_value >= value) {
            result = ResultInvalidWaitAddressValue;
        } else if (absolute_timeout <= 0) {
            result = ResultTimeout;
        } else {

            /* Set wait address state */
            WaitAddressArbiter wait_address_arbiter = {};
            current_fiber->waitable_object = std::addressof(wait_address_arbiter);
            current_fiber->wait_address    = wait_address;
            current_fiber->fiber_state     = FiberState_Waiting;
            current_fiber->timeout         = absolute_timeout;

            /* Find address in wait list */
            FiberLocalStorage *address_fiber = nullptr;
            for (FiberLocalStorage &waiting_fiber : m_wait_list) {
                if (wait_address == waiting_fiber.wait_address) {
                    address_fiber = std::addressof(waiting_fiber);
                    break;
                }
            }

            /* Push back to a wait list */
            if (address_fiber == nullptr) {
                m_wait_list.PushBack(*current_fiber);
            } else {
                address_fiber->wait_list.PushBack(*current_fiber);
            }
        }

        ::SwitchToFiber(this->GetSchedulerFiber(current_fiber));

        if (result == ResultSuccess) {
            result = current_fiber->last_result;
        }

        return result;
    }

    Result UserScheduler::WakeByAddressImpl(u32 *wait_address, u32 count) {

        /* Integrity checks */
        RESULT_RETURN_IF(wait_address == nullptr, ResultInvalidAddress);

        /* Lock scheduler */
        ScopedSchedulerLock lock(this);

        if (count == 0) { RESULT_RETURN_SUCCESS; }

        /* Find address in wait list */
        FiberLocalStorage *address_fiber = nullptr;
        for (FiberLocalStorage &waiting_fiber : m_wait_list) {
            if (wait_address == waiting_fiber.wait_address) {
                address_fiber = std::addressof(waiting_fiber);
                break;
            }
        }

        /* Integrity checks */
        RESULT_RETURN_IF(address_fiber == nullptr, ResultNoWaiters);

        /* Release parent waiter */
        address_fiber->waitable_object->EndWait(address_fiber, ResultSuccess);

        /* Release the waiting fibers */
        u32 i = 1;
        FiberLocalStorage::WaitList::iterator wait_list_iter = address_fiber->wait_list.begin();
        while (wait_list_iter != address_fiber->wait_list.end()) {

            /* Pre-iterate for list removal */
            FiberLocalStorage &waiting_fiber = *wait_list_iter;
            ++wait_list_iter;

            /* Terminate loop if specified waiters are waked */
            if (count < i) { break; }

            /* End a waiter's wait */
            waiting_fiber.wait_list_node.Unlink();
            waiting_fiber.waitable_object->EndWait(std::addressof(waiting_fiber), ResultSuccess);

            ++i;
        }

        /* Transfer wait list */
        if (address_fiber->wait_list.IsEmpty() == false) {

            FiberLocalStorage *next_address_parent = std::addressof(address_fiber->wait_list.PopFront());

            FiberLocalStorage::WaitList::iterator wait_list_iter = address_fiber->wait_list.begin();
            while (wait_list_iter != address_fiber->wait_list.end()) {

                /* Pre-iterate for list removal */
                FiberLocalStorage &waiting_fiber = *wait_list_iter;
                ++wait_list_iter;

                /* Detach from previous list */
                waiting_fiber.wait_list_node.Unlink();

                /* Add to new parent */
                next_address_parent->wait_list.PushBack(waiting_fiber);
            }

            m_wait_list.PushBack(*next_address_parent);
        }

        RESULT_RETURN_SUCCESS;
    }

    Result UserScheduler::WakeByAddressIncrementEqualImpl(u32 *wait_address, u32 value, u32 count) {

        /* Integrity checks */
        RESULT_RETURN_IF(wait_address == nullptr, ResultInvalidAddress);

        /* Lock scheduler */
        ScopedSchedulerLock lock(this);
        
        /* Check address value */
        RESULT_RETURN_IF(*wait_address != value, ResultValueOutOfRange);

        /* Set new value */
        ::InterlockedExchange(wait_address, value + 1);

        /* Find address in wait list */
        FiberLocalStorage *address_fiber = nullptr;
        for (FiberLocalStorage &waiting_fiber : m_wait_list) {
            if (wait_address == waiting_fiber.wait_address) {
                address_fiber = std::addressof(waiting_fiber);
                break;
            }
        }

        /* Check address value */
        RESULT_RETURN_IF(*wait_address != value, ResultNoWaiters);

        /* Release parent waiter */
        address_fiber->waitable_object->EndWait(address_fiber, ResultSuccess);

        /* Release the waiting fibers */
        u32 i = 1;
        for (FiberLocalStorage &waiting_fiber : address_fiber->wait_list) {

            if (count < i) { break; }

            waiting_fiber.wait_list_node.Unlink();
            waiting_fiber.waitable_object->EndWait(std::addressof(waiting_fiber), ResultSuccess);

            ++i;
        }

        /* Transfer wait list */
        if (address_fiber->wait_list.IsEmpty() == false) {

            FiberLocalStorage *next_address_parent = std::addressof(address_fiber->wait_list.PopFront());

            FiberLocalStorage::WaitList::iterator wait_list_iter = address_fiber->wait_list.begin();
            while (wait_list_iter != address_fiber->wait_list.end()) {

                /* Pre-iterate for list removal */
                FiberLocalStorage &waiting_fiber = *wait_list_iter;
                ++wait_list_iter;

                /* Detach from previous list */
                waiting_fiber.wait_list_node.Unlink();

                /* Add to new parent */
                next_address_parent->wait_list.PushBack(waiting_fiber);
            }

            m_wait_list.PushBack(*next_address_parent);
        }

        RESULT_RETURN_SUCCESS;
    }

    Result UserScheduler::WakeByAddressModifyLessThanImpl(u32 *wait_address, u32 value, u32 count) {

        /* Integrity checks */
        RESULT_RETURN_IF(wait_address == nullptr, ResultInvalidAddress);

        /* Lock scheduler */
        ScopedSchedulerLock lock(this);

        /* Check address value */
        if (*wait_address != value) {
            return ResultValueOutOfRange;
        }

        /* Find address in wait list */
        FiberLocalStorage *address_fiber = nullptr;
        for (FiberLocalStorage &waiting_fiber : m_wait_list) {
            if (wait_address == waiting_fiber.wait_address) {
                address_fiber = std::addressof(waiting_fiber);
                break;
            }
        }

        /* Determine value to signal */
        u32 signal = -1;

        if (address_fiber == nullptr) { 
            signal = 1; 
        } else if (0 < count) {
            /* Count waiters */
            signal = 0;
            u32 i  = address_fiber->wait_list.GetCount();

            /* Signal -1 if less waiters to wakeup then count */
            if (i < count) { signal = -1; }
        }

        /* Only modify if non-zero signal */
        if (signal != 0) {
            ::InterlockedExchange(wait_address, value + signal);
        }

        /* Release parent waiter */
        address_fiber->waitable_object->EndWait(address_fiber, ResultSuccess);

        /* Release the waiting fibers */
        u32 i = 1;
        FiberLocalStorage::WaitList::iterator wait_list_iter = address_fiber->wait_list.begin();
        while (wait_list_iter != address_fiber->wait_list.end()) {

            /* Pre-iterate for list removal */
            FiberLocalStorage &waiting_fiber = *wait_list_iter;
            ++wait_list_iter;

            if (count < i) { break; }

            waiting_fiber.wait_list_node.Unlink();
            waiting_fiber.waitable_object->EndWait(std::addressof(waiting_fiber), ResultSuccess);

            ++i;
        }

        /* Transfer wait list */
        if (address_fiber->wait_list.IsEmpty() == false) {

            FiberLocalStorage *next_address_parent = std::addressof(address_fiber->wait_list.PopFront());

            FiberLocalStorage::WaitList::iterator wait_list_iter = address_fiber->wait_list.begin();
            while (wait_list_iter != address_fiber->wait_list.end()) {

                /* Pre-iterate for list removal */
                FiberLocalStorage &waiting_fiber = *wait_list_iter;
                ++wait_list_iter;

                /* Detach from previous list */
                waiting_fiber.wait_list_node.Unlink();

                /* Add to new parent */
                next_address_parent->wait_list.PushBack(waiting_fiber);
            }

            m_wait_list.PushBack(*next_address_parent);
        }

        RESULT_RETURN_SUCCESS;
    }

}
