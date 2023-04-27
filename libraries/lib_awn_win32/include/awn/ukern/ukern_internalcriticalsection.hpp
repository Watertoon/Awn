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

            void Enter() {

                const ThreadType *current_thread = ukern::GetCurrentThread();
                const UKernHandle tag            = current_thread->ukern_fiber_handle;

                /* Acquire loop */
                for (;;) {
                    /* Try to acquire the critical section */
                    const UKernHandle other_waiter = ::InterlockedCompareExchange(std::addressof(m_handle), tag, 0);
                    if (other_waiter == 0) { return; }

                    /* Set tag bit */
                    if (((other_waiter >> 0x1e) & 1) == 0) {
                        const UKernHandle prev_value = ::InterlockedOr(reinterpret_cast<volatile long int*>(std::addressof(m_handle)), FiberLocalStorage::HasChildWaitersBit);
                        if (prev_value != other_waiter) { continue; }
                    }

                    /* If we fail, lock the thread */
                    RESULT_ABORT_UNLESS(impl::GetScheduler()->ArbitrateLockImpl(other_waiter & (~FiberLocalStorage::HasChildWaitersBit), std::addressof(m_handle), tag), ResultSuccess);
                    if ((m_handle & (~FiberLocalStorage::HasChildWaitersBit)) == tag) {
                        return;
                    }
                }
            }

            bool TryEnter() {
                const ThreadType *current_thread = ukern::GetCurrentThread();
                const UKernHandle tag            = current_thread->ukern_fiber_handle;
                const UKernHandle other_waiter   = ::InterlockedCompareExchange(std::addressof(m_handle), tag, 0);
                return other_waiter == 0;
            }

            void Leave() {

                /* Unlock waiters */
                if (((m_handle >> 0x1e) & 1) == 1) { RESULT_ABORT_UNLESS(impl::GetScheduler()->ArbitrateUnlockImpl(std::addressof(m_handle)), ResultSuccess); }
                else { ::InterlockedExchange(std::addressof(m_handle), 0); }
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
