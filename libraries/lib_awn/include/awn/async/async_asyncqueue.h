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

    struct AsyncQueuePerSetCount {
        u8 thread_count_primary;
        u8 thread_count_boost;
    };

    struct AsyncQueueInfo {
        u32 priority_level_count;
        u32 thread_set_count;
        const AsyncQueuePerSetCount *per_set_count_array;
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
            static constexpr u32 cInvalidThreadLink = 0xffff'ffff;
            static constexpr u32 cInvalidPriorityLevel = 0xffff'ffff;
        public:
            using AsyncQueueThreadArray = vp::util::PointerArray<AsyncQueueThread>;
        public:
            struct ThreadBoost {
                u32 set_index;
                u32 linked_thread;
                bool is_runnable;
                bool is_in_boost;
            };
            struct ThreadSet {
                AsyncQueueThreadArray thread_array_primary;
                AsyncQueueThreadArray thread_array_boost;
                u32 priority;
            };
            struct PriorityLevel {
                u32 is_paused;
                AsyncTask *async_task_head;
                sys::Event priority_cleared_event;
            };
        public:
            using AsyncTaskList = vp::util::IntrusiveListTraits<AsyncTask, &AsyncTask::m_queue_list_node>::List;
            using PriorityLevelArray = vp::util::HeapArray<PriorityLevel>;
            using ThreadBoostArray = vp::util::PointerArrayAllocator<ThreadBoost>;
            using ThreadSetArray = vp::util::HeapArray<ThreadSet>;
            using ThreadSetLoopFunction = vp::util::IFunction<void(AsyncQueueThread*)>;
        private:
            AsyncTaskList m_task_list;
            u32 m_task_count;
            PriorityLevelArray m_priority_level_array;
            AsyncQueueThreadArray m_task_thread_array;
            ThreadBoostArray m_thread_boost_array;
            ThreadSetArray m_thread_set_array;
            sys::Event m_all_task_complete_event;
            sys::Mutex m_queue_mutex;
            bool m_is_multi_set;
            u32 m_highest_priority;
            u32 m_current_thread_set;
        protected:
            AsyncTask *AcquireNextTask(AsyncQueueThread *queue_thread);
            void EnqueueTask(AsyncTask *task, u32 priority);

            void AdjustPriority(AsyncTask *task, u32 priority);
            bool IsAnyThreadHaveTaskPriority(u32 priority);

            bool UpdateAllTaskCompletion();
            void UpdatePriorityLevelCompletion();
            void UpdateCompletion();
        protected:
            ThreadBoost *GetThreadBoost(AsyncQueueThread *thread);
            void LoopThreadSet(const ThreadSetLoopFunction *loop_function, u32 set_index);

            void WakeThread(u32 thread_index, AsyncQueueThread *thread);
            void BoostThread(u32 thread_index, AsyncQueueThread *this_thread);
            void WakeThreadSet(u32 set_index, AsyncQueueThread *this_thread);

            bool RaisePriorityForThreadSet();
            bool LowerPriorityForThreadSet();
            bool UpdateThreadSetForAcquireTask(u32 *out_boost_id, AsyncTask *acquired_task, AsyncQueueThread *this_thread);
            bool UpdateThreadSetForCancelTask();
            bool UpdateThreadSetForCompleteTask(u32 last_priority, AsyncQueueThread *this_thread);
            bool UpdateThreadSetForAdjustPriority(bool is_raise);
        public:
            constexpr  AsyncQueue() : m_task_list(), m_task_count(), m_priority_level_array(), m_task_thread_array(), m_thread_boost_array(), m_thread_set_array(), m_all_task_complete_event(), m_queue_mutex(), m_is_multi_set(), m_highest_priority(), m_current_thread_set() {/*...*/}
            constexpr ~AsyncQueue() {/*...*/}

            Result Initialize(mem::Heap *heap, const AsyncQueueInfo *queue_info);
            void Finalize();

            void AddThread(AsyncQueueThread *thread, u32 thread_set, bool is_boost);

            Result CancelTask(AsyncTask *task, bool is_wait_for_task);

            void CancelPriorityLevel(u32 priority);
            void CancelThreadPriorityLevel(u32 priority);

            void Wait();
            void WaitForPriorityLevel(u32 priority);

            void ForceCalcSyncOnThread(AsyncQueueThread *thread, u32 up_to_priority);

            constexpr sys::Mutex *GetQueueMutex() { return std::addressof(m_queue_mutex); }
    };
}
