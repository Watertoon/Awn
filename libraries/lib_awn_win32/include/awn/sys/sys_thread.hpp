#pragma once

namespace awn::sys {

    class Thread : public ThreadBase {
        private:
            ukern::UKernHandle m_thread_handle;
        public:
            Thread(const char *name, mem::Heap *thread_heap, ThreadRunMode run_mode, size_t exit_code, u32 max_messages, u32 stack_size, s32 priority) : ThreadBase(thread_heap, run_mode, exit_code, max_messages, stack_size, priority), m_thread_handle(0) {

                /* Create a thread on the default core */
                const Result result0 = ukern::CreateThread(std::addressof(m_thread_handle), ThreadBase::InternalThreadMain, reinterpret_cast<uintptr_t>(this), stack_size, priority, -1);
                VP_ASSERT(result0 == ResultSuccess);

                ukern::SetThreadName(m_thread_handle, name);
            }
            virtual ~Thread() override {/*...*/}

            virtual void StartThread() override {
                const Result result0 = ukern::StartThread(m_thread_handle);
                VP_ASSERT(result0 == ResultSuccess);
            }
            virtual void WaitForThreadExit() override {
                ukern::ExitThread(m_thread_handle);
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
