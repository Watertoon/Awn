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

    void DependencyJobThreadControl::SetNextJobFromLocalRing() {

        /* Lock ring */
        std::scoped_lock l(m_local_ring_mutex);

        /* Pop a job */
        JobQueueNode *next_node = m_local_job_ring.RemoveFront();

        /* Set next job */
        vp::util::InterlockedExchange(std::addressof(m_next_job), next_node);

        return;
    }

    void DependencyJobQueue::RegisterDependency(JobQueueNode *parent, JobQueueNode *dependent) {

        /* Integrity check */
        VP_ASSERT(m_used_dependent_link_count < m_dependent_link_array.GetCount());
        DependentJobLink *dependent_link = std::addressof(m_dependent_link_array[m_used_dependent_link_count]);
        ++m_used_dependent_link_count;

        /* Set dependency relation */
        dependent_link->dependent = dependent;
        ++dependent->parent_count;

        parent->dependent_list.PushBack(*dependent_link);

        return;
    }

    void DependencyJobQueue::ForceRemoveForCompleteOnce(JobQueueNode *queue_node) {

        /* Lock queue */
        std::scoped_lock l(m_queue_mutex);

        /* Clear multi-run count */
        const u32 previous_state = vp::util::InterlockedAnd(std::addressof(queue_node->multi_run_state), 0xffff'0000u);

        /* Check if job is already removed */
        if ((previous_state & 0xffff) == 0) { return; }

        /* Remove job */
        JobQueueNode **iter = m_job_priority_queue.FindIterTo(queue_node);

        if (iter == nullptr) { return; }

        m_job_priority_queue.Remove(iter);

        return;
    }

    void DependencyJobQueue::SetReadyToExit() {

        /* Lock queue */
        std::scoped_lock l(m_queue_mutex);

        /* Set exit flag */
        ::InterlockedOr(reinterpret_cast<long int*>(std::addressof(m_queue_flags)), (1 << 1));

        /* Unblock idling threads */
        for (u32 i = 0; i < m_thread_control_array.GetUsedCount(); ++i) {

            /* Clear next job if blocked */
            JobQueueNode *last = nullptr;
            vp::util::InterlockedCompareExchange(std::addressof(last), std::addressof(m_thread_control_array[i]->m_next_job), reinterpret_cast<JobQueueNode*>(DependencyJobThreadControl::cIsBlocked), static_cast<JobQueueNode*>(nullptr));

            /* Signal wakeup */
            m_thread_control_array[i]->m_out_of_jobs_event.Signal();
        }

        return;
    }

    void DependencyJobQueue::RemoveDependencies(JobQueueNode *queue_node) {

        {
            /* Lock queue */
            std::scoped_lock l(m_queue_mutex);

            /* Visit dependent nodes */
            for (DependentJobLink &dep_link : queue_node->dependent_list) {

                /* Decrement parent count of child nodes */
                const u32 last_parent_count = vp::util::InterlockedDecrement(std::addressof(dep_link.dependent->parent_count));

                /* Insert into priority queue when out of parents */
                if (last_parent_count != 1) { continue; }
                m_job_priority_queue.Insert(dep_link.dependent);
            }
        }

        /* Clear dependent list */
        queue_node->parent_count = 0xffff'ffff;
        queue_node->dependent_list.Clear();

        if (queue_node != std::addressof(m_final_node)) { return; }

        /* If this node is the last, set exit */
        this->SetReadyToExit();

        return;
    }

    void DependencyJobQueue::OnJobFinish(JobQueueNode *queue_node) {

        /* Resolve multi run */
        if (queue_node->is_multi_run_complete_once == true && queue_node->multi_run_count != 0) {
            this->ForceRemoveForCompleteOnce(queue_node);
        }

        /* Decrement active run state */
        const u32 last_run_state = vp::util::InterlockedFetchSubtract(std::addressof(queue_node->multi_run_state), 0x1'0000u);

        /* Leave dependencies if the job can still run */
        if (last_run_state != 0x1'0000) { return; }

        /* Remove dependencies */
        this->RemoveDependencies(queue_node);

        return;
    }

    void DependencyJobQueue::WaitForJob(DependencyJobThreadControl *thread_control) {

        /* Sleep if mainthread */
        if (m_primary_core_number == sys::GetCurrentCoreNumber()) {
            sys::SleepThread(vp::util::c100MicroSeconds);
            return;
        }

        /* Set from null to blocked */
        JobQueueNode *original = nullptr;
        vp::util::InterlockedCompareExchange(std::addressof(original), std::addressof(thread_control->m_next_job), static_cast<JobQueueNode*>(nullptr), reinterpret_cast<JobQueueNode*>(DependencyJobThreadControl::cIsBlocked));

        /* Check if a job was queued */
        if (DependencyJobThreadControl::cIsBlocked < original) { return; }

        /* Attempt to set next job from local ring if null */
        if (nullptr == original) { 
            thread_control->SetNextJobFromLocalRing();
        }

        /* Check if a new job has been added */
        if (m_job_priority_queue.GetUsedCount() == 0 && thread_control->m_next_job == DependencyJobThreadControl::cIsBlocked) {

            /* Try clear wait bit */
            JobQueueNode *wait = nullptr;
            vp::util::InterlockedCompareExchange(std::addressof(wait), std::addressof(thread_control->m_next_job), reinterpret_cast<JobQueueNode*>(DependencyJobThreadControl::cIsBlocked), static_cast<JobQueueNode*>(nullptr));
            if (wait == DependencyJobThreadControl::cIsBlocked) { return; }
        }

        /* Wait */
        thread_control->m_out_of_jobs_event.Wait();

        return;
    }

    void DependencyJobQueue::QueueNextJobByCore(JobQueueNode *node) {

        /* Try to queue to the thread responsible */
        for (u32 i = 0; i < m_thread_control_array.GetUsedCount(); ++i) {
            if (m_thread_control_array[i]->m_core_number != node->core_number && m_thread_control_array[i]->m_is_ready_to_exit == false) { continue; }

            /* Try to atomicly swap next job if empty */
            JobQueueNode *original = m_thread_control_array[i]->m_next_job;
            while (original <= DependencyJobThreadControl::cIsBlocked) {

                JobQueueNode *swapped = nullptr;
                vp::util::InterlockedCompareExchange(std::addressof(swapped), std::addressof(m_thread_control_array[i]->m_next_job), node, original);
                if (swapped == original) {
                    if (swapped == DependencyJobThreadControl::cIsBlocked) { m_thread_control_array[i]->m_out_of_jobs_event.Signal(); }
                    return;
                }

                original = m_thread_control_array[i]->m_next_job;
            }

            /* Queue to local */
            std::scoped_lock l(m_thread_control_array[i]->m_local_ring_mutex);

            m_thread_control_array[i]->m_local_job_ring.Insert(node);

            break;
        }

        return;
    }

    u32 DependencyJobQueue::ScheduleNextJob(JobQueueNode **out_queue_node, DependencyJobThreadControl *thread_control) {

        /* Check if the next job is set */
        if (DependencyJobThreadControl::cIsBlocked < thread_control->m_next_job) {

            thread_control->SetNextJobFromLocalRing();

            *out_queue_node = thread_control->m_next_job;

            return m_job_priority_queue.GetUsedCount();
        }

        /* Lock queue */
        std::scoped_lock l(m_queue_mutex);

        /* Check jobs exist */
        if (m_job_priority_queue.GetUsedCount() == 0) { return cRequiresWait; }

        /* Peek the next job */
        JobQueueNode *next_node = m_job_priority_queue.Peek();

        /* Decrement multi-run, increment active reference */
        const u32 previous_state = vp::util::InterlockedFetchAdd(std::addressof(next_node->multi_run_state), 0xffffu);

        /* Check if the job needs to be removed */
        if ((previous_state & 0xffff) != 1) {

            *out_queue_node = next_node;

            return m_job_priority_queue.GetUsedCount(); 
        }

        /* Remove the job for scheduling */
        m_job_priority_queue.RemoveFront();

        /* Check this is a viable core */
        if (next_node->core_number == cJobAnyCore || next_node->core_number == thread_control->m_core_number) {

            *out_queue_node = next_node;

            return m_job_priority_queue.GetUsedCount();  
        }

        /* Pin job to another thread if this is the wrong core */
        this->QueueNextJobByCore(next_node);

        return cContinue;
    }

    u32 DependencyJobQueue::AcquireNextJob(JobQueueNode **out_queue_node, DependencyJobThreadControl *thread_control) {

        /* Acquire loop */
        while (m_is_ready_to_exit == false) {

            /* Schedule the next job */
            const u32 result = this->ScheduleNextJob(out_queue_node, thread_control);

            if (result != cRequiresWait && result != cContinue) { return result; }

            /* Wait for a job if none are available */
            this->WaitForJob(thread_control);
        }

        *out_queue_node = nullptr;

        return 0;
    }

    void DependencyJobQueue::Initialize(mem::Heap *heap, const DependencyJobQueueInfo *queue_info) {

        /* Initialize arrays */
        m_job_queue_node_array.Initialize(heap, queue_info->max_job_count);
        m_dependent_link_array.Initialize(heap, queue_info->max_link_count);
        m_used_job_queue_node_array.Initialize(heap, queue_info->max_job_count);
        m_thread_control_array.Initialize(heap, sys::GetCoreCount());
        m_job_priority_queue.Initialize(heap, queue_info->max_job_count);

        m_final_node.SetDefaults();

        return;
    }

    void DependencyJobQueue::Finalize() {

        /* Finalize arrays */
        m_job_queue_node_array.Finalize();
        m_dependent_link_array.Finalize();
        m_used_job_queue_node_array.Finalize();
        m_thread_control_array.Finalize();
        m_job_priority_queue.Finalize();

        m_final_node.SetDefaults();

        return;
    }

    void DependencyJobQueue::BuildJobGraph(DependencyJobGraph *job_graph) {

        /* Lock queue */
        std::scoped_lock l(m_queue_mutex);

        /* Add job nodes */
        const u32 base_register_id = m_used_job_queue_node_array.GetUsedCount();
        for (u32 i = 0; i < job_graph->m_job_node_allocator.GetUsedCount(); ++i) {

            /* Get graph and queue job nodes */
            JobGraphNode *graph_node     = job_graph->m_job_node_allocator[i];
            JobQueueNode *new_queue_node = std::addressof(m_job_queue_node_array[i]);

            /* Convert graph to queue node */
            new_queue_node->job             = graph_node->job;
            new_queue_node->core_number     = graph_node->core_number;
            new_queue_node->priority        = graph_node->priority;
            new_queue_node->multi_run_count = graph_node->multi_run_count;

            /* Add final node as a dependency */
            this->RegisterDependency(new_queue_node, std::addressof(m_final_node));

            /* Add to used job queue array */
            m_used_job_queue_node_array.PushPointer(new_queue_node);
        }

        /* Resolve dependencies */
        for (u32 i = 0; i < job_graph->m_register_link_allocator.GetUsedCount(); ++i) {

            /* Calculate queue node index */
            const u32 parent_index    = base_register_id + static_cast<u32>(job_graph->m_register_link_allocator[i]->parent_register_id);
            const u32 dependent_index = base_register_id + static_cast<u32>(job_graph->m_register_link_allocator[i]->dependent_register_id);

            JobQueueNode *parent_node    = std::addressof(m_job_queue_node_array[parent_index]);
            JobQueueNode *dependent_node = std::addressof(m_job_queue_node_array[dependent_index]);

            /* Register dependency */
            this->RegisterDependency(parent_node, dependent_node);
        }

        return;
    }

    void DependencyJobQueue::SetupRun() {

        /* Lock queue */
        std::scoped_lock l(m_queue_mutex);

        /* Add all non-dependents to priority queue */
        for (u32 i = 0; i < m_used_job_queue_node_array.GetUsedCount(); ++i) {
            if (0 < m_used_job_queue_node_array[i]->parent_count) { continue; }

            m_job_priority_queue.Insert(m_used_job_queue_node_array[i]);
        }

        return;
    }

    void DependencyJobQueue::Process(DependencyJobThreadControl *thread_control) {

        JobQueueNode *next_job = nullptr;
        for (;;) {

            /* Cleanup last job */
            if (next_job != nullptr) {
                this->OnJobFinish(next_job);
            }

            /* Check whether force exit */
            if (thread_control->m_is_ready_to_exit != 0) { break; }

            /* Find next job */
            u32 remaining_job_count = this->AcquireNextJob(std::addressof(next_job), thread_control);

            /* Wake any waiting workers to take care of remaining jobs */
            for (u32 i = 0; i < m_thread_control_array.GetUsedCount() && i < remaining_job_count; ++i) {
                if (m_thread_control_array[i]->m_is_ready_to_exit == true || m_thread_control_array[i]->m_next_job != DependencyJobThreadControl::cIsBlocked) { continue; }

                JobQueueNode *last_job = nullptr;
                vp::util::InterlockedCompareExchange(std::addressof(last_job), std::addressof(m_thread_control_array[i]->m_next_job), reinterpret_cast<JobQueueNode*>(DependencyJobThreadControl::cIsBlocked), static_cast<JobQueueNode*>(nullptr));

                if (last_job != DependencyJobThreadControl::cIsBlocked) { continue; }

                m_thread_control_array[i]->m_out_of_jobs_event.Signal();

                --remaining_job_count;
            }

            if (next_job == nullptr) { break; }

            /* Run job */
            next_job->job->Invoke();
        }

        /* Memory barrier */
        vp::util::MemoryBarrierReadWrite();

        return;
    }

    void DependencyJobQueue::Clear() {

        /* Clear queues */
        m_used_job_queue_node_array.Clear();
        m_job_priority_queue.Clear();
        m_final_node.SetDefaults();
        m_used_dependent_link_count = 0;

        return;
    }
}
