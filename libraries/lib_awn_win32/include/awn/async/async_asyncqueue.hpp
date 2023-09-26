#pragma once

namespace awn::async {

    struct AsyncQueueInfo {
        u32 priority_level_count;
        u32 queue_thread_count;
    };
    class AsyncQueue {
        public:
            friend class AsyncTask;
            friend class AsyncQueueThread;
        public:
            static constexpr u32 cInvalidPriorityLevel = 0xffff'ffff;
        public:
            struct PriorityLevel {
                u32                is_paused;
                AsyncTask         *async_task_head;
                sys::ServiceEvent  priority_cleared_event;
            };
        public:
            using AsyncTaskList      = vp::util::IntrusiveListTraits<AsyncTask, &AsyncTask::m_queue_list_node>::List;
            using PriorityLevelArray = vp::util::HeapArray<PriorityLevel>;
            using TaskThreadArray    = vp::util::PointerArray<AsyncQueueThread>;
        private:
            AsyncTaskList                m_task_list;
            u32                          m_task_count;
            PriorityLevelArray           m_priority_level_array;
            TaskThreadArray              m_task_thread_array;
            sys::ServiceEvent            m_all_task_complete_event;
            sys::ServiceCriticalSection  m_queue_cs;
        protected:
            AsyncTask *AcquireNextTask(AsyncQueueThread *queue_thread);

            bool UpdateAllTaskCompletion();
            void UpdatePriorityLevelCompletion();
            void UpdateCompletion();
        public:
            constexpr  AsyncQueue() : m_task_list(), m_task_count(), m_priority_level_array(), m_task_thread_array(), m_all_task_complete_event(), m_queue_cs() {/*...*/}
            constexpr ~AsyncQueue() {/*...*/}

            void Initialize(mem::Heap *heap, AsyncQueueInfo *queue_info);

            void Finalize();

            void CancelTask(AsyncTask *task);

            void CancelPriorityLevel(u32 priority);

            ALWAYS_INLINE void Wait() {
                m_all_task_complete_event.Wait();
            }

            void WaitForPriorityLevel(u32 priority) {
                if (m_priority_level_array[priority].is_paused == true || m_priority_level_array[priority].async_task_head != nullptr) { return; }
                m_priority_level_array[priority].priority_cleared_event.Wait();
            }
    };
}
