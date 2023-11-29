#pragma once

namespace awn::async {

    class AsyncTaskForAllocator;

    class AsyncTaskWatcher {
        public:
            friend class AsyncTaskForAllocator;
        public:
            enum class State : u32 {
                Uninitialized = 0,
                Pending       = 1,
                Complete      = 2,
                Cancelled     = 3,
            };
        private:
            u32                    m_state;
            u32                    m_reference_count;
            AsyncTaskForAllocator *m_async_task;
            AsyncQueue            *m_queue;
        public:
            constexpr  AsyncTaskWatcher() : m_state(), m_reference_count(), m_async_task(), m_queue() {/*...*/}
            constexpr ~AsyncTaskWatcher() {/*...*/}

            void Reference();

            void ReleaseReference();

            void CancelTask();

            void WaitForCompletion();
    };
}
