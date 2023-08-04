#pragma once

namespace awn::sys {

    class DelegateThread : public Thread {
        public:
            using ThreadDelegate = vp::util::Delegate<DelegateThread, size_t>;
        private:
            ThreadDelegate *m_delegate;
        private:

            virtual void ThreadMain(size_t message) override {
                m_delegate->Invoke(message);
            }
        public:
            explicit DelegateThread(ThreadDelegate *delegate, const char *name, mem::Heap *heap, ThreadRunMode run_mode, size_t exit_code, u32 max_messages, u32 stack_size, u32 priority) : Thread(name, heap, run_mode, exit_code, max_messages, stack_size, priority) {
                VP_ASSERT(delegate != nullptr);

                delegate->m_parent = this;
                m_delegate         = delegate;
            }
            virtual ~DelegateThread() override {/*...*/}
    };

    template<typename Parent>
    class DelegateServiceThread : public ServiceThread {
        public:
            using ThreadDelegate = vp::util::Delegate<Parent, size_t>;
        private:
            ThreadDelegate *m_delegate;
        private:

            virtual void ThreadMain(size_t message) override {
                m_delegate->Invoke(message);
            }
        public:
            explicit DelegateServiceThread(ThreadDelegate *delegate, const char *name, mem::Heap *heap, ThreadRunMode run_mode, size_t exit_code, u32 max_messages, u32 stack_size, u32 priority) : ServiceThread(name, heap, run_mode, exit_code, max_messages, stack_size, priority) {
                VP_ASSERT(delegate != nullptr);

                m_delegate         = delegate;
            }
            virtual ~DelegateServiceThread() override {/*...*/}
    };
}
