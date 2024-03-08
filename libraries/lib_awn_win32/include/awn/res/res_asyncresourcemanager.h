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

    void FormatPathWithCompressionExtension(MaxPathString *out_string, MaxPathString *file_path, CompressionType compression_type);

    class IAsyncResourceApplicationImpl {
        public:
            constexpr IAsyncResourceApplicationImpl() {/*...*/}
            virtual ~IAsyncResourceApplicationImpl() {/*...*/}
            
            virtual void Initialize(mem::Heap *heap) { VP_UNUSED(heap); }
            virtual u32 GetExtensionArrayCount() const { return 0; }
            virtual ExtensionInfo *GetExtensionArray() const { return nullptr; }
            
            virtual constexpr bool HasDefaultFileDevice() const { return false; }
            virtual FileDeviceBase *GetDefaultFileDevice(FileDeviceBase *base_device, const char *formatted_path, const char *original_path) { VP_UNUSED(base_device, formatted_path, original_path); return nullptr; }
    };

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

    using ManagedThreadArray = vp::util::PointerArray<sys::ThreadBase>;

    struct ResourceSizePrepareResult {
        size_t          resource_size;
        bool            is_default_archive_file_device;
        FileDeviceBase *file_device;
    };
    struct ResourceSizePrepareInfo {
        size_t               user_resource_size;
        s32                  file_alignment;
        ArchiveResource     *archive_resource;
        FileDeviceBase      *file_device;
        ResourceFactoryBase *resource_factory;
        CompressionType      compression_type;
        const char          *formatted_path;
        const char          *original_path;
    };

    class ResourceBinder;
    class ResourceMemoryManager;
    class ResourceUnit;

    class AsyncResourceManager {
        public:
            friend class ResourceBinder;
            friend class ResourceUnit;
            friend class ResourceMemoryManager;
        public:
            static constexpr u32 cControlPriorityLevelCount = 0x8;
            static constexpr u32 cMemoryPriorityLevelCount  = 0xf;
            static constexpr u32 cLoadPriorityLevelCount    = 0x5;
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
            sys::ServiceCriticalSection    m_finalize_list_cs;

            async::AsyncQueueThread       *m_async_queue_thread_control;
            async::AsyncQueueThread       *m_async_queue_thread_memory;
            async::AsyncQueue              m_async_queue_control;
            async::AsyncQueue              m_async_queue_memory;
            async::AsyncQueue              m_async_queue_load;
            AsyncQueueThreadArray          m_async_queue_load_thread_array;

            async::AsyncTaskAllocator      m_async_task_allocator_load;

            sys::ServiceCriticalSection    m_free_frame_cs;
            ResourceUnitIndexFrame         m_resource_unit_free_frame_buffer;

            FileDeviceBase                *m_default_file_device;
            ArchiveFileDevice              m_default_archive_file_device;
            ArchiveFileDevice              m_default_archive_file_device_for_size_prepare;
            ResourceBinder                 m_default_archive_binder;
            u32                            m_default_archive_reference_count;
            ThreadLocalArchiveManager      m_thread_local_archive_manager;

            ResourceSizeTableManager       m_resource_size_table_manager;

            DecompressorManager            m_decompressor_manager;

            sys::ServiceCriticalSection    m_managed_thread_array_cs;
            ManagedThreadArray            *m_managed_thread_array;
            bool                           m_enable_managed;

            LoadTask                      *m_task_for_calculate;
            LoadTask                      *m_task_for_load_sync;
            async::AsyncTask              *m_task_for_clear_cache_for_allocate;
            async::AsyncTask              *m_task_for_force_reserve_unload;
            async::AsyncTask              *m_task_for_force_clear_all_caches;
            MemberTaskFunction             m_calc_exe;
            MemberTaskFunction             m_load_async_exe;
            MemberTaskFunction             m_schedule_load_exe;
            MemberResultFunction           m_schedule_load_result;
            MemberTaskFunction             m_load_file_exe;
            MemberTaskFunction             m_reserve_unload_exe;
            MemberTaskFunction             m_clear_cache_for_allocate_exe;
            async::StaticTaskFunction      m_heap_adjust_exe;
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
            void          FreeResourceUnit(ResourceUnit *resource_unit);

            void RequestUnloadResourceUnit(ResourceUnit *resource_unit);
            void FinalizeResourceUnitSync(ResourceUnit *resource_unit);

            bool TryReferenceResourceUnit(ResourceUnit *resource_unit, LoadTask::LoadUserData *load_info);

            void AddResourceUnitToFinalizeList(ResourceUnit *resource_unit);
            void RemoveResourceUnitFromFinalizeList(ResourceUnit *resource_unit);
        private:
            void LoadAsyncExe(void *load_task_user_data_arg);

            void   ScheduleLoadExe(void *resource_unit);
            Result ScheduleLoadResult(async::TaskResultInvokeInfo *result_info);

            void LoadFileExe(void *resource_unit);
            
            void ReserveUnloadExe(void *arg);

            void ClearCacheForAllocateExe(void *count) {
                m_memory_manager.ClearCacheForAllocate(static_cast<u32>(reinterpret_cast<uintptr_t>(count)));
            }

            static void HeapAdjustExe(void *resource_unit);

            static void UnloadForSyncExe(void *arg);
            static Result UnloadForSyncResult(async::TaskResultInvokeInfo *result_info);

            static void   UnloadExe(void *resource_unit);
            static Result UnloadResult(async::TaskResultInvokeInfo *result_info);

            static void ClearCacheExe(void *resource_unit);
            static Result ClearCacheResult(async::TaskResultInvokeInfo *result_info);

            void CalculateExe(void *arg);
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
            m_async_queue_thread_control(), m_async_queue_thread_memory(), m_async_queue_control(), m_async_queue_memory(), m_async_queue_load(), m_async_queue_load_thread_array(), m_async_task_allocator_load(),
            m_free_frame_cs(), m_resource_unit_free_frame_buffer(), 
            m_default_file_device(), m_default_archive_file_device(), m_default_archive_file_device_for_size_prepare(), m_default_archive_binder(), m_default_archive_reference_count(), m_thread_local_archive_manager(), m_resource_size_table_manager(),
            m_managed_thread_array_cs(), m_managed_thread_array(), m_enable_managed(),
            m_task_for_calculate(), m_task_for_load_sync(), m_task_for_clear_cache_for_allocate(), m_task_for_force_reserve_unload(), m_task_for_force_clear_all_caches(),

            m_calc_exe(this, CalculateExe), m_load_async_exe(this, LoadAsyncExe), m_schedule_load_exe(this, ScheduleLoadExe), m_schedule_load_result(this, ScheduleLoadResult), m_load_file_exe(this, LoadFileExe), 
            m_reserve_unload_exe(this, ReserveUnloadExe), m_clear_cache_for_allocate_exe(this, ClearCacheForAllocateExe), m_heap_adjust_exe(HeapAdjustExe), m_clear_cache_exe(ClearCacheExe), m_clear_cache_result(ClearCacheResult),
            m_unload_exe(UnloadExe), m_unload_result(UnloadResult), m_unload_for_sync_exe(UnloadForSyncExe), m_unload_for_sync_result(UnloadForSyncResult) {/*...*/}

            ~AsyncResourceManager() {/*...*/}

            void Initialize(mem::Heap *heap, AsyncResourceManagerInfo *manager_info);
            void Finalize();

            void Calculate();
            void ForceClearAllCaches();
        public:
            u32 AllocateDecompressor() {
                return m_decompressor_manager.AllocateDecompressorHandle();
            }
            void FreeDecompressor(u32 handle) {
                m_decompressor_manager.FreeDecompressorHandle(handle);
            }
            IDecompressor *GetDecompressor(u32 handle, CompressionType decompressor_type, u32 priority, sys::CoreMask core_mask) {
                return m_decompressor_manager.GetDecompressor(handle, decompressor_type, priority, core_mask);
            }
        public:
            void SetControlThreadPriority(u32 priority) { m_async_queue_thread_control->SetPriority(priority); }
            void SetMemoryThreadPriority(u32 priority)  { m_async_queue_thread_memory->SetPriority(priority); }
            void SetLoadThreadsPriority(u32 priority)   {
                for (async::AsyncQueueThread *&thread : m_async_queue_load_thread_array) {
                    thread->SetPriority(priority); 
                }
            }

            void SuspendControlThread();
            void SuspendMemoryThread();
            void SuspendLoadThreads();

            void ResumeControlThread();
            void ResumeMemoryThread();
            void ResumeLoadThreads();

            bool IsLoadThreadSuspended() {
                for (async::AsyncQueueThread *&thread : m_async_queue_load_thread_array) {
                    if (thread->IsSuspended() == true) { return true; }
                }
                return false;
            }
        public:
            void SetManagedThreadArray(ManagedThreadArray *array) {
                std::scoped_lock l(m_managed_thread_array_cs);
                m_managed_thread_array = array;
                m_enable_managed       = true;
            }
            void ReleaseManagedThreadArray() {
                std::scoped_lock l(m_managed_thread_array_cs);
                m_managed_thread_array = nullptr;
                m_enable_managed       = false;
            }

            bool IsManagedThread() {
                std::scoped_lock l(m_managed_thread_array_cs);
                if (m_enable_managed) { return false; }
                for (async::AsyncQueueThread *&thread : m_async_queue_load_thread_array) {
                    if (thread->IsSuspended() == true) { return true; }
                }
                return false;
            }
        public:
            ResourceMemoryManager *GetMemoryManager(MaxPathString *path);
            CompressionType GetCompressionTypeByExtension(MaxExtensionString *extension);
        public:
            constexpr FileDeviceBase    *GetDefaultFileDevice() { return m_default_file_device; }
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
