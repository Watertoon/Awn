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

namespace awn::res {

    void ResourceUnit::AdjustReferenceCount(s32 adjust_amount) {

        /* Adjust ref */
        const u32 new_count = vp::util::InterlockedAdd(std::addressof(m_reference_count), adjust_amount);

        /* Remove last reference if managed, ensure last reference */
        if (new_count == 1) {
            this->ReleaseManagedReference();
        } else if (new_count != 0) { return; }

        /* Fail cache unload guard */
        auto fail_cache_unload_guard = SCOPE_GUARD {
            this->UnregisterFromResourceUnitManager();
            m_is_cache_unload_for_no_ref = false;
        };

        /* Check if cache unload is enabled */
        if (m_is_cache_unload == false || m_is_cache_unload_for_no_ref == false || m_is_cache_unload_for_no_error == false) { return; }

        /* Update cache unload for error */
        switch (m_status) {
            case Status::Error:
            case Status::FailedToInitializeResource:
            case Status::FailedToPostInitializeResource:
                /* The resource has failed */
                break;
            case Status::InResourceInitialize:
            case Status::ResourceInitialized:
                /* Wait for resource to initialize, ensure success */
                m_status_update_event.Wait();
                if (m_status == Status::FailedToPostInitializeResource) { return; }
                [[fallthrough]];
            default:
                fail_cache_unload_guard.Cancel();
                break;
        };

        return;
    }

    void ResourceUnit::IncrementReference() {
        vp::util::InterlockedIncrement(std::addressof(m_reference_count));
        m_is_freeable_for_memory_manager = false;
    }

    void ResourceUnit::DecrementReference() {
        vp::util::InterlockedDecrement(std::addressof(m_reference_count));
    }

    void ResourceUnit::ReleaseManagedReference() {
        if (m_is_managed == false) { return; }
        this->DecrementReference();
        m_is_managed = false;
    }

    void ResourceUnit::SetToBinder(ResourceBinder *resource_binder) {
        resource_binder->m_resource_unit = this;
        this->IncrementReference();
    }

    void ResourceUnit::ScheduleClearCache() {
        AsyncResourceManager *manager = AsyncResourceManager::GetInstance();
        async::AsyncTaskPushInfo push_info = {
            .queue_thread    = manager->m_async_queue_thread_memory,
            .task_function   = std::addressof(manager->m_clear_cache_exe),
            .result_function = std::addressof(manager->m_clear_cache_result),
            .priority        = 0xd,
        };
        m_unload_task.PushTask(std::addressof(push_info));
    }

    void ResourceUnit::FinalizeIfOutOfReference() {

        /* Fail if not out of reference */
        if (0 < m_reference_count) { return; }

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
            return async::ResultRescheduled;
        }

        /* Release archive binder */
        if (m_archive_resource != nullptr) {
            m_archive_resource = nullptr;
            m_archive_binder.Finalize();
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
            priority = (priority << 1) + (m_allow_archive_reference == true);                    
        } else if (queue != load_queue) { return; }

        /* Change priority */
        m_load_task.ChangePriority(priority);

