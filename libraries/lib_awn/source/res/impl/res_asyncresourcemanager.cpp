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

namespace awn::res::impl {

    AWN_SINGLETON_TRAITS_IMPL(AsyncResourceManager);

    void AsyncResourceManager::RequestAdjustReference(ResourceUnit *unit) {

        /* Get index of res unit */
        const u32 index = m_resource_unit_allocator.GetResourceUnitIndex(unit);

        /* Lock frame */
        std::scoped_lock l(m_adjust_ref_frame_cs);

        /* Decrement deferred ref count */
        const u32 frame_index = m_adjust_ref_frame_buffer.GetCurrentFrameIndex();
        const s32 new_count   = --unit->m_deferred_adjust_count[frame_index];

        /* Request ref change if out of ref */
        if (new_count != -1) { return; }

        /* Push to frame */
        vp::util::FrameArray<u16> *frame = m_adjust_ref_frame_buffer.GetCurrentFrameArray();
        frame->PushValue(index);

        return;
    }

    void AsyncResourceManager::RequestUnload(ResourceUnit *unit) {

        /* Get index of res unit */
        const u32 index = m_resource_unit_allocator.GetResourceUnitIndex(unit);

        /* Lock frame */
        std::scoped_lock l(m_unload_frame_cs);

        /* Push to frame */
        vp::util::FrameArray<u16> *frame = m_unload_frame_buffer.GetCurrentFrameArray();
        frame->PushValue(index);

        return;
    }

    void AsyncResourceManager::CancelUnloadRequest(ResourceUnit *unit) {

        /* Get index of res unit */
        const u32 index = m_resource_unit_allocator.GetResourceUnitIndex(unit);

        /* Lock frame */
        std::scoped_lock l(m_unload_frame_cs);

        /* Stub index in frame */
        vp::util::FrameArray<u16> *frame1 = m_unload_frame_buffer.GetFrameArray(1);
        const u32 count1 = frame1->GetUsedCount();
        for (u32 i = 0; i < count1; ++i) {
            if ((*frame1)[i] == index) {
                frame1->RemoveRange(i, 1);
                return;
            }
        }
        vp::util::FrameArray<u16> *frame0 = m_unload_frame_buffer.GetFrameArray(0);
        const u32 count0 = frame0->GetUsedCount();
        for (u32 i = 0; i < count0; ++i) {
            if ((*frame0)[i] == index) {
                frame0->RemoveRange(i, 1);
                return;
            }
        }

        return;
    }

    void AsyncResourceManager::RequestHeapAdjust(ResourceUnit *unit) {

        /* Get index of res unit */
        const u32 index = m_resource_unit_allocator.GetResourceUnitIndex(unit);

        /* Lock frame */
        std::scoped_lock l(m_heap_adjust_frame_cs);

        /* Push to frame */
        vp::util::FrameArray<u16> *frame = m_heap_adjust_frame_buffer.GetCurrentFrameArray();
        frame->PushValue(index);

        return;
    }

    void AsyncResourceManager::CancelHeapAdjustRequest(ResourceUnit *unit) {

        /* Get index of res unit */
        const u32 index = m_resource_unit_allocator.GetResourceUnitIndex(unit);

        /* Lock frame */
        std::scoped_lock l(m_heap_adjust_frame_cs);

        /* Stub index in frame */
        vp::util::FrameArray<u16> *frame1 = m_heap_adjust_frame_buffer.GetFrameArray(1);
        const u32 count1 = frame1->GetUsedCount();
        for (u32 i = 0; i < count1; ++i) {
            if ((*frame1)[i] == index) {
                frame1->RemoveRange(i, 1);
                return;
            }
        }
        vp::util::FrameArray<u16> *frame0 = m_heap_adjust_frame_buffer.GetFrameArray(0);
        const u32 count0 = frame0->GetUsedCount();
        for (u32 i = 0; i < count0; ++i) {
            if ((*frame0)[i] == index) {
                frame0->RemoveRange(i, 1);
                return;
            }
        }

        return;
    }

    void AsyncResourceManager::ProcessAdjustReference() {

        /* Ensure back ring has elements */
        const u32 frame_index = m_adjust_ref_frame_buffer.GetCurrentFrameIndex();
        vp::util::FrameArray<u16> *frame = m_adjust_ref_frame_buffer.GetFrameArray(frame_index);
        const u32 used_count = frame->GetUsedCount();
        if (used_count == 0) { return; }

        /* Advance frame */
        {
            std::scoped_lock l(m_adjust_ref_frame_cs);
            m_adjust_ref_frame_buffer.AdvanceFrameArray();
        }

        /* Adjust references for previous frame */
        for (const u16 &index : *frame) {

            /* Get resource unit */
            ResourceUnit *unit = m_resource_unit_allocator.GetResourceUnitByIndex(index);
            if (unit == nullptr) { continue; }

            /* Adjust the deferred ref count */
            unit->AdjustReferenceCount(unit->m_deferred_adjust_count[frame_index]);
            unit->m_deferred_adjust_count[frame_index] = 0;
            if (0 < unit->GetReferenceCount()) { continue; }

            /* Schedule unload for out of ref */
            this->RequestUnload(unit);
        }

        return;
    }

    void AsyncResourceManager::ProcessUnload() {

        /* Ensure back ring has elements */
        const u32 frame_index = m_adjust_ref_frame_buffer.GetCurrentFrameIndex();
        vp::util::FrameArray<u16> *frame = m_adjust_ref_frame_buffer.GetFrameArray(frame_index);
        const u32 used_count = frame->GetUsedCount();
        if (used_count == 0) { return; }

        /* Advance frame */
        {
            std::scoped_lock l(m_adjust_ref_frame_cs);
            m_adjust_ref_frame_buffer.AdvanceFrameArray();
        }

        /* Setup unload push info constants */
        async::AsyncTaskPushInfo push_info = {
            .queue_thread    = m_thread_memory,
            .task_function   = std::addressof(m_unload_exe),
            .result_function = std::addressof(m_unload_result),
            .priority        = 0xd,
        };
        /* Schedule unload operations for previous frame */
        for (const u16 &index : *frame) {

            /* Get resource unit */
            ResourceUnit *unit = m_resource_unit_allocator.GetResourceUnitByIndex(index);
            if (unit == nullptr) { continue; }

            /* Defer unload again while the unit is busy */
            if (!unit->IsUnloadable()) {
                this->RequestUnload(unit);
                continue;
            }

            /* Schedule unload */
            push_info.user_data = unit;
            RESULT_ABORT_UNLESS(unit->m_unload_task.PushTask(std::addressof(push_info)));
        }

        return;
    }

