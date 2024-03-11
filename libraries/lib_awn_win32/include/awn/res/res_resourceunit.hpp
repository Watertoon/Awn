#pragma once

namespace awn::res {

    class ResourceBinder;
    class ResourceMemoryManager;
    class ResourceUnitManager;
    class AsyncResourceManager;
    struct ResourceSizePrepareResult;
    struct ResourceSizePrepareInfo;

    struct AsyncResourceLoadInfo {
        union {
            u32 options;
            struct {
                u32 priority                   : 4;
                u32 is_allow_loose_files       : 1;
                u32 is_allow_archive_reference : 1;
                u32 is_allow_compression       : 1;
                u32 is_require_heap_adjust     : 1;
                u32 is_transient               : 1;
                u32 is_managed                 : 1;
                u32 is_cache_on_unload         : 1;
                u32 compression_type           : 3;
                u32 resource_heap_type         : 2;
                u32 reserve0                   : 18;
            };
        };
        s32                  file_alignment;
        size_t               resource_size;
        mem::Heap           *resource_heap;
        ResourceFactoryBase *resource_factory;
        FileDeviceBase      *file_device;
        ResourceBinder      *archive_binder;
        ResourceUserContext *resource_user_context;
    };

    struct ResourceUnitInfo {
        AsyncResourceLoadInfo *async_resource_info;
        ResourceBinder        *resource_binder;
        ResourceUnitManager   *resource_unit_manager;
        const char            *file_path;
    };

    class ResourceUnit {
        public:
            friend class ResourceBinder;
            friend class ResourceMemoryManager;
            friend class ResourceUnitManager;
            friend class AsyncResourceManager;
        public:
            enum class Status : u32 {
                Uninitialized                  = 0,
                Freed                          = 1,
                InLoad                         = 2,
                Loaded                         = 3,
                InResourceInitialize           = 4,
                ResourceInitialized            = 5,
                ResourcePostInitialized        = 6,
                InResourcePreFinalize          = 7,
                ResourcePreFinalized           = 8,
                InResourceFinalize             = 9,
                ResourceFinalized              = 10,
                Error                          = 11,
                FailedToInitializeResource     = 12,
                FailedToPostInitializeResource = 13,
                FailedToPreFinalizeResource    = 14,
            };
        public:
            using ResourceUnitTreeNode = vp::util::IntrusiveRedBlackTreeNode<u32>;
        private:
            union {
                u32 m_state;
                struct {
                    u32 m_is_resource_initializable      : 1;
                    u32 m_is_external_heap               : 1;
                    u32 m_is_tracked_memory_usage_global : 1;
                    u32 m_is_tracked_memory_usage_active : 1;
                    u32 m_is_heap_adjusted               : 1;
                    u32 m_has_user_resource_size         : 1;
                    u32 m_is_retry_on_busy               : 1;
                    u32 m_allow_archive_reference        : 1;
                    u32 m_is_file_not_found              : 1;
                    u32 m_is_fail_get_file_size          : 1;
                    u32 m_is_fail_init_resource          : 1;
                    u32 m_is_bad_user_resource_size      : 1;
                    u32 m_is_bad_resource_size           : 1;
                    u32 m_is_file_not_available          : 1;
                    u32 m_is_memory_allocation_failure   : 1;
                    u32 m_is_finalized                   : 1;
                    u32 m_is_managed                     : 1;
                    u32 m_is_cache_unload                : 1;
                    u32 m_is_cache_unload_for_no_ref     : 1;
                    u32 m_is_cache_unload_for_no_error   : 1;
                    u32 m_is_require_heap_adjust         : 1;
                    u32 m_is_part_of_resource_unit_mgr   : 1;
                    u32 m_is_freeable_for_memory_manager : 1;
                    u32 m_is_transient_on_load           : 1;
                    u32 m_is_user_resource_size          : 1;
                    u32 m_heap_type                      : 3;
                    u32 m_compression_type               : 3;
                    u32 m_reserve0                       : 1;
                };
            };
            Status                       m_status;
            s32                          m_reference_count;
            s32                          m_deferred_adjust_count[2];
            u32                          m_resource_initialize_guard;
            size_t                       m_user_resource_size;
            s32                          m_file_alignment;

            ResourceUnitManager         *m_resource_unit_manager;
            ResourceUnitTreeNode         m_resource_unit_manager_tree_node;
            MaxPathString                m_file_path;
            FileDeviceBase              *m_file_device;
            ResourceFactoryBase         *m_resource_factory;
            ResourceBinder               m_archive_binder;
            ArchiveResource             *m_archive_resource;
            Resource                    *m_resource;

