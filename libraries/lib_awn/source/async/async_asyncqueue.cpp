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

        /* Handle out of tasks */
		if (m_task_count == 0) {
			if (this->UpdateAllTaskCompletion()) {
				queue_thread->m_current_task = nullptr;
				return nullptr;
			}
		}

		/* Find next task */
		AsyncTask *next_task = nullptr;
		PriorityLevel *priority_level = nullptr;
        u32 i = m_priority_level_array.GetCount() - 1;
		for (; i != 0xffff'ffff; --i) {
			if (m_priority_level_array[i].is_paused) { continue; }
			priority_level = std::addressof(m_priority_level_array[i]);
			next_task = priority_level->async_task_head;
            if (next_task == nullptr) { continue; }
			break;
		}

		/* Bail if no task */
		if (next_task == nullptr) {
            this->UpdateAllTaskCompletion();
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

    void AsyncQueue::EnqueueTask(AsyncTask *task, u32 priority) {

        u32 thread_set;
        {            
            std::scoped_lock l(m_queue_mutex);

            /* Save last highest priority */
            const u32 prev_highest_priority = (0 < m_task_count) ? m_task_list.Back().m_priority : 0;

            /* Bounds check and set priority */
            const u32 max_priority = m_priority_level_array.GetCount();
            const u32 clamp_priority = (priority < max_priority) ? priority : max_priority - 1;
            task->m_priority = clamp_priority;

            /* Find lowest populated priority level */
            u32 priority_iter = clamp_priority;
            do {
                priority_iter = priority_iter - 1;
            } while(priority_iter != cInvalidPriorityLevel && m_priority_level_array[priority_iter].async_task_head == nullptr);

            /* Insert into list at respective priority level */
            if (priority_iter == cInvalidPriorityLevel) {

                /* Insert as head since there are no other tasks */
                m_task_list.PushBack(*task);

                /* Clear priorty level event */
                m_priority_level_array[clamp_priority].priority_cleared_event.Clear();
            } else {

                /* Link next to back of current priority */
                m_priority_level_array[priority_iter].async_task_head->m_queue_list_node.LinkNext(std::addressof(task->m_queue_list_node));
            }

            /* Increment task list count */
            const u32 task_count = m_task_count + 1;
            m_task_count  = task_count;

            /* Set priority level head */
            if (m_priority_level_array[clamp_priority].async_task_head == nullptr) {
                m_priority_level_array[clamp_priority].async_task_head = task;
            }
            task->m_status = static_cast<u32>(AsyncTask::Status::Queued);

            /* Clear event if first job */
            if (task_count == 1) {
                m_all_task_complete_event.Clear();
            }

            if (prev_highest_priority != priority && m_is_multi_set) {
                this->LowerPriorityForThreadSet();
            }
            thread_set = m_current_thread_set;
        }

        /* Resume queue threads */
        AsyncQueueThread *thread = reinterpret_cast<AsyncQueueThread*>(sys::GetCurrentThread());
        this->WakeThreadSet(thread_set, thread);

        return;
    }

	bool AsyncQueue::UpdateAllTaskCompletion() {

		/* Signal all task completion event if all threads are out of task */
		bool is_any_task = false;
        if (0 < m_task_count) {
            is_any_task = true;
        } else {
            for (AsyncQueueThread *&queue_thread : m_task_thread_array) {
                is_any_task = queue_thread->m_state.GetBit(AsyncQueueThread::State_IsTask);
                if (is_any_task) { break; }
            }
        }
		if (!is_any_task) {
			m_all_task_complete_event.Signal();
		}

		return !is_any_task;
	}

	void AsyncQueue::UpdatePriorityLevelCompletion() {

		/* Signal priority level completion if priority is empty */
		const u32 priority_count = m_priority_level_array.GetCount();
		for (u32 i = 0; i < priority_count; ++i) {

			bool is_level_incomplete = false;
			for (AsyncQueueThread *&queue_thread : m_task_thread_array) {
				if (queue_thread->m_current_task == nullptr)       { continue; }
				if (queue_thread->m_current_task->m_priority != i) { continue; }
				is_level_incomplete = true;
                break;
			}
			if (!is_level_incomplete && m_priority_level_array[i].async_task_head == nullptr) {
				m_priority_level_array[i].priority_cleared_event.Signal();
			}
		}

		return;
	}

	void AsyncQueue::UpdateCompletion() {
		this->UpdateAllTaskCompletion();
		this->UpdatePriorityLevelCompletion();
	}

    AsyncQueue::ThreadBoost *AsyncQueue::GetThreadBoost(AsyncQueueThread *thread) {
        const u32 index = m_task_thread_array.IndexOf(thread);
        return m_thread_boost_array[index];
    }

    void AsyncQueue::LoopThreadSet(const ThreadSetLoopFunction *loop_function, u32 set_index) {
        ThreadSet *set = std::addressof(m_thread_set_array[set_index]);
        for (AsyncQueueThread *&thread : set->thread_array_primary) {
            loop_function->Invoke(thread);
        }
        for (AsyncQueueThread *&thread : set->thread_array_boost) {
            loop_function->Invoke(thread);
        }
        return;
    }

    void AsyncQueue::WakeThread(u32 thread_index, AsyncQueueThread *thread) {
        ThreadBoost *boost = m_thread_boost_array[thread_index];
        while (boost->is_runnable && !thread->m_state.GetBit(AsyncQueueThread::State_IsNotScheduling)) {
            size_t message = 0;
            const bool is_message = thread->TryPeekMessage(std::addressof(message));
            if (is_message || thread->GetExitMessage() == message) { return; }
            const bool is_sent = thread->TrySendMessage(static_cast<size_t>(AsyncQueueThread::Message::Continue));
            if (is_sent) { return; }
            sys::SleepThread(0);
        }
        return;
    }

    void AsyncQueue::WakeThreadSet(u32 set_index, AsyncQueueThread *this_thread) {
        const auto wake_lambda = [&](AsyncQueueThread *thread) {
            if (this_thread == thread) { return; }
            const u32 index = m_task_thread_array.IndexOf(thread);
            VP_ASSERT(index != AsyncQueueThreadArray::cInvalidEntryIndex);
            this->WakeThread(index, thread);
            return;
        };
        const auto wake_function = vp::util::MakeLambdaFunction<void(AsyncQueueThread*)>(wake_lambda);
        std::scoped_lock l(m_queue_mutex);
        this->LoopThreadSet(std::addressof(wake_function), set_index);
    }

    void AsyncQueue::BoostThread(u32 thread_index, AsyncQueueThread *this_thread) {

        /* Lock queue */
        std::scoped_lock l(m_queue_mutex);

        /* Fail boost if no boost thread, or we are the boost thread */
        AsyncQueueThread *thread = m_task_thread_array[thread_index];
        if (thread == nullptr || thread == this_thread) { return; }

        /* Wake boost thread */
        this->WakeThread(thread_index, thread);

        return;
    }

    bool AsyncQueue::RaisePriorityForThreadSet() {

        /* Check we are already in the highest priority range */
        const u32 prev_set = m_current_thread_set;
        if (prev_set < 1) {
            m_highest_priority = 0;
            return false;
        }

        /* Ensure we have a task */
        if (m_task_count < 1) {
            m_current_thread_set = 0;
            m_highest_priority = 0;
            return false;
        }

        /* Get priority of highest priority task */
        AsyncTask *task = std::addressof(m_task_list.Back());
        const u32 task_priority = task->m_priority;

        /* Ensure valid priority */
        if (task_priority == cInvalidPriorityLevel) {
            m_current_thread_set = 0;
            m_highest_priority = 0;
            return false;
        }

        /* Check if our highest priority is within the current set */
        u32 set_i = prev_set - 1;
        u32 set_p = m_thread_set_array[set_i].priority;
        if (set_p < task_priority) {
            m_highest_priority = set_p + 1;
            return false;
        }

        /* Walk down the thread sets to the highest priority */
        do {
            const u32 index = set_i - 1;
            m_current_thread_set = set_i;
            if (set_i == 0) { m_highest_priority = 0; break; }
            set_i = index;
            set_p = m_thread_set_array[index].priority;
        } while (task_priority <= set_p);
        m_highest_priority = set_p + 1;

        return true;
    }

    bool AsyncQueue::LowerPriorityForThreadSet() {

        /* On exit update highest priority to set */
        const u32 set_count = m_current_thread_set;
        u32 set_i = m_current_thread_set;
        ON_SCOPE_EXIT {
            if (0 < set_i) {
                const u32 set_p2 = m_thread_set_array[set_i - 1].priority;
                m_highest_priority = (set_p2 != cInvalidPriorityLevel) ? set_p2 + 1 : set_p2;
            } else {
                m_highest_priority = 0;
            }
        };

        /* Check we have a lower priority set */
        if (set_i + 1 >= set_count) {
            return false;
        }

        /* Ensure we have a task */
        if (m_task_count < 1) {
            return false;
        }

        /* Get priority of highest priority task */
        AsyncTask *task = std::addressof(m_task_list.Back());
        const u32 task_priority = task->m_priority;

        /* Ensure valid priority */
        if (task_priority == cInvalidPriorityLevel) {
            return false;
        }

        /* Check if our highest priority is within the current set */
        u32 set_p = m_thread_set_array[set_i].priority;
        if (task_priority <= set_p) {
            return false;
        }

        /* Walk up the thread sets to a lower priority range */
        do {
            set_i = set_i + 1;
            m_current_thread_set = set_i;
            if (set_count <= set_i + 1) { break; }
            set_p = m_thread_set_array[set_i].priority;
        } while (set_p < task_priority);

        return true;
    }

    bool AsyncQueue::UpdateThreadSetForAcquireTask(u32 *out_boost_id, AsyncTask *acquired_task, AsyncQueueThread *this_thread) {

        /* Ensure multi-set */
        if (!m_is_multi_set) { return false; }

        /* Ignore boost if no task */
        if (acquired_task == nullptr) { return this->RaisePriorityForThreadSet(); }

        /* Activate or disable boost */
        ThreadBoost *boost = this->GetThreadBoost(this_thread);
        if (!acquired_task->m_is_thread_boost) {
            boost->is_runnable = true;
            boost->is_in_boost = false;
            ThreadBoost *linked_boost = m_thread_boost_array[boost->linked_thread];
            linked_boost->is_runnable = false;
            linked_boost->is_in_boost = false;
        } else if (!boost->is_in_boost && boost->linked_thread != cInvalidThreadLink) {
            boost->is_runnable = true;
            boost->is_in_boost = true;
            ThreadBoost *linked_boost = m_thread_boost_array[boost->linked_thread];
            linked_boost->is_runnable = true;
            linked_boost->is_in_boost = true;
            *out_boost_id = boost->linked_thread;
        }

        /* Raise current set priority */
        ThreadSet *set = std::addressof(m_thread_set_array[m_current_thread_set]);
        if (set->priority == AsyncTask::cInvalidPriorityLevel || set->priority < acquired_task->m_priority) {
            set->priority = acquired_task->m_priority;
        }

        return this->RaisePriorityForThreadSet();
    }

    bool AsyncQueue::UpdateThreadSetForCancelTask() {

        /* Ensure multi-set */
        if (!m_is_multi_set) { return false; }

        return this->RaisePriorityForThreadSet();
    }

    bool AsyncQueue::UpdateThreadSetForCompleteTask(u32 last_priority, AsyncQueueThread *this_thread) {

        /* Ensure multi-set */
        if (!m_is_multi_set) { return false; }

        /* End boost */
        ThreadBoost *boost = this->GetThreadBoost(this_thread);
        AsyncTask *task = (m_task_count < 1) ? nullptr : std::addressof(m_task_list.Back());
        if ((task == nullptr || task->m_is_thread_boost) && boost->is_in_boost) {
            boost->is_runnable = true;
            boost->is_in_boost = false;
            ThreadBoost *linked_boost = m_thread_boost_array[boost->linked_thread];
            linked_boost->is_runnable = false;
            linked_boost->is_in_boost = false;
        }

        /* Find next highest priority task in set */
        const u32 boost_set = boost->set_index;
        ThreadSet *set = std::addressof(m_thread_set_array[boost_set]);
        if (set->priority == last_priority) {
            u32 priority = cInvalidPriorityLevel;
            bool is_priority = false;
            const auto get_highest_priority_lambda = [&](AsyncQueueThread *thread) {
                AsyncTask *task = thread->m_current_task;
                if (task == nullptr) { return; }
                is_priority = true;
                priority = vp::util::Max(task->m_priority, priority);
            };
            const auto get_highest_priority_function = vp::util::MakeLambdaFunction<void(AsyncQueueThread*)>(get_highest_priority_lambda);
            this->LoopThreadSet(std::addressof(get_highest_priority_function), boost_set);
            if (is_priority) { set->priority = priority; }
        }

        return this->LowerPriorityForThreadSet();
    }

    bool AsyncQueue::UpdateThreadSetForAdjustPriority(bool is_raise) {

        /* Ensure multi-set */
        if (!m_is_multi_set) { return false; }

        return (is_raise) ? this->RaisePriorityForThreadSet() : this->LowerPriorityForThreadSet();
    }

	Result AsyncQueue::Initialize(mem::Heap *heap, const AsyncQueueInfo *queue_info) {

        /* Integrity checks */
        RESULT_RETURN_UNLESS(0 < queue_info->priority_level_count, ResultRangeNG);
        RESULT_RETURN_UNLESS(0 < queue_info->thread_set_count, ResultRangeNG);

		/* Initialize arrays */
		RESULT_TRY(m_priority_level_array.Initialize(heap, queue_info->priority_level_count));
        auto fail_guard0 = SCOPE_GUARD {
            this->Finalize();
        };

        u32 thread_count = 0;
		RESULT_TRY(m_thread_set_array.Initialize(heap, queue_info->thread_set_count));
        for (u32 i = 0; i < queue_info->thread_set_count; ++i) {
            const u8 primary_count = queue_info->per_set_count_array[i].thread_count_primary;
            const u8 boost_count = queue_info->per_set_count_array[i].thread_count_boost;
            RESULT_RETURN_UNLESS(0 < primary_count, ResultRangeNG);
            RESULT_TRY(m_thread_set_array[i].thread_array_primary.Initialize(heap, primary_count));
            thread_count += primary_count;
            if (boost_count < 1) { continue; }
            RESULT_TRY(m_thread_set_array[i].thread_array_boost.Initialize(heap, boost_count));
            thread_count += boost_count;
        }

		RESULT_TRY(m_task_thread_array.Initialize(heap, thread_count));
		RESULT_TRY(m_thread_boost_array.Initialize(heap, thread_count));
        m_is_multi_set = (1 < queue_info->thread_set_count) || (0 < m_thread_set_array[0].thread_array_boost.GetMaxCount());

		/* Initialize events */
		for (PriorityLevel &priority_level : m_priority_level_array) {
			priority_level.priority_cleared_event.Initialize(sys::SignalState::Signaled, sys::ResetMode::Manual);
		}
		m_all_task_complete_event.Initialize(sys::SignalState::Cleared, sys::ResetMode::Manual);

        fail_guard0.Cancel();
		RESULT_RETURN_SUCCESS;
	}

	void AsyncQueue::Finalize() {

        for (ThreadSet &set : m_thread_set_array) {
            set.thread_array_primary.Finalize();
            set.thread_array_boost.Finalize();
        }
        m_thread_set_array.Finalize();
        m_thread_boost_array.Finalize();
		m_task_thread_array.Finalize();
		m_priority_level_array.Finalize();

		return;
	}

    void AsyncQueue::AddThread(AsyncQueueThread *thread, u32 thread_set, bool is_boost) {

        std::scoped_lock l(m_queue_mutex);

        /* Add thread */
        const u32 global_index = m_task_thread_array.PushPointer(thread);
        thread->m_queue = this;

        /* Link to thread boost */
        ThreadBoost *boost = m_thread_boost_array.Allocate();
        boost->set_index = thread_set;
        boost->is_runnable = !is_boost;
        boost->linked_thread = cInvalidThreadLink;
        boost->is_in_boost = false;

        /* Add to thread set */
        ThreadSet *set = std::addressof(m_thread_set_array[thread_set]);
        const u32 local_index = (is_boost) ? set->thread_array_boost.PushPointer(thread) : set->thread_array_primary.PushPointer(thread);

        /* Get other thread (primary if boost, boost if primary) */
        AsyncQueueThread *other_thread;
        if (is_boost && local_index < set->thread_array_primary.GetUsedCount()) {
            other_thread = set->thread_array_primary[local_index];  
        } else if (local_index < set->thread_array_boost.GetUsedCount()) {
            other_thread = set->thread_array_boost[local_index];  
        } else {
            return;
        }

        /* Link primary thread to boost thread if available, inherit primary core mask to boost thread */
        const u32 other_index = m_task_thread_array.IndexOf(other_thread);
        VP_ASSERT(other_index != AsyncQueueThreadArray::cInvalidEntryIndex);
        m_thread_boost_array[global_index]->linked_thread = other_index;
        m_thread_boost_array[other_index]->linked_thread = global_index;
        if (is_boost) {
            thread->SetCoreMask(m_task_thread_array[other_index]->GetCoreMask());
        } else {
            m_task_thread_array[other_index]->SetCoreMask(thread->GetCoreMask());
        }

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

	Result AsyncQueue::CancelTask(AsyncTask *task, bool is_wait_for_execution) {

        /* Ensure we were passed a task, and we are it's queue */
        RESULT_RETURN_UNLESS(task != nullptr, ResultNoTask);
        RESULT_RETURN_UNLESS(task->m_queue == this, ResultDifferentQueue);

		/* Try cancel task if running */
        bool is_success = false;
        bool is_change_set = false;
        u32 thread_set = AsyncQueue::cInvalidThreadLink;
		{
			std::scoped_lock l(m_queue_mutex);

            /* Ensure the task, is still pending */
            RESULT_RETURN_UNLESS(task->IsPending(), ResultIsNotPending);

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
                AsyncTask *next_task = (m_task_count < 1) ? nullptr : std::addressof(m_task_list.Back());
                task->m_queue_list_node.Unlink();
                --m_task_count;

                /* Cancel task */
                task->Cancel();
                this->UpdateCompletion();

                /* Update thread set if we just cancelled the highest priority task */
                if (next_task == task) {
                    is_change_set = this->UpdateThreadSetForCancelTask();
                    thread_set = m_current_thread_set;
                }
                is_success = true;

            } else if (is_wait_for_execution) {
                /* Cancel task while it's active */
                task->m_finish_event.Clear();
                task->CancelWhileActive();
            }
		}

        /* Change thread set */
        if (is_change_set) {
            AsyncQueueThread *current_thread = reinterpret_cast<AsyncQueueThread*>(sys::GetCurrentThread());
            this->WakeThreadSet(thread_set, current_thread);
        }

        /* Tell the application we succeeded in cancelling the task, it's executing, or we waited for it to execute */
        if (is_success) { RESULT_RETURN_SUCCESS; }
        RESULT_RETURN_IF(!is_wait_for_execution, ResultTaskExecuting);
        task->m_finish_event.Wait();
        return ResultTaskExecuted;
	}

	void AsyncQueue::CancelPriorityLevel(u32 priority) {

        bool has_priority;
        bool is_change_set = false;
        u32 thread_set = AsyncQueue::cInvalidThreadLink;
        {
            /* Lock thread */
            std::scoped_lock l(m_queue_mutex);

            /* Cancel all tasks on the priority level */
            AsyncTask *head = m_priority_level_array[priority].async_task_head;
            if (head != nullptr) {

                AsyncTaskList::iterator list_iter = m_task_list.IteratorTo(*head);
                AsyncTask *task_iter = nullptr;
                while (list_iter != m_task_list.end() && priority == (*list_iter).m_priority) {
                    task_iter = std::addressof(*list_iter);
                    ++list_iter;

                    task_iter->m_queue_list_node.Unlink();
                    --m_task_count;
                    task_iter->Cancel();
                }
            }
            m_priority_level_array[priority].async_task_head = nullptr;

            /* Clear priority event */
            has_priority = this->IsAnyThreadHaveTaskPriority(priority);
            if (has_priority) {
                m_priority_level_array[priority].priority_cleared_event.Clear();
            }

            this->UpdateCompletion();
            this->CancelThreadPriorityLevel(priority);

            /* Update thread set */
            is_change_set = this->UpdateThreadSetForCancelTask();
            thread_set = m_current_thread_set;
        }

        /* Change thread set */
        if (is_change_set) {
            AsyncQueueThread *current_thread = reinterpret_cast<AsyncQueueThread*>(sys::GetCurrentThread());
            this->WakeThreadSet(thread_set, current_thread);
        }

        /* Wait for priority event */
        if (has_priority) {
            m_priority_level_array[priority].priority_cleared_event.Wait();
        }

		return;
	}

    void AsyncQueue::Wait() {
        m_all_task_complete_event.Wait();
    }

    void AsyncQueue::WaitForPriorityLevel(u32 priority) {
        if (m_priority_level_array[priority].is_paused == true || m_priority_level_array[priority].async_task_head != nullptr) { return; }
        m_priority_level_array[priority].priority_cleared_event.Wait();
    }

    void AsyncQueue::ForceCalcSyncOnThread(AsyncQueueThread *thread, u32 up_to_priority) {

        /* Lock queue */
        std::scoped_lock l(m_queue_mutex);

        /* Calc tasks */
        for (AsyncTask &task : m_task_list) {
            if (task.m_priority < up_to_priority) { break; }
            task.CancelTask();
            task.InvokeSync(thread);
        }

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

    void AsyncQueue::AdjustPriority(AsyncTask *task, u32 priority) {

        /* Ensure same queue */
        if (task->m_queue != this) { return; }

        const u32 max_priority = m_priority_level_array.GetCount();
        const u32 priority_clamp = (max_priority < priority) ? max_priority : priority;
        
        /* Lock queue */
        std::scoped_lock l(m_queue_mutex);

        /* Ensure priority is changed */
        const u32 last_priority = task->m_priority;
        if (priority_clamp == last_priority) { return; }

        /* Ensure task is queued */
        if (!task->IsPending()) { return; }
        if (task->m_status != static_cast<u32>(AsyncTask::Status::Queued)) { return; }

        /* Save priority of highest priority task */
        VP_ASSERT(0 < m_task_count);
        AsyncTask *last_high_task = std::addressof(m_task_list.Back());
        const u32 last_high_priority = last_high_task->m_priority;

        /* Replace head task if head */
        AsyncTask *head = m_priority_level_array[last_priority].async_task_head;
        if (head == task) {
            AsyncTask *next_head = nullptr;
            if (head->m_queue_list_node.IsLinked()) {
                AsyncTask *next_task = AsyncQueue::AsyncTaskList::GetNext(std::addressof(head->m_queue_list_node));
                if (next_task->m_priority == last_priority) { next_head = next_task; }
            }
            m_priority_level_array[last_priority].async_task_head = next_head;
        }

        /* Unlink from queue */
        task->m_queue_list_node.Unlink();

        /* Find lowest populated priority level */
        u32 priority_iter = priority_clamp;
        do {
            priority_iter = priority_iter - 1;
        } while(priority_iter != cInvalidPriorityLevel && m_priority_level_array[priority_iter].async_task_head == nullptr);

        /* Insert into list at respective priority level */
        if (priority_iter == cInvalidPriorityLevel) {

            /* Insert as head since there are no other tasks */
            m_task_list.PushBack(*task);

            /* Clear priorty level event */
            m_priority_level_array[priority_clamp].priority_cleared_event.Clear();
        } else {

            /* Link next to back of current priority */
            m_priority_level_array[priority_iter].async_task_head->m_queue_list_node.LinkNext(std::addressof(task->m_queue_list_node));
        }

        /* Set priority level head */
        if (m_priority_level_array[priority_clamp].async_task_head == nullptr) {
            m_priority_level_array[priority_clamp].async_task_head = task;
        }

        /* Get highest priority post cancel */
        if (m_task_count < 1) { return; }
        AsyncTask *next_high_task = std::addressof(m_task_list.Back());
        const u32 next_high_priority = next_high_task->m_priority;

        /* Update thread set if the priority changed */
        if (last_high_priority == next_high_priority) { return; }
        const bool is_raise = (last_high_priority < next_high_priority);
        if (!this->UpdateThreadSetForAdjustPriority(is_raise)) { return; }

        /* Change thread set */
        AsyncQueueThread *thread = reinterpret_cast<AsyncQueueThread*>(sys::GetCurrentThread());
        this->WakeThreadSet(m_current_thread_set, thread);

        return;
    }
}
