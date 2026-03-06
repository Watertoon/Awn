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

namespace awn::para::impl {

    void DocumentTask::FormatPushInfo(async::AsyncTaskPushInfo *push_info) {

        /* Cast push info */
        DocumentTaskPushInfo *document_push_info = reinterpret_cast<DocumentTaskPushInfo*>(push_info);

        /* Set document binder */
        m_user_data.document_binder = document_push_info->document_binder;

        /* Remove "Work/" in path */
        const char *path_no_work = AdvanceNoWorkExtension(document_push_info->file_path);
        m_user_data.file_path.SetString(path_no_work);

        /* Clobber previous extension with bgyml and attempt to load the file */
        const u32 index = m_user_data.file_path.LastIndexOfChar(0, '.');
        if (index != res::MaxPathString::cInvalidEntryIndex) {
            m_user_data.file_path.AppendAt(index + 1, "bgyml");
            res::AsyncResourceLoadInfo load_info = {
                .resource_factory = DocumentManager::GetInstance()->GetResourceFactory(),
            };
            RESULT_ABORT_UNLESS(m_user_data.bgyml_binder.TryLoadAsync(m_user_data.file_path.GetString(), std::addressof(load_info)));
        }

        /* Reference archive binder */
        if (document_push_info->archive_binder.IsComposed()) {
            RESULT_ABORT_UNLESS(m_user_data.archive_binder.ReferenceBinderSync(std::addressof(document_push_info->archive_binder)));
        }

        /* Set push info user data */
        push_info->user_data = std::addressof(m_user_data);

        return;
    }

    AWN_SINGLETON_TRAITS_IMPL(DocumentQueue);

    void DocumentQueue::Exe(void *user_data_base) {

        /* Cast to our Task user data*/
        DocumentTask::UserData *user_data = reinterpret_cast<DocumentTask::UserData*>(user_data_base);
        
        /* Set local archive of binder to exe thread */
        res::ScopedThreadLocalArchive local_archive_scope;
        if (user_data->archive_binder.IsComposed()) { local_archive_scope.SetArchive(std::addressof(user_data->archive_binder)); }
        
        /* Reference local archive if requested */
        if (user_data->is_reference_local_archive) { res::ReferenceThreadLocalArchive(); }

        /* Release thread local archive reference on exit */
        ON_SCOPE_EXIT {
            user_data->archive_binder.Release();
            user_data->bgyml_binder.Release();
            if (user_data->is_reference_local_archive) { res::ReleaseThreadLocalArchive(); } 
        };

        /* Parse out file name and */
        const u32 extension_offset = user_data->file_path.LastIndexOfChar(0, '.');
        const u32 class_offset = user_data->file_path.LastIndexOfChar(extension_offset + 1, '.');

        /* Split file path string to separate file and class names */
        char *file_path = user_data->file_path.m_string_array;
        file_path[class_offset] = '\0';
        file_path[extension_offset] = '\0';

        /* Get file name and class name */
        const char *file_name = file_path;
        const char *class_name = file_path + class_offset + 1;

        /* Wait for document to load */
        user_data->bgyml_binder.WaitForLoad();

        /* Try initialize resource */
        if (!user_data->bgyml_binder.IsLoaded()) {
            ResourceDocument::CreateInfo document_create_info(file_name, class_name);
            const bool result = user_data->bgyml_binder.Compose(std::addressof(document_create_info));
            VP_ASSERT(result);
        }

        /* Load document */
        {
            res::ScopedThreadLocalArchiveReference local_reference;
            user_data->document_binder->LoadDocument(file_name, class_name, user_data->is_allow_dummy);
        }

        return;
    }

    DocumentQueue::~DocumentQueue() {/*...*/}

    Result DocumentQueue::Initialize(mem::Heap *heap, u32 task_count) {

        /* Queue */
        async::AsyncQueuePerSetCount set_count = {
            .thread_count_primary = 1,
        };
        async::AsyncQueueInfo q_info = {
            .priority_level_count = 2,
            .thread_set_count = 1,
            .per_set_count_array = std::addressof(set_count),
        };
        RESULT_TRY(m_queue.Initialize(heap, std::addressof(q_info)));
        auto fail_guard0 = SCOPE_GUARD { this->Finalize(); };

        /* Thread */
        async::AsyncQueueThreadInfo t_info = {
            .name = "awn::para::DocumentQueue",
            .queue = std::addressof(m_queue),
            .stack_size = vp::util::c64KB + vp::util::c32KB,
            .priority = sys::cPriorityLow,
            .requests_per_yield = 1,
        };
        RESULT_TRY(m_task_thread.Initialize(heap, std::addressof(t_info)));
        m_task_thread.StartThread();

        /* Allocator */
        const auto create_lambda = [](async::AsyncTaskAllocator *allocator, mem::Heap *heap) -> async::AsyncTaskForAllocator* {
            VP_UNUSED(allocator);
            return new (heap, alignof(DocumentTask)) DocumentTask();
        };
        auto create_func = vp::util::MakeLambdaFunction<async::AsyncTaskForAllocator*(async::AsyncTaskAllocator*,mem::Heap*)>(create_lambda);
        RESULT_TRY(m_task_allocator.Initialize(heap, std::addressof(create_func), task_count));

        fail_guard0.Cancel();
        RESULT_RETURN_SUCCESS;
    }

    void DocumentQueue::Finalize() {
        m_queue.Wait();
        m_queue.Finalize();
        m_task_thread.ExitThread(sys::ExitType::Jam);
        m_task_thread.Finalize();
        m_task_allocator.Finalize();
    }

    Result DocumentQueue::LoadDocumentAsync(DocumentBinderAsync *binder, const char *file_path, bool is_allow_dummy) {

        /* Get local archive binder */
        DocumentTaskPushInfo push_info = {};
        const bool is_archive = res::ReferenceThreadLocalArchiveToBinder(std::addressof(push_info.archive_binder));

        /* Push task */
        push_info.queue = std::addressof(m_queue);
        push_info.priority = is_archive;
        push_info.task_function = std::addressof(m_exe_function);
        push_info.file_path = file_path;
        push_info.document_binder = std::addressof(binder->m_binder);
        push_info.is_allow_dummy = is_allow_dummy;
        push_info.is_reference_local_archive = is_archive;
        async::AsyncTaskScheduleInfo schedule_info = {
            .push_info = std::addressof(push_info),
            .watcher = std::addressof(binder->m_watcher),
        };
        return m_task_allocator.ScheduleTask(std::addressof(schedule_info));
    }
}