            async::AsyncTask             m_load_task;
            async::AsyncTask             m_heap_adjust_task;
            async::AsyncTask             m_unload_task;

            mem::Heap                   *m_resource_heap;
            mem::Heap                   *m_gpu_heap;
            ResourceMemoryManager       *m_memory_manager;
            vp::util::IntrusiveListNode  m_finalize_async_res_mgr_list_node;
            vp::util::IntrusiveListNode  m_memory_manager_node;
            vp::util::IntrusiveListNode  m_memory_manager_free_cache_node;
            sys::ServiceEvent            m_status_update_event;
        private:
            void AdjustReferenceCount(s32 adjust_amount);

            void IncrementReference();
            void DecrementReference();

            void ReleaseManagedReference();

            void SetToBinder(ResourceBinder *resource_binder);
            constexpr void SetBinderStatusFromError(ResourceBinder *binder) const {

                /* Convert error */
                if (m_is_file_not_found == true)            { binder->m_status = ResourceBinder::Status::FileNotFound;                return; }
                if (m_is_fail_get_file_size == true)        { binder->m_status = ResourceBinder::Status::FailedToGetDecompressedSize; return; }
                if (m_is_fail_init_resource == true)        { binder->m_status = ResourceBinder::Status::FailedToInitializeResource;  return; }
                if (m_is_bad_user_resource_size == true)    { binder->m_status = ResourceBinder::Status::InvalidUserResourceSize;     return; }
                if (m_is_bad_resource_size == true)         { binder->m_status = ResourceBinder::Status::InvalidResourceSize;         return; }
                if (m_is_file_not_available == true)        { binder->m_status = ResourceBinder::Status::FileNotAvailable;            return; }
                if (m_is_memory_allocation_failure == true) { binder->m_status = ResourceBinder::Status::MemoryAllocationFailure;     return; }

                binder->m_status = ResourceBinder::Status::UnknownError;

                return;
            }

            void ScheduleClearCache();

            void   FinalizeIfOutOfReference();
            Result FreeToResourceUnitAllocator(awn::async::TaskResultInvokeInfo *result_info);

            void ChangeLoadPriority(async::AsyncQueue *memory_queue, async::AsyncQueue *load_queue, u32 priority);
        public:
            void WaitForLoad() { m_load_task.Wait(); }

            constexpr bool IsInLoad() const {
                const async::AsyncTask::Status status = m_load_task.GetStatus();
                if ((async::AsyncTask::Status::Uninitialized == status) || (async::AsyncTask::Status::Complete == status)) { return (m_status == Status::InLoad); }
                if (async::AsyncTask::Status::Cancelled != status)                                                         { return true; }
                return false;
            }
            constexpr bool IsLoaded() const {
                const async::AsyncTask::Status status = m_load_task.GetStatus();
                return (async::AsyncTask::Status::Cancelled == status) || (((async::AsyncTask::Status::Uninitialized == status) || (async::AsyncTask::Status::Complete == status)) && (m_status != Status::InLoad));
            }
            constexpr bool IsResourceInitialized() const {
                if (m_status == Status::ResourceInitialized) { return true; }
                return (m_resource != nullptr) && (m_status == Status::Loaded) && (m_is_resource_initializable == false);
            }
            constexpr bool IsErrorStatus() const { return static_cast<u32>(Status::Error) <= static_cast<u32>(m_status);}
        public:
            void FreeResourceHeap();

            void TrackMemoryUsage();

            void UpdateMemoryUsage();

            void ReleaseMemoryUsageGlobal();
            void ReleaseMemoryUsageActive();
        public:
            Result HeapAdjust();

            void PrepareSizeImpl(ResourceSizePrepareResult *prepare_result, const ResourceSizePrepareInfo *prepare_info);

            size_t PrepareResourceLoad();
            Result ScheduleLoad();
            Result LoadFile();

            Result Unload();
            Result ClearCache();
        public:
            Result TryLoadFile(const char *file_path);
            void   ScheduleUnload();
        public:
            void CancelUnloadTask();

            void UnregisterFromResourceUnitManager();

            constexpr void UpdateStatusForReference() {
                const Status status   = m_status;
                if      (status == Status::InResourceFinalize) { m_status = Status::ResourcePreFinalized; }
                else if (status == Status::InResourceFinalize) { m_status = Status::ResourcePostInitialized; }
                else if (status == Status::InResourceFinalize) { m_status = Status::ResourcePostInitialized; }
            }
        public:
            bool InitializeResource(ResourceUserContext *user_context);