    void AsyncResourceManager::FinalizeResourceUnitSync(ResourceUnit *resource_unit) {

        /* Ensure freeable */
        if (resource_unit == nullptr) { return; }
        if (!resource_unit->m_state.GetBit(ResourceUnit::State_IsFreeableForMemoryManager)) { return; }
        if (resource_unit->m_status == ResourceUnit::Status::Freed) { return; }
        if (resource_unit->m_resource_unit_manager == nullptr) { return; }
        if (0 < resource_unit->GetReferenceCount()) { return; }
        if (resource_unit->m_state.GetBit(ResourceUnit::State_IsFinalized)) { return; }

        /* Clear cache sync */
        resource_unit->m_state.SetBit(ResourceUnit::State_IsFinalized);
        resource_unit->CancelUnload(this);
        resource_unit->UnregisterFromResourceUnitManager();
        resource_unit->ClearCache();

        /* Release archive binder */
        if (resource_unit->m_archive_resource != nullptr) {
            resource_unit->m_archive_resource = nullptr;
            resource_unit->m_archive_binder.Release();
        }

        /* Free to resource unit allocator */
        this->FreeResourceUnit(resource_unit);

        return;
    }

    void AsyncResourceManager::FreeResourceUnitFromBinder(ResourceBinder *binder) {

        /* Ensure binder has unit */
        ResourceUnit *unit = binder->m_resource_unit;
        if (unit == nullptr) { return; }

        /* Clear unit from binder */
        binder->m_resource_unit = nullptr;

        /* Release resource unit reference */
        this->RequestAdjustReference(unit);

        return;
    }

    ResourceUnit *AsyncResourceManager::AllocateResourceUnit() {

        /* Try to allocate a resource unit */
        ResourceUnit *resource_unit = m_resource_unit_allocator.Allocate();
        if (resource_unit != nullptr) { return resource_unit; }

        /* Force process cleanup tasks to free units */
        m_thread_memory->GetQueue()->ForceCalcSyncOnThread(m_thread_memory, 7);

        /* Try to allocate a resource unit */
        resource_unit = m_resource_unit_allocator.Allocate();
        if (resource_unit != nullptr) { return resource_unit; }

        /* Schedule force clear caches sync */
        async::AsyncTaskPushInfo push_info = {
            .queue_thread  = m_thread_memory,
            .task_function = std::addressof(m_clear_cache_for_allocate_exe),
            .user_data     = reinterpret_cast<void*>(100),
            .priority      = 0xc,
            .is_sync       = true,
        };
        RESULT_ABORT_UNLESS(m_task_for_clear_cache_for_allocate->PushTask(std::addressof(push_info)));

        /* Try to allocate a resource unit */
        resource_unit = m_resource_unit_allocator.Allocate();
        if (resource_unit != nullptr) { return resource_unit; }

        /* Force process cleanup tasks again to free units */
        m_thread_memory->GetQueue()->ForceCalcSyncOnThread(m_thread_memory, 7);

        /* Allocate a resource unit */
        resource_unit = m_resource_unit_allocator.Allocate();
        VP_ASSERT(resource_unit != nullptr);

        return resource_unit;
    }

    void AsyncResourceManager::FreeResourceUnit(ResourceUnit *resource_unit) {
        m_resource_unit_allocator.Free(resource_unit);
    }

    bool AsyncResourceManager::TryReferenceResourceUnit(ResourceUnit *resource_unit, LoadTask::LoadUserData *load_info) {

        if (resource_unit == nullptr) { return false; }
        /* Check cleanup for file not found and memory allocation error */
        if (resource_unit->m_state.GetBit(ResourceUnit::State_IsFileNotFound) || resource_unit->m_state.GetBit(ResourceUnit::State_IsMemoryAllocationFailure)) { resource_unit->FinalizeIfOutOfReference(); return false; }
        /* Check cleanup for finalization */
        if (resource_unit->m_state.GetBit(ResourceUnit::State_IsFinalized)) { resource_unit->UnregisterFromResourceUnitManager(); return false; }
        /* Check cleanup for uninitialized */
        if (resource_unit->m_status == ResourceUnit::Status::Uninitialized) { resource_unit->FinalizeIfOutOfReference(); return false; }

        /* Cancel unload as we want to reclaim the resource unit */
        resource_unit->CancelUnload(this);

        /* Fail if resource unit became inactive */
        if (resource_unit->m_finalize_async_res_mgr_list_node.IsLinked()) { resource_unit->UnregisterFromResourceUnitManager(); return false; }

        /* Update status */
        resource_unit->UpdateStatusForReference();

        /* Increment ref count */
        resource_unit->SetToBinder(load_info->m_resource_binder);

        /* Update priority if it's increased */
        const u32 priority = load_info->m_async_load_info.priority;
        resource_unit->ChangeLoadPriority(std::addressof(m_queue_memory), std::addressof(m_queue_load), priority);

        /* Reference unit */
        load_info->m_resource_binder->m_status = (resource_unit->m_reference_count == 1) ? ResourceBinder::Status::Reclaimed : ResourceBinder::Status::Referenced;

        return true;
    }

    void AsyncResourceManager::LoadControlExe(void *load_task_user_data_arg) {

        /* Get user data of load task */
        LoadTask::LoadUserData *load_info = reinterpret_cast<LoadTask::LoadUserData*>(load_task_user_data_arg);

        /* Adjust reference counts and schedule pending unloads */
        this->ProcessAdjustReference();
        this->ProcessUnload();

        /* Lookup resource manager */
        MaxExtensionString extension;
        vp::util::GetExtension(std::addressof(extension), std::addressof(load_info->m_file_path));
        ResourceUnitManager *unit_manager = m_extension_manager.GetResourceUnitManager(nullptr, extension.GetString());
        VP_ASSERT(unit_manager != nullptr);

        /* Try find referencable resource unit */
        ResourceUnit *ref_res_unit = unit_manager->FindResourceUnit(load_info->m_file_path.GetString());

        /* Try to reference resource unit */
        const bool result = this->TryReferenceResourceUnit(ref_res_unit, load_info);
        if (result) { return; }

        /* Acquire default resource factory for file type */
        ResourceFactoryBase *resource_factory = load_info->m_async_load_info.resource_factory;
        if (resource_factory == nullptr) {                    
            resource_factory = ResourceFactoryManager::GetInstance()->FindResourceFactory(extension.GetString());
            load_info->m_async_load_info.resource_factory = resource_factory;
        }

        /* Set file alignment to factory default if not provided */
        if (load_info->m_async_load_info.file_alignment == 0) {                    
            load_info->m_async_load_info.file_alignment = resource_factory->GetFileAlignment();
        }

        /* Set compression type to factory default if not provided */
        if (load_info->m_async_load_info.compression_type == static_cast<u32>(CompressionType::Auto)) {                    
            load_info->m_async_load_info.compression_type = static_cast<u32>(resource_factory->GetDefaultCompressionType());
        }

        /* Allocate resource unit */
        ResourceUnit *resource_unit = this->AllocateResourceUnit();
        VP_ASSERT(resource_unit != nullptr);

        /* Initialize resource unit */
        const bool is_res_initializable = resource_factory->IsInitializeOnCreate();
        const bool is_boost_compose = resource_factory->IsBoostCompose();
        const bool is_compose = (is_res_initializable && load_info->m_async_load_info.is_compose && this->IsComposeEnabled() && load_info->m_resource_binder != nullptr && load_info->m_async_load_info.resource_user_context != nullptr);
        ResourceUnitInfo unit_info = {
            .async_resource_info   = std::addressof(load_info->m_async_load_info),
            .resource_binder       = load_info->m_resource_binder,
            .resource_unit_manager = unit_manager,
            .file_path             = load_info->m_file_path.GetString(),
            .is_initialize_resource = is_res_initializable,
            .is_compose = is_compose,
            .is_boost_compose = is_boost_compose,
        };
        resource_unit->Initialize(std::addressof(unit_info));

        /* Add resource unit to resource manager */
        unit_manager->RegisterResourceUnit(resource_unit);

        /* Convert priority */
        const u32 convert_priority = (load_info->m_async_load_info.priority << 1) + resource_unit->m_state.GetBit(ResourceUnit::State_IsAllowArchiveReference);

        /* Push load task */
        LoadTaskPushInfo push_info = {
            {                
                .queue_thread    = m_thread_memory,
                .task_function   = std::addressof(m_load_memory_exe),
                .result_function = std::addressof(m_load_memory_result),
                .user_data       = resource_unit,
                .priority        = convert_priority,
            },
        };
        RESULT_ABORT_UNLESS(resource_unit->m_load_task.PushTask(std::addressof(push_info)));

        load_info->m_resource_binder->m_status = ResourceBinder::Status::InLoad;

        return;
    }

