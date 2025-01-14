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

namespace awn::res {
    class ResourceBinder;
}

namespace awn::res::impl {

    struct AsyncResourceThreadInfo {
        const char *name;
        u32         core_number;
        u32         priority;
        u32         stack_size;
    };

    using AsyncResourceThreadCreateFunction    = vp::util::IFunction<async::AsyncQueueThread*(mem::Heap*, async::AsyncQueue*, AsyncResourceThreadInfo*)>;
    using AsyncResourceThreadCreateAnyFunction = vp::util::AnyFunction<4, async::AsyncQueueThread*(mem::Heap*, async::AsyncQueue*, AsyncResourceThreadInfo*)>;

    struct AsyncResourceManagerInfo {
        IAsyncResourceApplicationImpl        *app_impl;
        size_t                                zsdic_heap_size;
        const char                           *resource_size_table_path;
        u32                                   max_thread_local_archive_count;
        u32                                   max_resource_unit_count;
        u32                                   load_task_count;
        AsyncResourceThreadInfo               control_thread_info;
        AsyncResourceThreadInfo               memory_thread_info;
        u32                                   load_thread_count;
        AsyncResourceThreadInfo              *load_thread_info_array;
        AsyncResourceThreadCreateAnyFunction  control_thread_create_function;
        AsyncResourceThreadCreateAnyFunction  memory_thread_create_function;
        AsyncResourceThreadCreateAnyFunction  load_thread_create_function;
    };

    using PersistantThreadArray = vp::util::PointerArray<sys::ThreadBase>;

    struct ResourceSizePrepareResult {
        size_t          resource_size;
        bool            is_default_archive_file_device;
        FileDeviceBase *file_device;
    };
    struct ResourceSizePrepareInfo {
        size_t                     user_resource_size;
        s32                        file_alignment;
        ArchiveResource           *archive_resource;
        FileDeviceBase            *file_device;
        const ResourceFactoryBase *resource_factory;
        CompressionType            compression_type;
        const char                *formatted_path;
        const char                *original_path;
    };

    class ResourceMemoryManager;
    class ResourceUnit;

    class AsyncResourceManager {
        public:
            friend class res::ResourceBinder;
            friend class ResourceUnit;
            friend class ResourceMemoryManager;
        public:
            static constexpr u32 cControlPriorityLevelCount = 0x8;
            static constexpr u32 cMemoryPriorityLevelCount  = 0xf;
            static constexpr u32 cLoadPriorityLevelCount    = 0x5;
            static constexpr u32 cComposePriorityLevelCount = 0x5;
            static constexpr u16 cInvalidRequestIndex = 0xffff;
        public:
            using ResourceUnitIndexFrame   = vp::util::BufferedFrameArray<u16, 2>;
            using MemberTaskFunction       = vp::util::MemberFunction<AsyncResourceManager, void(void*)>;
            using MemberResultFunction     = vp::util::MemberFunction<AsyncResourceManager, Result(async::TaskResultInvokeInfo*)>;
            using ResourceUnitFinalizeList = vp::util::IntrusiveListTraits<ResourceUnit, &ResourceUnit::m_finalize_async_res_mgr_list_node>::List;
            using AsyncQueueThreadArray    = vp::util::PointerArray<async::AsyncQueueThread>;
        private:
            mem::Heap                     *m_system_heap;
            IAsyncResourceApplicationImpl *m_app_impl;
            ExtensionManager               m_extension_manager;
            ResourceMemoryManager          m_memory_manager;
            u32                            m_is_trigger_clear_all_caches;
            ResourceUnitAllocator          m_resource_unit_allocator;
            ResourceUnitFinalizeList       m_resource_unit_finalize_list;
            sys::Mutex                     m_finalize_list_cs;

