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

    Thread::Thread(const char *name, mem::Heap *thread_heap, ThreadRunMode run_mode, size_t exit_code, u32 max_messages, u32 stack_size, s32 priority) : ThreadBase(thread_heap, run_mode, exit_code, max_messages, stack_size, priority), m_handle(), m_waitable_timer_for_sleep(), m_name('\0'), m_name_utf16(u'\0') {

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

    Thread::Thread(mem::Heap *thread_heap) : ThreadBase(thread_heap), m_handle(), m_waitable_timer_for_sleep(), m_name('\0') {/*...*/}

    Thread::~Thread() {
        this->ExitThread(awn::sys::ExitType::Send);
    }

    void Thread::StartThread() {
        s32 result0 = ::ResumeThread(m_handle);
        VP_ASSERT(result0 != -1);
    }

    void Thread::ExitThread(ExitType exit_type) {
        this->ThreadBase::ExitThread(exit_type);
        if (m_waitable_timer_for_sleep != nullptr) {                    
            ::CloseHandle(m_waitable_timer_for_sleep);
            m_waitable_timer_for_sleep = nullptr;
        }
    }

    void Thread::WaitForThreadExit() {
        s32 result0 = ::WaitForSingleObject(m_handle, INFINITE);
        VP_ASSERT(result0 == WAIT_OBJECT_0);
    }

    void Thread::ResumeThread() {
        s32 result0 = ::ResumeThread(m_handle);
        VP_ASSERT(result0 != -1);
    }

    void Thread::SuspendThread() {
        s32 result0 = ::SuspendThread(m_handle);
        VP_ASSERT(result0 != -1);
    }

    //namespace {
    //    void TimerApcDummy(LPVOID arg, DWORD timer_low, DWORD timer_high) {
    //        VP_UNUSED(arg, timer_low, timer_high);
    //    }
    //}

    void Thread::SleepThread(vp::TimeSpan timeout_ns) {

        const LARGE_INTEGER timeout_100ns = { .QuadPart = -(timeout_ns.GetNanoSeconds() / 100)};
        const bool is_set = ::SetWaitableTimer(m_waitable_timer_for_sleep, std::addressof(timeout_100ns), 0, nullptr, nullptr, false);
        if (!is_set) {
            ::printf("LastError: %ld\n", ::GetLastError());
        }
        VP_ASSERT(is_set);

        const u32 result0 = ::WaitForSingleObjectEx(m_waitable_timer_for_sleep, INFINITE, false);
        if (result0 != WAIT_OBJECT_0) {
            ::printf("LastError: %ld\n", ::GetLastError());
        }
        VP_ASSERT(result0 == WAIT_OBJECT_0);
    }

    void Thread::YieldThread() {
        ::SwitchToThread();
    }

    void Thread::SetPriority(s32 priority) {
        VP_ASSERT(priority < cMaxPriority)
        bool result0 = ::SetThreadPriority(m_handle, ConvertAwnPriorityToWin32(priority));
        VP_ASSERT(result0);
        m_priority = priority;
    }

    void Thread::SetCoreMask(CoreMask core_mask) {
        bool result0 = ::SetThreadAffinityMask(m_handle, core_mask);
        VP_ASSERT(result0);
        m_core_mask = core_mask;
    }
}