    void AsyncResourceManager::LoadMemoryExe(void *resource_unit) {
        reinterpret_cast<ResourceUnit*>(resource_unit)->LoadMemory();
    }

    Result AsyncResourceManager::LoadMemoryResult(async::TaskResultInvokeInfo *result_info) {
        ResourceUnit *resource_unit = reinterpret_cast<ResourceUnit*>(result_info->user_data);
        return resource_unit->LoadMemoryResult(result_info);
    }

    void AsyncResourceManager::ReserveUnloadExe([[maybe_unused]] void *arg) {
        /* Adjust reference counts and schedule pending unloads */
        this->ProcessAdjustReference();
        this->ProcessUnload();
    }

    void AsyncResourceManager::HeapAdjustExe(void *resource_unit) {
        reinterpret_cast<ResourceUnit*>(resource_unit)->HeapAdjust();
    }

    void AsyncResourceManager::LoadFileExe(void *resource_unit) {
        reinterpret_cast<ResourceUnit*>(resource_unit)->LoadFile();
    }

    Result AsyncResourceManager::LoadFileResult(async::TaskResultInvokeInfo *result_info) {
        ResourceUnit *resource_unit = reinterpret_cast<ResourceUnit*>(result_info->user_data);
        resource_unit->RequestComposeForAfter();
        RESULT_RETURN_SUCCESS;
    }

    void AsyncResourceManager::ComposeExe(void *resource_unit) {
        reinterpret_cast<ResourceUnit*>(resource_unit)->Compose();
    }

    void AsyncResourceManager::UnloadForSyncExe(void *resource_unit) {
        reinterpret_cast<ResourceUnit*>(resource_unit)->AdjustReferenceCount(-1);
    }

    Result AsyncResourceManager::UnloadForSyncResult(async::TaskResultInvokeInfo *result_info) {
        reinterpret_cast<ResourceUnit*>(result_info->user_data)->ScheduleUnload();
        RESULT_RETURN_SUCCESS;
    }

    void AsyncResourceManager::UnloadExe(void *resource_unit) {
        reinterpret_cast<ResourceUnit*>(resource_unit)->Unload();
    }

    void AsyncResourceManager::AddResourceUnitToFinalizeList(ResourceUnit *resource_unit) {
        std::scoped_lock l(m_finalize_list_cs);
        if (resource_unit->m_finalize_async_res_mgr_list_node.IsLinked() == true) { return; }
        m_resource_unit_finalize_list.PushBack(*resource_unit);
    }

    void AsyncResourceManager::RemoveResourceUnitFromFinalizeList(ResourceUnit *resource_unit) {
        std::scoped_lock l(m_finalize_list_cs);
        if (resource_unit->m_finalize_async_res_mgr_list_node.IsLinked() == false) { return; }
        m_resource_unit_finalize_list.Remove(*resource_unit);
    }

    Result AsyncResourceManager::UnloadResult(async::TaskResultInvokeInfo *result_info) {

        /* Get resource unit */
        ResourceUnit *resource_unit = reinterpret_cast<ResourceUnit*>(result_info->user_data);

        /* Free to allocator if already freed */
        if (resource_unit->m_status == ResourceUnit::Status::Freed) {                    
            return resource_unit->FreeToResourceUnitAllocator(result_info);
        }

        /* Rechedule unload if resource failed prefinalize */
        if (resource_unit->m_status == ResourceUnit::Status::FailedToPreFinalizeResource) {
            resource_unit->ScheduleUnload();
            return async::ResultRescheduled;
        }

        /* Skip finalize list for load or cache unload */
        if (!resource_unit->m_load_task.IsCancelled() && !resource_unit->m_load_task.IsUninitialized() 
            && resource_unit->m_state.GetBit(ResourceUnit::State_IsCacheUnload)
            && resource_unit->m_cache_unload_flags.GetBit(ResourceUnit::CacheUnloadFlag_IsValid)
            && resource_unit->m_cache_unload_no_error_flags.GetBit(ResourceUnit::CacheUnloadNoErrorFlag_IsValid)) { RESULT_RETURN_SUCCESS; }

        /* Add to finalize list */
        GetInstance()->AddResourceUnitToFinalizeList(resource_unit);

        RESULT_RETURN_SUCCESS;
    }

    void AsyncResourceManager::ClearCacheExe(void *resource_unit) {
        reinterpret_cast<ResourceUnit*>(resource_unit)->ClearCache();
    }

    Result AsyncResourceManager::ClearCacheResult(async::TaskResultInvokeInfo *result_info) {
        ResourceUnit *resource_unit = reinterpret_cast<ResourceUnit*>(result_info->user_data);
        return resource_unit->FreeToResourceUnitAllocator(result_info);
    }

    void AsyncResourceManager::CalculateExe([[maybe_unused]] void *arg) {

        /* Adjust reference counts and schedule pending unloads */
        this->ProcessAdjustReference();
        this->ProcessUnload();
        
        /* TODO; Process trigger clear all caches */
        if (m_is_trigger_clear_all_caches) {
            m_is_trigger_clear_all_caches = false;
            this->SuspendMemoryThread();
            //this->ClearAllCaches();
            this->ResumeMemoryThread();
        }

        /* TODO; schedule clear cache for finalize list */
        
        /* TODO; clear cache for keep alloc size */
        

        /* TODO; schedule heap adjust on memory */

        /* Calculate thread local archive manager */
        m_thread_local_archive_manager.Calculate();

        return;
    }

