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

    void DependencyJobThread::ThreadMain(size_t message) {

        /* Integrity checks */
        VP_ASSERT(message == 1);
        VP_ASSERT(m_queue != nullptr);

        /* Process jobs */
        m_queue->Process(std::addressof(m_thread_control));

        /* Signal completion */
        m_finish_event.Signal();

        return;
    }

    void DependencyJobThreadManager::BuildThreadControl() {

        /* Clear thread control state */
        m_queue->m_thread_control_array.Clear();
        m_active_core_mask = 0;

        /* Add thread controls to queue */
        const u32 current_core = sys::GetCurrentCoreNumber();
        for (u32 i = 0; i < m_thread_array.GetCount(); ++i) {
            
            /* Skip unactive threads */
            const u64 thread_core_mask = m_thread_array[i].thread->GetCoreMask();
            if ((m_is_process_in_mainthread == true && thread_core_mask == (1ull << current_core)) || (m_all_core_mask & thread_core_mask) == 0) { continue; }

            /* Add thread control to queue */
            m_queue->m_thread_control_array.PushPointer(std::addressof(m_thread_array[i].thread->m_thread_control));

            /* Set active mask */
            m_active_core_mask |= (1 << i);
        }

        /* Clear thread control state */
        for (u32 i = 0; i < m_thread_array.GetCount(); ++i) {
            if (((m_active_core_mask >> i) & 1) == 0) { continue; }
            vp::util::MemoryBarrierReadWrite();

            m_thread_array[i].thread->m_thread_control.m_next_job = nullptr;
            m_thread_array[i].thread->m_thread_control.m_out_of_jobs_event.Clear();
            m_thread_array[i].thread->m_finish_event.Clear();
            m_thread_array[i].thread->m_queue = m_queue;

            vp::util::MemoryBarrierReadWrite();
        }

        return;
    }

    void DependencyJobThreadManager::StartThreads() {

        /* Start threads for job processing */
        for (u32 i = 0; i < m_thread_array.GetCount(); ++i) {
            if (((m_active_core_mask >> i) & 1) == 0) { continue; }

            vp::util::InterlockedOr(std::addressof(m_thread_array[i].thread->m_thread_control.m_is_ready_to_exit), 1u);

            m_thread_array[i].thread->SendMessage(1);
        }

        return;
    }

    void DependencyJobThreadManager::Dispatch() {

        /* Get current core number */
        const u32 current_core = sys::GetCurrentCoreNumber();

        /* Set mainthread state */
        if (m_is_process_in_mainthread == true) {

            /* Setup main thread control */
            m_main_thread_control.m_core_number = current_core;
            m_main_thread_control.m_next_job    = nullptr;
            m_main_thread_control.m_out_of_jobs_event.Clear();

            /* Add main thread control to queue */
            m_queue->m_thread_control_array.PushPointer(std::addressof(m_main_thread_control));
        }
        if (m_all_core_mask == 0) { return; }

        /* Build thread controls */
        this->BuildThreadControl();

        /* Start threads */
        this->StartThreads();

        return;
    }

    void DependencyJobThreadManager::Initialize(mem::Heap *heap) {

        /* Initialize thread array */
        m_thread_array.Initialize(heap, sys::GetCoreCount());

        /* Clear state */
        m_all_core_mask    = 0;
        m_active_core_mask = 0;

        /* Setup threads */
        for (u32 i = 0; i < m_thread_array.GetCount(); ++i) {

            m_thread_array[i].thread_name.Format("DepWorker%d", i);
            m_thread_array[i].thread = new DependencyJobThread(m_thread_array[i].thread_name.GetString(), heap, 0x20000, sys::cPriorityNormal);

            m_thread_array[i].thread->SetCoreMask((1 << i));
            m_thread_array[i].thread->StartThread();

            m_all_core_mask |= (1 << i);
        }

        return;
    }

    void DependencyJobThreadManager::Finalize() {

        /* Exit threads */
        for (u32 i = 0; i < m_thread_array.GetCount(); ++i) {
            m_thread_array[i].thread->SendMessage(0);
            delete m_thread_array[i].thread;
        }
        m_thread_array.Finalize();

        return;
    }

    void DependencyJobThreadManager::SubmitGraph(DependencyJobQueue *job_queue, DependencyJobGraph *graph) {

        /* Build a new job graph */
        job_queue->Clear();
        job_queue->BuildJobGraph(graph);

        /* Set queue */
        m_queue = job_queue;

        /* Setup run */
        job_queue->SetupRun();

        /* Dispatch */
        this->Dispatch();

        return;
    }

    void DependencyJobThreadManager::FinishRun() {

        /* Integrity check */
        VP_ASSERT(m_queue != nullptr);

        /* Process on mainthread */
        if (m_is_process_in_mainthread == true) {

            const u32 current_core = sys::GetCurrentCoreNumber();

            /* Try to abort any workers on the same core */
            for (u32 i = 0 ; i < m_thread_array.GetCount(); ++i) {
                if (m_thread_array[i].thread->GetCoreMask() != (1ull << current_core)) { continue; }

                /* Signal thread control to exit */
                vp::util::InterlockedOr(std::addressof(m_thread_array[i].thread->m_thread_control.m_is_ready_to_exit), (1u << 1));
            }

            /* Update main core */
            m_main_thread_control.m_core_number = current_core;

            /* Process jobs */
            m_queue->Process(std::addressof(m_main_thread_control));
        }

        /* Wait for all threads to complete */
        for (u32 i = 0; i < m_thread_array.GetCount(); ++i) {
            if (((m_active_core_mask >> i) & 1) == 0) { continue; }
            m_thread_array[i].thread->m_finish_event.Wait();
        }

        return;
    }
}