            async::AsyncQueue              m_queue_control;
            async::AsyncQueue              m_queue_memory;
            async::AsyncQueue              m_queue_load;
            async::AsyncQueue              m_queue_compose;
            async::AsyncQueueThread       *m_thread_control;
            async::AsyncQueueThread       *m_thread_memory;
            AsyncQueueThreadArray          m_load_thread_array;
            AsyncQueueThreadArray          m_compose_thread_array;

            async::AsyncTaskAllocator      m_async_task_allocator_load;

            sys::Mutex                     m_unload_frame_cs;
            ResourceUnitIndexFrame         m_unload_frame_buffer;
            sys::Mutex                     m_adjust_ref_frame_cs;
            ResourceUnitIndexFrame         m_adjust_ref_frame_buffer;
            sys::Mutex                     m_heap_adjust_frame_cs;
            ResourceUnitIndexFrame         m_heap_adjust_frame_buffer;

            ArchiveFileDevice              m_default_archive_file_device;
            ArchiveFileDevice              m_default_archive_file_device_for_size_prepare;
            ResourceBinder                 m_default_archive_binder;
            u32                            m_default_archive_reference_count;
            ThreadLocalArchiveManager      m_thread_local_archive_manager;

            ResourceSizeTableManager       m_resource_size_table_manager;

            mem::Heap                     *m_zsdic_heap;
            ParallelDecompressorManager    m_decompressor_manager;

            sys::Mutex                     m_persistant_thread_array_cs;
            PersistantThreadArray         *m_persistant_thread_array;
            bool                           m_is_enable_persistant;

            u32                            m_clear_cache_tick_count;
            u32                            m_clear_cache_tick_counter;

            LoadTask                      *m_task_for_calculate;
            LoadTask                      *m_task_for_load_sync;
            async::AsyncTask              *m_task_for_clear_cache_for_allocate;
            async::AsyncTask              *m_task_for_force_reserve_unload;
            async::AsyncTask              *m_task_for_force_clear_all_caches;
            async::AsyncTask              *m_task_for_heap_adjust;
            MemberTaskFunction             m_calc_exe;
            MemberTaskFunction             m_process_heap_adjust_exe;
            MemberTaskFunction             m_load_control_exe;
            async::StaticTaskFunction      m_load_memory_exe;
            async::StaticResultFunction    m_load_memory_result;
            async::StaticTaskFunction      m_load_file_exe;
            async::StaticResultFunction    m_load_file_result;
            async::StaticTaskFunction      m_compose_exe;
            MemberTaskFunction             m_reserve_unload_exe;
            MemberTaskFunction             m_clear_cache_for_allocate_exe;
            async::StaticTaskFunction      m_heap_adjust_exe;
            async::StaticTaskFunction      m_defer_clear_cache_exe;
            MemberResultFunction           m_defer_clear_cache_result;
            async::StaticTaskFunction      m_clear_cache_exe;
            async::StaticResultFunction    m_clear_cache_result;
            async::StaticTaskFunction      m_unload_exe;
            async::StaticResultFunction    m_unload_result;
            async::StaticTaskFunction      m_unload_for_sync_exe;
            async::StaticResultFunction    m_unload_for_sync_result;
        public:
            AWN_SINGLETON_TRAITS(AsyncResourceManager);
        private:
            static constexpr ALWAYS_INLINE u32 ConvertPriorityControlThreadToMemoryThread(u32 priority) { return (priority * 2) + 1; }
            static constexpr ALWAYS_INLINE u32 ConvertPriorityMemoryThreadToLoadThread(u32 priority) { return (0xa < priority) ? 0xff : priority / 2; }
        private:
            void FreeResourceUnitFromBinder(ResourceBinder *binder);
            void ReserveUnload();

            ResourceUnit *AllocateResourceUnit();
            void FreeResourceUnit(ResourceUnit *resource_unit);

            void RequestAdjustReference(ResourceUnit *resource_unit);
            void RequestUnload(ResourceUnit *resource_unit);
            void CancelUnloadRequest(ResourceUnit *resource_unit);
            void RequestHeapAdjust(ResourceUnit *resource_unit);
            void CancelHeapAdjustRequest(ResourceUnit *resource_unit);

