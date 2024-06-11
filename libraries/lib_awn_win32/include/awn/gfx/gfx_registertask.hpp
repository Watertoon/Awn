#pragma once

namespace awn::gfx {

    struct RegisterTaskPushInfo : public async::AsyncTaskPushInfo {
        void                 *binder;
        void                 *res;
        const void           *gfx_info;
        MemoryPropertyFlags   memory_property_flags;
        u32                   copy_count;
        const void     *const*copy_memory_array;
    };

    class RegisterTask : public async::AsyncTaskForAllocator {
        public:
            struct UserData {
                void                *binder;
                void                *res;
                const void          *gfx_info;
                MemoryPropertyFlags  memory_property_flags;
                u32                  priority;
                u32                  copy_count;
                const void          *copy_memory_array[Context::cTargetMaxMipLevelCount];
            };
        private:
            UserData m_user_data;
        public:
            RegisterTask() : AsyncTaskForAllocator(), m_user_data() {/*...*/}
            virtual ~RegisterTask() override {/*...*/}

            virtual void FormatPushInfo(async::AsyncTaskPushInfo *register_task_push_info) override {

                RegisterTaskPushInfo *push_info = reinterpret_cast<RegisterTaskPushInfo*>(register_task_push_info);

                /* Set arguments */
                m_user_data.binder                = push_info->binder;
                m_user_data.res                   = push_info->res;
                m_user_data.gfx_info              = push_info->gfx_info;
                m_user_data.memory_property_flags = push_info->memory_property_flags;
                m_user_data.priority              = push_info->priority;
                m_user_data.copy_count            = push_info->copy_count;
                for (u32 i = 0; i < push_info->copy_count; ++i) {
                    m_user_data.copy_memory_array[i] = push_info->copy_memory_array[i];
                }

                /* Set user data */
                if (push_info->user_data == nullptr) {
                    push_info->user_data = std::addressof(m_user_data);
                }

                return;
            }
    };
}
