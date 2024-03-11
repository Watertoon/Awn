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

    struct AsyncTaskScheduleInfo {
        AsyncTaskForAllocator *task_for_allocator;
        AsyncTaskPushInfo     *push_info;
        AsyncTaskWatcher      *watcher;
    };

    using AsyncTaskCreateFunction = vp::util::IFunction<AsyncTaskForAllocator*(AsyncTaskAllocator*, mem::Heap*)>;

    class AsyncTaskAllocator {
        public:
            friend class AsyncTaskForAllocator;
        public:
            using AsyncTaskList  = vp::util::IntrusiveListTraits<AsyncTaskForAllocator, &AsyncTaskForAllocator::m_queue_list_node>::List;
            using AsyncTaskArray = vp::util::PointerArray<AsyncTaskForAllocator>;
        private:
            sys::ServiceEvent           m_acquire_event;
            sys::ServiceEvent           m_available_event;
            sys::ServiceCriticalSection m_acquire_cs;
            sys::ServiceCriticalSection m_free_cs;
            u32                         m_list_index;
            AsyncTaskList               m_task_list[2];
            AsyncTaskArray              m_task_array;
        private:
            AsyncTaskForAllocator *AcquireTaskImpl() {

                /* Fail if list is empty */
                if (m_task_list[m_list_index].IsEmpty() == true) { return nullptr; }

                /* Loop until acquiring task */
                for (;;) {

                    /* Get current acquire list */
                    AsyncTaskList &list = m_task_list[m_list_index];

                    for (AsyncTaskForAllocator &task : list) {
                        if (task.m_is_free_for_allocator == false) { continue; }

                        /* Acquire task */
                        task.SetAllocator(this);
                        return std::addressof(task);
                    }

                    m_acquire_event.TimedWait(vp::TimeSpan::FromTick(vp::util::GetSystemTickFrequency() / 1000));
                }

                return nullptr;
            }
            
            AsyncTaskForAllocator *AcquireTask() {

                /* Lock For acquire */
                std::scoped_lock l(m_acquire_cs);

                AsyncTaskForAllocator *task = nullptr;
                do {
                    /* Try acquire task from acquire list */
                    task = this->AcquireTaskImpl();
                    if (task != nullptr) { return task; }
                    
                    /* Wait for a task to become available in the free list */
                    m_available_event.Wait();

                    /* Swap the free list with the acquire list */
                    {
                        std::scoped_lock l(m_free_cs);
                        m_list_index ^= 1;
                        m_available_event.Clear();
                    }

                } while (task == nullptr);

                return nullptr;
            }
        protected:
            void FreeTask(AsyncTaskForAllocator *task) {

                /* Lock for free */
                std::scoped_lock l(m_free_cs);

                /* Add task to free list and signal there's a new free task */
                task->m_is_free_for_allocator = true;
                task->m_task_allocator        = nullptr;
                m_task_list[m_list_index ^ 1].PushBack(*task);
                m_available_event.Signal();

                return;
            }
        public:
            constexpr  AsyncTaskAllocator() : m_acquire_event(), m_available_event(), m_acquire_cs(), m_free_cs(), m_list_index(0), m_task_list{}, m_task_array()  {/*...*/}
            constexpr ~AsyncTaskAllocator() {/*...*/}

            void Initialize(mem::Heap *heap, AsyncTaskCreateFunction *create_function, u32 count) {

                /* Integrity checks */
                VP_ASSERT(create_function != nullptr);

                /* Initialize events */
                m_acquire_event.Initialize(sys::SignalState::Cleared, sys::ResetMode::Manual);
                m_available_event.Initialize(sys::SignalState::Cleared, sys::ResetMode::Manual);

                /* Initialize task pointer array */
                m_task_array.Initialize(heap, count);

                /* Create and add tasks to acquire list */
                for (u32 i = 0; i < count; ++i) {
                    AsyncTaskForAllocator *allocator_task = create_function->Invoke(this, heap);
                    VP_ASSERT(allocator_task != nullptr);

                    allocator_task->m_is_free_for_allocator = true;
                    m_task_array.PushPointer(allocator_task);
                    m_task_list[m_list_index].PushBack(*allocator_task);
                }

                return;
            }

            void Finalize() {

                /* Finalize events */
                m_acquire_event.Finalize();
                m_available_event.Finalize();

                /* Free all tasks */
                for (u32 i = 0; i < m_task_array.GetUsedCount(); ++i) {
                    m_task_array[i]->m_queue_list_node.Unlink();
                    delete m_task_array[i];
                }

                /* Free pointer array */
                m_task_array.Finalize();

                return;
            }

            void ScheduleTask(AsyncTaskScheduleInfo *schedule_info) {

                /* Acquire task */
                bool is_acquired_task       = false;
                AsyncTaskForAllocator *task = schedule_info->task_for_allocator;
                if (task == nullptr) {
                    task             = this->AcquireTask();
                    is_acquired_task = true;
                }

                /* Set watcher */
                if (schedule_info->watcher != nullptr) {
                    AsyncQueue *queue = schedule_info->push_info->GetQueue();
                    task->SetWatcher(schedule_info->watcher, queue);
                }

                /* Try push task */
                if (task->PushTask(schedule_info->push_info) != ResultSuccess && AsyncTaskForAllocator::CheckRuntimeTypeInfoStatic(task) == true) {
                    if (is_acquired_task == false) { return; }
                    this->FreeTask(task);
                }

                return;
            }

    };
}