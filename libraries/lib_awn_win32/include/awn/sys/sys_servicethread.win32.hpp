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

	class ServiceThread : public sys::ThreadBase {
		private:
			HANDLE            m_handle;
            HANDLE            m_waitable_timer_for_sleep;
            long unsigned int m_win32_thread_id;
            char              m_name[ukern::cMaxFiberNameLength];
		public:
			ServiceThread(const char *name, mem::Heap *thread_heap, ThreadRunMode run_mode, size_t exit_code, u32 max_messages, u32 stack_size, s32 priority) : ThreadBase(thread_heap, run_mode, exit_code, max_messages, stack_size, priority), m_handle(INVALID_HANDLE_VALUE), m_waitable_timer_for_sleep(INVALID_HANDLE_VALUE), m_name('\0') {

                /* Create win32 thread */
                m_handle = ::CreateThread(nullptr, stack_size, ThreadBase::InternalServiceThreadMain, this, CREATE_SUSPENDED, std::addressof(m_win32_thread_id));
                VP_ASSERT(m_handle != nullptr);

                /* Create waitable time for sleep */
                m_waitable_timer_for_sleep = ::CreateWaitableTimerA(nullptr, true, nullptr);
                VP_ASSERT(m_waitable_timer_for_sleep != nullptr);

                /* Set priority */
                bool result0 = ::SetThreadPriority(m_handle, priority);
                VP_ASSERT(result0 == true);

                /* Set thread name */
                ::strncpy(m_name, name, ukern::cMaxFiberNameLength);
            }
            virtual ~ServiceThread() override {/*...*/}

            virtual void StartThread() {
                s32 result0 = ::ResumeThread(m_handle);
                VP_ASSERT(result0 != -1);
            }
            virtual void WaitForThreadExit() {
                s32 result0 = ::WaitForSingleObject(m_handle, INFINITE);
                VP_ASSERT(result0 == WAIT_OBJECT_0);
            }
            virtual void ResumeThread() {
                s32 result0 = ::ResumeThread(m_handle);
                VP_ASSERT(result0 != -1);
            }
            virtual void SuspendThread() {
                s32 result0 = ::SuspendThread(m_handle);
                VP_ASSERT(result0 != -1);
            }
            virtual void SleepThread(vp::TimeSpan timeout_ns) {

                const LARGE_INTEGER timeout_100ns = { .QuadPart = -timeout_ns.GetNanoSeconds() / 100};
                const bool is_set = ::SetWaitableTimer(m_waitable_timer_for_sleep, std::addressof(timeout_100ns), 0, nullptr, nullptr, false);
                VP_ASSERT(is_set == true);

                const u32 result0 = ::WaitForSingleObject(m_waitable_timer_for_sleep, INFINITE);
                VP_ASSERT(result0 == WAIT_OBJECT_0);
            }

            virtual void SetPriority(s32 priority) {
                bool result0 = ::SetThreadPriority(m_handle, priority);
                VP_ASSERT(result0 == true);
                m_priority = priority;
            }
            virtual void SetCoreMask(CoreMask core_mask) {
                bool result0 = ::SetThreadAffinityMask(m_handle, core_mask);
                VP_ASSERT(result0 == true);
                m_core_mask = core_mask;
            }
	};
}