    void AsyncResourceManager::FinalizeBinder(ResourceBinder *binder) {

        /* Release resource unit */
        ResourceUnit *resource_unit = binder->m_resource_unit;
        if (resource_unit == nullptr) { return; }
        binder->m_resource_unit = nullptr;

        /* Push unload */
        this->RequestAdjustReference(resource_unit);

        return;
    }

    void AsyncResourceManager::FinalizeBinderSync(ResourceBinder *binder) {

        /* Release resource unit */
        ResourceUnit *resource_unit = binder->m_resource_unit;
        if (resource_unit == nullptr) { return; }
        binder->m_resource_unit = nullptr;

        /* Schedule unload async if we are the control thread */
        sys::ThreadBase *thread = sys::GetCurrentThread();
        if (thread == m_thread_control) {
            resource_unit->AdjustReferenceCount(-1);
            resource_unit->ScheduleUnload();
            return;
        }

        /* Schedule unload task for sync */
        LoadTaskPushInfo push_info = {
            async::AsyncTaskPushInfo {
                .queue_thread    = m_thread_control,
                .task_function   = std::addressof(m_unload_for_sync_exe),
                .result_function = std::addressof(m_unload_for_sync_result),
                .user_data       = resource_unit,
                .priority        = 5,
                .is_sync         = true,
            },
        };
        async::AsyncTaskScheduleInfo schedule_info = {
            .push_info = std::addressof(push_info),
        };
        RESULT_ABORT_UNLESS(m_async_task_allocator_load.ScheduleTask(std::addressof(schedule_info)));

        return;
    }

    Result AsyncResourceManager::TryLoadAsync(const char *path, ResourceBinder *binder, const AsyncResourceLoadInfo *in_load_info) {

        /* Format push info for async with copy of load info */
        AsyncResourceLoadInfo load_info = (in_load_info == nullptr) ? AsyncResourceLoadInfo{} : *in_load_info;
        LoadTaskPushInfo push_info = {
            async::AsyncTaskPushInfo {
                .queue_thread   = m_thread_control,
                .task_function  = std::addressof(m_load_control_exe),
                .is_sync        = false,
            },
            binder,
            path,
            std::addressof(load_info),
        };
        AsyncResourceLoadInfo *push_load_info = std::addressof(load_info);

        /* Try get thread local archive or default archive binder */
        bool is_default_archive = false;
        ON_SCOPE_EXIT {
            if (is_default_archive) { this->ReleaseDefaultArchive(); }
        };
        if (push_load_info->archive_binder == nullptr) {

            /* Attempt to get a thread local archive */
            ResourceBinder *archive_binder = m_thread_local_archive_manager.GetThreadLocalArchiveBinder();
            if (archive_binder == nullptr) { is_default_archive = this->AcquireDefaultArchive(std::addressof(archive_binder)); }
            push_load_info->archive_binder = archive_binder;
        }

        /* Ensure managed thread for managed reference */
        const bool is_persistant = this->IsManagedThread() & (push_load_info->is_persistant != 0);
        push_load_info->is_persistant = is_persistant;

        /* Boost priority by 1 if the thread local archive exists */
        const bool is_active_local_archive = m_thread_local_archive_manager.IsThreadLocalArchiveInReference();
        const u32  priority = push_load_info->priority + static_cast<u32>(is_active_local_archive);

        /* Schedule load async */
        push_info.priority = priority;
        async::AsyncTaskScheduleInfo schedule_info = {
            .task_for_allocator = nullptr,
            .push_info          = std::addressof(push_info),
            .watcher            = std::addressof(binder->m_watcher),
        };
        RESULT_ABORT_UNLESS(m_async_task_allocator_load.ScheduleTask(std::addressof(schedule_info)));

        RESULT_RETURN_SUCCESS;
    }

    Result AsyncResourceManager::TryLoadSync(const char *path, ResourceBinder *binder, const AsyncResourceLoadInfo *in_load_info) {

        /* Format push info for sync with copy of load info */
        AsyncResourceLoadInfo load_info = (in_load_info == nullptr) ? AsyncResourceLoadInfo{} : *in_load_info;
        LoadTaskPushInfo push_info = {
            async::AsyncTaskPushInfo {
                .queue_thread   = m_thread_control,
                .task_function  = std::addressof(m_load_control_exe),
                .is_sync        = true,
            },
            binder,
            path,
            std::addressof(load_info),
        };
        AsyncResourceLoadInfo *push_load_info = std::addressof(load_info);

        /* Get archive binder auto */
        bool is_default_archive = false;
        ON_SCOPE_EXIT {
            if (is_default_archive) { this->ReleaseDefaultArchive(); }
        };
        if (push_load_info->archive_binder == nullptr) {

            /* Attempt to get the thread local archive */
            ResourceBinder *archive_binder = m_thread_local_archive_manager.GetThreadLocalArchiveBinder();
            if (archive_binder == nullptr) { is_default_archive = this->AcquireDefaultArchive(std::addressof(archive_binder)); }
            push_load_info->archive_binder = archive_binder;
        }

        /* Ensure managed thread for managed reference */
        const bool is_persistant = this->IsManagedThread() & (push_load_info->is_persistant != 0);
        push_load_info->is_persistant = is_persistant;

        /* Check if thread local archive is active */
        const bool is_active_local_archive = m_thread_local_archive_manager.IsThreadLocalArchiveInReference();

        /* Check if loading threads are active */
        u32 priority = push_load_info->priority;
        if (is_active_local_archive) {

            /* Ensure a loading thread is not suspended */
            if (this->IsLoadThreadSuspended()) {
                binder->m_status = ResourceBinder::Status::RequiresReprocess;
                binder->Complete(push_load_info->resource_user_context);
                RESULT_RETURN(ResultInactiveLoadThread);
            }

            /* Force low priority for local archive */
            priority = 4;
        }

        /* Schedule load for sync */
        push_info.priority = priority;
        async::AsyncTaskScheduleInfo schedule_info = {
            .task_for_allocator = (this->IsCurrentThreadControlThread()) ? m_task_for_load_sync : nullptr,
            .push_info          = std::addressof(push_info),
            .watcher            = std::addressof(binder->m_watcher),
        };
        RESULT_ABORT_UNLESS(m_async_task_allocator_load.ScheduleTask(std::addressof(schedule_info)));

        /* Complete load */
        binder->WaitForLoad();
        binder->Complete(push_load_info->resource_user_context);

        RESULT_RETURN_SUCCESS;
    }

