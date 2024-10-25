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
#include <awn.hpp>

namespace awn::sys {

    void Event::WaitImpl(u32 last) {

        if (last != 1) { return; }

        u32 value = 1;
        u32 prev = 0;
        do {
            ::WaitOnAddress(std::addressof(m_state), std::addressof(value), sizeof(u32), INFINITE);
            prev = m_state;
        } while (prev == value);

        return;
    }

    Result Event::WaitTimeoutImpl(s64 timeout_tick, u32 last) {

        if (last != 1) { RESULT_RETURN_SUCCESS; }

        TimeSpan time_left = TimeSpan::GetTimeLeftOnTarget(timeout_tick);
        u32 value = 1;
        const bool result = ::WaitOnAddress(std::addressof(m_state), std::addressof(value), sizeof(u32), time_left.GetMilliSeconds());
        if (!result) {
            time_left = TimeSpan::GetTimeLeftOnTarget(timeout_tick); 
            if (time_left.GetNanoSeconds() == 0) { 
                RESULT_RETURN(ResultTimeout); 
            }
        }
        RESULT_RETURN_SUCCESS;
    }

    void Event::Wait() {

		/* Ensure memory coherancy */
		ON_SCOPE_EXIT { vp::util::MemoryBarrierReadWrite(); };

		/* Check signal state */
		if (!m_is_auto_reset) { 

			/* Attempt to set state to waiting if no waiters */
			u32 last;
			const bool cmp_result = vp::util::InterlockedCompareExchange(std::addressof(last), std::addressof(m_state), 1u, 0u);
			if (cmp_result || last == 1) {
				this->WaitImpl(1);
			} else {
				VP_ASSERT(last == 2);
			}

		} else {

			/* Set waiter bit */
			u32 last;
			for(;;) {
				/* Set is waiters */
                if (vp::util::InterlockedCompareExchange(std::addressof(last), std::addressof(m_state), 1u, 0u)) { continue; }

				/* Is waiters */
				if (last == 1) { continue; }

				/* Ensure valid state */
				VP_ASSERT(last == 2);

				/* Try auto clear */
				if (vp::util::InterlockedCompareExchange(std::addressof(last), std::addressof(m_state), 0u, 2u)) { return; }

				/* Wait until conditions improve */
                this->WaitImpl(last);
			}
		}

		return;
	}

	void Event::Signal() {

        bool result = 0;
        do {

            /* Attempt to set state to signaled if no waiters */
            u32 last;
            const bool cmp_result = vp::util::InterlockedCompareExchange(std::addressof(last), std::addressof(m_state), 2u, 0u);
            if (cmp_result) { return; }

            /* Check already signaled */
            if (last != 1) {
                VP_ASSERT(last == 2);
                return;
            }

            /* Attempt to set state to signaled and wake waiters */
            result = vp::util::InterlockedCompareExchange(std::addressof(last), std::addressof(m_state), 2u, 1u);
            if (result) {
                if (!m_is_auto_reset) {                        
                    ::WakeByAddressAll(std::addressof(m_state));
                } else {
                    ::WakeByAddressSingle(std::addressof(m_state));
                }
                return;
            }
        } while (!result);
        RESULT_ABORT_UNLESS(result);

		return;
	}

	void Event::Clear() {
		u32 last;
		vp::util::InterlockedCompareExchange(std::addressof(last), std::addressof(m_state), 0u, 2u);
	}

	bool Event::TimedWait(TimeSpan timeout) {

		/* Get absolute timeout time */
		const TickSpan timeout_tick = TimeSpan::GetAbsoluteTimeToWakeup(timeout);

		/* Ensure memory coherancy on exit */
		ON_SCOPE_EXIT { vp::util::MemoryBarrierReadWrite(); };

		/* Check signal state */
		Result result = ResultSuccess;
		if (!m_is_auto_reset) { 

			/* Attempt to set state to waiting if no waiters */
			u32 last;
			const bool cmp_result = vp::util::InterlockedCompareExchange(std::addressof(last), std::addressof(m_state), 1u, 0u);
			if (cmp_result || last == 1) {

				/* Wait impl */
				TimeSpan time_left = TimeSpan::GetTimeLeftOnTarget(timeout_tick);
                u32 value = 1;
                do {
                    const bool wait_result = ::WaitOnAddress(std::addressof(m_state), std::addressof(value), sizeof(u32), time_left.GetMilliSeconds());
                    if (wait_result == false && ::GetLastError() == ERROR_TIMEOUT) { 
                        time_left = TimeSpan::GetTimeLeftOnTarget(timeout_tick); 
                        if (time_left.GetNanoSeconds() == 0) { 
                            result = ResultTimeout; 
                            break; 
                        } 
                    }
                } while (time_left.GetNanoSeconds() != 0 && (vp::util::InterlockedCompareExchange(std::addressof(last), std::addressof(m_state), 1u, 0u) || last <= 1));
			}
			VP_ASSERT(last == 2);

		} else {

			/* Set waiter bit */
			u32 last;
			do {
				last = m_state;

				/* Set is waiters */
				if (last == 0) {
					if (vp::util::InterlockedCompareExchange(std::addressof(last), std::addressof(m_state), 1u, 0u)) { continue; }
				}

				/* Is waiters */
				if (last == 1) { continue; }

				/* Ensure valid state */
				VP_ASSERT(last == 2);

				/* Try auto clear */
				if (vp::util::InterlockedCompareExchange(std::addressof(last), std::addressof(m_state), 0u, 2u)) { return true; }

				/* Wait until conditions improve */
			} while (RESULT_SUCCEEDED(this->WaitTimeoutImpl(timeout_tick, last)));
		}

		return result != ResultTimeout;
	}
}
