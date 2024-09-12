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

    using UserId     = u8;
    using RegisterId = u16;

    static constexpr u32 cJobAnyCore        = 0x41;
    static constexpr u32 cJobNormalPriority = 0x80;

    struct JobGraphNode {
        vp::util::Job *job;
        bool           is_runnable;
        bool           enable_user_id;
        UserId         user_id;
        bool           is_multi_run_complete_once;
        u16            multi_run_count;
        u16            core_number;
        u16            priority;
        RegisterId     register_id;
    };

    struct JobRegisterLink {
        RegisterId parent_register_id;
        RegisterId dependent_register_id;
    };
    struct JobUserLink {
        UserId parent_user_id;
        UserId dependent_user_id;
        bool  has_register_link;
    };

    struct JobGraphRegisterInfo {
        vp::util::Job *job;
        bool           is_runnable;
        bool           enable_user_id;
        UserId         user_id;
        bool           is_multi_run_complete_once;
        u16            multi_run_count;
        u16            core_number;
        u16            priority;
    };

    struct DependencyJobGraphInfo {
        u16 max_job_count;
        u16 max_link_count;
    };

    class DependencyJobQueue;

    class DependencyJobGraph {
        public:
            friend class DependencyJobQueue;
        public:
            static constexpr size_t cMaxUserIdCount    = 0x100;
            static constexpr u16    cInvalidRegisterId = 0xffff;
            static constexpr u16    cAllCores          = 0x41;
        public:
            using JobGraphNodeAllocator = vp::util::PointerArrayAllocator<JobGraphNode>;
            using RegisterLinkAllocator = vp::util::PointerArrayAllocator<JobRegisterLink>;
            using UserLinkAllocator     = vp::util::PointerArrayAllocator<JobUserLink>;
        private:
            JobGraphNodeAllocator m_job_node_allocator;
            RegisterLinkAllocator m_register_link_allocator;
            UserLinkAllocator     m_user_link_allocator;
            RegisterId            m_register_id_iter;
            RegisterId            m_user_id_to_register_id_map[cMaxUserIdCount];
        private:
            void ClearRegisterIds();
        public:
            constexpr  DependencyJobGraph() : m_job_node_allocator(), m_register_link_allocator(), m_user_link_allocator(), m_register_id_iter(), m_user_id_to_register_id_map() {/*...*/}
            constexpr ~DependencyJobGraph() {/*...*/}

            void Initialize(mem::Heap *heap, const DependencyJobGraphInfo *job_graph_info);
            void Finalize();

            RegisterId RegisterJob(JobGraphRegisterInfo *register_info);

            void RegisterDependency(RegisterId parent_register_id, RegisterId dependent_register_id);
            void RegisterDependencyByDependentUserId(RegisterId parent_register_id, UserId dependent_user_id);
            void RegisterDependencyByParentUserId(UserId parent_user_id, RegisterId dependent_register_id);
            void RegisterDependencyByUserId(UserId parent_user_id, UserId dependent_user_id);

            void Clear();
    };
}
