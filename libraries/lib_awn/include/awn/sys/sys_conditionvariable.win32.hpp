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
            CONDITION_VARIABLE m_cond_var;
        public:
            constexpr  ConditionVariable() : m_cond_var(CONDITION_VARIABLE_INIT) {/*...*/}
            constexpr ~ConditionVariable() {/*...*/}

            void Wait(Mutex *mutex) {
                const u32 last_lock_count = mutex->m_lock_count;
                sys::ThreadBase *last_owner = mutex->m_owner;
                mutex->m_lock_count = 0;
                mutex->m_owner = nullptr;
                ::SleepConditionVariableSRW(std::addressof(m_cond_var), mutex->GetSRWLock(), INFINITE, 0);
                mutex->m_lock_count = last_lock_count;
                mutex->m_owner = last_owner;
            }
            void TimedWait(Mutex *mutex, TimeSpan timeout) {
                const u32 last_lock_count = mutex->m_lock_count;
                sys::ThreadBase *last_owner = mutex->m_owner;
                mutex->m_lock_count = 0;
                mutex->m_owner = nullptr;
                ::SleepConditionVariableSRW(std::addressof(m_cond_var), mutex->GetSRWLock(), timeout.GetMilliSeconds(), 0);
                mutex->m_lock_count = last_lock_count;
                mutex->m_owner = last_owner;
            }
            void Signal() {
                ::WakeConditionVariable(std::addressof(m_cond_var));
            }
            void Broadcast() {
                ::WakeAllConditionVariable(std::addressof(m_cond_var));
            }
    };
}
