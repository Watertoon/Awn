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

    class Thread : public ThreadBase {
        private:
            ukern::UKernHandle m_thread_handle;
        public:
            Thread(const char *name, mem::Heap *thread_heap, ThreadRunMode run_mode, size_t exit_code, u32 max_messages, u32 stack_size, s32 priority) : ThreadBase(thread_heap, run_mode, exit_code, max_messages, stack_size, priority), m_thread_handle(0) {

                /* Create a thread on the default core */
                RESULT_ABORT_UNLESS(ukern::CreateThread(std::addressof(m_thread_handle), ThreadBase::InternalThreadMain, reinterpret_cast<uintptr_t>(this), stack_size, priority, -2));

                ukern::SetThreadName(m_thread_handle, name);
            }
            virtual ~Thread() override {/*...*/}

            virtual void StartThread() override {
                RESULT_ABORT_UNLESS(ukern::StartThread(m_thread_handle));
            }
            virtual void WaitForThreadExit() override {
                ukern::ExitThread(m_thread_handle);
            }
            virtual void ResumeThread() override {
                RESULT_ABORT_UNLESS(ukern::ResumeThread(m_thread_handle));
            }
            virtual void SuspendThread() override {
                RESULT_ABORT_UNLESS(ukern::SuspendThread(m_thread_handle));
            }

            virtual void SleepThread(vp::TimeSpan timeout_ns) override {
                ukern::Sleep(timeout_ns);
            }

            virtual void SetPriority(s32 priority) override {
                const Result result = ukern::SetThreadPriority(m_thread_handle, priority);
                if (result == ukern::ResultSamePriority) { return; }
                RESULT_ABORT_UNLESS(result);
                m_priority = priority;
            }
            virtual void SetCoreMask(u64 core_mask) override {
                const Result result = ukern::SetThreadCoreMask(m_thread_handle, core_mask);
                if (result == ukern::ResultSameCoreMask) { return; }
                RESULT_ABORT_UNLESS(result);
                m_core_mask = core_mask;
            }
    };
}
