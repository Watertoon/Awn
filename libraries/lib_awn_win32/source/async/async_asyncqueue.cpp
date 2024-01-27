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

	AsyncTask *AsyncQueue::AcquireNextTask(AsyncQueueThread *queue_thread) {

        /* Lock for scheduling */
        std::scoped_lock l(m_queue_mutex);
        vp::util::MemoryBarrierReadWrite();

        /* Handle out of tasks */
		if (m_task_count == 0) {
			if (this->UpdateAllTaskCompletion() == true) {
				queue_thread->m_current_task = nullptr;
				return nullptr;
			}
		}

		/* Find next task */
		AsyncTask     *next_task      = nullptr;
		PriorityLevel *priority_level = nullptr;
        u32            i              = m_priority_level_array.GetCount() - 1;
		for (; i != 0xffff'ffff; --i) {
			if (m_priority_level_array[i].is_paused == true) { continue; }
			next_task      = m_priority_level_array[i].async_task_head;
			priority_level = std::addressof(m_priority_level_array[i]);
			break;
		}

		/* Bail if no task */
		if (next_task == nullptr) {
			queue_thread->m_current_task = nullptr;
			return nullptr;
		}

		/* Remove task from task list if not end */
		AsyncTask *new_head = std::addressof(m_task_list.GetNext(*next_task));
		if (new_head == std::addressof(*m_task_list.end()) || new_head->m_priority != i) {
			new_head = nullptr;
		}
		priority_level->async_task_head = new_head;
		next_task->m_queue_list_node.Unlink();
		--m_task_count;
		next_task->m_status = static_cast<u32>(AsyncTask::Status::Acquired);

        next_task->m_queue_thread    = queue_thread;
		queue_thread->m_current_task = next_task;

		return next_task;
	}

	bool AsyncQueue::UpdateAllTaskCompletion() {

		/* Signal all task completion event if all threads are idle */
		bool is_all_finished   = true;
		for (AsyncQueueThread *&queue_thread : m_task_thread_array) {
			if (queue_thread->m_is_finished == true) { continue; }
			is_all_finished = false;
		}
		if (is_all_finished == true) {
			m_all_task_complete_event.Signal();
		}

		return is_all_finished;
	}
	void AsyncQueue::UpdatePriorityLevelCompletion() {

		/* Signal priority level completion if priority is empty */
		const u32 priority_count    = m_priority_level_array.GetCount();
		for (u32 i = 0; i < priority_count; ++i) {

			bool is_level_finished = true;
			for (AsyncQueueThread *&queue_thread : m_task_thread_array) {
				if (queue_thread->m_current_task == nullptr)       { continue; }
				if (queue_thread->m_current_task->m_priority != i) { continue; }
				is_level_finished = false;
			}
			if (is_level_finished == true && m_priority_level_array[i].async_task_head == nullptr) {
				m_priority_level_array[i].priority_cleared_event.Signal();
			}
		}

		return;
	}
	void AsyncQueue::UpdateCompletion() {
		this->UpdateAllTaskCompletion();
		this->UpdatePriorityLevelCompletion();
	}

	void AsyncQueue::Initialize(mem::Heap *heap, AsyncQueueInfo *queue_info) {

        /* Integrity checks */
        VP_ASSERT(queue_info != nullptr);
        VP_ASSERT(0 < queue_info->priority_level_count);
        VP_ASSERT(0 < queue_info->queue_thread_count);

        /* Initialize service mutex */
        m_queue_mutex.Initialize();

		/* Initialize arrays */
		m_priority_level_array.Initialize(heap, queue_info->priority_level_count);
		m_task_thread_array.Initialize(heap, queue_info->queue_thread_count);

		/* Initialize events */
		for (PriorityLevel &priority_level : m_priority_level_array) {
			priority_level.priority_cleared_event.Initialize(sys::SignalState::Signaled, sys::ResetMode::Manual);
		}
		m_all_task_complete_event.Initialize(sys::SignalState::Cleared, sys::ResetMode::Manual);

		return;
	}

	void AsyncQueue::Finalize() {

        m_queue_mutex.Finalize();

		m_all_task_complete_event.Finalize();
		for (PriorityLevel &priority_level : m_priority_level_array) {
			priority_level.priority_cleared_event.Finalize();
		}

		m_task_thread_array.Finalize();
		m_priority_level_array.Finalize();

		return;
	}

    bool AsyncQueue::IsAnyThreadHaveTaskPriority(u32 priority) {

        std::scoped_lock l(m_queue_mutex);
        for (AsyncQueueThread *&queue_thread : m_task_thread_array) {
            AsyncTask *task = queue_thread->m_current_task;
            if (task != nullptr && task->m_priority == priority) { return true; }
        }

        return false;
    }

	void AsyncQueue::CancelTask(AsyncTask *task) {

		/* Try cancel task if running */
		{
			std::scoped_lock l(m_queue_mutex);
            
            /* Ensure task is not complete */
            if (task->m_status == static_cast<u32>(AsyncTask::Status::Complete)) { return; }
            /* Ensure task is not uninitialized */
			if (task->m_status <= static_cast<u32>(AsyncTask::Status::Cancelled)) { return; }

            /* Handle task if it's queued */
            if (task->m_status == static_cast<u32>(AsyncTask::Status::Queued)) {

                /* Unschedule task */
                const u32      priority       = task->m_priority;
                PriorityLevel *priority_level = std::addressof(m_priority_level_array[priority]);
                if (priority_level->async_task_head == task) {

                    AsyncTask *new_head = std::addressof(m_task_list.GetNext(*task));
                    if (new_head == std::addressof(*m_task_list.end()) || new_head->m_priority != priority) {
                        new_head = nullptr;
                    }
                    priority_level->async_task_head = new_head;
                }
                task->m_queue_list_node.Unlink();

                /* Cancel task */
                task->Cancel();
                this->UpdateCompletion();

                return;
            }

            /* Cancel task while it's active */
			task->CancelWhileActive();
		}

        /* Wait for task to finish */
		task->m_finish_event.Wait();

        return;
	}

	void AsyncQueue::CancelPriorityLevel(u32 priority) {

        {
            /* Lock thread */
            std::scoped_lock l(m_queue_mutex);

            /* Cancel all tasks on the priority level */
            AsyncTask *head = m_priority_level_array[priority].async_task_head;
            if (head != nullptr) {

                AsyncTaskList::iterator list_iter = m_task_list.IteratorTo(*head);
                AsyncTask *task_iter              = nullptr;
                while (list_iter != m_task_list.end() && priority == (*list_iter).m_priority) {
                    task_iter = std::addressof(*list_iter);
                    ++list_iter;

                    task_iter->m_queue_list_node.Unlink();
                    --m_task_count;
                    task_iter->Cancel();
                }
            }
            m_priority_level_array[priority].async_task_head = nullptr;

            const bool has_priority = this->IsAnyThreadHaveTaskPriority(priority);
            if (has_priority == false) {
                this->UpdateCompletion();
                this->CancelThreadPriorityLevel(priority);
                return;
            }

            /* Clear priority event */
            m_priority_level_array[priority].priority_cleared_event.Clear();

            this->UpdateCompletion();
            this->CancelThreadPriorityLevel(priority);
        }

        /* WAit for priority event */
        m_priority_level_array[priority].priority_cleared_event.Wait();

		return;
	}

    void AsyncQueue::CancelThreadPriorityLevel(u32 priority) {

            /* Lock queue */
            std::scoped_lock l(m_queue_mutex);

            /* Cancel all threads */
            for (AsyncQueueThread *&queue_thread : m_task_thread_array) {
                queue_thread->CancelCurrentTaskIfPriority(priority);
            }

            return;
    }
}
