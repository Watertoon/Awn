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
#include <awn.hpp>

namespace awn::async {

    AsyncTaskForAllocator *AsyncTaskAllocator::AcquireTaskImpl() {

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

    AsyncTaskForAllocator *AsyncTaskAllocator::AcquireTask() {

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

    void AsyncTaskAllocator::FreeTask(AsyncTaskForAllocator *task) {

        /* Lock for free */
        std::scoped_lock l(m_free_cs);

        /* Add task to free list and signal there's a new free task */
        task->m_is_free_for_allocator = true;
        task->m_task_allocator        = nullptr;
        m_task_list[m_list_index ^ 1].PushBack(*task);
        m_available_event.Signal();

        return;
    }
    
    void AsyncTaskAllocator::Initialize(mem::Heap *heap, AsyncTaskCreateFunction *create_function, u32 count) {

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

    void AsyncTaskAllocator::Finalize() {

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

    void AsyncTaskAllocator::ScheduleTask(AsyncTaskScheduleInfo *schedule_info) {

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
}