        return;
    }

    void ResourceUnit::FreeResourceHeap() {
        if (m_resource_heap  == nullptr) { return; }
        if (m_memory_manager == nullptr) { return; }
        m_memory_manager->FreeHeap(m_resource_heap, nullptr, this);
    }

    void ResourceUnit::TrackMemoryUsage() {

        /* Skip tracking memory memory if external heap */
        if (m_is_external_heap == true) { m_is_heap_adjusted = true; return; }

        /* Update memory usage sync if no heap adjust is required */
        if (m_is_heap_adjusted == true || m_is_require_heap_adjust == false) { this->UpdateMemoryUsage(); }

        /* Schedule a heap adjust */
        AsyncResourceManager *manager = AsyncResourceManager::GetInstance();
        async::AsyncTaskPushInfo push_info = {
            .queue_thread  = manager->m_async_queue_thread_memory,
            .task_function = std::addressof(manager->m_heap_adjust_exe),
            .user_data     = this,
            .priority      = 0xe,
        };
        m_heap_adjust_task.PushTask(std::addressof(push_info));

        return;
    }

    void ResourceUnit::UpdateMemoryUsage() {

        /* Track global */
        if (m_is_tracked_memory_usage_global == false) {
            m_memory_manager->TrackMemoryUsageGlobal(m_resource_heap);
            m_is_tracked_memory_usage_global = true;
        }

        /* Track resident */
        if (m_is_tracked_memory_usage_active == false && m_is_freeable_for_memory_manager == false) {
            m_memory_manager->TrackMemoryUsageActive(m_resource_heap);
            m_is_tracked_memory_usage_active = true;
        }

        return;
    }

    void ResourceUnit::ReleaseMemoryUsageGlobal() {

        /* Track global */
        if (m_is_tracked_memory_usage_global == true) {
            m_memory_manager->ReleaseMemoryUsageGlobal(m_resource_heap);
            m_is_tracked_memory_usage_global = false;
        }

        return;
    }
    void ResourceUnit::ReleaseMemoryUsageActive() {

        /* Track global */
        if (m_is_tracked_memory_usage_active == true) {
            m_memory_manager->ReleaseMemoryUsageActive(m_resource_heap);
            m_is_tracked_memory_usage_active = false;
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
        m_file_device = (prepare_result->is_default_archive_file_device == true) ? nullptr : prepare_result->file_device;

        /* TODO */

        return;
    }

    size_t ResourceUnit::PrepareResourceLoad() {
        
        /* Format compression extension */
        MaxPathString path_with_compression;
        FormatPathWithCompressionExtension(std::addressof(path_with_compression), std::addressof(m_file_path), static_cast<CompressionType>(m_compression_type));

        /* Lookup resource size */
        ResourceSizePrepareInfo prepare_info = {
            .user_resource_size = m_user_resource_size,
            .file_alignment     = m_file_alignment,
            .archive_resource   = m_archive_resource,
            .file_device        = m_file_device,
            .resource_factory   = m_resource_factory,
            .compression_type   = static_cast<CompressionType>(m_compression_type),
            .formatted_path     = path_with_compression.GetString(),
            .original_path      = m_file_path.GetString(),
        };
        ResourceSizePrepareResult prepare_result = {};
        this->PrepareSizeImpl(std::addressof(prepare_result), std::addressof(prepare_info));

        /* Perform fallback lookups */
        AsyncResourceManager *manager = AsyncResourceManager::GetInstance();
        if (prepare_result.resource_size == 0) {

            /* Nothing else to try if no archive or compresion */
            if (m_allow_archive_reference == false && static_cast<CompressionType>(m_compression_type) == CompressionType::None) { return 0; }

            /* Get auto compression type */
            MaxExtensionString extension;
            vp::util::GetExtension(std::addressof(extension), std::addressof(m_file_path));
            const CompressionType auto_comp_type = manager->GetCompressionTypeByExtension(std::addressof(extension));

            /* Try archive */
            if (m_archive_resource != nullptr && m_allow_archive_reference == true) {

                /* Try archive with auto compression */
                if (auto_comp_type != CompressionType::None && static_cast<CompressionType>(m_compression_type) == CompressionType::None) {
                    FormatPathWithCompressionExtension(std::addressof(path_with_compression), std::addressof(m_file_path), auto_comp_type);
                    prepare_info.formatted_path = path_with_compression.GetString();
                    this->PrepareSizeImpl(std::addressof(prepare_result), std::addressof(prepare_info));
                    if (prepare_result.resource_size != 0) {
                        m_compression_type = static_cast<u32>(auto_comp_type);
                        return prepare_result.resource_size;
                    }
                }

                /* Try archive no compression */
                if (m_compression_type != static_cast<u32>(CompressionType::None)) {
                    prepare_info.formatted_path = m_file_path.GetString();
                    this->PrepareSizeImpl(std::addressof(prepare_result), std::addressof(prepare_info));
                    if (prepare_result.resource_size != 0) {
                        m_compression_type = static_cast<u32>(CompressionType::None);
                        return prepare_result.resource_size;
                    }
                }

                /* Release archive since we failed to find the file in it */
                m_archive_resource = nullptr;
                m_archive_binder.Finalize();
                m_allow_archive_reference = false;
            }

            /* Try loose auto compression */
            if (auto_comp_type != CompressionType::None && static_cast<CompressionType>(m_compression_type) == CompressionType::None) {
                FormatPathWithCompressionExtension(std::addressof(path_with_compression), std::addressof(m_file_path), auto_comp_type);
                prepare_info.formatted_path = path_with_compression.GetString();
                this->PrepareSizeImpl(std::addressof(prepare_result), std::addressof(prepare_info));
                if (prepare_result.resource_size != 0) {
                    m_compression_type = static_cast<u32>(auto_comp_type);
                    return prepare_result.resource_size;
                }
            }

            /* Try loose no compression */
            if (static_cast<CompressionType>(m_compression_type) != CompressionType::None) {
                prepare_info.formatted_path = m_file_path.GetString();
                this->PrepareSizeImpl(std::addressof(prepare_result), std::addressof(prepare_info));
                if (prepare_result.resource_size != 0) {
                    m_compression_type = static_cast<u32>(CompressionType::None);
                    return prepare_result.resource_size;
                }
            }

            return 0;
        }

        /* Finish if no archive binder */
        if (m_allow_archive_reference == false) { return prepare_result.resource_size; }

        /* Check archive file exists */
        const bool is_archive_file = manager->CheckArchiveFileExists(m_archive_resource, std::addressof(path_with_compression));
        if (is_archive_file == true) { return prepare_result.resource_size; }

        /* Try again without compression */
        const bool is_no_compress_archive_file = manager->CheckArchiveFileExists(m_archive_resource, std::addressof(m_file_path));
        if (is_no_compress_archive_file == true) { m_compression_type = static_cast<u32>(CompressionType::None); return prepare_result.resource_size; }

        /* If it's a user file device respect it */
        if (m_file_device != nullptr) { return prepare_result.resource_size; }

        /* Remove archive resource */
        m_archive_resource = nullptr;
        m_archive_binder.Finalize();
        m_allow_archive_reference = false;

        /* Use default file device */
        m_file_device = manager->GetDefaultFileDevice();

        /* Use default compression type */
        MaxExtensionString extension;
        vp::util::GetExtension(std::addressof(extension), std::addressof(m_file_path));
        m_compression_type = static_cast<u32>(manager->GetCompressionTypeByExtension(std::addressof(extension)));

        return prepare_result.resource_size;
    }

    Result ResourceUnit::ScheduleLoad() {

        /* Handle transience */
        if (m_is_transient_on_load == true) { m_is_transient_on_load = false; AsyncResourceManager::GetInstance()->RequestUnloadResourceUnit(this); }

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
            this->ReleaseManagedReference();
        };

        /* Fail if no size was returned */
        if (size == 0) { m_is_file_not_found = true; return ResultFileNotFound; }

        /* Check if user heap */
        if (m_is_external_heap == true) {
            /* Fail is user heap is null */
            if (m_resource_heap == nullptr) { error_signal_guard.Cancel(); return ResultNoExternalHeap; }
        } else {

            /* Find memory manager  */
            ResourceMemoryManager *memory_manager = AsyncResourceManager::GetInstance()->GetMemoryManager(std::addressof(m_file_path));
            if (memory_manager == nullptr) { m_is_memory_allocation_failure = true; return ResultMemoryAllocationFailure; }

            /* Create resource heap */
            mem::Heap *heap = memory_manager->CreateResourceHeap(this, m_file_path.GetString(), size, static_cast<ResourceHeapType>(m_heap_type));
            if (heap == nullptr) { m_is_memory_allocation_failure = true; return ResultMemoryAllocationFailure; }

            /* Set heap and memory manager */
            m_resource_heap  = heap;
            m_memory_manager = memory_manager;
        }

        /* Cancel error guards */
        error_signal_guard.Cancel();
        release_managed_ref_on_error_guard.Cancel();

        /* Add to memory manager free cache if freeable */
        if (m_is_freeable_for_memory_manager == true) {
            m_memory_manager->AddResourceUnitToFreeCache(this);
        }

        /* Load sync for archive */
        if (m_allow_archive_reference == true) {                    
            return this->LoadFile();
        }

        RESULT_RETURN_SUCCESS;
    }

    void ResourceUnit::ScheduleUnload() {
        AsyncResourceManager *manager = AsyncResourceManager::GetInstance();
        async::AsyncTaskPushInfo push_info = {
            .queue_thread    = manager->m_async_queue_thread_memory,
            .task_function   = std::addressof(manager->m_unload_exe),
            .result_function = std::addressof(manager->m_unload_result),
            .priority        = 0xd,
        };
        m_unload_task.PushTask(std::addressof(push_info));
    }

    Result ResourceUnit::LoadFile() {

        /* Format file path with compression extension */
        MaxPathString path_with_compression;
        FormatPathWithCompressionExtension(std::addressof(path_with_compression), std::addressof(m_file_path), static_cast<CompressionType>(m_compression_type));

        /* Setup file device */
        FileDeviceBase *og_device = m_file_device;
        if (og_device == nullptr)                                                 { m_file_device = AsyncResourceManager::GetInstance()->GetDefaultArchiveFileDevice(); }
        if (ArchiveFileDevice::CheckRuntimeTypeInfoStatic(m_file_device) == true) { reinterpret_cast<ArchiveFileDevice*>(m_file_device)->SetArchiveResource(m_archive_resource); }

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
        m_archive_binder.Finalize();

        /* Check if no resource or is not initializable */
        if (m_resource == nullptr || m_resource->IsRequireInitializeOnCreate() == false) {
            m_is_resource_initializable = false;
            if (m_resource == nullptr) {
                m_status = Status::Error;
            } else {
                this->TrackMemoryUsage();
                m_status = Status::Loaded;
            }
            m_status_update_event.Signal();

            RESULT_RETURN_UNLESS(m_status == Status::Loaded, ResultFailedToLoadResource);

            RESULT_RETURN_SUCCESS;
        }

        /* Set resource as initializable */
        m_is_resource_initializable = true;
        m_status                    = Status::Loaded;
        m_status_update_event.Clear();

        RESULT_RETURN_UNLESS(m_status == Status::Loaded, ResultFailedToLoadResource);

        RESULT_RETURN_SUCCESS;
    }

    Result ResourceUnit::Unload() {

        /* Release active memory usage for unload */
        this->ReleaseMemoryUsageActive();

        /* Cancel resource if not cachable */
        if (m_is_cache_unload == false || m_is_cache_unload_for_no_ref == false || m_is_cache_unload_for_no_error == false) {                    

            /* Set as uninitialized on completion */
            auto uninitialized_guard = SCOPE_GUARD {
                m_status = Status::Uninitialized;
            };

            /* Advance on status */
            switch (m_status) {
                case Status::Loaded:
                case Status::Error:
                    break;
                case Status::InLoad:
                    /* Cancel load task  */
                    m_load_task.CancelTask();

                    /* Free archive reference */
                    m_archive_resource = nullptr;
                    m_archive_binder.Finalize();

                    /* Request unload for transience */
                    if (m_load_task.IsCancelled() == true && m_is_transient_on_load == true) {
                        m_is_transient_on_load = false;
                        AsyncResourceManager::GetInstance()->RequestUnloadResourceUnit(this);
                    }
                    [[fallthrough]];
                case Status::InResourceInitialize:
                case Status::ResourceInitialized:
                    /* Wait for resource to initialize */
                    m_status_update_event.Wait();
                    [[fallthrough]];
                case Status::ResourcePostInitialized:
                case Status::FailedToPreFinalizeResource:
                case Status::FailedToPostInitializeResource:
                    /* PreFinalize resource */
                    {                        
                        const bool is_finalized = this->PreFinalizeResource();
                        if (is_finalized == false) { return ResultFailedToPreFinalizeResource; }
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

            m_is_freeable_for_memory_manager = true;

        } else {

            /* Advance when cache unloadable */
            switch (m_status) {
                default:
                    break;
                case Status::Uninitialized:
                {
                    if (m_load_task.GetStatus() != async::AsyncTask::Status::Cancelled) { break; }
                    m_is_cache_unload_for_no_error = false;
                    m_is_freeable_for_memory_manager = true;
                    break;
                }
                case Status::FailedToPreFinalizeResource:
                {                        
                    const bool is_finalized = this->PreFinalizeResource();
                    if (is_finalized == false) { return ResultFailedToPreFinalizeResource; }
                    break;
                }
                case Status::Error:
                {
                    m_is_cache_unload_for_no_error = false;
                    m_is_freeable_for_memory_manager = true;
                    break;
                }
                case Status::InResourceInitialize:
                {
                    m_status_update_event.Wait();
                    if (m_status == Status::FailedToPostInitializeResource) {
                        m_is_cache_unload_for_no_error   = false;
                        
                        const bool is_finalized = this->PreFinalizeResource();
                        if (is_finalized == false) { return ResultFailedToPreFinalizeResource; }
                        this->FinalizeResource();
                        
                        m_status                         = Status::Uninitialized;
                        m_is_freeable_for_memory_manager = true;
                        break;
                    }
                    if (m_status == Status::ResourcePostInitialized) {
                        const bool is_finalized = this->PreFinalizeResource();
                        if (is_finalized == false) { return ResultFailedToPreFinalizeResource; }
                        this->FinalizeResource();
                    }
                    m_is_cache_unload_for_no_error = false;
                    m_is_freeable_for_memory_manager = true;
                    break;
                }
                case Status::FailedToInitializeResource:
                {
                    m_is_cache_unload_for_no_error = false;
                    this->FinalizeResource();
                    m_status = Status::Uninitialized;
                    m_is_freeable_for_memory_manager = true;
                    break;
                }
                case Status::ResourceInitialized:
                {
                    m_status_update_event.Wait();
                    if (m_status == Status::FailedToPostInitializeResource) {
                        m_is_cache_unload_for_no_error   = false;
                        
                        const bool is_finalized = this->PreFinalizeResource();
                        if (is_finalized == false) { return ResultFailedToPreFinalizeResource; }
                        this->FinalizeResource();
                        
                        m_status                         = Status::Uninitialized;
                        m_is_freeable_for_memory_manager = true;
                        break;
                    }
                    if (m_status == Status::ResourcePostInitialized) {
                        const bool is_finalized = this->PreFinalizeResource();
                        if (is_finalized == false) { return ResultFailedToPreFinalizeResource; }
                        this->FinalizeResource();
                    }
                    break;
                }
                case Status::ResourcePostInitialized:
                {
                    const bool is_finalized = this->PreFinalizeResource();
                    if (is_finalized == false) { return ResultFailedToPreFinalizeResource; }                        
                    break;
                }
                case Status::FailedToPostInitializeResource:
                {
                    m_is_cache_unload_for_no_error = false;
                    const bool is_finalized = this->PreFinalizeResource();
                    if (is_finalized == false) { return ResultFailedToPreFinalizeResource; }
                    this->FinalizeResource();
                    m_status = Status::Uninitialized;
                    m_is_freeable_for_memory_manager = true;
                    break;
                }
            };
        }

        /* Add to clear cache if part of res mgr */
        if (m_is_part_of_resource_unit_mgr == false) {

            /* Clear cache sync */
            m_is_finalized = true;
            this->ClearCache();

        } else {

            /* Set as freeable */
            m_is_freeable_for_memory_manager = true;

            /* Add to free cache if cacheable */
            if (m_is_cache_unload == false || m_is_cache_unload_for_no_ref == false || m_is_cache_unload_for_no_error == false || m_memory_manager == nullptr) { RESULT_RETURN_SUCCESS; }
            m_memory_manager->AddResourceUnitToFreeCache(this);
        }

        RESULT_RETURN_SUCCESS;
    }

    Result ResourceUnit::ClearCache() {

        /* Still in reference */
        RESULT_RETURN_IF(0 < m_reference_count, ResultStillInReference);

        /* Cancel the resource */
        this->CancelResource();

        /* Transition to Freed */
        m_status = Status::Freed;

        /* Free resource and resource heap */
        this->FreeResource();
        this->FreeResourceHeap();

        RESULT_RETURN_SUCCESS;
    }

    Result ResourceUnit::TryLoadFile(const char *file_path) {

        /* Setup load context */
        ResourceLoadContext load_context = {
            .file_load_context = {                
                .file_heap = (ArchiveFileDevice::CheckRuntimeTypeInfoStatic(m_file_device) == true) ? nullptr : m_resource_heap,
            },
            .file_device      = m_file_device,
            .resource_factory = m_resource_factory,
            .resource_heap    = m_resource_heap,
        };

        /* Load without decompression if none */
        const CompressionType decompressor_type = static_cast<CompressionType>(m_compression_type);
        if (decompressor_type == CompressionType::None) {
            return res::LoadResource(std::addressof(m_resource), file_path, std::addressof(load_context));
        }

        /* Select decompressor */
        const u32             handle            = res::AllocateDecompressor();
        IDecompressor *decompressor      = res::GetDecompressor(handle, decompressor_type, 0xd, (1 << (sys::GetCurrentCoreNumber() & 0x3f)));

        /* Load */
        const Result result = res::LoadResourceWithDecompressor(std::addressof(m_resource), file_path, std::addressof(load_context), decompressor);

        /* Free decompressor */
        res::FreeDecompressor(handle);

        return result;
    }

    void ResourceUnit::CancelUnloadTask() {
        m_unload_task.CancelTask();
    }
    void ResourceUnit::UnregisterFromResourceUnitManager() {
        if (m_resource_unit_manager == nullptr) { return; }
        m_resource_unit_manager->UnregisterResourceUnit(this);
    }

    bool ResourceUnit::InitializeResource(ResourceUserContext *user_context) {

        /* Guard resource initialize */
        const u32 last_count = vp::util::InterlockedIncrement(std::addressof(m_resource_initialize_guard));
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
        m_load_task.Wait();

        /* Fail if resource is allocated */
        if (m_resource == nullptr) {
            m_status = Status::FailedToInitializeResource;
            return false;
        }

        /* Succeed if resource does not require initialization */
        if (m_is_resource_initializable == false) { return true; }

        /* Release managed reference and update status on exit */
        Status guard_status = Status::FailedToInitializeResource;
        auto no_resource_exit_guard = SCOPE_GUARD {

            /* Release managed reference */
            this->ReleaseManagedReference();

            /* Set status */
            m_status = guard_status;
        };

        /* Track memory usage changes on exit */
        ON_SCOPE_EXIT {
            this->TrackMemoryUsage();
        };

        /* Initialize resource */
        if (m_status == Status::Loaded) {
            m_status            = Status::InResourceInitialize;
            const Result result = m_resource->Initialize(m_resource_heap, m_gpu_heap, user_context, m_resource->GetFile(), m_resource->GetFileSize());
            if (result != ResultSuccess) { return false; }
            guard_status = Status::ResourceInitialized;
            if (m_resource == nullptr) { return false; }
        }
        m_status = Status::ResourceInitialized;

        /* Post initialize resource */
        if (m_is_resource_initializable == true) {
            const Result result = m_resource->PostInitialize(m_resource_heap, m_gpu_heap, user_context, m_resource->GetFile(), m_resource->GetFileSize());
            guard_status        = Status::FailedToPostInitializeResource;
            if (result != ResultSuccess) { return false; }
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
        if (m_is_resource_initializable == true) {
            m_status = Status::InResourcePreFinalize;
            const Result result = m_resource->PreFinalize();
            if (result != ResultSuccess) { m_status = Status::FailedToPreFinalizeResource; return false; }
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
        if ((m_status != Status::ResourcePreFinalized && m_status != Status::FailedToInitializeResource) || m_resource == nullptr || m_is_resource_initializable == false) { m_status = Status::Loaded; return; }

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

        return;
    }

    bool ResourceUnit::CancelResource() {

        /* Advance on status */
        switch (m_status) {
            default:
            case Status::Freed:
                return false;
            case Status::FailedToPreFinalizeResource:
            {
                const bool is_finalized = this->PreFinalizeResource();
                if (is_finalized == false) { return false; }
                break;
            }
            case Status::InLoad:
            {
                /* Cancel load task  */
                m_load_task.CancelTask();

                /* Free archive reference */
                m_archive_resource = nullptr;
                m_archive_binder.Finalize();

                /* Request unload for transience */
                if (m_load_task.IsCancelled() == true && m_is_transient_on_load == true) {
                    m_is_transient_on_load = false;
                    AsyncResourceManager::GetInstance()->RequestUnloadResourceUnit(this);
                }
                [[fallthrough]];
            }
            case Status::InResourceInitialize:
            case Status::ResourceInitialized:
            {
                /* Wait for resource to initialize */
                m_status_update_event.Wait();
                [[fallthrough]];
            }
            case Status::ResourcePostInitialized:
            case Status::FailedToPostInitializeResource:
            {
                /* PreFinalize resource */
                const bool is_finalized = this->PreFinalizeResource();
                if (is_finalized == false) { return false; }
                [[fallthrough]];
            }
            case Status::ResourcePreFinalized:
            case Status::FailedToInitializeResource:
            {
                /* Finalize resource */
                this->FinalizeResource();
                break;
            }
        };

        return true;
    }

	void ResourceUnit::Initialize(ResourceUnitInfo *unit_info) {

        /* Set defaults */
        AsyncResourceLoadInfo *async_resource_load_info = unit_info->async_resource_info;
        m_status                    = Status::Uninitialized;
        m_reference_count           = 0;
        m_deferred_adjust_count[0]  = 0;
        m_deferred_adjust_count[1]  = 0;
        m_resource_initialize_guard = 0;
        m_state                     = 0;

        /* Set manager */
        m_resource_unit_manager     = unit_info->resource_unit_manager;

        /* Clear status event */
        m_status_update_event.Clear();                

        /* Set file path */
        m_file_path  = unit_info->file_path;

        /* Setup user resource size */
        m_is_user_resource_size     = async_resource_load_info->resource_size != 0;
        m_user_resource_size        = async_resource_load_info->resource_size;

        /* Setup external heap */
        mem::Heap *heap = async_resource_load_info->resource_heap;
        if (heap != nullptr) {
            m_is_external_heap = true;
            m_resource_heap    = async_resource_load_info->resource_heap;
        }

        /* Set archive binder */
        if (async_resource_load_info->archive_binder != 0) {
            m_archive_binder.ReferenceBinderSync(async_resource_load_info->archive_binder);
            ArchiveResource *archive_resource = m_archive_binder.GetResource<ArchiveResource>();
            m_archive_resource                = archive_resource;
        }

        /* Set key */
        const u32 hash = vp::util::HashCrc32b(m_file_path.GetString());
        m_resource_unit_manager_tree_node.SetKey(hash);

        /* Setup cache on unload */
        if (async_resource_load_info->is_managed == false && async_resource_load_info->is_cache_on_unload == true) {
            m_is_cache_unload              = true;
            m_is_cache_unload_for_no_ref   = true;
            m_is_cache_unload_for_no_error = true;
        }

        /* Set is managed */
        m_is_managed = async_resource_load_info->is_managed;

        /* Set is transient */
        m_is_transient_on_load = async_resource_load_info->is_transient;

        /* Set heap type */
        m_heap_type = async_resource_load_info->resource_heap_type;

        /* Increment reference for binder or transience */
        ResourceBinder *resource_binder = unit_info->resource_binder;
        if (resource_binder != nullptr) {
            this->SetToBinder(resource_binder);
        } else if (m_is_transient_on_load == true) {
            this->IncrementReference();
        }

        /* Increment reference for managed */
        if (m_is_managed == true) {
            this->IncrementReference();
        }

        return;
    }
}
