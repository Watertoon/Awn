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
            impl::InternalConditionVariable m_cv;
        public:
            constexpr  ConditionVariable() : m_cv() {/*...*/}
            constexpr ~ConditionVariable() {/*...*/}

            void Wait(Mutex *mutex) {
                const u32 last_lock_count = mutex->m_lock_count;
                mutex->m_lock_count = 0;
                m_cv.Wait(mutex->GetInternalCriticalSection());
                mutex->m_lock_count = last_lock_count;
            }

            void Signal() {
                m_cv.Signal();
            }

            void Broadcast() {
                m_cv.Broadcast();
            }
    };
}