    Result AsyncResourceManager::ReferenceLocalArchiveBinder(ResourceBinder *binder) {

        /* Acquire thread local or default archive */
        ResourceBinder *local_archive = m_thread_local_archive_manager.GetThreadLocalArchiveBinder();
        bool is_default_archive = false;
        ON_SCOPE_EXIT {
            if (is_default_archive) {                        
                this->ReleaseDefaultArchive();
            }
        };
        if (local_archive == nullptr) {
            is_default_archive = this->AcquireDefaultArchive(std::addressof(local_archive));
            if (!is_default_archive) { binder->Release(); RESULT_RETURN(ResultNoLocalArchive); }
        }

        /* Reference binder */
        return binder->ReferenceBinderSync(local_archive);
    }

    void AsyncResourceManager::CancelLoadTasks() {
        m_thread_control->CancelPriorityLevel(0);
        m_thread_control->CancelPriorityLevel(1);
        m_thread_control->CancelPriorityLevel(2);
        m_thread_memory->CancelPriorityLevel(1);
        m_thread_memory->CancelPriorityLevel(2);
        m_thread_memory->CancelPriorityLevel(3);
        m_thread_memory->CancelPriorityLevel(4);
        m_thread_memory->CancelPriorityLevel(5);
        m_thread_memory->CancelPriorityLevel(6);
        m_queue_load.CancelPriorityLevel(0);
        m_queue_load.CancelPriorityLevel(1);
        m_queue_load.CancelPriorityLevel(2);
    }

    /* System api */
    void AsyncResourceManager::Initialize(mem::Heap *heap, AsyncResourceManagerInfo *manager_info) {

        /* Set application impl */
        IAsyncResourceApplicationImpl *app_impl = manager_info->app_impl;
        m_app_impl = app_impl;
        VP_ASSERT(app_impl != nullptr);

        /* Create system heap */
        mem::Heap *system_heap = mem::ExpHeap::TryCreate("awn::res::AsyncResourceManager", heap, 0, alignof(size_t), false);
        m_system_heap = system_heap;
        VP_ASSERT(system_heap != nullptr);

        /* Initialize memory manager */
        ResourceMemoryManagerInfo memory_manager_info = {
            .name              = "awn::res::ResourceMemoryManager_Main",
            .manager_heap_type = ManagerHeapType::VirtualAddressHeap,
        };
        m_memory_manager.Initialize(system_heap, std::addressof(memory_manager_info));

        /* Initialize extension manager with application extensions */
        ExtensionManagerInfo ext_mgr_info = {
            .extension_count = app_impl->GetExtensionArrayCount(),
            .extension_info_array = app_impl->GetExtensionArray(),
        };
        m_extension_manager.Initialize(system_heap, std::addressof(ext_mgr_info));

        /* Initialize thread local archive manager */
        m_thread_local_archive_manager.Initialize(system_heap, manager_info->max_thread_local_archive_count);

        /* Initialize resource unit allocator */
        VP_ASSERT(0 < manager_info->max_resource_unit_count);
        m_resource_unit_allocator.Initialize(system_heap, manager_info->max_resource_unit_count);

        /* Initialize frame buffers */
        m_unload_frame_buffer.Initialize(system_heap, manager_info->max_resource_unit_count);
        m_adjust_ref_frame_buffer.Initialize(system_heap, manager_info->max_resource_unit_count);
        m_heap_adjust_frame_buffer.Initialize(system_heap, manager_info->max_resource_unit_count);

        /* Initialize task allocator */
        auto load_task_create_lambda = []([[maybe_unused]] async::AsyncTaskAllocator*, mem::Heap *heap) { 
            return new (heap, alignof(LoadTask)) LoadTask(); 
        };
        auto load_task_create_function = vp::util::MakeLambdaFunction<async::AsyncTaskForAllocator*(async::AsyncTaskAllocator*, mem::Heap*)>(load_task_create_lambda);
        m_async_task_allocator_load.Initialize(system_heap, std::addressof(load_task_create_function), manager_info->load_task_count);

        /* Initialize control queue */
        const async::AsyncQueuePerSetCount per_set_count = {
            .thread_count_primary = 1,
        };
        const async::AsyncQueueInfo control_queue_info = {
            .priority_level_count = cControlPriorityLevelCount,
            .thread_set_count = 1,
            .per_set_count_array = std::addressof(per_set_count),
        };
        m_queue_control.Initialize(system_heap, std::addressof(control_queue_info));

        /* Initialize control thread with user settings */
        async::AsyncQueueThreadInfo control_queue_thread_info = {
            .queue = std::addressof(m_queue_control),
        };
        m_thread_control = (manager_info->control_thread_create_function.IsValid()) ? manager_info->control_thread_create_function.Invoke(system_heap, std::addressof(m_queue_control), std::addressof(manager_info->control_thread_info)) : new (system_heap, alignof(async::AsyncQueueThread)) async::AsyncQueueThread(std::addressof(control_queue_thread_info), manager_info->control_thread_info.name, system_heap, manager_info->control_thread_info.stack_size, manager_info->control_thread_info.priority);
        VP_ASSERT(m_thread_control != nullptr);
        VP_ASSERT(async::AsyncQueueThread::CheckRuntimeTypeInfoStatic(m_thread_control->GetRuntimeTypeInfo()) == true);

        m_thread_control->SetCoreMask((1 << (manager_info->control_thread_info.core_number & 0x3f)));
        m_thread_control->StartThread();

        /* Initialize memory queue */
        const async::AsyncQueueInfo memory_queue_info = {
            .priority_level_count = cMemoryPriorityLevelCount,
            .thread_set_count = 1,
            .per_set_count_array = std::addressof(per_set_count),
        };
        m_queue_memory.Initialize(system_heap, std::addressof(memory_queue_info));

        /* Initialize memory thread with user settings */
        async::AsyncQueueThreadInfo memory_queue_thread_info = {
            .queue = std::addressof(m_queue_memory),
        };
        m_thread_memory = (manager_info->memory_thread_create_function.IsValid() == true) ? manager_info->memory_thread_create_function.Invoke(system_heap, std::addressof(m_queue_memory), std::addressof(manager_info->memory_thread_info)) : new (system_heap, alignof(async::AsyncQueueThread)) async::AsyncQueueThread(std::addressof(memory_queue_thread_info), manager_info->memory_thread_info.name, system_heap, manager_info->memory_thread_info.stack_size, manager_info->memory_thread_info.priority);
        VP_ASSERT(m_thread_memory != nullptr);
        VP_ASSERT(async::AsyncQueueThread::CheckRuntimeTypeInfoStatic(m_thread_memory->GetRuntimeTypeInfo()));

        m_thread_memory->SetCoreMask((1 << (manager_info->memory_thread_info.core_number & 0x3f)));
        m_thread_memory->StartThread();

        /* Initialize load async queue with user thread count */
        const async::AsyncQueuePerSetCount per_set_count_load = {
            .thread_count_primary = static_cast<u8>(manager_info->load_thread_count),
        };
        const async::AsyncQueueInfo load_queue_info = {
            .priority_level_count = cLoadPriorityLevelCount,
            .thread_set_count = 1,
            .per_set_count_array = std::addressof(per_set_count_load),
        };
        m_queue_load.Initialize(system_heap, std::addressof(load_queue_info));

        /* Initialize load async threads with user thread settings */
        m_load_thread_array.Initialize(system_heap, manager_info->load_thread_count);
        if (manager_info->load_thread_create_function.IsValid()) {
            for (u32 i = 0; i < manager_info->load_thread_count; ++i) {
                async::AsyncQueueThread *thread = manager_info->load_thread_create_function.Invoke(system_heap, std::addressof(m_queue_load), std::addressof(manager_info->load_thread_info_array[i]));
                VP_ASSERT(thread != nullptr);
                VP_ASSERT(async::AsyncQueueThread::CheckRuntimeTypeInfoStatic(thread->GetRuntimeTypeInfo()));
                m_load_thread_array.PushPointer(thread);
    
                thread->SetCoreMask((1 << (manager_info->load_thread_info_array[i].core_number & 0x3f)));
                thread->StartThread();
            }
        } else {
            async::AsyncQueueThreadInfo memory_queue_thread_info = {
                .queue = std::addressof(m_queue_load),
            };
            for (u32 i = 0; i < manager_info->load_thread_count; ++i) {
                async::AsyncQueueThread *thread = new (system_heap, alignof(async::AsyncQueueThread)) async::AsyncQueueThread(std::addressof(memory_queue_thread_info), manager_info->load_thread_info_array[i].name, system_heap, manager_info->load_thread_info_array[i].stack_size, manager_info->load_thread_info_array[i].priority);
                VP_ASSERT(thread != nullptr);
                m_load_thread_array.PushPointer(thread);

                thread->SetCoreMask((1 << (manager_info->load_thread_info_array[i].core_number & 0x3f)));
                thread->StartThread();
            }
        }

        /* Initialize system tasks */
        m_task_for_calculate                = new (system_heap, alignof(LoadTask)) LoadTask();
        m_task_for_load_sync                = new (system_heap, alignof(LoadTask)) LoadTask();
        m_task_for_clear_cache_for_allocate = new (system_heap, alignof(async::AsyncTask)) async::AsyncTask();
        m_task_for_force_reserve_unload     = new (system_heap, alignof(async::AsyncTask)) async::AsyncTask();
        m_task_for_force_clear_all_caches   = new (system_heap, alignof(async::AsyncTask)) async::AsyncTask();

        /* Initialize decompressor manager with a decompressor set for each load thread */
        m_decompressor_manager.Initialize(system_heap, manager_info->load_thread_count);

        /* Create Zsdic Heap */
        if (manager_info->zsdic_heap_size != 0) {
            m_zsdic_heap = mem::FrameHeap::TryCreate("awn::res::ZsDicHeap", system_heap, manager_info->zsdic_heap_size, 0x20, false);
        }

        /* Initialize resource size table manager with main table path */
        m_resource_size_table_manager.Initialize(manager_info->resource_size_table_path);

        /* Adjust system heap */
        system_heap->AdjustHeap();

        return;
    }