            void ClearAllCaches();

            void ProcessAdjustReference();
            void ProcessUnload();
            void ProcessHeapAdjust();

            void FinalizeResourceUnitSync(ResourceUnit *resource_unit);

            bool TryReferenceResourceUnit(ResourceUnit *resource_unit, LoadTask::LoadUserData *load_info);

            void AddResourceUnitToFinalizeList(ResourceUnit *resource_unit);
            void RemoveResourceUnitFromFinalizeList(ResourceUnit *resource_unit);
        private:
            void LoadControlExe(void *load_task_user_data_arg);

            static void LoadMemoryExe(void *resource_unit);
            static Result LoadMemoryResult(async::TaskResultInvokeInfo *result_info);

            static void LoadFileExe(void *resource_unit);
            static Result LoadFileResult(async::TaskResultInvokeInfo *result_info);

            static void ComposeExe(void *resource_unit);

            void ReserveUnloadExe(void *arg);

            void ClearCacheForAllocateExe(void *count);

            static void HeapAdjustExe(void *resource_unit);

            static void UnloadForSyncExe(void *arg);
            static Result UnloadForSyncResult(async::TaskResultInvokeInfo *result_info);

            static void UnloadExe(void *resource_unit);
            static Result UnloadResult(async::TaskResultInvokeInfo *result_info);

            static void DeferClearCacheExe(void *resource_unit);
            Result DeferClearCacheResult(async::TaskResultInvokeInfo *result_info);
    
            static void ClearCacheExe(void *resource_unit);
            static Result ClearCacheResult(async::TaskResultInvokeInfo *result_info);

            void CalculateExe(void *arg);
            void ProcessHeapAdjustExe(void *arg) {
                VP_UNUSED(arg);
                this->ProcessHeapAdjust();
            }
        public:
            void FinalizeBinder(ResourceBinder *binder);
            void FinalizeBinderSync(ResourceBinder *binder);
        private:
            Result TryLoadAsync(const char *path, ResourceBinder *binder, const AsyncResourceLoadInfo *load_info);
            Result TryLoadSync(const char *path, ResourceBinder *binder, const AsyncResourceLoadInfo *in_load_info);

            Result ReferenceLocalArchiveBinder(ResourceBinder *binder);
        private:
            void CancelLoadTasks();
        public:
            constexpr AsyncResourceManager() : m_system_heap(), m_app_impl(), m_extension_manager(), m_memory_manager(), m_is_trigger_clear_all_caches(), m_resource_unit_allocator(), m_resource_unit_finalize_list(), m_finalize_list_cs(),
            m_queue_control(), m_queue_memory(), m_queue_load(), m_queue_compose(),m_thread_control(), m_thread_memory(), m_load_thread_array(), m_compose_thread_array(), m_async_task_allocator_load(),
            m_unload_frame_cs(), m_unload_frame_buffer(), m_adjust_ref_frame_cs(), m_adjust_ref_frame_buffer(), m_heap_adjust_frame_cs(), m_heap_adjust_frame_buffer(),
            m_default_archive_file_device(), m_default_archive_file_device_for_size_prepare(), m_default_archive_binder(), m_default_archive_reference_count(), m_thread_local_archive_manager(), m_resource_size_table_manager(),
            m_zsdic_heap(), m_decompressor_manager(), m_persistant_thread_array_cs(), m_persistant_thread_array(), m_is_enable_persistant(), m_clear_cache_tick_count(1), m_clear_cache_tick_counter(),
            m_task_for_calculate(), m_task_for_load_sync(), m_task_for_clear_cache_for_allocate(), m_task_for_force_reserve_unload(), m_task_for_force_clear_all_caches(), m_task_for_heap_adjust(),
            m_calc_exe(this, &AsyncResourceManager::CalculateExe), m_process_heap_adjust_exe(this, &AsyncResourceManager::ProcessHeapAdjustExe), m_load_control_exe(this, &AsyncResourceManager::LoadControlExe), m_load_memory_exe(LoadMemoryExe), m_load_memory_result(LoadMemoryResult), m_load_file_exe(LoadFileExe), m_load_file_result(LoadFileResult), m_compose_exe(ComposeExe),
            m_reserve_unload_exe(this, &AsyncResourceManager::ReserveUnloadExe), m_clear_cache_for_allocate_exe(this, &AsyncResourceManager::ClearCacheForAllocateExe), m_heap_adjust_exe(HeapAdjustExe), m_defer_clear_cache_exe(DeferClearCacheExe), m_defer_clear_cache_result(this, &AsyncResourceManager::DeferClearCacheResult), m_clear_cache_exe(ClearCacheExe), m_clear_cache_result(ClearCacheResult),
            m_unload_exe(UnloadExe), m_unload_result(UnloadResult), m_unload_for_sync_exe(UnloadForSyncExe), m_unload_for_sync_result(UnloadForSyncResult) {/*...*/}

