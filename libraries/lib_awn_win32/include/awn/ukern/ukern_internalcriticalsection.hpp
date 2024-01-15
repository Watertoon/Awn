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
    
    class InternalCriticalSection {
        private:
            friend class InternalConditionVariable;
        private:
            UKernHandle m_handle;
        public:
            constexpr ALWAYS_INLINE InternalCriticalSection() : m_handle(0) {/*...*/}
            constexpr ~InternalCriticalSection() {/*...*/}

            void Enter() {

                /* Get current thread handle */
                const ThreadType *current_thread = ukern::GetCurrentThread();
                const UKernHandle tag            = current_thread->ukern_fiber_handle;

                /* Acquire loop */
                for (;;) {
                    /* Try to acquire the tag for the current thread */
                    u32 handle = 0;
                    const bool result = vp::util::InterlockedCompareExchange(std::addressof(handle), std::addressof(m_handle), static_cast<u32>(tag), 0u);
                    if (result == true) { return; }

                    /* Set arbitration bit */
                    if (((handle >> 0x1e) & 1) == 0) {
                        const u32 last_handle = vp::util::InterlockedFetchOr(std::addressof(m_handle), FiberLocalStorage::HasChildWaitersBit);
                        if (handle != last_handle) { continue; }
                    }

                    /* If we fail, lock the thread */
                    RESULT_ABORT_UNLESS(impl::GetScheduler()->ArbitrateLockImpl(handle & (~FiberLocalStorage::HasChildWaitersBit), std::addressof(m_handle), tag));

                    /* Ensure non-spurious wakeup */
                    handle = vp::util::InterlockedLoad(std::addressof(m_handle));
                    if ((handle & (~FiberLocalStorage::HasChildWaitersBit)) == tag) { return; }
                }
            }

            bool TryEnter() {

                /* Get current thread handle */
                const ThreadType *current_thread = ukern::GetCurrentThread();
                const UKernHandle tag            = current_thread->ukern_fiber_handle;

                /* Try to acquire the tag for the current thread */
                u32 handle = 0;
                const bool result = vp::util::InterlockedCompareExchange(std::addressof(handle), std::addressof(m_handle), static_cast<u32>(tag), 0u);

                return result == true;
            }

            void Leave() {

                /* Get current thread handle */
                const ThreadType *current_thread = ukern::GetCurrentThread();
                const UKernHandle tag            = current_thread->ukern_fiber_handle;

                /* Release and Unlock waiters if arbitration bit is set */
                u32 handle = 0;
                const bool result = vp::util::InterlockedCompareExchangeRelease(std::addressof(handle), std::addressof(m_handle), 0u, tag);
                if (result == false) { RESULT_ABORT_UNLESS(impl::GetScheduler()->ArbitrateUnlockImpl(std::addressof(m_handle))); }

                return;
            }

            void lock() {
                this->Enter();
            }
            void unlock() {
                this->Leave();
            }
            bool try_lock() {
                return this->TryEnter();
            }

            bool IsLockedByCurrentThread() {
                return (m_handle & (~FiberLocalStorage::HasChildWaitersBit)) == ukern::GetCurrentThread()->ukern_fiber_handle;
            }
    };
    static_assert(sizeof(InternalCriticalSection) == sizeof(UKernHandle));
}