    void AsyncResourceManager::Finalize() {

        /* Init guard */
        if (m_system_heap == nullptr) { return; }

        /* Cancel all tasks */
        this->SuspendControlThread();
        this->SuspendMemoryThread();
        this->SuspendLoadThreads();
        this->CancelLoadTasks();

        /* Free free frame buffer */
        m_unload_frame_buffer.Finalize();
        m_adjust_ref_frame_buffer.Finalize();
        m_heap_adjust_frame_buffer.Finalize();

        /* Finalize resource size table manager */
        m_resource_size_table_manager.Finalize();

        /* Resume threads for force clear cache */
        this->ResumeLoadThreads();
        this->ResumeMemoryThread();
        this->ResumeControlThread();
        this->ForceClearAllCaches();

        /* Finalize decompressor manager */
        m_decompressor_manager.Finalize();

        /* Finalize extension manager */
        m_extension_manager.Finalize();

        /* Delete system tasks */
        if (m_task_for_calculate != nullptr) {
            m_task_for_calculate->Finalize();
            delete m_task_for_calculate;
            m_task_for_calculate = nullptr;
        }
        if (m_task_for_load_sync != nullptr) {
            m_task_for_load_sync->Finalize();
            delete m_task_for_load_sync;
            m_task_for_load_sync = nullptr;
        }
        if (m_task_for_clear_cache_for_allocate != nullptr) {
            m_task_for_clear_cache_for_allocate->Finalize();
            delete m_task_for_clear_cache_for_allocate;
            m_task_for_clear_cache_for_allocate = nullptr;
        }
        if (m_task_for_force_reserve_unload != nullptr) {
            m_task_for_force_reserve_unload->Finalize();
            delete m_task_for_force_reserve_unload;
            m_task_for_force_reserve_unload = nullptr;
        }
        if (m_task_for_force_clear_all_caches != nullptr) {
            m_task_for_force_clear_all_caches->Finalize();
            delete m_task_for_force_clear_all_caches;
            m_task_for_force_clear_all_caches = nullptr;
        }

        /* Finalize resource unit allocator */
        m_resource_unit_allocator.Finalize();
        
        /* Finalize thread local archive manager */
        m_thread_local_archive_manager.Finalize();

        /* Finalize threads and queues */
        if (m_thread_control != nullptr) {
            delete m_thread_control;
            m_thread_control = nullptr;
        }
        if (m_thread_memory != nullptr) {
            delete m_thread_memory;
            m_thread_memory = nullptr;
        }
        for (u32 i = 0; i < m_load_thread_array.GetUsedCount(); ++i) {
            delete m_load_thread_array[i];
        }
        for (u32 i = 0; i < m_compose_thread_array.GetUsedCount(); ++i) {
            delete m_compose_thread_array[i];
        }
        m_compose_thread_array.Finalize();
        m_load_thread_array.Finalize();
        m_queue_control.Finalize();
        m_queue_memory.Finalize();
        m_queue_load.Finalize();

        /* Finalize memory manager */
        m_memory_manager.Finalize();

        /* Finalize zsdic heap */
        if (m_zsdic_heap != nullptr) {
            m_zsdic_heap->Finalize();
            delete m_zsdic_heap;
            m_zsdic_heap = nullptr;
        }

        /* Destroy manager heap */
        m_system_heap->Finalize();
        delete m_system_heap;
        m_system_heap = nullptr;

        return;
    }

    void AsyncResourceManager::Calculate() {

        /* Try schedule calc task */
        if (m_task_for_calculate->IsPending()) { return; }

        async::AsyncTaskPushInfo push_info = {
            .queue_thread  = m_thread_control,
            .task_function = std::addressof(m_calc_exe),
            .priority      = 7,
        };
        RESULT_ABORT_UNLESS(m_task_for_calculate->PushTask(std::addressof(push_info)));

        return;
    }

