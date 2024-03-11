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

    void ServiceEvent::Wait() {

		/* Ensure memory coherancy */
		ON_SCOPE_EXIT { vp::util::MemoryBarrierReadWrite(); };

		/* Check signal state */
		if (m_is_auto_reset == false) { 

			/* Attempt to set state to waiting if no waiters */
			u32 last;
			const bool cmp_result = vp::util::InterlockedCompareExchange(std::addressof(last), std::addressof(m_state), 1u, 0u);
			if (cmp_result == true || last == 1) {

				/* Wait impl */
				if (::IsThreadAFiber() == false) {

					u32 value = 1;
					do {
						::WaitOnAddress(std::addressof(m_state), std::addressof(value), sizeof(u32), INFINITE);
					} while (last == value);

				} else {
					const Result result = ukern::WaitOnAddress(reinterpret_cast<uintptr_t>(std::addressof(m_state)), ukern::ArbitrationType_WaitIfEqual, 1, TimeSpan::cMaxTime);
					VP_ASSERT(result == ukern::ResultInvalidWaitAddressValue || result == ResultSuccess);
				}

			} else {
				VP_ASSERT(last == 2);
			}

		} else {

			/* Set waiter bit */
			Result result;
			u32 last;
			do {
				last = m_state;

				/* Set is waiters */
				if (last == 0) {
					if (vp::util::InterlockedCompareExchange(std::addressof(last), std::addressof(m_state), 1u, 0u) == true) { continue; }
				}

				/* Is waiters */
				if (last == 1) { continue; }

				/* Ensure valid state */
				VP_ASSERT(last == 2);

				/* Try auto clear */
				if (vp::util::InterlockedCompareExchange(std::addressof(last), std::addressof(m_state), 0u, 2u) == true) { return; }

				/* Wait until conditions improve */
			} while (result = this->WaitImpl(last), result == ukern::ResultInvalidWaitAddressValue);
			RESULT_ABORT_UNLESS(result);
		}

		return;
	}

	void ServiceEvent::Signal() {

		if (m_is_auto_reset == false) {

			Result result = ResultSuccess;
			do {

				/* Attempt to set state to signaled if no waiters */
				u32 last;
				const bool cmp_result = vp::util::InterlockedCompareExchange(std::addressof(last), std::addressof(m_state), 2u, 0u);
				if (cmp_result == true) { return; }

				/* Check already signaled */
				if (last != 1) {
					VP_ASSERT(last == 2);
					return;
				}

				/* Wake all waiters */
				result = ukern::WakeByAddress(reinterpret_cast<uintptr_t>(std::addressof(m_state)), ukern::SignalType_SignalAndIncrementIfEqual, 1, 0xffff'ffff);
				::WakeByAddressAll(std::addressof(m_state));

			} while (result == ukern::ResultInvalidWaitAddressValue);
			RESULT_ABORT_UNLESS(result);

		} else {

			Result result = ResultSuccess;
			do {

				/* Attempt to set state to signaled if no waiters */
				u32 last;
				const bool cmp_result = vp::util::InterlockedCompareExchange(std::addressof(last), std::addressof(m_state), 2u, 0u);
				if (cmp_result == true) { return; }

				/* Check already signaled */
				if (last != 1) {
					VP_ASSERT(last == 2);
					return;
				}

				/* Wake single thread */
				result = ukern::WakeByAddress(reinterpret_cast<uintptr_t>(std::addressof(m_state)), ukern::SignalType_SignalAndModifyByWaiterCountIfEqual, 1, 1);
				::WakeByAddressSingle(std::addressof(m_state));

			} while (result == ukern::ResultInvalidWaitAddressValue);
			RESULT_ABORT_UNLESS(result);
		}

		return;
	}

	void ServiceEvent::Clear() {
		u32 last;
		vp::util::InterlockedCompareExchange(std::addressof(last), std::addressof(m_state), 0u, 2u);
	}

	bool ServiceEvent::TimedWait(TimeSpan timeout) {

		/* Get absolute timeout time */
		const TickSpan timeout_tick = TimeSpan::GetAbsoluteTimeToWakeup(timeout);

		/* Ensure memory coherancy on exit */
		ON_SCOPE_EXIT { vp::util::MemoryBarrierReadWrite(); };

		/* Check signal state */
		Result result = ResultSuccess;
		if (m_is_auto_reset == false) { 

			/* Attempt to set state to waiting if no waiters */
			u32 last;
			const bool cmp_result = vp::util::InterlockedCompareExchange(std::addressof(last), std::addressof(m_state), 1u, 0u);
			if (cmp_result == true || last == 1) {

				/* Wait impl */
				TimeSpan time_left = TimeSpan::GetTimeLeftOnTarget(timeout_tick);
				if (::IsThreadAFiber() == false) {

					u32 value = 1;
					do {
						const bool wait_result = ::WaitOnAddress(std::addressof(m_state), std::addressof(value), sizeof(u32), time_left.GetMilliSeconds());
						if (wait_result == false && ::GetLastError() == ERROR_TIMEOUT) { 
							time_left = TimeSpan::GetTimeLeftOnTarget(timeout_tick); 
							if (time_left.GetNanoSeconds() == 0) { 
								result = ukern::ResultTimeout; 
								break; 
							} 
						}
					} while (time_left.GetNanoSeconds() != 0 && (vp::util::InterlockedCompareExchange(std::addressof(last), std::addressof(m_state), 1u, 0u) == true || last <= 1));
					VP_ASSERT(last == 2);

				} else {
					result = ukern::WaitOnAddress(reinterpret_cast<uintptr_t>(std::addressof(m_state)), ukern::ArbitrationType_WaitIfEqual, 1, time_left.GetNanoSeconds());
					VP_ASSERT(result == ukern::ResultTimeout || result == ukern::ResultInvalidWaitAddressValue || result == ResultSuccess);
				}

			} else {
				VP_ASSERT(last == 2);
			}

		} else {

			/* Set waiter bit */
			u32 last;
			do {
				last = m_state;

				/* Set is waiters */
				if (last == 0) {
					if (vp::util::InterlockedCompareExchange(std::addressof(last), std::addressof(m_state), 1u, 0u) == true) { continue; }
				}

				/* Is waiters */
				if (last == 1) { continue; }

				/* Ensure valid state */
				VP_ASSERT(last == 2);

				/* Try auto clear */
				if (vp::util::InterlockedCompareExchange(std::addressof(last), std::addressof(m_state), 0u, 2u) == true) { return true; }

				/* Wait until conditions improve */
			} while (result = this->WaitTimeoutImpl(timeout_tick, last), result == ukern::ResultInvalidWaitAddressValue);
			VP_ASSERT(result == ResultSuccess || result == ukern::ResultTimeout);
		}

		return result != ukern::ResultTimeout;
	}
}
