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

    enum class ResetMode : bool {
        Manual = 0,
        Auto   = 1,
    };
    enum class SignalState : bool {
        Cleared  = 0,
        Signaled = 1,
    };

    class Event {
        private:
            SRWLOCK m_srwlock;
            CONDITION_VARIABLE m_cv;
            bool m_state;
            bool m_is_auto_reset;
            u64 m_broadcast_id;
        private:
            void WaitImpl(u32 last);
            Result WaitTimeoutImpl(s64 timeout_tick, u32 last);
        public:
            constexpr Event() : m_srwlock(SRWLOCK_INIT), m_cv(CONDITION_VARIABLE_INIT), m_state(), m_is_auto_reset(), m_broadcast_id() {/*...*/}
            constexpr Event(SignalState signal_state, ResetMode reset_mode) : m_srwlock(SRWLOCK_INIT), m_cv(CONDITION_VARIABLE_INIT), m_state((signal_state == SignalState::Signaled)), m_is_auto_reset((reset_mode == ResetMode::Auto)), m_broadcast_id() {/*...*/}
            constexpr ~Event() {/*...*/}

            constexpr void Initialize(SignalState signal_state, ResetMode reset_mode) {
                m_is_auto_reset = (reset_mode == ResetMode::Auto);
                m_state = (signal_state == SignalState::Signaled);
            }

            constexpr void Finalize() {
                m_is_auto_reset = 0;
                m_state = 0;
            }

            void Wait() {
                ::AcquireSRWLockExclusive(std::addressof(m_srwlock));
                if (!m_state) {
                    const u64 b_id = m_broadcast_id;
                    do {
                        if (b_id != m_broadcast_id) { break; }
                        ::SleepConditionVariableSRW(std::addressof(m_cv), std::addressof(m_srwlock), INFINITE, 0);
                    } while (!m_state);
                }
                if (m_is_auto_reset) {
                    m_state = 0;
                }
                ::ReleaseSRWLockExclusive(std::addressof(m_srwlock));
            }
            bool TryWait() {
                ::AcquireSRWLockExclusive(std::addressof(m_srwlock));
                u8 state = m_state;
                if (m_is_auto_reset) {
                    m_state = 0;
                }
                ::ReleaseSRWLockExclusive(std::addressof(m_srwlock));
                return state != 0;
            }
            bool TimedWait(TimeSpan timeout) {
                ::AcquireSRWLockExclusive(std::addressof(m_srwlock));
                if (!m_state) {
                    const u64 b_id = m_broadcast_id;
                    do {
                        if (b_id != m_broadcast_id) { break; }
                        const bool is_timeout = ::SleepConditionVariableSRW(std::addressof(m_cv), std::addressof(m_srwlock), timeout.GetMilliSeconds(), 0);
                        if (is_timeout && ::GetLastError() == ERROR_TIMEOUT) {
                            ::ReleaseSRWLockExclusive(std::addressof(m_srwlock));
                            return false;
                        }
                    } while (!m_state);
                }
                if (m_is_auto_reset) {
                    m_state = 0;
                }
                ::ReleaseSRWLockExclusive(std::addressof(m_srwlock));
                return true;
            }

            void Signal() {
                ::AcquireSRWLockExclusive(std::addressof(m_srwlock));
                if (!m_state) {
                    m_state = 1;
                    if (!m_is_auto_reset) {
                        ++m_broadcast_id;
                        ::WakeAllConditionVariable(std::addressof(m_cv));
                    } else {
                        ::WakeConditionVariable(std::addressof(m_cv));
                    }
                }
                ::ReleaseSRWLockExclusive(std::addressof(m_srwlock));
            }

            void Clear() {
                ::AcquireSRWLockExclusive(std::addressof(m_srwlock));
                m_state = 0;
                ::ReleaseSRWLockExclusive(std::addressof(m_srwlock));
            }

    };
}
