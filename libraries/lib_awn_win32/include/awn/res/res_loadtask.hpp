#pragma once

namespace awn::res {

    struct LoadTaskPushInfo : public async::AsyncTaskPushInfo {
        ResourceBinder        *binder;
        const char            *file_path;
        AsyncResourceLoadInfo *async_load_info;
    };

	class LoadTask : public async::AsyncTaskForAllocator {
        public:
            struct LoadUserData {
                AsyncResourceLoadInfo  m_async_load_info;
                ResourceBinder        *m_resource_binder;
                ResourceBinder         m_archive_binder_ref;
                MaxPathString          m_file_path;
            };
		private:
            LoadUserData m_load_user_data;
        public:
            LoadTask() : AsyncTaskForAllocator(), m_load_user_data() {/*...*/}
            virtual ~LoadTask() override {/*...*/}

            virtual void OnFinishExecute() override {

                m_load_user_data.m_archive_binder_ref.Finalize();

                return;
            }
            virtual void FormatPushInfo(async::AsyncTaskPushInfo *load_task_push_info) override {

                /* Allocate file path storage */
                LoadTaskPushInfo *push_info = reinterpret_cast<LoadTaskPushInfo*>(load_task_push_info);
                m_load_user_data.m_file_path = push_info->file_path;

                /* Copy async load info */
                m_load_user_data.m_async_load_info = *push_info->async_load_info;

                /* Reference archive */
                if (push_info->async_load_info->archive_binder != nullptr) {                    
                    RESULT_ABORT_UNLESS(m_load_user_data.m_archive_binder_ref.ReferenceBinderSync(push_info->async_load_info->archive_binder));
                }

                /* Set user data */
                if (m_user_data != nullptr) {
                    m_user_data = std::addressof(m_load_user_data);
                }

                return;
            }
	};
}
