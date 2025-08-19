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

    Result Thread::Initialize(mem::Heap *thread_heap, const ThreadInfo *info) {

        /* Create win32 thread */
        m_handle = ::CreateThread(nullptr, info->stack_size, ThreadBase::InternalServiceThreadMain, this, CREATE_SUSPENDED, std::addressof(m_win32_thread_id));
        RESULT_RETURN_UNLESS(m_handle != nullptr, ResultFailedToCreateThread);
        auto fail_guard0 = SCOPE_GUARD {
            ::CloseHandle(m_handle);
            m_handle = nullptr;
        };

        /* Create waitable time for sleep */
        m_waitable_timer_for_sleep = ::CreateWaitableTimerExA(nullptr, nullptr, CREATE_WAITABLE_TIMER_HIGH_RESOLUTION, TIMER_ALL_ACCESS);
        RESULT_RETURN_UNLESS(m_waitable_timer_for_sleep != nullptr, ResultFailedToCreateThread);
        auto fail_guard1 = SCOPE_GUARD {
            ::CloseHandle(m_waitable_timer_for_sleep);
            m_waitable_timer_for_sleep = nullptr;
        };

        /* Set priority */
        const s32 win_priority = ConvertAwnPriorityToWin32(info->priority);
        bool result0 = ::SetThreadPriority(m_handle, win_priority);
        RESULT_RETURN_UNLESS(result0, ResultInvalidPriority);

        /* Set thread name for system */
        m_name.SetString(info->name);
        m_name_utf16.SetString(std::addressof(m_name));
        ::SetThreadDescription(m_handle, reinterpret_cast<wchar_t*>(m_name_utf16.GetString()));

        /* Initialize base */
        this->ThreadBase::Initialize(thread_heap, info);

        fail_guard0.Cancel();
        fail_guard1.Cancel();
        RESULT_RETURN_SUCCESS;
    }

    void Thread::Finalize() {
        return;
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
        if (m_handle != nullptr) {
            ::CloseHandle(m_handle);
            m_handle = nullptr;
        }
        return;
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
        //if (!is_set) {
        //    ::printf("LastError: %ld\n", ::GetLastError());
        //}
        VP_ASSERT(is_set);

        const u32 result0 = ::WaitForSingleObjectEx(m_waitable_timer_for_sleep, INFINITE, false);
        //if (result0 != WAIT_OBJECT_0) {
        //    ::printf("LastError: %ld\n", ::GetLastError());
        //}
        VP_ASSERT(result0 == WAIT_OBJECT_0);

        return;
    }

    void Thread::YieldThread() {
        ::SwitchToThread();
    }

    void Thread::SetPriority(u32 priority) {
        VP_ASSERT(priority < cMaxPriority);
        const s32 win_priority = ConvertAwnPriorityToWin32(priority);
        bool result0 = ::SetThreadPriority(m_handle, win_priority);
        VP_ASSERT(result0);
        m_priority = priority;
    }

    void Thread::SetCoreMask(CoreMask core_mask) {
        bool result0 = ::SetThreadAffinityMask(m_handle, core_mask);
        VP_ASSERT(result0);
        m_core_mask = core_mask;
    }
}
