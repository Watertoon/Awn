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

    constexpr s32 ConvertAwnPriorityToWin32(s32 priority) {
        if (priority <= 0x12) {
            if (0xe <= priority) { return THREAD_PRIORITY_NORMAL; }
            if (0x8 <= priority) { return THREAD_PRIORITY_ABOVE_NORMAL; }
            if (0x1 <= priority) { return THREAD_PRIORITY_HIGHEST; }
            if (0x0 <= priority) { return THREAD_PRIORITY_TIME_CRITICAL; }
        } else if (priority <= 0x18) {
            return THREAD_PRIORITY_BELOW_NORMAL;
        } else if (priority <= 0x1e) {
            return THREAD_PRIORITY_LOWEST;
        } else if (priority <= 0x1f) {
            return THREAD_PRIORITY_IDLE;
        }
        VP_ASSERT(false);
        return THREAD_PRIORITY_NORMAL;
    }

    static constexpr u32 cMaxThreadNameLength = 0x40;

	class Thread : public sys::ThreadBase {
		protected:
			HANDLE             m_handle;
            HANDLE             m_waitable_timer_for_sleep;
            long unsigned int  m_win32_thread_id;
            char               m_name[cMaxThreadNameLength];
            char16_t           m_name_utf16[cMaxThreadNameLength];
        public:
            VP_RTTI_DERIVED(SystemThread, ThreadBase);
        public:
			Thread(const char *name, mem::Heap *thread_heap, ThreadRunMode run_mode, size_t exit_code, u32 max_messages, u32 stack_size, s32 priority) : ThreadBase(thread_heap, run_mode, exit_code, max_messages, stack_size, priority), m_handle(), m_waitable_timer_for_sleep(), m_name('\0'), m_name_utf16(u'\0') {

                /* Create win32 thread */
                m_handle = ::CreateThread(nullptr, stack_size, ThreadBase::InternalServiceThreadMain, this, CREATE_SUSPENDED, std::addressof(m_win32_thread_id));
                VP_ASSERT(m_handle != nullptr);

                /* Create waitable time for sleep */
                m_waitable_timer_for_sleep = ::CreateWaitableTimerExA(nullptr, nullptr, CREATE_WAITABLE_TIMER_HIGH_RESOLUTION, TIMER_ALL_ACCESS);
                VP_ASSERT(m_waitable_timer_for_sleep != nullptr);

                /* Set priority */
                bool result0 = ::SetThreadPriority(m_handle, ConvertAwnPriorityToWin32(priority));
                VP_ASSERT(result0);

                /* Set local thread name */
                const u32 name_length = vp::util::StringLength(name);
                VP_ASSERT((cMaxThreadNameLength - 1) > name_length);
                ::memcpy(m_name, name, name_length);
                m_name[name_length] = '\0';

                /* Set thread name for system */
                u32 size_utf16;
                const u32 name_result = vp::util::ConvertStringUtf8ToUtf16(std::addressof(size_utf16), m_name_utf16, 0x20, m_name, name_length);
                VP_ASSERT(name_result == 0);
                VP_ASSERT((cMaxThreadNameLength - 1) > size_utf16);
                m_name_utf16[size_utf16] = u'\0';
                ::SetThreadDescription(m_handle, reinterpret_cast<wchar_t*>(m_name_utf16));

                return;
            }
			Thread(mem::Heap *thread_heap) : ThreadBase(thread_heap), m_handle(), m_waitable_timer_for_sleep(), m_name('\0') {/*...*/}
            virtual ~Thread() override {
                this->ExitThread(awn::sys::ExitType::Send);
            }

            virtual void StartThread() override {
                s32 result0 = ::ResumeThread(m_handle);
                VP_ASSERT(result0 != -1);
            }
            virtual void ExitThread(ExitType exit_type) override {
                this->ThreadBase::ExitThread(exit_type);
                if (m_waitable_timer_for_sleep != nullptr) {                    
                    ::CloseHandle(m_waitable_timer_for_sleep);
                    m_waitable_timer_for_sleep = nullptr;
                }
            }
            virtual void WaitForThreadExit() override {
                s32 result0 = ::WaitForSingleObject(m_handle, INFINITE);
                VP_ASSERT(result0 == WAIT_OBJECT_0);
            }
            virtual void ResumeThread() override {
                s32 result0 = ::ResumeThread(m_handle);
                VP_ASSERT(result0 != -1);
            }
            virtual void SuspendThread() override {
                s32 result0 = ::SuspendThread(m_handle);
                VP_ASSERT(result0 != -1);
            }
            virtual void SleepThread(vp::TimeSpan timeout_ns) override {

                const LARGE_INTEGER timeout_100ns = { .QuadPart = -(timeout_ns.GetNanoSeconds() / 100)};
                const bool is_set = ::SetWaitableTimer(m_waitable_timer_for_sleep, std::addressof(timeout_100ns), 0, nullptr, nullptr, false);
                if (!is_set) {
                    ::printf("LastError: %ld\n", ::GetLastError());
                }
                VP_ASSERT(is_set);

                const u32 result0 = ::WaitForSingleObject(m_waitable_timer_for_sleep, INFINITE);
                VP_ASSERT(result0 == WAIT_OBJECT_0);
            }

            virtual void YieldThread() override {
                ::SwitchToThread();
            }

            virtual void SetPriority(s32 priority) override {
                VP_ASSERT(priority < cMaxPriority)
                bool result0 = ::SetThreadPriority(m_handle, ConvertAwnPriorityToWin32(priority));
                VP_ASSERT(result0);
                m_priority = priority;
            }
            virtual void SetCoreMask(CoreMask core_mask) override {
                bool result0 = ::SetThreadAffinityMask(m_handle, core_mask);
                VP_ASSERT(result0);
                m_core_mask = core_mask;
            }

            constexpr HANDLE GetWin32Handle() const { return m_handle; }
            constexpr long unsigned int GetWin32ThreadId() const { return m_win32_thread_id; }
	};
}
