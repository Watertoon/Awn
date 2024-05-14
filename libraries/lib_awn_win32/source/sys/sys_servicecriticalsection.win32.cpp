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

	void ServiceCriticalSection::Enter() {

		/* Get current thread */
		sys::ThreadBase *thread = sys::GetCurrentThread();

		/* Ukern thread impl */
		for(;;) {

			/* Try acquire ownership atomically */
			sys::ThreadBase *last0;
			const bool result = vp::util::InterlockedCompareExchange(std::addressof(last0), std::addressof(m_owner), thread, static_cast<sys::ThreadBase*>(nullptr));
			if (result == true) { return; }

			/* Fallback wait */
			if (::IsThreadAFiber() == false) {
				u32 value = 0;
				::WaitOnAddress(std::addressof(m_wait_value), std::addressof(value), sizeof(u32), INFINITE);
			} else {                        
				const Result result = ukern::WaitOnAddress(reinterpret_cast<uintptr_t>(std::addressof(m_wait_value)), ukern::ArbitrationType_WaitIfEqual, 0, TimeSpan::cMaxTime);
				if (result != ukern::ResultInvalidWaitAddressValue) { RESULT_ABORT_UNLESS(result); }
			}
		}
	}

	bool ServiceCriticalSection::TryEnter() {

		/* Get current thread */
		sys::ThreadBase *thread = sys::GetCurrentThread();

		/* Try to acquire ownership */
		sys::ThreadBase *last;
		const bool result = vp::util::InterlockedCompareExchange(std::addressof(last), std::addressof(m_owner), thread, static_cast<sys::ThreadBase*>(nullptr));

		return result;
	}

	void ServiceCriticalSection::Leave() {

		/* Release owner */
		m_owner = nullptr;

		/* Wake 1 waiter of either type */
        ukern::WakeByAddress(reinterpret_cast<uintptr_t>(std::addressof(m_wait_value)), ukern::SignalType_Signal, 0, 1);
        ::WakeByAddressSingle(std::addressof(m_wait_value));

		return;
	}
	
	void ServiceMutex::Enter() {

		/* Get current thread */
		sys::ThreadBase *thread = sys::GetCurrentThread();
		if (thread == m_owner) { ++m_lock_count; return; }

		/* Ukern thread impl */
		for(;;) {

			/* Try acquire ownership atomically */
			sys::ThreadBase *last0;
			const bool result = vp::util::InterlockedCompareExchange(std::addressof(last0), std::addressof(m_owner), thread, static_cast<sys::ThreadBase*>(nullptr));
			if (result == true) { ++m_lock_count; return; }

			/* Fallback wait */
			if (::IsThreadAFiber() == false) {
				u32 value = 0;
				::WaitOnAddress(std::addressof(m_wait_value), std::addressof(value), sizeof(u32), INFINITE);
			} else {                        
				const Result result = ukern::WaitOnAddress(reinterpret_cast<uintptr_t>(std::addressof(m_wait_value)), ukern::ArbitrationType_WaitIfEqual, 0, TimeSpan::cMaxTime);
				if (result != ukern::ResultInvalidWaitAddressValue) { RESULT_ABORT_UNLESS(result); }
			}
		}
	}

	bool ServiceMutex::TryEnter() {

		/* Get current thread */
		sys::ThreadBase *thread = sys::GetCurrentThread();

		/* Try to acquire ownership */
		sys::ThreadBase *last;
		const bool result = vp::util::InterlockedCompareExchange(std::addressof(last), std::addressof(m_owner), thread, static_cast<sys::ThreadBase*>(nullptr));
		if (result == true) { ++m_lock_count; }

		return result;
	}

	void ServiceMutex::Leave() {

		/* Decrement lock count */
		const u32 reduce_count = m_lock_count - 1;
		m_lock_count = reduce_count;
		if (0 < reduce_count) { return; }

		/* Release owner */
		m_owner = nullptr;

		/* Wake 1 waiter for either type */
        ukern::WakeByAddress(reinterpret_cast<uintptr_t>(std::addressof(m_wait_value)), ukern::SignalType_Signal, 0, 1);
        ::WakeByAddressSingle(std::addressof(m_wait_value));

		return;
	}
}
