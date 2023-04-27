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

namespace vp::util {

    class ConditionVariable {
        private:
            CONDITION_VARIABLE m_condition_variable;
        public:
            constexpr ConditionVariable() : m_condition_variable{0} {/*...*/}

            void Wait(CriticalSection *cs) {
                const size_t thread_id = cs->UnsetId();
                ::SleepConditionVariableSRW(std::addressof(m_condition_variable), cs->GetSRWLOCK(), INFINITE, 0);
                cs->SetId(thread_id);
            }

            void TimedWait(CriticalSection *cs, u32 timeout_ms) {
                const size_t thread_id = cs->UnsetId();
                ::SleepConditionVariableSRW(std::addressof(m_condition_variable), cs->GetSRWLOCK(), timeout_ms, 0);
                cs->SetId(thread_id);
            }

            void Signal() {
                ::WakeConditionVariable(std::addressof(m_condition_variable));
            }

            void Broadcast() {
                ::WakeAllConditionVariable(std::addressof(m_condition_variable));
            }
    };
}