            ~AsyncResourceManager() {/*...*/}

            void Initialize(mem::Heap *heap, AsyncResourceManagerInfo *manager_info);
            void Finalize();

            void Calculate();
            void ForceClearAllCaches();
        public:
            u32 AllocateDecompressor();
            void FreeDecompressor(u32 handle);

            IParallelDecompressor *GetParallelDecompressor(u32 handle, CompressionType decompressor_type, u32 priority, sys::CoreMask core_mask);
        public:
            bool IsComposeEnabled() { return false; }

            void SetControlThreadPriority(u32 priority);
            void SetMemoryThreadPriority(u32 priority);
            void SetLoadThreadsPriority(u32 priority);
            void SetComposeThreadsPriority(u32 priority);

            void SuspendControlThread();
            void SuspendMemoryThread();
            void SuspendLoadThreads();
            void SuspendComposeThreads();

            void ResumeControlThread();
            void ResumeMemoryThread();
            void ResumeLoadThreads();
            void ResumeComposeThreads();

            bool IsLoadThreadSuspended();
            bool IsComposeThreadSuspended();

            bool IsCurrentThreadControlThread();
            bool IsCurrentThreadMemoryThread();
            bool IsCurrentThreadLoadThread();
            bool IsCurrentThreadComposeThread();
        public:
            void SetPersistantThreadArray(PersistantThreadArray *array);
            void ReleasePersistantThreadArray();

            bool IsPersistantThread();
        public:
            ResourceMemoryManager *GetMemoryManager(MaxPathString *path);
            CompressionType GetCompressionTypeByExtension(MaxExtensionString *extension);
        public:
            constexpr FileDeviceBase    *GetDefaultFileDevice()        { return FileDeviceManager::GetInstance()->GetDefaultFileDevice(); }
            constexpr ArchiveFileDevice *GetDefaultArchiveFileDevice() { return std::addressof(m_default_archive_file_device); }

            bool CheckArchiveFileExists(ArchiveResource *archive_resource, MaxPathString *path);

            constexpr ThreadLocalArchiveManager *GetThreadLocalArchiveManager() {return std::addressof(m_thread_local_archive_manager); }

            bool SetDefaultArchive(ResourceBinder *archive_binder);
            bool AcquireDefaultArchive(ResourceBinder **out_binder);
            void ReleaseDefaultArchive();
        public:
            void RegisterResourceSizeTable(mem::Heap *heap, void *rsizetable, u32 rsizetable_size);
            void ReleaseResourceSizeTables();

            size_t LookupResourceSize(const char *path);

            void PrepareSize(ResourceSizePrepareResult *out_result, const ResourceSizePrepareInfo *prepare_info);
        public:
            void ReleaseZsDic();

            void LoadZstandardDictionaryArchive(const char *zs_dic_path);
    };
}
