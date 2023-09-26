#include <awn.hpp>

namespace awn::async {

	AsyncTask *AsyncQueue::AcquireNextTask(AsyncQueueThread *queue_thread) {

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
		for (u32 i = m_priority_level_array.GetCount() - 1; i != 0xffff'ffff; --i) {
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

		/* Remove task from task list */
		AsyncTask *new_head = nullptr;
		if (std::addressof(m_task_list.GetNext(*next_task)) != std::addressof(m_task_list.Back())) {
			new_head = std::addressof(m_task_list.GetNext(*next_task));
		}
		priority_level->async_task_head = new_head;
		next_task->m_queue_list_node.Unlink();
		--m_task_count;
		next_task->m_status = static_cast<u32>(AsyncTask::Status::Acquired);

		queue_thread->m_current_task = next_task;

		return next_task;
	}

	bool AsyncQueue::UpdateAllTaskCompletion() {

		/* Signal all task completion event if all threads are idle */
		const u32 task_thread_count = m_task_thread_array.GetUsedCount();
		bool      is_all_finished   = true;
		for (u32 i = 0; i < task_thread_count; ++i) {
			if (m_task_thread_array[i]->m_is_finished == true) { continue; }
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
		const u32 task_thread_count = m_task_thread_array.GetUsedCount();
		for (u32 i = 0; i < priority_count; ++i) {

			bool is_level_finished = true;
			for (u32 j = 0; j < task_thread_count; ++j) {
				if (m_task_thread_array[j]->m_current_task == nullptr) { continue; }
				if (m_task_thread_array[j]->m_current_task->m_priority != i) { continue; }
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

		/* Initialize arrays */
		m_priority_level_array.Initialize(heap, queue_info->priority_level_count);
		m_task_thread_array.Initialize(heap, queue_info->queue_thread_count);

		/* Initialize events */
		for (u32 i = 0; i < queue_info->priority_level_count; ++i) {
			m_priority_level_array[i].priority_cleared_event.Initialize();
			m_priority_level_array[i].priority_cleared_event.Signal();
		}
		m_all_task_complete_event.Initialize();

		return;
	}

	void AsyncQueue::Finalize() {

		m_all_task_complete_event.Finalize();
		for (u32 i = 0; i < m_priority_level_array.GetCount(); ++i) {
			m_priority_level_array[i].priority_cleared_event.Finalize();
		}

		m_task_thread_array.Finalize();
		m_priority_level_array.Finalize();

		return;
	}

	void AsyncQueue::CancelTask(AsyncTask *task) {

		/* Try cancel task if running */
		{
			std::scoped_lock l(m_queue_cs);

			if (task->m_status <= static_cast<u32>(AsyncTask::Status::Cancelled) || task->m_status == static_cast<u32>(AsyncTask::Status::Complete)) { return; }

			task->m_finish_event.Clear();
		}

		task->m_finish_event.Wait();
	}

	void AsyncQueue::CancelPriorityLevel(u32 priority) {

		/* Lock thread */
		std::scoped_lock l(m_queue_cs);

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

		/* Update events */
		this->UpdateCompletion();

		return;
	}
}
