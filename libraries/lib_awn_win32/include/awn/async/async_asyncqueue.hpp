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

namespace awn::async {

    struct AsyncQueueInfo {
        u32 priority_level_count;
        u32 queue_thread_count;
    };

    class AsyncTaskForAllocator;
    class AsyncTaskWatcher;

    class AsyncQueue {
        public:
            friend class AsyncTask;
            friend class AsyncTaskForAllocator;
            friend class AsyncTaskWatcher;
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
            AsyncTaskList      m_task_list;
            u32                m_task_count;
            PriorityLevelArray m_priority_level_array;
            TaskThreadArray    m_task_thread_array;
            sys::ServiceEvent  m_all_task_complete_event;
            sys::ServiceMutex  m_queue_mutex;
        protected:
            AsyncTask *AcquireNextTask(AsyncQueueThread *queue_thread);

            bool IsAnyThreadHaveTaskPriority(u32 priority);

            bool UpdateAllTaskCompletion();
            void UpdatePriorityLevelCompletion();
            void UpdateCompletion();
        public:
            constexpr  AsyncQueue() : m_task_list(), m_task_count(), m_priority_level_array(), m_task_thread_array(), m_all_task_complete_event(), m_queue_mutex() {/*...*/}
            constexpr ~AsyncQueue() {/*...*/}

            void Initialize(mem::Heap *heap, const AsyncQueueInfo *queue_info);

            void Finalize();

            void CancelTask(AsyncTask *task);

            void CancelPriorityLevel(u32 priority);
            void CancelThreadPriorityLevel(u32 priority);

            ALWAYS_INLINE void Wait() {
                m_all_task_complete_event.Wait();
            }

            void WaitForPriorityLevel(u32 priority) {
                if (m_priority_level_array[priority].is_paused == true || m_priority_level_array[priority].async_task_head != nullptr) { return; }
                m_priority_level_array[priority].priority_cleared_event.Wait();
            }

            void ForceCalcSyncOnThread(AsyncQueueThread *thread, u32 up_to_priority) {

                /* Lock queue */
                std::scoped_lock l(m_queue_mutex);

                /* Calc tasks */
                for (AsyncTask &task : m_task_list) {
                    if (task.m_priority < up_to_priority) { break; }
                    if (task.m_queue != nullptr) { task.m_queue->CancelTask(std::addressof(task)); }
                    task.InvokeSync(thread);
                }

                return;
            }

            sys::ServiceMutex *GetQueueMutex() { return std::addressof(m_queue_mutex); }
    };
}