            bool PreFinalizeResource();
            void FinalizeResource();

            void FreeResource();
            bool CancelResource();

            constexpr Resource *GetResource() { return m_resource; }
        public:
            ResourceUnit() : m_state(), m_status(Status::Uninitialized), m_reference_count(), m_deferred_adjust_count{}, m_resource_initialize_guard(), m_user_resource_size(), m_file_alignment(), m_resource_unit_manager(),
                             m_resource_unit_manager_tree_node(), m_file_path(), m_file_device(), m_resource_factory(), m_archive_binder(), m_archive_resource(), m_resource(), m_load_task(), m_heap_adjust_task(), m_unload_task(), 
                             m_resource_heap(), m_gpu_heap(), m_memory_manager(), m_finalize_async_res_mgr_list_node(), m_memory_manager_node(), m_memory_manager_free_cache_node(), m_status_update_event() 
            {
                m_status_update_event.Initialize(sys::SignalState::Cleared, sys::ResetMode::Manual);
            }
            ~ResourceUnit() {/*...*/}

            void Initialize(ResourceUnitInfo *unit_info);
    };

    constexpr Resource *ResourceBinder::GetResourceDirect() const {

        /* Check load guard */
        if (m_complete_guard == false) { return nullptr; }
        /* Check watcher is pending */
        if (m_watcher.IsPending() == true) { return nullptr; }
        /* Check resource unit exists */
        if (m_resource_unit == nullptr) { return nullptr; }
        /* Check resource unit is initialized */
        if (m_resource_unit->IsResourceInitialized() == false) { return nullptr; }
        /* Check completion state */
        if (m_complete_guard == false) { return nullptr; }

        Resource *resource = m_resource_unit->m_resource;
        /* Succeed if resource unit is initialized */
        if (m_resource_unit->m_status == ResourceUnit::Status::Loaded) { return resource; }
        /* Check resource exists */
        if (resource == nullptr) { return nullptr; }
        /* Check resource unit status */
        if (m_resource_unit->m_status != ResourceUnit::Status::Loaded) { return nullptr; }
        /* Check resource is initialized */
        //if (m_resource_unit->m_is_require_resource_initalize == true) { return nullptr; }

        return m_resource_unit->m_resource;
    }

    constexpr bool ResourceBinder::IsResourceInitialized() const {

        /* Ensure load guard set */
        if (m_load_guard == false)                { return false; }
        /* Ensure watcher not pending */
        if (m_watcher.IsPending() == true)        { return false; }
        /* Ensure resource unit */
        if (m_resource_unit == nullptr)           { return false; }
        /* Ensure resource unit is loaded */
        if (m_resource_unit->IsLoaded() == false) { return false; }
        /* Ensure binder is completed */
        if (m_complete_guard == false)            { return false; }

        /* Check if resource is initialized */
        return m_resource_unit->IsResourceInitialized();
    }

    constexpr bool ResourceBinder::IsInLoad() const {

        /* Succeed if the watcher is pending */
        if (m_watcher.IsPending() == true) { return true; }
        /* Ensure the load guard is set */
        if (m_load_guard == false)         { return false; }
        /* Ensure there's a resource unit */
        if (m_resource_unit == nullptr)    { return false; }

        /* Check if the resource unit is in load */
        return m_resource_unit->IsInLoad();
    }

    constexpr bool ResourceBinder::IsLoaded() const {

        /* Ensure the load guard is set */
        if (m_load_guard == false)                    { return false; }
        /* Ensure finalize guard is not set */
        if (m_is_finalize == true)                    { return false; }
        /* Ensure the load task is pending */
        if (m_watcher.IsPending() == true)            { return false; }
        /* Ensure there's a resource unit */
        if (m_resource_unit == nullptr)               { return false; }

        /* Check if the resource unit is loaded */
        return m_resource_unit->IsLoaded();
    }

    constexpr bool ResourceBinder::IsFailed() const {

        /* Ensure not in load */
        if (this->IsInLoad() == true) { return false; }
        /* If there's a resource unit check if it's failed */
        bool is_res_unit_failed = false;
        if (m_load_guard == true && m_resource_unit != nullptr) {
            is_res_unit_failed = m_resource_unit->IsErrorStatus();
        }

        /* Check if binder had an error, or resource unit failed */
        return this->IsErrorStatus() | is_res_unit_failed;
    }
}
