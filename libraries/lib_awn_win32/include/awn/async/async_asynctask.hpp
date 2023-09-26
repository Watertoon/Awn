#pragma once

namespace awn::async {

	class AsyncTask;
    class AsyncQueue;
    class AsyncQueueThread;

    struct TaskResultInvokeInfo {
        AsyncTask *task;
        void      *user_data;
    };

    using TaskDelegate   = vp::util::IDelegate<void*>;
    using ResultDelegate = vp::util::IDelegateReturn<Result, const TaskResultInvokeInfo*>;

    struct AsyncTaskPushInfo {
        AsyncQueue       *queue;
        AsyncQueueThread *queue_thread;
        TaskDelegate     *task_delegate;
        ResultDelegate   *result_delegate;
        void             *user_data;
        u32               priority;
        bool              is_sync;
    };

    class AsyncTask {
        public:
            friend class AsyncQueue;
            friend class AsyncQueueThread;
        public:
            enum class Status : u32 {
                Uninitialized = 0,
                Cancelled     = 1,
                Queued        = 2,
                Acquired      = 3,
                FreeExecute   = 4,
                PostExecute   = 5,
                Complete      = 6,
            };
        public:
            static constexpr u32 cInvalidPriorityLevel = 0xffff'ffff;
        private:
            u32                          m_priority;
            u32                          m_status;
            AsyncQueue                  *m_queue;
            AsyncQueueThread            *m_queue_thread;
            TaskDelegate                *m_task_delegate;
            ResultDelegate              *m_result_delegate;
            void                        *m_user_data;
            sys::ServiceEvent            m_finish_event;
            vp::util::IntrusiveListNode  m_queue_list_node;
        protected:
            virtual void Execute() {
                if (m_task_delegate == nullptr) { return; }
                m_task_delegate->Invoke(m_user_data);
            }

            virtual void PostExecute() {/*...*/}

            virtual void FreeExecute() {/*...*/}

            virtual void FormatPushInfo(AsyncTaskPushInfo *push_info) {/*...*/}

            virtual void FreeCancel() {/*...*/}
        protected:
            bool TryInvokeSync();
        
            void Invoke(AsyncQueueThread *thread);

            void Cancel();
        public:
             AsyncTask() : m_priority(), m_status(), m_queue(), m_queue_thread(), m_task_delegate(), m_result_delegate(), m_user_data(), m_finish_event(), m_queue_list_node() { m_finish_event.Initialize(); }
            ~AsyncTask() { m_finish_event.Finalize(); }

            Result PushTask(AsyncTaskPushInfo *push_info);
    };
}
