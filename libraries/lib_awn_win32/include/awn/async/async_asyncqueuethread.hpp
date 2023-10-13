#pragma once

namespace awn::async {

    class AsyncQueueThread : public sys::ServiceThread {
        public:
            friend class AsyncTask;
            friend class AsyncQueue;
        public:
            enum class Message : size_t {
                Exit    = 0,
                Start   = 1,
                Suspend = 2,
                Resume  = 3,
            };
        private:
            u32                m_is_finished;
            u32                m_requests_per_yield;
            AsyncTask         *m_current_task;
            AsyncQueue        *m_queue;
            sys::ServiceEvent  m_execute_event;
        public:
             AsyncQueueThread(AsyncQueue *async_queue, const char *name, mem::Heap *thread_heap, sys::ThreadRunMode run_mode, size_t exit_code, u32 max_messages, u32 stack_size, s32 priority) : ServiceThread(name, thread_heap, run_mode, exit_code, max_messages, stack_size, priority), m_is_finished(true), m_requests_per_yield(), m_current_task(), m_queue(), m_execute_event() { 

                async_queue->m_task_thread_array.PushPointer(this);

                m_execute_event.Initialize();
                m_execute_event.Signal();
            }
            ~AsyncQueueThread() { m_execute_event.Finalize(); }

            virtual void ThreadMain(size_t message) override {

                /* Task main loop */
                u32 yield_iter = m_requests_per_yield;
                m_is_finished = false;
                do {

                    {
                        /* Lock queue */
                        std::scoped_lock l(m_queue->m_queue_cs);

                        /* Schedule async task */
                        AsyncTask *task = m_queue->AcquireNextTask(this);
                        if (task == nullptr) { break; }

                        /* Execute task */
                        task->Invoke(this);

                        /* Signal execute event */
                        m_execute_event.Signal();
                        
                        /* Signal finished tasks */
                        m_queue->UpdateCompletion();
                    }

                    /* Memory barrier */
                    vp::util::MemoryBarrierReadWrite();

                    /* Handle yield */
                    if (0 < m_requests_per_yield) {
                        yield_iter = yield_iter - 1;
                        if (yield_iter == 0) {
                            yield_iter = m_requests_per_yield;
                            sys::SleepThread(0);
                        }
                    }

                    /* Peek next message */
                    this->TryPeekMessage(std::addressof(message));

                } while (message != m_exit_message && message != static_cast<size_t>(Message::Suspend));

                m_is_finished = true;

                return;
            }

            ALWAYS_INLINE void WaitForExecute() {
                m_execute_event.Wait();
            }
    };
}
