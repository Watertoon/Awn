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

namespace awn::res::impl {

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
                m_load_user_data.m_archive_binder_ref.Release();
            }

            virtual void FormatPushInfo(async::AsyncTaskPushInfo *load_task_push_info) override {

                /* Allocate file path storage */
                LoadTaskPushInfo *push_info = reinterpret_cast<LoadTaskPushInfo*>(load_task_push_info);
                m_load_user_data.m_file_path.SetString(push_info->file_path);

                /* Set resource binder */
                m_load_user_data.m_resource_binder = push_info->binder;

                /* Handle async load info */
                AsyncResourceLoadInfo *load_info = push_info->async_load_info;
                if (load_info != nullptr) {

                    /* Copy async load info */
                    m_load_user_data.m_async_load_info = *load_info;

                    /* Reference archive */
                    if (load_info->archive_binder != nullptr) {                    
                        RESULT_ABORT_UNLESS(m_load_user_data.m_archive_binder_ref.ReferenceBinderSync(load_info->archive_binder));
                    }
                }

                /* Set user data */
                if (m_user_data == nullptr) {
                    m_user_data = std::addressof(m_load_user_data);
                }

                return;
            }
	};
}