    void AsyncResourceManager::ForceClearAllCaches() {

        /* Wait for control thread to clear */
        m_queue_control.WaitForPriorityLevel(7);
        m_queue_control.WaitForPriorityLevel(6);
        m_queue_control.WaitForPriorityLevel(5);

        /* Force reserve unload */
        if (!m_task_for_force_reserve_unload->IsPending()) {
            async::AsyncTaskPushInfo push_info = {
                .queue_thread  = m_thread_control,
                .task_function = std::addressof(m_reserve_unload_exe),
                .priority      = 5,
                .is_sync       = true,
            };
            RESULT_ABORT_UNLESS(m_task_for_force_reserve_unload->PushTask(std::addressof(push_info)));
        }

        /* Clear resource memory manager */
        async::AsyncTaskPushInfo push_info = {
            .queue_thread  = m_thread_memory,
            .task_function = std::addressof(m_clear_cache_for_allocate_exe),
            .user_data     = reinterpret_cast<void*>(0xffff'ffff),
            .priority      = 0xd,
            .is_sync       = true,
        };
        RESULT_ABORT_UNLESS(m_task_for_force_clear_all_caches->PushTask(std::addressof(push_info)));

        return;
    }

    /* Thread api */
    void AsyncResourceManager::SetControlThreadPriority(u32 priority) { m_thread_control->SetPriority(priority); }

    void AsyncResourceManager::SetMemoryThreadPriority(u32 priority)  { m_thread_memory->SetPriority(priority); }

    void AsyncResourceManager::SetLoadThreadsPriority(u32 priority) {
        for (async::AsyncQueueThread *&thread : m_load_thread_array) {
            thread->SetPriority(priority); 
        }
    }

    void AsyncResourceManager::SetComposeThreadsPriority(u32 priority) {
        for (async::AsyncQueueThread *&thread : m_compose_thread_array) {
            thread->SetPriority(priority); 
        }
    }

    void AsyncResourceManager::SuspendControlThread() { m_thread_control->Suspend(); }

    void AsyncResourceManager::SuspendMemoryThread() { m_thread_memory->Suspend(); }

    void AsyncResourceManager::SuspendLoadThreads() { 
        for (async::AsyncQueueThread *&thread : m_load_thread_array) {
            thread->Suspend();
        }
    }

    void AsyncResourceManager::SuspendComposeThreads() { 
        for (async::AsyncQueueThread *&thread : m_compose_thread_array) {
            thread->Suspend();
        }
    }

    void AsyncResourceManager::ResumeControlThread() { m_thread_control->Resume(); }

    void AsyncResourceManager::ResumeMemoryThread() { m_thread_memory->Resume(); }

    void AsyncResourceManager::ResumeLoadThreads() { 
        for (async::AsyncQueueThread *&thread : m_load_thread_array) {
            thread->Resume();
        }
    }

    void AsyncResourceManager::ResumeComposeThreads() { 
        for (async::AsyncQueueThread *&thread : m_compose_thread_array) {
            thread->Resume();
        }
    }

    bool AsyncResourceManager::IsLoadThreadSuspended() {
        for (async::AsyncQueueThread *&thread : m_load_thread_array) {
            if (thread->IsSuspended()) { return true; }
        }
        return false;
    }

    bool AsyncResourceManager::IsComposeThreadSuspended() {
        for (async::AsyncQueueThread *&thread : m_compose_thread_array) {
            if (thread->IsSuspended()) { return true; }
        }
        return false;
    }

    bool AsyncResourceManager::IsCurrentThreadControlThread() {
        const sys::ThreadBase *current_thread = sys::GetCurrentThread();
        return current_thread == m_thread_control;
    }

    bool AsyncResourceManager::IsCurrentThreadMemoryThread() {
        const sys::ThreadBase *current_thread = sys::GetCurrentThread();
        return current_thread == m_thread_memory;
    }

    bool AsyncResourceManager::IsCurrentThreadLoadThread() {
        const sys::ThreadBase *current_thread = sys::GetCurrentThread();
        for (async::AsyncQueueThread *&thread : m_load_thread_array) {
            if (current_thread == thread) { return true; }
        }
        return false;
    }

    bool AsyncResourceManager::IsCurrentThreadComposeThread() {
        const sys::ThreadBase *current_thread = sys::GetCurrentThread();
        for (async::AsyncQueueThread *&thread : m_compose_thread_array) {
            if (current_thread == thread) { return true; }
        }
        return false;
    }

    ResourceMemoryManager *AsyncResourceManager::GetMemoryManager([[maybe_unused]] MaxPathString *path) {
        return std::addressof(m_memory_manager);
    }

    CompressionType AsyncResourceManager::GetCompressionTypeByExtension(MaxExtensionString *extension) {
        return m_extension_manager.GetCompressionExtension(nullptr, extension->GetString());
    }

    /* Archive api */
    bool AsyncResourceManager::CheckArchiveFileExists(ArchiveResource *archive_resource, MaxPathString *path) {
        if (archive_resource == nullptr) { return false; }
        return archive_resource->TryGetEntryIndex(path->GetString()) != ArchiveResource::cInvalidEntryIndex;
    }

    bool AsyncResourceManager::SetDefaultArchive(ResourceBinder *archive_binder) {

        /* Ensure archive binder */
        if (archive_binder == nullptr) { return false; }

        /* Fail if archive is already set */
        const bool is_loaded = m_default_archive_binder.IsLoaded();
        if (is_loaded == true) { return false; }

        /* Set archive binder */
        const Result result = m_default_archive_binder.ReferenceBinderSync(archive_binder);
        if (RESULT_FAILED(result)) { return false; }
        vp::util::InterlockedIncrement(std::addressof(m_default_archive_reference_count));

        return true;                
    }

    bool AsyncResourceManager::AcquireDefaultArchive(ResourceBinder **out_binder) {

        /* Increment ref count and ensure loaded */
        if (m_default_archive_reference_count == 0) { return false; }
        const u32 last_value = vp::util::InterlockedIncrement(std::addressof(m_default_archive_reference_count));
        if (last_value < 1 || m_default_archive_binder.IsLoaded() == false) { return false; }

        /* Acquire archive */
        *out_binder = std::addressof(m_default_archive_binder);

        return true;
    }

    void AsyncResourceManager::ReleaseDefaultArchive() {

        /* Decrement reference and finalize if out of ref */
        const u32 last_value = vp::util::InterlockedDecrement(std::addressof(m_default_archive_reference_count));
        if (last_value == 1) { m_default_archive_binder.Release(); }

        return;
    }

    /* Resource size api */
    void AsyncResourceManager::RegisterResourceSizeTable(mem::Heap *heap, void *rsizetable, u32 rsizetable_size) {
        m_resource_size_table_manager.RegisterResourceSizeTable(heap, rsizetable, rsizetable_size);
    }

    void AsyncResourceManager::ReleaseResourceSizeTables() { m_resource_size_table_manager.Clear(); }

    size_t AsyncResourceManager::LookupResourceSize(const char *path) {
        const size_t size = m_resource_size_table_manager.GetResourceSize(path);
        return size;
    }

    void AsyncResourceManager::PrepareSize(ResourceSizePrepareResult *out_result, const ResourceSizePrepareInfo *prepare_info) {
    
        /* Get alignment, minimum 0x20 */
        const s32 alignment_base = prepare_info->file_alignment;
        const s32 alignment      = (alignment_base < 0x20) ? 0x20 : alignment_base;

        /* Get file device from user, the default archive device if an archive is available, or either the user default device or system default device */
        FileDeviceBase *file_device = prepare_info->file_device;
        if (file_device == nullptr) {
            if (prepare_info->archive_resource == nullptr) {
                if (m_app_impl->IsDefaultFileDevice()) {
                    file_device = m_app_impl->GetDefaultFileDevice(this->GetDefaultFileDevice(), prepare_info->formatted_path, prepare_info->original_path);
                } else {
                    file_device = this->GetDefaultFileDevice();
                }
            } else {
                file_device = std::addressof(m_default_archive_file_device_for_size_prepare);
                out_result->is_default_archive_file_device = true;
            }
        }

        /* Set archive resource if we are using an archive device */
        if (ArchiveFileDevice::CheckRuntimeTypeInfoStatic(file_device->GetRuntimeTypeInfo())) { reinterpret_cast<ArchiveFileDevice*>(file_device)->SetArchiveResource(prepare_info->archive_resource); }

        /* Set resulting file device */
        out_result->file_device = file_device;

        /* Use user resource size if available */
        const size_t user_resource_size = prepare_info->user_resource_size;
        size_t       file_size          = 0;
        if (user_resource_size != 0) {
            const Result result = file_device->GetFileSize(std::addressof(file_size), prepare_info->formatted_path);
            if (RESULT_FAILED(result)) { return; }
            out_result->resource_size = user_resource_size;
            return;
        }

        /* Lookup resource size by table */
        const size_t resource_size = m_resource_size_table_manager.GetResourceSize(prepare_info->original_path);
        if (resource_size != ResourceSizeTableManager::cInvalidSize) {
            out_result->resource_size = resource_size + alignment + sizeof(size_t);
            return;
        }

        /* Get file size for fallback */
        if (prepare_info->compression_type != CompressionType::None) {
            
            /* Open file */
            FileHandle file_handle = {};
            ON_SCOPE_EXIT {
                file_handle.Close();
            };
            const Result open_result = file_device->OpenFile(std::addressof(file_handle), prepare_info->formatted_path, OpenMode::Read);
            if (RESULT_FAILED(open_result)) { return; }

            /* Read enough of file to parse compressed header */
            char compressed_data[0x20] = {};
            size_t size_read = 0;
            const Result read_result = file_device->ReadFile(compressed_data, std::addressof(size_read), std::addressof(file_handle), 0x20, 0);
            if (RESULT_FAILED(read_result)) { return; }

            /* Parse zstd header */
            if (prepare_info->compression_type == CompressionType::Zstandard) {
                const Result size_result = vp::codec::GetDecompressedSizeZstd(std::addressof(file_size), compressed_data, 0x20);
                if (RESULT_FAILED(size_result)) { return; }
            } else { return; }

        } else {            
            const Result result = file_device->GetFileSize(std::addressof(file_size), prepare_info->formatted_path);
            if (RESULT_FAILED(result)) { return; }
        }
        if (file_size == 0) { return; }

        /* Calculate resource size fallback */
        ResourceFactoryBase *resource_factory = prepare_info->resource_factory;
        const float  size_scale               = resource_factory->GetDefaultFallbackSizeScale();
        const float  scaled_size              = static_cast<float>(vp::util::AlignUp(file_size, 0x20)) * size_scale;
        const size_t extra_size               = resource_factory->GetDefaultExtraResourceSize();
        const size_t res_data_size            = resource_factory->GetResourceSize() + 0x740;

        out_result->resource_size =  static_cast<size_t>(scaled_size) + res_data_size + extra_size + (0.0f < scaled_size && static_cast<s32>(scaled_size) != scaled_size);

        return;
    }

    /* Zsdic api */
    //void AsyncResourceManager::ReleaseZsDic() {
    //
    //    /* Clear zsdic from decompressors */
    //    m_decompressor_manager.ClearZsDic();
    //
    //    /* Delete resources */
    //    for (ZsDicResource *&zsdic_res : m_zsdic_res_array) {
    //        delete zsdic_res->m_file;
    //        delete zsdic_res;
    //    }
    //
    //    /* Free resources array */
    //    m_zsdic_res_array.Finalize();
    //
    //    return;
    //}

    //void AsyncResourceManager::LoadZstandardDictionaryArchive(const char *zs_dic_path) {
    //
    //    /* Release existing zsdics */
    //    this->ReleaseZsDic();
    //
    //    /* Load zsdic archive */
    //    ResourceBinder binder;
    //    AsyncResourceLoadInfo load_info = {
    //        .is_allow_loose_files       = true,
    //        .is_allow_archive_reference = false,
    //        .is_allow_compression       = true,
    //        
    //    };
    //    RESULT_ABORT_UNLESS(binder.TryLoadSync(zs_dic_path, std::addressof(load_info)));
    //
    //    /* Create archive device */
    //    ArchiveResource   *zsdic_archive_res = binder.GetResource();
    //    ArchiveFileDevice  archive_device(zsdic_archive_res);
    //
    //    /* Load a copy of every zsdic */
    //    const u32 file_count = zsdic_archive_res->GetFileCount();
    //    m_zsdic_res_array.Initialize(m_manager_heap, file_count);
    //    for (u32 i = 0; i < file_count; ++i) {
    //
    //        /* Get file path by index */
    //        const char *file_path = nullptr;
    //        zsdic_archive_res->GetFilePathByIndex(std::addressof(file_path), i);
    //
    //        /* Load copy of zsdic file */
    //        ZsDicResource *dic_res = nullptr;
    //        ResourceLoadContext load_context = {
    //            .file_heap          = m_manager_heap,
    //            .file_device        = std::addressof(archive_device),
    //            .resource_heap      = m_manager_heap,
    //            .resource_alignment = 0x40,
    //        };
    //        RESULT_ABORT_UNLESS(res::LoadResource(std::addressof(dic_res), file_path, std::addressof(load_context)));
    //        m_zsdic_res_array.PushPointer(dic_res);
    //
    //        /* Add to compressor sets */
    //        m_decompressor_manager.AddZsDic(dic_res->GetZsDic());
    //    }
    //
    //    return;
    //}
}
