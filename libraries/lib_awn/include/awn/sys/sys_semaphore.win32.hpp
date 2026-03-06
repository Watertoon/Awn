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

    class Semaphore {
        private:
            SRWLOCK m_cs;
            CONDITION_VARIABLE m_cv;
            u32 m_counter;
        public:
            constexpr Semaphore() : m_cs(SRWLOCK_INIT), m_cv(CONDITION_VARIABLE_INIT), m_counter() {/*...*/}
            constexpr Semaphore(u32 counter) : m_cs(), m_cv(), m_counter(counter) {/*...*/}
            constexpr ~Semaphore() {/*...*/}

            constexpr void SetCounterUnsafe(u32 counter) {
                m_counter = counter;
            }
            constexpr u32 GetCounter() const {
                return m_counter;
            }

            void Acquire() {
                ::AcquireSRWLockExclusive(std::addressof(m_cs));
                while (m_counter == 0) {
                    ::SleepConditionVariableSRW(std::addressof(m_cv), std::addressof(m_cs), INFINITE, 0);
                }
                m_counter -= 1;
                ::ReleaseSRWLockExclusive(std::addressof(m_cs));
            }

            bool TryAcquire() {
                ::AcquireSRWLockExclusive(std::addressof(m_cs));
                const bool is_acq = (0 < m_counter);
                if (is_acq) {
                    m_counter -= 1;
                }
                ::ReleaseSRWLockExclusive(std::addressof(m_cs));
                return is_acq;
            }

            void Release() {
                ::AcquireSRWLockExclusive(std::addressof(m_cs));
                m_counter += 1;
                ::WakeConditionVariable(std::addressof(m_cv));
                ::ReleaseSRWLockExclusive(std::addressof(m_cs));
            }
            void Release(u32 count) {
                ::AcquireSRWLockExclusive(std::addressof(m_cs));
                m_counter += count;
                ::WakeAllConditionVariable(std::addressof(m_cv));
                ::ReleaseSRWLockExclusive(std::addressof(m_cs));
            }
    };
}
