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

    ResourceUnit::ResourceUnit() : m_state(), m_compression_flags(), m_cache_unload_flags(), m_cache_unload_no_error_flags(), m_is_part_of_resource_unit_mgr(), m_status(Status::Uninitialized), m_reference_count(), m_deferred_adjust_count{}, m_compose_reference_count(), m_resource_initialize_guard(), m_user_resource_size(), m_file_alignment(), m_resource_unit_manager(),
                     m_resource_unit_manager_tree_node(), m_file_path(), m_file_device(), m_resource_factory(), m_resource_user_context(), m_archive_binder(), m_archive_resource(), m_resource(), m_load_task(), m_compose_task(), m_unload_task(), 
                     m_resource_heap(), m_gpu_heap(), m_memory_manager(), m_finalize_async_res_mgr_list_node(), m_memory_manager_node(), m_memory_manager_free_cache_node(), m_status_update_event(sys::SignalState::Cleared, sys::ResetMode::Manual) 
    {/*...*/}

    ResourceUnit::~ResourceUnit() {

        /* Remove from resource unit manager */
        if (m_resource_unit_manager != nullptr) {
            m_resource_unit_manager->UnregisterResourceUnit(this);
            m_resource_unit_manager = nullptr;
        }
    }

    void ResourceUnit::DecrementComposeReferenceCount() {

        if (!m_compression_flags.GetBit(CompressionFlag_IsCompose) || m_reference_count != 0) { return; }

        /* Lock compose queue */
        sys::Mutex *queue_mutex = AsyncResourceManager::GetInstance()->m_queue_compose.GetQueueMutex();
        std::scoped_lock l(*queue_mutex);

        /* Decrement ref count */
        const s32 prev_count = vp::util::InterlockedFetchAdd(std::addressof(m_compose_reference_count), -1);

        /* Cancel compose or fixup ref */
        if (prev_count == 1) {
            m_compose_task.CancelTask();
        } else if (prev_count < 0) {
            m_compose_reference_count = -1;
        }

        return;
    }
    void ResourceUnit::AdjustReferenceCount(s32 adjust_amount) {

        /* Adjust ref */
        const s32 prev_count = vp::util::InterlockedFetchAdd(std::addressof(m_reference_count), adjust_amount);

        /* Fixup negative ref */
        if (m_reference_count == -1) {
            /* TODO; special ref */
            m_reference_count = 0;
        }

        /* Cancel compose */
        if (0 < prev_count) {
            this->DecrementComposeReferenceCount();
        }

        /* Release managed ref if last ref */
        if (this->GetReferenceCount() != 1) {            
            this->ReleasePersistantReference();
        }

        /* Unregister from unit manager if: */
        /* >Out of reference when */
        /* >Cache unload is not enabled or */
        /* >We had an error */
        if (this->GetReferenceCount() != 0) { return; }
        Status status;
        if (!m_state.GetBit(State_IsCacheUnload) 
            && !m_cache_unload_flags.GetBit(CacheUnloadFlag_IsValid)
            && !m_cache_unload_no_error_flags.GetBit(CacheUnloadNoErrorFlag_IsValid)
            && (status = m_status, (status != Status::FailedToPostInitializeResource && status != Status::FailedToInitializeResource && status != Status::Error))) { return; }
        this->UnregisterFromResourceUnitManager();
        m_cache_unload_flags.ClearBit(CacheUnloadFlag_IsValid);

        return;
    }

    void ResourceUnit::IncrementReference() {
        vp::util::InterlockedIncrement(std::addressof(m_reference_count));
        m_state.ClearBit(State_IsFreeableForMemoryManager);
    }

    void ResourceUnit::DecrementReference() {
        vp::util::InterlockedDecrement(std::addressof(m_reference_count));
    }

    void ResourceUnit::ReleaseTransient() {
        /* Release if transient */
        if (m_state.GetBit(State_IsTransient)) {
            m_state.ClearBit(State_IsTransient);
            AsyncResourceManager::GetInstance()->RequestAdjustReference(this);
        }
        return;
    }

    void ResourceUnit::ReleasePersistantReference() {
        if (!m_state.GetBit(State_IsPersistant)) { return; }
        this->DecrementReference();
        m_state.ClearBit(State_IsPersistant);
    }

    void ResourceUnit::SetToBinder(ResourceBinder *resource_binder) {
        resource_binder->m_resource_unit = this;
        this->IncrementReference();
    }

    void ResourceUnit::ScheduleClearCache() {
        AsyncResourceManager *manager = AsyncResourceManager::GetInstance();
        async::AsyncTaskPushInfo push_info = {
            .queue_thread    = manager->m_thread_memory,
            .task_function   = std::addressof(manager->m_clear_cache_exe),
            .result_function = std::addressof(manager->m_clear_cache_result),
            .user_data       = this,
            .priority        = 0xd,
        };
        RESULT_ABORT_UNLESS(m_unload_task.PushTask(std::addressof(push_info)));
    }

    void ResourceUnit::FinalizeIfOutOfReference() {

        /* Fail if not out of reference */
        if (0 < this->GetReferenceCount()) { return; }

        m_unload_task.Wait();
        AsyncResourceManager::GetInstance()->RemoveResourceUnitFromFinalizeList(this);
        this->UnregisterFromResourceUnitManager();

        this->ScheduleClearCache();
    }

    Result ResourceUnit::FreeToResourceUnitAllocator(awn::async::TaskResultInvokeInfo *result_info) {

        /* Schedule clear cache if failed */
        if (m_status == Status::FailedToPreFinalizeResource) { 
            if (result_info->is_cancelled == true) { RESULT_RETURN_SUCCESS; } 
            this->ScheduleClearCache(); 
            RESULT_RETURN(async::ResultRescheduled);
        }

        /* Release archive binder */
        if (m_archive_resource != nullptr) {
            m_archive_resource = nullptr;
            m_archive_binder.Release();
        }

        /* Free to resource unit allocator */
        AsyncResourceManager::GetInstance()->FreeResourceUnit(this);

        RESULT_RETURN_SUCCESS;
    }

    void ResourceUnit::ChangeLoadPriority(async::AsyncQueue *memory_queue, async::AsyncQueue *load_queue, u32 priority) {

        /* Priority can only be raised */
        if (priority <= m_load_task.GetPriority()) { return; }

        /* lock load queue */
        std::scoped_lock l(*load_queue->GetQueueMutex());

        /* Convert priority depending on queue */
        async::AsyncQueue *queue = m_load_task.GetQueue();
        if (queue == memory_queue) {
            priority = (priority << 1) + m_state.GetBit(State_IsAllowArchiveReference);                    
        } else if (queue != load_queue) { return; }

        /* Change priority */
        m_load_task.AdjustPriority(priority);

        return;
    }

    void ResourceUnit::WaitForLoad() {
        const bool is_compose_thread = AsyncResourceManager::GetInstance()->IsCurrentThreadComposeThread();
        m_load_task.Wait();
        if (is_compose_thread) {
            m_compose_task.CancelTask();
            return;
        }
        m_compose_task.Wait();
    }

    bool ResourceUnit::IsInLoad() const {
        /* Check load task is pending */
        if (m_load_task.IsPending()) { return true; }
        /* Loading completed if we are the compose thread */
        if (AsyncResourceManager::GetInstance()->IsCurrentThreadComposeThread()) { return false; }
        /* Loading completed if the load task was cancelled */
        if (m_load_task.IsCancelled()) { return false; }
        /* Loading is not complete if we are composing, not the compose thread, and the compose task is pending */
        if (m_compression_flags.GetBit(CompressionFlag_IsCompose)) { return m_compose_task.IsPending(); }
        /* Loading is complete if the resource unit is not InLoad */
        return m_status == Status::InLoad;
    }

    bool ResourceUnit::IsUnloadable() const {
        /* We are unload safe if not in compose */
        if (!m_compose_task.IsPending()) { return true; }
        /* We are unload safe if we are the compose thread and in post execute */
        if (m_compose_task.IsExecutingOnCurrentThread()) { return true; }
        return false;
    }

    void ResourceUnit::FreeResourceHeap() {
        if (m_resource_heap  == nullptr) { return; }
        if (m_memory_manager == nullptr) { return; }
        m_memory_manager->FreeHeap(m_resource_heap, nullptr, this);
    }

    void ResourceUnit::RequestHeapAdjust() {

        /* If we use an external heap, skip adjusting altogether */
        if (m_state.GetBit(State_IsExternalHeap)) { 
            m_state.SetBit(State_IsHeapAdjusted);
            return;
        }

        /* Update memory usage sync if no heap adjust is required */
        if (m_state.GetBit(State_IsHeapAdjusted) || !m_state.GetBit(State_IsRequireHeapAdjust)) {
            this->UpdateMemoryUsage();
            return;
        }

        /* Schedule a heap adjust */
        AsyncResourceManager *manager = AsyncResourceManager::GetInstance();
        manager->RequestHeapAdjust(this);

        return;
    }

    void ResourceUnit::UpdateMemoryUsage() {

        /* Track global */
        if (!m_state.GetBit(State_IsTrackedMemoryGlobal)) {
            m_memory_manager->TrackMemoryUsageGlobal(m_resource_heap);
            m_state.SetBit(State_IsTrackedMemoryGlobal);
        }

        /* Track active */
        if (!m_state.GetBit(State_IsTrackedMemoryActive) && !m_state.GetBit(State_IsFreeableForMemoryManager)) {
            m_memory_manager->TrackMemoryUsageActive(m_resource_heap);
            m_state.SetBit(State_IsTrackedMemoryActive);
        }

        return;
    }

    void ResourceUnit::ReleaseMemoryUsageGlobal() {

        /* Release global */
        if (m_state.GetBit(State_IsTrackedMemoryGlobal)) {
            m_memory_manager->ReleaseMemoryUsageGlobal(m_resource_heap);
            m_state.ClearBit(State_IsTrackedMemoryGlobal);
        }

        return;
    }
    void ResourceUnit::ReleaseMemoryUsageActive() {

        /* Release active */
        if (m_state.GetBit(State_IsTrackedMemoryActive)) {
            m_memory_manager->ReleaseMemoryUsageActive(m_resource_heap);
            m_state.ClearBit(State_IsTrackedMemoryActive);
        }

        return;
    }

    Result ResourceUnit::HeapAdjust() {

        /* Succeed if no heap  */
        if (m_resource_heap == nullptr) { RESULT_RETURN_SUCCESS; }

        /* Adjust resource heap */
        m_resource_heap->AdjustHeap();

        /* Update resource memory manager */
        this->UpdateMemoryUsage();

        RESULT_RETURN_SUCCESS;
    }

    void ResourceUnit::PrepareSizeImpl(ResourceSizePrepareResult *prepare_result, const ResourceSizePrepareInfo *prepare_info) {

        /* Manager prepare */
        AsyncResourceManager *manager = AsyncResourceManager::GetInstance();
        manager->PrepareSize(prepare_result, prepare_info);

        /* Fail if no size */
        if (prepare_result->resource_size == 0) { return; }

        /* Set file device */
        m_file_device = (prepare_result->is_default_archive_file_device) ? nullptr : prepare_result->file_device;

        return;
    }

    size_t ResourceUnit::PrepareResourceLoad() {
        
        /* Format compression extension */
        const CompressionType compression_type = this->GetCompressionType();
        MaxPathString path_with_compression;
        FormatPathWithCompressionExtension(std::addressof(path_with_compression), std::addressof(m_file_path), compression_type);

        /* Lookup resource size */
        ResourceSizePrepareInfo prepare_info = {
            .user_resource_size = m_user_resource_size,
            .file_alignment     = m_file_alignment,
            .archive_resource   = m_archive_resource,
            .file_device        = m_file_device,
            .resource_factory   = m_resource_factory,
            .compression_type   = compression_type,
            .formatted_path     = path_with_compression.GetString(),
            .original_path      = m_file_path.GetString(),
        };
        ResourceSizePrepareResult prepare_result = {};
        this->PrepareSizeImpl(std::addressof(prepare_result), std::addressof(prepare_info));

        /* Perform fallback lookups */
        AsyncResourceManager *manager = AsyncResourceManager::GetInstance();
        if (prepare_result.resource_size == 0) {

            /* Nothing else to try if no archive or compresion */
            const CompressionType comp_type = this->GetCompressionType();
            if (!m_state.GetBit(State_IsAllowArchiveReference) && comp_type == CompressionType::None) { return 0; }

            /* Get auto compression type */
            MaxExtensionString extension;
            vp::util::GetExtension(std::addressof(extension), std::addressof(m_file_path));
            const CompressionType auto_comp_type = manager->GetCompressionTypeByExtension(std::addressof(extension));

            /* Try archive */
            if (m_archive_resource != nullptr && m_state.GetBit(State_IsAllowArchiveReference)) {

                /* Try archive with auto compression */
                if (auto_comp_type != CompressionType::None && comp_type == CompressionType::None) {
                    FormatPathWithCompressionExtension(std::addressof(path_with_compression), std::addressof(m_file_path), auto_comp_type);
                    prepare_info.formatted_path = path_with_compression.GetString();
                    this->PrepareSizeImpl(std::addressof(prepare_result), std::addressof(prepare_info));
                    if (prepare_result.resource_size != 0) {
                        this->SetCompressionType(auto_comp_type);
                        return prepare_result.resource_size;
                    }
                }

                /* Try archive no compression */
                if (comp_type != CompressionType::None) {
                    prepare_info.formatted_path = m_file_path.GetString();
                    this->PrepareSizeImpl(std::addressof(prepare_result), std::addressof(prepare_info));
                    if (prepare_result.resource_size != 0) {
                        this->SetCompressionType(CompressionType::None);
                        return prepare_result.resource_size;
                    }
                }

                /* Release archive since we failed to find the file in it */
                m_archive_resource = nullptr;
                m_archive_binder.Release();
                m_state.ClearBit(State_IsAllowArchiveReference);
            }

            /* Try loose auto compression */
            if (auto_comp_type != CompressionType::None && comp_type == CompressionType::None) {
                FormatPathWithCompressionExtension(std::addressof(path_with_compression), std::addressof(m_file_path), auto_comp_type);
                prepare_info.formatted_path = path_with_compression.GetString();
                this->PrepareSizeImpl(std::addressof(prepare_result), std::addressof(prepare_info));
                if (prepare_result.resource_size != 0) {
                    this->SetCompressionType(auto_comp_type);
                    return prepare_result.resource_size;
                }
            }

            /* Try loose no compression */
            if (comp_type != CompressionType::None) {
                prepare_info.formatted_path = m_file_path.GetString();
                this->PrepareSizeImpl(std::addressof(prepare_result), std::addressof(prepare_info));
                if (prepare_result.resource_size != 0) {
                    this->SetCompressionType(CompressionType::None);
                    return prepare_result.resource_size;
                }
            }

            return 0;
        }

        /* Finish if no archive binder */
        if (m_state.GetBit(State_IsAllowArchiveReference)) { return prepare_result.resource_size; }

        /* Check archive file exists */
        const bool is_archive_file = manager->CheckArchiveFileExists(m_archive_resource, std::addressof(path_with_compression));
        if (is_archive_file) { return prepare_result.resource_size; }

        /* Check existance again but without compression extension */
        const bool is_no_compress_archive_file = manager->CheckArchiveFileExists(m_archive_resource, std::addressof(m_file_path));
        if (is_no_compress_archive_file) {
            this->SetCompressionType(CompressionType::None);
            return prepare_result.resource_size;
        }

        /* If it's a user file device respect it */
        if (m_file_device != nullptr) { return prepare_result.resource_size; }

        /* Release archive resource */
        m_archive_resource = nullptr;
        m_archive_binder.Release();
        m_state.ClearBit(State_IsAllowArchiveReference);

        /* Use default file device */
        m_file_device = manager->GetDefaultFileDevice();

        /* Use default compression type */
        MaxExtensionString extension;
        vp::util::GetExtension(std::addressof(extension), std::addressof(m_file_path));
        const CompressionType auto_comp = manager->GetCompressionTypeByExtension(std::addressof(extension));
        this->SetCompressionType(auto_comp);

        return prepare_result.resource_size;
    }


    Result ResourceUnit::LoadMemory() {

        /* Release if transient */
        this->ReleaseTransient();

        /* Set status to in load */
        m_status = Status::InLoad;

        /* Setup resource size */
        const size_t size = this->PrepareResourceLoad();

        /* Release managed reference if an error occurs */
        auto error_signal_guard = SCOPE_GUARD {
            m_status = Status::Error;
            m_status_update_event.Signal();
        };
        auto release_managed_ref_on_error_guard = SCOPE_GUARD {
            this->ReleasePersistantReference();
        };

        /* Fail if no size was returned */
        if (size == 0) {
            m_state.SetBit(State_IsFileNotFound);
            RESULT_RETURN(ResultFileNotFound);
        }

        /* Check if user heap */
        if (m_state.GetBit(State_IsExternalHeap)) {
            /* Fail is user heap is null */
            if (m_resource_heap == nullptr) { error_signal_guard.Cancel(); RESULT_RETURN(ResultNoExternalHeap); }
        } else {

            /* Find memory manager  */
            ResourceMemoryManager *memory_manager = AsyncResourceManager::GetInstance()->GetMemoryManager(std::addressof(m_file_path));
            if (memory_manager == nullptr) {
                m_state.SetBit(State_IsMemoryAllocationFailure);
                RESULT_RETURN(ResultMemoryAllocationFailure);
            }

            /* Create resource heap */
            mem::Heap *heap = memory_manager->CreateResourceHeap(this, m_file_path.GetString(), size, static_cast<ResourceHeapType>(m_compression_flags.GetBit(CompressionFlag_IsFrameHeap)));
            if (heap == nullptr) {
                m_state.SetBit(State_IsMemoryAllocationFailure);
                RESULT_RETURN(ResultMemoryAllocationFailure);
            }

            /* Set heap and memory manager */
            m_resource_heap  = heap;
            m_memory_manager = memory_manager;

            /* Todo; user data */
            //this->ForkUserData();
        }

        /* Cancel error guards */
        error_signal_guard.Cancel();
        release_managed_ref_on_error_guard.Cancel();

        /* Add to memory manager free cache if freeable */
        if (m_state.GetBit(State_IsFreeableForMemoryManager)) {
            m_memory_manager->AddResourceUnitToFreeCache(this);
        }

        /* Load sync for archive */
        if (m_state.GetBit(State_IsAllowArchiveReference)) {                    
            return this->LoadFile();
        }

        RESULT_RETURN_SUCCESS;
    }

    Result ResourceUnit::LoadMemoryResult(async::TaskResultInvokeInfo *result_info) {

        /* Ensure in load */
        if (m_resource_heap == nullptr || m_status != ResourceUnit::Status::InLoad) {
            /* Compose if we loaded from an archive */
            if (m_state.GetBit(State_IsAllowArchiveReference)) {                
                this->RequestComposeForAfter();
            }
            RESULT_RETURN_SUCCESS;
        }

        /* Queue load file  */
        AsyncResourceManager *manager = AsyncResourceManager::GetInstance();
        const u32 convert_priority = manager->ConvertPriorityMemoryThreadToLoadThread(result_info->task->GetPriority());
        async::AsyncTaskPushInfo push_info = {
            .queue         = std::addressof(manager->m_queue_load),
            .task_function = std::addressof(manager->m_load_file_exe),
            .result_function = std::addressof(manager->m_load_file_result),
            .user_data     = this,
            .priority      = convert_priority,
        };
        RESULT_ABORT_UNLESS(result_info->task->PushTask(std::addressof(push_info)));

        RESULT_RETURN(async::ResultRescheduled);
    }

    void ResourceUnit::ScheduleUnload() {
        AsyncResourceManager *manager = AsyncResourceManager::GetInstance();
        async::AsyncTaskPushInfo push_info = {
            .queue_thread    = manager->m_thread_memory,
            .task_function   = std::addressof(manager->m_unload_exe),
            .result_function = std::addressof(manager->m_unload_result),
            .priority        = 0xd,
        };
        RESULT_ABORT_UNLESS(m_unload_task.PushTask(std::addressof(push_info)));
    }

    Result ResourceUnit::LoadFile() {

        /* Format file path with compression extension */
        MaxPathString path_with_compression;
        CompressionType compression_type = this->GetCompressionType();
        FormatPathWithCompressionExtension(std::addressof(path_with_compression), std::addressof(m_file_path), compression_type);

        /* Setup file device */
        FileDeviceBase *og_device = m_file_device;
        if (og_device == nullptr) { m_file_device = AsyncResourceManager::GetInstance()->GetDefaultArchiveFileDevice(); }
        if (ArchiveFileDevice::CheckRuntimeTypeInfoStatic(m_file_device)) { reinterpret_cast<ArchiveFileDevice*>(m_file_device)->SetArchiveResource(m_archive_resource); }

        /* Try to load the file */
        const Result result = this->TryLoadFile(path_with_compression.GetString());

        /* TODO; Fallback on failure */
        RESULT_ABORT_UNLESS(result);

        /* Remove default device */
        if (og_device == nullptr) {
            m_file_device = nullptr;
        }

        /* Finalize archive binder */
        m_archive_resource = nullptr;
        m_archive_binder.Release();

        /* Check if no resource or is not initializable */
        if (m_resource == nullptr || !m_resource->IsRequireInitializeOnCreate()) {
            m_compression_flags.ClearBit(CompressionFlag_IsResourceInitializable);
            if (m_resource == nullptr) {
                m_status = Status::Error;
            } else {
                this->RequestHeapAdjust();
                m_status = Status::Loaded;
            }
            m_status_update_event.Signal();

            RESULT_RETURN_UNLESS(m_status == Status::Loaded, ResultFailedToLoadResource);

            RESULT_RETURN_SUCCESS;
        }

        /* Set resource as initializable */
        m_compression_flags.SetBit(CompressionFlag_IsResourceInitializable);
        m_status = Status::Loaded;
        m_status_update_event.Clear();

        RESULT_RETURN_UNLESS(m_status == Status::Loaded, ResultFailedToLoadResource);

        RESULT_RETURN_SUCCESS;
    }

    void ResourceUnit::RequestComposeForAfter() {

        /* Ensure compose */
        auto release_archive_exit = SCOPE_GUARD {
            m_archive_resource = nullptr;
            m_archive_binder.Release();
        };
        if (m_compression_flags.GetBit(CompressionFlag_IsCompose)) { return; }
        if (m_resource != nullptr && (!m_compression_flags.GetBit(CompressionFlag_IsResourceInitializable) || !m_state.GetBit(State_IsRequireResourceInitialize))) { return; }
        if (m_status != Status::Loaded) { return; }
        release_archive_exit.Cancel();

        /* Ensure we are not a compose thread */
        AsyncResourceManager *manager = AsyncResourceManager::GetInstance();
        if (manager->IsCurrentThreadComposeThread()) { return; }

        /* Convert priority and schedule compose */
        const u32 prev_priority = m_load_task.GetPriority();
        u8 conv_priority = 0;
        sys::ThreadBase *thread = sys::GetCurrentThread();
        if (reinterpret_cast<sys::ThreadBase*>(manager->m_thread_memory) == thread) {
            conv_priority = static_cast<u8>(0x0404030302020101ull >> (((prev_priority - 3) & 7) * CHAR_BIT));
        } else if (manager->IsCurrentThreadComposeThread()) {
            conv_priority = (prev_priority < 5) ? prev_priority : 0;
        }
        this->ScheduleCompose(conv_priority);

        return;
    }

    void ResourceUnit::ScheduleCompose(u32 priority) {
        
        /* Ensure compose */
        if (!m_compression_flags.GetBit(CompressionFlag_IsCompose)) { return; }

        /* Lock compose queue */
        AsyncResourceManager *manager = AsyncResourceManager::GetInstance();
        sys::Mutex *mutex = manager->m_queue_compose.GetQueueMutex();
        std::scoped_lock l(*mutex);

        /* Increment compose ref */
        s32 compose_count = vp::util::InterlockedFetchIncrement(std::addressof(m_compose_reference_count));
        if (compose_count != 0) { return; }

        /* Ensure not unloadable */
        if (this->IsUnloadable()) { return; }

        /* Push compose */
        async::AsyncTaskPushInfo push_info = {
            .queue         = std::addressof(manager->m_queue_compose),
            .task_function = std::addressof(manager->m_compose_exe),
            .user_data     = this,
            .priority      = priority,
        };
        RESULT_ABORT_UNLESS(m_compose_task.PushTask(std::addressof(push_info)));

        return;
    }

    Result ResourceUnit::Compose() {
        ScopedThreadLocalArchive l(std::addressof(m_archive_binder));
        this->InitializeResource(nullptr);
        if (m_archive_resource != nullptr) {
            m_archive_resource = nullptr;
            m_archive_binder.Release();
        }
        RESULT_RETURN_SUCCESS;
    }

    Result ResourceUnit::Unload() {

        /* Release transient ref */
        this->ReleaseTransient();

        /* Release active memory usage for unload */
        this->ReleaseMemoryUsageActive();

        /* Cancel resource if not cachable */
        if (!m_state.GetBit(State_IsCacheUnload) || !m_cache_unload_flags.GetBit(CacheUnloadFlag_IsValid) || !m_cache_unload_no_error_flags.GetBit(CacheUnloadNoErrorFlag_IsValid)) {                   

            /* Set as uninitialized on completion */
            auto uninitialized_guard = SCOPE_GUARD {
                m_status = Status::Uninitialized;
            };

            m_load_task.CancelTask();
            m_archive_resource = nullptr;
            m_archive_binder.Release();
            if (!m_load_task.IsCancelled()) {
                const Result cancel_result = m_compose_task.TryCancelTask();
                if (cancel_result == async::ResultTaskExecuting) {
                    m_state.SetBit(State_IsFreeableForMemoryManager);
                    RESULT_RETURN(ResultFailedToCancelResource);
                }

                switch (m_status) {
                    case Status::Loaded:
                    case Status::Error:
                        break;
                    case Status::InResourceInitialize:
                    case Status::ResourceInitialized:
                        /* Wait for resource to initialize */
                        m_status_update_event.Wait();
                        [[fallthrough]];
                    case Status::FailedToPreFinalizeResource:
                    case Status::ResourcePostInitialized:
                    case Status::FailedToPostInitializeResource:
                        /* PreFinalize resource */
                        {
                            const bool is_finalized = this->PreFinalizeResource();
                            if (!is_finalized) { RESULT_RETURN(ResultFailedToPreFinalizeResource); }
                            [[fallthrough]];
                        }
                    case Status::ResourcePreFinalized:
                    case Status::FailedToInitializeResource:
                        /* Finalize resource */
                        this->FinalizeResource();
                        break;
                    default:
                        uninitialized_guard.Cancel();
                        break;
                };
            }

            m_state.SetBit(State_IsFreeableForMemoryManager);

        } else {

            /* Advance when cache unloadable */
            auto invalidate_cache_unload_no_error_guard = SCOPE_GUARD {
                m_cache_unload_no_error_flags.ClearBit(CacheUnloadNoErrorFlag_IsValid);
            };
            auto set_freeable_for_memory_mgr_guard = SCOPE_GUARD {
                m_state.SetBit(State_IsFreeableForMemoryManager);
            };
            switch (m_status) {
                default:
                    break;
                case Status::Uninitialized:
                {
                    if (!m_load_task.IsCancelled()) { break; }
                    break;
                }
                case Status::FailedToPreFinalizeResource:
                {
                    invalidate_cache_unload_no_error_guard.Cancel();
                    set_freeable_for_memory_mgr_guard.Cancel();
                    const bool is_finalized = this->PreFinalizeResource();
                    if (!is_finalized) { RESULT_RETURN(ResultFailedToPreFinalizeResource); }
                    break;
                }
                case Status::Error:
                {
                    break;
                }
                case Status::InResourceInitialize:
                {
                    m_status_update_event.Wait();
                    if (m_status == Status::FailedToPostInitializeResource) {
                        
                        const bool is_finalized = this->PreFinalizeResource();
                        if (!is_finalized) { RESULT_RETURN(ResultFailedToPreFinalizeResource); }
                        this->FinalizeResource();
                        
                        m_status = Status::Uninitialized;
                        break;
                    }
                    if (m_status == Status::ResourcePostInitialized) {
                        invalidate_cache_unload_no_error_guard.Cancel();
                        const bool is_finalized = this->PreFinalizeResource();
                        if (!is_finalized) {
                            set_freeable_for_memory_mgr_guard.Cancel();
                            RESULT_RETURN(ResultFailedToPreFinalizeResource);
                        }
                        this->FinalizeResource();
                    }
                    break;
                }
                case Status::FailedToInitializeResource:
                {
                    this->FinalizeResource();
                    m_status = Status::Uninitialized;
                    break;
                }
                case Status::ResourceInitialized:
                {
                    m_status_update_event.Wait();
                    if (m_status == Status::FailedToPostInitializeResource) {

                        const bool is_finalized = this->PreFinalizeResource();
                        if (is_finalized == false) { RESULT_RETURN(ResultFailedToPreFinalizeResource); }
                        this->FinalizeResource();

                        m_status = Status::Uninitialized;
                        break;
                    }
                    invalidate_cache_unload_no_error_guard.Cancel();
                    set_freeable_for_memory_mgr_guard.Cancel();
                    if (m_status == Status::ResourcePostInitialized) {
                        const bool is_finalized = this->PreFinalizeResource();
                        if (!is_finalized) { RESULT_RETURN(ResultFailedToPreFinalizeResource); }
                        this->FinalizeResource();
                    }
                    break;
                }
                case Status::ResourcePostInitialized:
                {
                    invalidate_cache_unload_no_error_guard.Cancel();
                    set_freeable_for_memory_mgr_guard.Cancel();
                    const bool is_finalized = this->PreFinalizeResource();
                    if (!is_finalized) { RESULT_RETURN(ResultFailedToPreFinalizeResource); }                        
                    break;
                }
                case Status::FailedToPostInitializeResource:
                {
                    const bool is_finalized = this->PreFinalizeResource();
                    if (!is_finalized) {
                        set_freeable_for_memory_mgr_guard.Cancel();
                        RESULT_RETURN(ResultFailedToPreFinalizeResource);
                    }
                    this->FinalizeResource();
                    m_status = Status::Uninitialized;
                    break;
                }
            };
        }

        /* Add to clear cache if part of res mgr */
        if (!m_is_part_of_resource_unit_mgr) {

            /* Clear cache sync */
            m_state.SetBit(State_IsFinalized);
            this->ClearCache();

        } else {

            /* Set as freeable */
            m_state.SetBit(State_IsFreeableForMemoryManager);

            /* Add to free cache if cacheable */
            if (!m_state.GetBit(State_IsCacheUnload)) { RESULT_RETURN_SUCCESS; }
            if (!m_cache_unload_flags.GetBit(CacheUnloadFlag_IsValid)) { RESULT_RETURN_SUCCESS; }
            if (!m_cache_unload_no_error_flags.GetBit(CacheUnloadNoErrorFlag_IsValid)) { RESULT_RETURN_SUCCESS; }
            if (m_memory_manager == nullptr) { RESULT_RETURN_SUCCESS; }
            m_memory_manager->AddResourceUnitToFreeCache(this);
        }

        RESULT_RETURN_SUCCESS;
    }

    Result ResourceUnit::ClearCache() {

        /* Release transient ref */
        this->ReleaseTransient();

        /* Fail if still in reference */
        RESULT_RETURN_IF(0 < this->GetReferenceCount(), ResultStillInReference);
        /* Fail if already freed */
        RESULT_RETURN_IF(m_status == Status::Freed, ResultAlreadyFreed);

        /* Cancel the resource */
        const bool is_cancel = this->CancelResource();
        RESULT_RETURN_IF(!is_cancel, ResultFailedToCancelResource);

        /* Transition to Freed */
        m_status = Status::Freed;
        if (m_resource_heap == nullptr) {
            RESULT_RETURN_SUCCESS;
        }

        /* Free resource and resource heap */
        AsyncResourceManager::GetInstance()->CancelHeapAdjustRequest(this);
        this->ReleaseMemoryUsageGlobal();
        this->FreeResource();
        this->FreeResourceHeap();

        RESULT_RETURN_SUCCESS;
    }

    Result ResourceUnit::TryLoadFile(const char *file_path) {

        /* Setup resource load context */
        ResourceLoadContext load_context = {
            .file_load_context = {                
                .file_heap = (ArchiveFileDevice::CheckRuntimeTypeInfoStatic(m_file_device)) ? nullptr : m_resource_heap,
            },
            .file_device      = m_file_device,
            .resource_factory = m_resource_factory,
            .resource_heap    = m_resource_heap,
        };

        /* Load file with respect to decompression */
        const CompressionType decompressor_type = this->GetCompressionType();
        if (decompressor_type == CompressionType::None) {
            return res::LoadResource(std::addressof(m_resource), file_path, std::addressof(load_context));
        }

        /* Select decompressor */
        const DecompressorHandle handle = res::AllocateDecompressorHandle();
        IDecompressor *decompressor = res::GetDecompressor(handle, decompressor_type, 0xd, (1ull << (sys::GetCurrentCoreNumber() & (BITSIZEOF(sys::CoreMask) - 1))));

        /* Load and decompress */
        const Result result = res::LoadResourceWithDecompressor(std::addressof(m_resource), file_path, std::addressof(load_context), decompressor);

        /* Free decompressor */
        res::FreeDecompressorHandle(handle);

        return result;
    }

    void ResourceUnit::CancelUnload(AsyncResourceManager *manager) {
        manager->CancelUnloadRequest(this);
        m_unload_task.CancelTask();
    }

    void ResourceUnit::UnregisterFromResourceUnitManager() {
        if (m_resource_unit_manager == nullptr) { return; }
        m_resource_unit_manager->UnregisterResourceUnit(this);
    }

    void ResourceUnit::FinalizeForClearCache() {
        if (!m_state.GetBit(State_IsFreeableForMemoryManager) || 0 < this->GetReferenceCount() || m_status == Status::Freed) { return; }
        m_state.SetBit(State_IsFinalized);
        AsyncResourceManager *manager = AsyncResourceManager::GetInstance();
        async::AsyncTaskPushInfo push_info = {
            .queue_thread = manager->m_thread_control,
            .task_function = std::addressof(manager->m_defer_clear_cache_exe),
            .result_function = std::addressof(manager->m_defer_clear_cache_result),
            .user_data = this,
            .priority = 6,
        };
        RESULT_ABORT_UNLESS(m_unload_task.PushTask(std::addressof(push_info)));
    }

    bool ResourceUnit::InitializeResource(ResourceUserContext *user_context) {

        /* Guard resource initialize */
        const u32 last_count = vp::util::InterlockedFetchIncrement(std::addressof(m_resource_initialize_guard));
        if (0 < last_count) {
            m_status_update_event.Wait();
            const Status status = static_cast<Status>(m_status);
            return status != Status::FailedToInitializeResource && status != Status::FailedToPostInitializeResource;
        }

        /* Signal status update on exit */
        ON_SCOPE_EXIT {
            m_status_update_event.Signal();
        };

        /* Wait for initialize */
        //m_load_task.Wait();

        /* Fail if resource is allocated */
        if (m_resource == nullptr) {
            m_status = Status::FailedToInitializeResource;
            return false;
        }

        /* Succeed if resource does not require initialization */
        if (!m_state.GetBit(State_IsRequireResourceInitialize)) { return true; }

        /* Release managed reference and update status on exit */
        Status guard_status = Status::FailedToInitializeResource;
        auto no_resource_exit_guard = SCOPE_GUARD {

            /* Release managed reference */
            this->ReleasePersistantReference();

            /* Set status */
            m_status = guard_status;
        };

        /* Request heap adjust on exit */
        ON_SCOPE_EXIT {
            this->RequestHeapAdjust();
        };

        /* Initialize resource */
        if (m_status == Status::Loaded) {
            m_status = Status::InResourceInitialize;
            const Result result = m_resource->Initialize(m_resource_heap, m_gpu_heap, user_context, m_resource->GetFile(), m_resource->GetFileSize());
            if (RESULT_FAILED(result)) { return false; }
            guard_status = Status::ResourceInitialized;
            if (m_resource == nullptr) { return false; }
        }
        m_status = Status::ResourceInitialized;

        /* Post initialize resource */
        if (m_compression_flags.GetBit(CompressionFlag_IsResourceInitializable)) {
            const Result result = m_resource->PostInitialize(m_resource_heap, m_gpu_heap, user_context, m_resource->GetFile(), m_resource->GetFileSize());
            guard_status = Status::FailedToPostInitializeResource;
            if (RESULT_FAILED(result)) { return false; }
        }
        m_status = Status::ResourcePostInitialized;

        /* Cancel no resource guard since we succeeded */
        no_resource_exit_guard.Cancel();

        return true;
    }

    bool ResourceUnit::PreFinalizeResource() {

        /* Check if prefinalization necessary*/
        if ((m_status != Status::ResourcePostInitialized && m_status != Status::FailedToPostInitializeResource && m_status != Status::FailedToPreFinalizeResource) || m_resource == nullptr) { m_status = Status::ResourcePreFinalized; return false; }

        /* Prefinalize resource */
        if (m_compression_flags.GetBit(CompressionFlag_IsResourceInitializable)) {
            m_status = Status::InResourcePreFinalize;
            const Result result = m_resource->PreFinalize();
            if (RESULT_FAILED(result)) { 
                m_status = Status::FailedToPreFinalizeResource;
                return false;
            }
        } 

        /* Release guard */
        m_resource_initialize_guard = 0;
        m_status_update_event.Signal();

        /* Update status */
        m_status = Status::ResourcePreFinalized;

        return true;
    }

    void ResourceUnit::FinalizeResource() {

        /* Check unit is pre finalized and has resource */
        if ((m_status != Status::ResourcePreFinalized && m_status != Status::FailedToInitializeResource) || m_resource == nullptr || !m_compression_flags.GetBit(CompressionFlag_IsResourceInitializable)) { m_status = Status::Loaded; return; }

        /* Set current heap to resource heap */
        mem::ScopedCurrentThreadHeap l(m_resource_heap);

        /* Finalize resource */
        m_status = Status::InResourceFinalize;
        m_resource->Finalize();

        /* Release guard */
        m_resource_initialize_guard = 0;
        m_status_update_event.Clear();

        /* Revert to loaded */
        m_status = Status::Loaded;

        return;               
    }

    void ResourceUnit::FreeResource() {

        /* Check unit is freed and has resource */
        if (m_status != Status::Freed || m_resource == nullptr) { return; }
        
        /* Set current heap to resource heap */
        mem::ScopedCurrentThreadHeap l(m_resource_heap);
        
        /* Delete resource */
        if (m_resource != nullptr) {
            delete m_resource;
        }
        m_resource = nullptr;

        /* Todo; delete user data */
        //if (m_user_data_status == UserDataStatus::Copy && m_user_data != nullptr) {
        //    delete m_user_data;
        //}
        //m_user_data = nullptr;
        //m_user_data_status = UserDataStatus::IsSet;

        return;
    }

    bool ResourceUnit::CancelResource() {

        /* Cancel load task */
        m_load_task.CancelTask();

        /* Release our archive resource */
        m_archive_resource = nullptr;
        m_archive_binder.Release();

        /* If the task was sucessfully cancelled we have no resource */
        if (m_load_task.IsCancelled()) { return true; }

        /* Try cancel compose */
        const Result cancel_result = m_compose_task.TryCancelTask();
        if (cancel_result == async::ResultTaskExecuting) {
            AsyncResourceManager::GetInstance()->AddResourceUnitToFinalizeList(this);
            return false;
        }

        /* Finalize resource for not cancelled */
        switch (m_status) {
            case Status::FailedToPreFinalizeResource:
            case Status::ResourcePostInitialized:
            case Status::FailedToPostInitializeResource:
            {
                const bool is_finalized = this->PreFinalizeResource();
                if (!is_finalized) { return false; }
            }
            [[fallthrough]];
            case Status::ResourcePreFinalized:
            case Status::FailedToInitializeResource:
            {
                this->FinalizeResource();
            }
            default:
            break;
        };

        return true;
    }

	void ResourceUnit::Initialize(ResourceUnitInfo *unit_info) {

        /* Set defaults */
        AsyncResourceLoadInfo *async_resource_load_info = unit_info->async_resource_info;
        m_status = Status::Uninitialized;
        m_reference_count = 0;
        m_deferred_adjust_count[0]  = 0;
        m_deferred_adjust_count[1]  = 0;
        m_resource_initialize_guard = 0;
        m_state.SetBitMask(0, 0);

        /* Set file device, resource factory, and resource user context */
        m_file_device           = async_resource_load_info->file_device;
        m_resource_factory      = async_resource_load_info->resource_factory;
        m_resource_user_context = async_resource_load_info->resource_user_context;

        /* Set manager */
        m_resource_unit_manager = unit_info->resource_unit_manager;

        /* Clear status event */
        m_status_update_event.Clear();                

        /* Set file path */
        m_file_path = unit_info->file_path;

        /* Setup user resource size */
        if (async_resource_load_info->resource_size != 0) {
            m_state.SetBit(State_IsUserResourceSize);
        }
        m_user_resource_size = async_resource_load_info->resource_size;

        /* Setup external heap */
        mem::Heap *heap = async_resource_load_info->resource_heap;
        if (heap != nullptr) {
            m_state.SetBit(State_IsExternalHeap);
            m_resource_heap = heap;
        }

        /* Set archive binder */
        if (async_resource_load_info->archive_binder != 0) {
            m_archive_binder.ReferenceBinderSync(async_resource_load_info->archive_binder);
            ArchiveResource *archive_resource = m_archive_binder.GetResource<ArchiveResource>();
            m_archive_resource = archive_resource;
        }

        /* Set key */
        const vp::util::HashString<u32> hash_string = {
            .hash = vp::util::HashCrc32b(m_file_path.GetString()),
            .string = m_file_path.GetString(),
        };
        m_resource_unit_manager_tree_node.SetKey(hash_string);

        /* Setup cache on unload */
        if (!async_resource_load_info->is_persistant && async_resource_load_info->is_cache_on_unload) {
            m_state.SetBit(State_IsCacheUnload);
            m_cache_unload_flags.SetBit(CacheUnloadFlag_IsValid);
            m_cache_unload_no_error_flags.SetBit(CacheUnloadNoErrorFlag_IsValid);
        }

        /* Set flags */
        if (unit_info->is_initialize_resource) {
            m_state.SetBit(State_IsRequireResourceInitialize);
        }
        if (unit_info->is_boost_compose) {
            m_state.SetBit(State_IsBoostCompose);
        }
        if (unit_info->is_compose) {
            m_compression_flags.SetBit(CompressionFlag_IsCompose);
        }
        if (async_resource_load_info->is_require_heap_adjust) {
            m_state.SetBit(State_IsRequireHeapAdjust);
        }
        if (async_resource_load_info->is_persistant) {
            m_state.SetBit(State_IsPersistant);
        }
        if (async_resource_load_info->is_transient) {
            m_state.SetBit(State_IsTransient);
        }
        m_compose_reference_count = -async_resource_load_info->is_transient;

        /* Set heap type */
        if (async_resource_load_info->resource_heap_type == 1) {            
            m_compression_flags.SetBit(CompressionFlag_IsFrameHeap);
        }

        /* Increment reference for binder or transience */
        ResourceBinder *resource_binder = unit_info->resource_binder;
        if (resource_binder != nullptr) {
            this->SetToBinder(resource_binder);
        } else if (async_resource_load_info->is_transient) {
            m_cache_unload_flags.SetBit(CacheUnloadFlag_IsTransientRef);
        }

        /* Increment reference for managed */
        if (m_state.GetBit(State_IsPersistant)) {
            this->IncrementReference();
        }

        return;
    }
}
