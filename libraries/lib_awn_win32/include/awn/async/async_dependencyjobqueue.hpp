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

    struct JobQueueNode;

    struct DependentJobLink {
        vp::util::IntrusiveListNode  node;
        JobQueueNode                *dependent;
    };

    struct JobQueueNode {
        using DependentJobList = vp::util::IntrusiveListTraits<DependentJobLink, &DependentJobLink::node>::List;

        vp::util::Job   *job;
        u16              core_number;
        u16              priority;
        bool             is_multi_run_complete_once;
        union {
            u32     multi_run_state;
            struct {
                u16 multi_run_count;
                u16 active_running_count;
            };
        };
        u32               parent_count;
        DependentJobList  dependent_list;

        constexpr void SetDefaults() {
            job             = nullptr;
            core_number     = cJobAnyCore;
            priority        = cJobNormalPriority;
            parent_count    = 0;
            multi_run_state = 1;
            dependent_list.Clear();
        }
    };

    class DependencyJobQueue;
    class DependencyJobThreadManager;

    class DependencyJobThreadControl {
        public:
            friend class DependencyJobQueue;
            friend class DependencyJobThreadManager;
        public:
            static constexpr size_t       cMaxLocalJobCount = 8;
            static constinit inline void *cIsBlocked        = reinterpret_cast<void*>(0x1);
        public:
            using LocalJobRingBuffer = vp::util::FixedRingBuffer<JobQueueNode*, cMaxLocalJobCount>;
        private:
            JobQueueNode       *m_next_job;
            LocalJobRingBuffer  m_local_job_ring;
            sys::Event          m_out_of_jobs_event;
            sys::Mutex          m_local_ring_mutex;
            u32                 m_core_number;
            u32                 m_is_ready_to_exit;
        public:
            constexpr  DependencyJobThreadControl() : m_next_job(), m_local_job_ring(), m_out_of_jobs_event(sys::SignalState::Cleared, sys::ResetMode::Auto), m_local_ring_mutex(), m_core_number(), m_is_ready_to_exit() {/*...*/}
            constexpr ~DependencyJobThreadControl() {/*...*/}

            void SetNextJobFromLocalRing();
    };

    struct DependencyJobQueueInfo {
        u16 max_job_count;
        u16 max_link_count;
    };

    class DependencyJobQueue {
        public:
            friend class DependencyJobThreadManager;
        public:
            using JobQueueNodeArray         = vp::util::HeapArray<JobQueueNode>;
            using DependentLinkArray        = vp::util::HeapArray<DependentJobLink>;
            using UsedJobQueueNodeArray     = vp::util::PointerArray<JobQueueNode>;
            using ThreadControlArray        = vp::util::PointerArray<DependencyJobThreadControl>;
            using JobQueueNodePriorityQueue = vp::util::PriorityQueue<JobQueueNode, &JobQueueNode::priority>;
        private:
            static constexpr u32 cRequiresWait = 0xffff'ffff;
            static constexpr u32 cContinue     = 0xffff'fffe;
        private:
            JobQueueNodeArray         m_job_queue_node_array;
            DependentLinkArray        m_dependent_link_array;
            u32                       m_used_dependent_link_count;
            union {
                u32 m_queue_flags;
                struct {
                    u32 m_is_runnable      : 1;
                    u32 m_is_ready_to_exit : 1;
                    u32 m_reserve0         : 30;
                };
            };
            UsedJobQueueNodeArray     m_used_job_queue_node_array;
            ThreadControlArray        m_thread_control_array;
            JobQueueNodePriorityQueue m_job_priority_queue;
            JobQueueNode              m_final_node;
            sys::Mutex                m_queue_mutex;
            u32                       m_primary_core_number;
        public:
            void RegisterDependency(JobQueueNode *parent, JobQueueNode *dependent);

            void ForceRemoveForCompleteOnce(JobQueueNode *queue_node);

            void SetReadyToExit();

            void RemoveDependencies(JobQueueNode *queue_node);

            void OnJobFinish(JobQueueNode *queue_node);

            void WaitForJob(DependencyJobThreadControl *thread_control);

            void QueueNextJobByCore(JobQueueNode *node);

            u32 ScheduleNextJob(JobQueueNode **out_queue_node, DependencyJobThreadControl *thread_control);

            u32 AcquireNextJob(JobQueueNode **out_queue_node, DependencyJobThreadControl *thread_control);
        public:
            constexpr  DependencyJobQueue() : m_job_queue_node_array(), m_dependent_link_array(), m_used_dependent_link_count(), m_queue_flags(), m_used_job_queue_node_array(), m_thread_control_array(), m_job_priority_queue(), m_final_node(), m_queue_mutex(), m_primary_core_number() {/*...*/}
            constexpr ~DependencyJobQueue() {/*...*/}

            void Initialize(mem::Heap *heap, const DependencyJobQueueInfo *queue_info);
            void Finalize();

            void BuildJobGraph(DependencyJobGraph *job_graph);
            void SetupRun();
            void Process(DependencyJobThreadControl *thread_control);

            void Clear();
    };
}
