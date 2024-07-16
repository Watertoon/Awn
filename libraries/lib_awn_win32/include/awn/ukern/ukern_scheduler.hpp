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

namespace awn::ukern::impl {

    static constexpr const u32 cPriorityNormal    = THREAD_PRIORITY_NORMAL;
    static constexpr const u32 cDefaultCoreId     = static_cast<u32>(-2);
    static constexpr const u32 cDefaultCoreIdMask = 1;

    class UserScheduler {
        public:
            friend class ScopedSchedulerLock;
            friend class WaitableObject;
            friend class LockArbiter;
            friend class KeyArbiter;
            friend class WaitAddressArbiter;
        private:
            using CoreConnection        = vp::util::FixedConnection<u64, cMaxCoreCount>;
            using ThreadQueue           = vp::util::FixedPriorityQueue<FiberLocalStorage, &FiberLocalStorage::priority, cMaxThreadCount>;
            using LocalThreadRingBuffer = vp::util::FixedRingBuffer<FiberLocalStorage*, cMaxThreadCount>;
            using WaitList              = vp::util::IntrusiveListTraits<FiberLocalStorage, &FiberLocalStorage::wait_list_node>::List;
        protected:
            SRWLOCK                   m_scheduler_lock;
            HANDLE                    m_scheduler_thread_table[cMaxCoreCount];
            void                     *m_scheduler_fiber_table[cMaxCoreCount];
            LocalThreadRingBuffer     m_scheduler_local_ring_table[cMaxCoreCount];
            ThreadQueue               m_thread_queue;
            WaitList                  m_wait_list;
            CoreConnection            m_core_connection;
            u32                       m_allocated_user_threads;
            u32                       m_default_core;
            u32                       m_core_count;
            u32                       m_active_cores;
            u32                       m_runnable_fibers;
            HandleTable               m_handle_table;
        private:
            static long unsigned int InternalSchedulerFiberMain(void *arg);
            static void InternalSchedulerMainThreadFiberMain(void *arg);
            static void UserFiberMain(void *arg);
            void SchedulerFiberMain(size_t core_number);

            void ExitFiberImpl();
        private:
            void RemoveFromSchedulerUnsafe(FiberLocalStorage *fiber_local);
            void AddToSchedulerUnsafe(FiberLocalStorage *fiber_local);

            void Dispatch(FiberLocalStorage *fiber_local, u32 core_number);
        public:
            constexpr ALWAYS_INLINE UserScheduler()  : m_scheduler_lock(0) , m_scheduler_thread_table{nullptr}, m_scheduler_fiber_table{nullptr}, m_scheduler_local_ring_table{}, m_thread_queue(), m_wait_list(), m_core_connection(), m_allocated_user_threads(), m_default_core(), m_core_count(), m_active_cores(), m_runnable_fibers(), m_handle_table() {/*...*/}
            constexpr ~UserScheduler() {/*...*/}

            void Initialize(CoreMask core_mask);

            u32 GetCoreCount() const { return m_core_count; }
        private:
            bool TransferFiberForSignalKey(FiberLocalStorage *waiting_fiber);
        public:
            Result CreateThreadImpl(ukern::Handle *out_handle, ThreadFunction thread_func, uintptr_t arg, size_t stack_size, s32 priority, u32 core_id);

            Result StartThread(ukern::Handle handle);
            void   ExitThreadImpl(ukern::Handle handle);

            Result SetPriorityImpl(ukern::Handle handle, s32 priority);
            Result SetCoreMaskImpl(ukern::Handle handle, u64 new_mask);
            Result SetActivityImpl(ukern::Handle handle, u16 activity_level);

            void   SleepThreadImpl(u64 absolute_timeout);

            Result ArbitrateLockImpl(ukern::Handle handle, u32 *address, u32 tag);
            Result ArbitrateUnlockImpl(u32 *address);

            Result WaitKeyImpl(u32 *address, u32 *cv_key, u32 tag, s64 absolute_timeout);
            Result SignalKeyImpl(u32 *cv_key, u32 count);

            Result WaitForAddressIfEqualImpl(u32 *address, u32 value, s64 absolute_timeout);
            Result WaitForAddressIfLessThanImpl(u32 *address, u32 value, s64 absolute_timeout, bool do_decrement);
            Result WakeByAddressImpl(u32 *address, u32 count);
            Result WakeByAddressIncrementEqualImpl(u32 *address, u32 value, u32 count);
            Result WakeByAddressModifyLessThanImpl(u32 *address, u32 value, u32 count);

            ALWAYS_INLINE FiberLocalStorage *GetCurrentThreadImpl() {
                return reinterpret_cast<FiberLocalStorage*>(::GetFiberData());
            }

            static void SetInitialFiberNameUnsafe(FiberLocalStorage *fiber_local) {
                /* Special case for main thread */
                if (fiber_local->user_function == nullptr) { ::strncpy(fiber_local->fiber_name_storage, "MainThread", cMaxFiberNameLength); return; }

                /* Otherwise use fiber's initial function address */
                ::snprintf(fiber_local->fiber_name_storage, cMaxFiberNameLength, "Thread0x%08llx", reinterpret_cast<size_t>(fiber_local->user_function));
            }

            constexpr ALWAYS_INLINE void *GetSchedulerFiber(FiberLocalStorage *fiber_local) {
                VP_ASSERT(m_core_count > fiber_local->current_core);
                return m_scheduler_fiber_table[fiber_local->current_core];
            }

            FiberLocalStorage *GetFiberByHandle(ukern::Handle handle);
        public:
            void SuspendAllOtherCoresImpl() {

                /* Get current core number */
                FiberLocalStorage *current_fiber = this->GetCurrentThreadImpl();
                const u32 current_core = current_fiber->current_core;

                /* Suspend all cores except current */
                for (u32 i = 0; i < m_core_count; ++i) {
                    if (current_core != i) {
                        ::SuspendThread(m_scheduler_thread_table[i]);
                    }
                }
            }

            void OutputBackTraceImpl([[maybe_unused]] HANDLE file) {

                /* Print backtrace for this fiber */
                //FiberLocalStorage *current_fiber = this->GetCurrentThreadImpl();

                /* Iterate through all other fibers for backtrace */
                ::puts("Backtrace was called. But it's not yet implemented.");
                ::fflush(stdout);
            }
    };

    class ScopedSchedulerLock {
        private:
            SRWLOCK *m_lock;
        public:
            explicit ALWAYS_INLINE ScopedSchedulerLock(UserScheduler *scheduler) : m_lock(std::addressof(scheduler->m_scheduler_lock)) {
                ::AcquireSRWLockExclusive(m_lock);
            }

            ALWAYS_INLINE ~ScopedSchedulerLock() {
                ::ReleaseSRWLockExclusive(m_lock);
            }
    };
}
