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

    class ConditionVariable {
        private:
            CONDITION_VARIABLE m_cv;
            u64 m_counter;
        public:
            constexpr  ConditionVariable() : m_cv(CONDITION_VARIABLE_INIT), m_counter() {/*...*/}
            constexpr ~ConditionVariable() {/*...*/}

            void Wait(Mutex *mutex) {
                const u32 last_lock_count = mutex->m_lock_count;
                mutex->m_lock_count = 0;
                const u32 tag = mutex->m_tag;
                mutex->m_tag = 0;
                const u64 count = m_counter;
                for (;;) {
                    const bool result = ::SleepConditionVariableSRW(std::addressof(m_cv), std::addressof(mutex->m_srwlock), INFINITE, 0);
                    VP_ASSERT(result);

                    u64 prev;
                    const u64 clear = count | 0x8000'0000'0000'0000;
                    const bool is_reset = vp::util::InterlockedCompareExchangeAcquire(std::addressof(prev), std::addressof(m_counter), count, clear);
                    if (is_reset) { break; }
                    if (count < prev) { break; }
                }
                mutex->m_tag = tag;
                mutex->m_lock_count = last_lock_count;
                return;
            }

            void Signal() {
                vp::util::InterlockedOrRelease(std::addressof(m_counter), 0x8000'0000'0000'0000);
                ::WakeConditionVariable(std::addressof(m_cv));
            }

            void Broadcast() {
                u64 prev = m_counter;
                for (;;) {
                    const u64 next = (prev & 0x7fff'ffff'ffff'ffff) + 1;
                    const bool result = vp::util::InterlockedCompareExchangeRelease(std::addressof(prev), std::addressof(m_counter), next, prev);
                    if (result) { break; }
                }
                ::WakeAllConditionVariable(std::addressof(m_cv));
            }
    };
}
