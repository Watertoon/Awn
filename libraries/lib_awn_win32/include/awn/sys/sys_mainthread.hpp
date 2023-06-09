#pragma once

namespace awn::sys {
    
    class MainThread : public ThreadBase {
        private:
            ukern::UKernHandle m_thread_handle;
        public:
            MainThread(mem::Heap *thread_heap) : ThreadBase(thread_heap), m_thread_handle(ukern::GetCurrentThread()->ukern_fiber_handle) { ukern::GetCurrentThread()->user_arg = this; }
            virtual ~MainThread() override {/*...*/}

            virtual void StartThread() {
                VP_ASSERT(false);
            }
            virtual void WaitForThreadExit() {
                VP_ASSERT(false);
            }
            virtual void ResumeThread() {
                const Result result0 = ukern::ResumeThread(m_thread_handle);
                VP_ASSERT(result0 == ResultSuccess);
            }
            virtual void SuspendThread() {
                const Result result0 = ukern::SuspendThread(m_thread_handle);
                VP_ASSERT(result0 == ResultSuccess);
            }

            virtual void SetPriority(s32 priority) {
                const Result result0 = ukern::SetThreadPriority(m_thread_handle, priority);
                VP_ASSERT(result0 == ResultSuccess);
                m_priority = priority;
            }
            virtual void SetCoreMask(u64 core_mask) {
                const Result result0 = ukern::SetThreadCoreMask(m_thread_handle, core_mask);
                VP_ASSERT(result0 == ResultSuccess);
                m_core_mask = core_mask;
            }
    };
}
