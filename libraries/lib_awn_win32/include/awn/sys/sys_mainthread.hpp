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
    
    class MainThread : public ThreadBase {
        private:
            ukern::UKernHandle m_thread_handle;
        public:
            MainThread(mem::Heap *thread_heap) : ThreadBase(thread_heap), m_thread_handle(ukern::GetCurrentThread()->ukern_fiber_handle) { ukern::GetCurrentThread()->user_arg = this; }
            virtual ~MainThread() override {/*...*/}

            virtual void StartThread() override {
                VP_ASSERT(false);
            }
            virtual void WaitForThreadExit() override {
                VP_ASSERT(false);
            }
            virtual void ResumeThread() override {
                const Result result0 = ukern::ResumeThread(m_thread_handle);
                VP_ASSERT(result0 == ResultSuccess);
            }
            virtual void SuspendThread() override {
                const Result result0 = ukern::SuspendThread(m_thread_handle);
                VP_ASSERT(result0 == ResultSuccess);
            }

            virtual void SleepThread(vp::TimeSpan timeout_ns) override {
                ukern::Sleep(timeout_ns);
            }

            virtual void SetPriority(s32 priority) override {
                const Result result0 = ukern::SetThreadPriority(m_thread_handle, priority);
                VP_ASSERT(result0 == ResultSuccess);
                m_priority = priority;
            }
            virtual void SetCoreMask(u64 core_mask) override {
                const Result result0 = ukern::SetThreadCoreMask(m_thread_handle, core_mask);
                VP_ASSERT(result0 == ResultSuccess);
                m_core_mask = core_mask;
            }
    };
}
