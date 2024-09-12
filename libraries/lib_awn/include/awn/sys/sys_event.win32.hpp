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
            bool m_is_auto_reset;
            u32  m_state;
        private:
            void WaitImpl(u32 last);
            Result WaitTimeoutImpl(s64 timeout_tick, u32 last);
        public:
            constexpr Event() : m_is_auto_reset(), m_state() {/*...*/}
            constexpr ALWAYS_INLINE Event(SignalState signal_state, ResetMode reset_mode) : m_is_auto_reset(static_cast<bool>(reset_mode)), m_state((signal_state == SignalState::Signaled) ? 2 : 0) {/*...*/}
            constexpr ~Event() {/*...*/}

            constexpr void Initialize(SignalState signal_state, ResetMode reset_mode) {
                m_is_auto_reset = static_cast<bool>(reset_mode);
                m_state         = (signal_state == SignalState::Signaled) ? 2 : 0;
            }

            constexpr void Finalize() {
                m_is_auto_reset = 0;
                m_state         = 0;
            }

            void Wait();
            void Signal();
            void Clear();

            bool TimedWait(TimeSpan timeout);
    };
}
