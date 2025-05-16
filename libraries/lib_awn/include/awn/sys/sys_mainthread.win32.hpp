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

    class MainThread : public Thread {
        public:
            VP_RTTI_DERIVED(MainThread, Thread);
        public:
            constexpr MainThread() : Thread() {/*...*/}
            virtual ~MainThread() override {/*...*/}

            void InitializeForMainThread(mem::Heap *thread_heap) {

                /* Create win32 thread */
                HANDLE main_thread_handle;
                const bool result = ::DuplicateHandle(sys::GetProcessHandle(), ::GetCurrentThread(), sys::GetProcessHandle(), std::addressof(main_thread_handle), 0, false, DUPLICATE_SAME_ACCESS);
                VP_ASSERT(result);
                m_handle = main_thread_handle;

                /* Create waitable time for sleep */
                m_waitable_timer_for_sleep = ::CreateWaitableTimerExA(nullptr, nullptr, CREATE_WAITABLE_TIMER_HIGH_RESOLUTION, TIMER_ALL_ACCESS);
                VP_ASSERT(m_waitable_timer_for_sleep != nullptr);

                /* Set thread name */
                ::strcpy(m_name, "MainThread");
                ::wcscpy(reinterpret_cast<wchar_t*>(m_name_utf16), reinterpret_cast<const wchar_t*>(u"MainThread"));
                ::SetThreadDescription(main_thread_handle, reinterpret_cast<wchar_t*>(m_name_utf16));

                m_thread_heap = thread_heap;

                return;
            }

            virtual void StartThread() override {
                VP_NOT_IMPLEMENTED();
            }
            virtual void ExitThread(ExitType exit_type) override {
                VP_UNUSED(exit_type);
                VP_NOT_IMPLEMENTED();
            }
            virtual void WaitForThreadExit() override {
                VP_NOT_IMPLEMENTED();
            }
    };
}
