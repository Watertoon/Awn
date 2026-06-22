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
        const sys::CoreMask current_core_mask = (1ull << current_core);
        for (DependencyJobThread &thread : m_thread_array) {
            
            /* Skip unactive threads */
            const sys::CoreMask thread_core_mask = thread.GetCoreMask();
            if ((m_is_process_in_mainthread && thread_core_mask == current_core_mask) || (m_all_core_mask & thread_core_mask) == 0) { continue; }

            /* Add thread control to queue */
            m_queue->m_thread_control_array.PushPointer(std::addressof(thread.m_thread_control));

            /* Set active mask */
            m_active_core_mask |= thread_core_mask;
        }

        /* Clear thread control state */
        for (DependencyJobThread &thread : m_thread_array) {
            if (!thread.IsActive(m_active_core_mask)) { continue; }
            vp::util::MemoryBarrierInnerReadWrite();

            thread.m_thread_control.m_next_job = nullptr;
            thread.m_thread_control.m_out_of_jobs_event.Clear();
            thread.m_finish_event.Clear();
            thread.m_queue = m_queue;

            vp::util::MemoryBarrierInnerReadWrite();
        }

        return;
    }

    void DependencyJobThreadManager::StartThreads() {

        /* Start threads for job processing */
        for (DependencyJobThread &thread : m_thread_array) {
            if (!thread.IsActive(m_active_core_mask)) { continue; }

            vp::util::InterlockedOr(std::addressof(thread.m_thread_control.m_is_ready_to_exit), 1u);
            thread.SendMessage(1);
        }

        return;
    }

    void DependencyJobThreadManager::Dispatch() {

        /* Get current core number */
        const u32 current_core = sys::GetCurrentCoreNumber();

        /* Set mainthread state */
        if (m_is_process_in_mainthread) {

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

    Result DependencyJobThreadManager::Initialize(mem::Heap *heap, const DependencyJobThreadManagerInfo *info) {

        /* Initialize thread array */
        sys::CoreConnection core_connection;
        core_connection.SetBitMask(0, info->per_core_worker_core_mask);
        const u32 thread_count = core_connection.CountOneBits();
        RESULT_RETURN_UNLESS(0 < thread_count, ResultRangeNG);
        RESULT_RETURN_UNLESS(thread_count <= sys::cMaxCoreCount, ResultRangeNG);
        RESULT_TRY(m_thread_array.Initialize(heap, thread_count));
        auto fail_guard0 = SCOPE_GUARD { this->Finalize(); };

        /* Clear state */
        m_all_core_mask    = 0;
        m_active_core_mask = 0;

        /* Setup threads */
        vp::util::FixedString8<sys::cMaxThreadNameLength> thread_name;
        DependencyJobThreadInfo thread_info = {
            .name = thread_name.GetString(),
            .thread_manager = this,
            .stack_size = info->worker_stack_size,
            .priority = info->worker_priority,
        };
        auto iter = core_connection.begin();
        auto end = core_connection.end();
        for (DependencyJobThread &thread : m_thread_array) {
            const u32 core_id = *iter;
            ++iter;
            thread_name.Format("%s_Core%d", info->name, core_id);
            const sys::CoreMask bit = 1ull << core_id;
            RESULT_TRY(thread.Initialize(heap, std::addressof(thread_info)));
            thread.SetCoreMask(bit);
            thread.StartThread();
            m_all_core_mask |= bit;
        }

        fail_guard0.Cancel();
        RESULT_RETURN_SUCCESS;
    }

    void DependencyJobThreadManager::Finalize() {

        /* Exit threads */
        for (DependencyJobThread &thread : m_thread_array) {
            thread.ExitThread(sys::ExitType::Jam);
            std::destroy_at(std::addressof(thread));
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
        if (m_is_process_in_mainthread) {

            /* Try to abort any workers on the same core */
            const u32 current_core = sys::GetCurrentCoreNumber();
            const sys::CoreMask current_core_mask = (1ull << current_core);
            for (DependencyJobThread &thread : m_thread_array) {
                if (thread.GetCoreMask() != current_core_mask) { continue; }

                /* Signal thread control to exit */
                vp::util::InterlockedOrRelaxed(std::addressof(thread.m_thread_control.m_is_ready_to_exit), (1u << 1));
            }

            /* Update main core */
            m_main_thread_control.m_core_number = current_core;

            /* Process jobs */
            m_queue->Process(std::addressof(m_main_thread_control));
        }

        /* Wait for all threads to complete */
        for (DependencyJobThread &thread : m_thread_array) {
            if (!thread.IsActive(m_active_core_mask)) { continue; }
            thread.m_finish_event.Wait();
        }

        return;
    }
}
