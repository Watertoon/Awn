#pragma once

namespace awn::frm {

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
            void ClearRegisterIds() {

                /* Clear register ids */
                m_register_id_iter = 0;
                ::memset(m_user_id_to_register_id_map, 0xff, sizeof(RegisterId) * cMaxUserIdCount);

                return;
            }
        public:
            constexpr  DependencyJobGraph() : m_job_node_allocator(), m_register_link_allocator(), m_user_link_allocator(), m_register_id_iter(), m_user_id_to_register_id_map() {/*...*/}
            constexpr ~DependencyJobGraph() {/*...*/}

            void Initialize(mem::Heap *heap, const DependencyJobGraphInfo *job_graph_info) {

                /* Integrity checks */
                VP_ASSERT(job_graph_info != nullptr);
                VP_ASSERT(0 < job_graph_info->max_job_count && 0 < job_graph_info->max_link_count);

                /* Initialize allocators */
                m_job_node_allocator.Initialize(heap, job_graph_info->max_job_count);
                m_register_link_allocator.Initialize(heap, job_graph_info->max_link_count);
                m_user_link_allocator.Initialize(heap, job_graph_info->max_link_count);

                /* Clear register ids */
                this->ClearRegisterIds();

                return;
            }

            void Finalize() {

                m_job_node_allocator.Finalize();
                m_register_link_allocator.Finalize();
                m_user_link_allocator.Finalize();
            }

            RegisterId RegisterJob(JobGraphRegisterInfo *register_info) {

                /* Allocate new job node */
                JobGraphNode *new_node = m_job_node_allocator.Allocate();
                VP_ASSERT(new_node != nullptr);

                /* Set job node info */
                new_node->job                        = register_info->job;
                new_node->is_runnable                = register_info->is_runnable;
                new_node->enable_user_id             = register_info->enable_user_id;
                new_node->user_id                    = register_info->user_id;
                new_node->is_multi_run_complete_once = register_info->is_multi_run_complete_once;
                new_node->multi_run_count            = register_info->multi_run_count;
                new_node->core_number                = register_info->core_number;
                new_node->priority                   = register_info->priority;

                /* Generate register id */
                new_node->register_id = m_register_id_iter;
                ++m_register_id_iter;

                if (new_node->enable_user_id == false) { return new_node->register_id; }

                /* Set user id to register id map */
                const u16 user_id = new_node->user_id;
                m_user_id_to_register_id_map[user_id] = new_node->register_id;

                /* Try to resolve user dependencies */
                for (u32 i = 0; i < m_user_link_allocator.GetUsedCount(); ++i) {
                    if (m_user_link_allocator[i]->has_register_link == true || (m_user_link_allocator[i]->parent_user_id != user_id && m_user_link_allocator[i]->parent_user_id != user_id)) { continue; }

                    const u16 other_user_id = (m_user_link_allocator[i]->parent_user_id != user_id) ? m_user_link_allocator[i]->parent_user_id : m_user_link_allocator[i]->dependent_user_id;

                    if (m_user_id_to_register_id_map[other_user_id] != cInvalidRegisterId) { continue; }

                    this->RegisterDependency(m_user_id_to_register_id_map[m_user_link_allocator[i]->parent_user_id], m_user_id_to_register_id_map[m_user_link_allocator[i]->dependent_user_id]);
                    m_user_link_allocator[i]->has_register_link = true;
                }

                return new_node->register_id;
            }

            void RegisterDependency(RegisterId parent_register_id, RegisterId dependent_register_id) {

                /* Allocate new register link */
                JobRegisterLink *new_link = m_register_link_allocator.Allocate();
                VP_ASSERT(new_link != nullptr);

                /* Set register ids */
                new_link->parent_register_id    = parent_register_id;
                new_link->dependent_register_id = dependent_register_id;

                return;
            }

            void RegisterDependencyByDependentUserId(RegisterId parent_register_id, UserId dependent_user_id) {

                /* Convert dependent user id */
                const u16 dependent_register_id = m_user_id_to_register_id_map[dependent_user_id];
                VP_ASSERT(dependent_register_id != cInvalidRegisterId);

                /* Allocate new register link */
                JobRegisterLink *new_link = m_register_link_allocator.Allocate();
                VP_ASSERT(new_link != nullptr);

                /* Set register ids */
                new_link->parent_register_id    = parent_register_id;
                new_link->dependent_register_id = dependent_register_id;

                return;
            }
            void RegisterDependencyByParentUserId(UserId parent_user_id, RegisterId dependent_register_id) {

                /* Convert dependent user id */
                const u16 parent_register_id = m_user_id_to_register_id_map[parent_user_id];
                VP_ASSERT(parent_register_id != cInvalidRegisterId);

                /* Allocate new register link */
                JobRegisterLink *new_link = m_register_link_allocator.Allocate();
                VP_ASSERT(new_link != nullptr);

                /* Set register ids */
                new_link->parent_register_id    = parent_register_id;
                new_link->dependent_register_id = dependent_register_id;

                return;
            }

            void RegisterDependencyByUserId(UserId parent_user_id, UserId dependent_user_id) {

                /* Allocate new user link */
                JobUserLink *new_user_link = m_user_link_allocator.Allocate();
                VP_ASSERT(new_user_link != nullptr);

                /* Set user ids */
                new_user_link->parent_user_id = parent_user_id;
                new_user_link->dependent_user_id = dependent_user_id;

                /* Convert user ids to register ids */
                const u16 parent_register_id    = m_user_id_to_register_id_map[parent_user_id];
                const u16 dependent_register_id = m_user_id_to_register_id_map[dependent_user_id];

                /* Postpone registering if a register id slot is invalid*/
                if (parent_register_id    == cInvalidRegisterId) { return; }
                if (dependent_register_id == cInvalidRegisterId) { return; }

                /* Allocate new register link */
                JobRegisterLink *new_link = m_register_link_allocator.Allocate();
                VP_ASSERT(new_link != nullptr);

                /* Set register ids */
                new_link->parent_register_id    = parent_register_id;
                new_link->dependent_register_id = dependent_register_id;

                new_user_link->has_register_link = true;

                return;
            }

            void Clear() {

                /* Reset allocators */
                m_job_node_allocator.Clear();
                m_register_link_allocator.Clear();
                m_user_link_allocator.Clear();

                /* Reset register ids */
                this->ClearRegisterIds();

                return;
            }
    };
}
