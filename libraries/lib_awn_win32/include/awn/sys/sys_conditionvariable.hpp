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
            ukern::InternalConditionVariable m_cond_var;
        public:
            constexpr  ConditionVariable() : m_cond_var() {/*...*/}
            constexpr ~ConditionVariable() {/*...*/}

            void Wait(Mutex *mutex) {
                mutex->m_lock_count = mutex->m_lock_count - 1;
                m_cond_var.Wait(std::addressof(mutex->m_internal_cs));
                mutex->m_lock_count = mutex->m_lock_count + 1;
            }
            void TimedWait(Mutex *mutex, TimeSpan timeout) {
                mutex->m_lock_count = mutex->m_lock_count - 1;
                m_cond_var.TimedWait(std::addressof(mutex->m_internal_cs), timeout);
                mutex->m_lock_count = mutex->m_lock_count + 1;
            }
            void Signal() {
                m_cond_var.Signal();
            }
            void Broadcast() {
                m_cond_var.Broadcast();
            }
    };
}
