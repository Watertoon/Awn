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
    
    class InternalConditionVariable {
        private:
            u32 m_cv;
        public:
            constexpr  InternalConditionVariable() : m_cv(0) {/*...*/}
            constexpr ~InternalConditionVariable() {/*...*/}

            void Wait(InternalCriticalSection *cs) {

                const FiberLocalStorage *fiber_local = ukern::GetCurrentThread();
                const UKernHandle tag = fiber_local->ukern_fiber_handle;

                VP_ASSERT(tag == (cs->m_handle & (~0x4000'0000)));

                RESULT_ABORT_UNLESS(impl::GetScheduler()->WaitKeyImpl(std::addressof(cs->m_handle), std::addressof(m_cv), tag, -1));

                return;
            }
            
            void TimedWait(InternalCriticalSection *cs, s64 timeout_ns) {
                
                const FiberLocalStorage *fiber_local = ukern::GetCurrentThread();
                const UKernHandle tag = fiber_local->ukern_fiber_handle;
                
                VP_ASSERT(tag == (cs->m_handle & (~0x4000'0000)));
                
                impl::GetScheduler()->WaitKeyImpl(std::addressof(cs->m_handle), std::addressof(m_cv), tag, impl::GetAbsoluteTimeToWakeup(timeout_ns));
            }
            
            void Signal(u32 count = 1) {
                if (m_cv != 0) {
                    impl::GetScheduler()->SignalKeyImpl(std::addressof(m_cv), count);
                }
            }
            void Broadcast() {
                if (m_cv != 0) {
                    impl::GetScheduler()->SignalKeyImpl(std::addressof(m_cv), -1);
                }
            }
    };
}
