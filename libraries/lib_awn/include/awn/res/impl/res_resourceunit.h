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

    struct LoadUserData;

    class AsyncResourceManager;
    class ResourceMemoryManager;
    class ResourceUnitAllocator;
    class ResourceUnitManager;
    struct ResourceSizePrepareResult;
    struct ResourceSizePrepareInfo;

    struct ResourceUnitInfo {
        AsyncResourceLoadInfo *async_resource_info;
        ResourceBinder *resource_binder;
        ResourceUnitManager *resource_unit_manager;
        const char *file_path;
        bool is_initialize_resource;
        bool is_compose;
        bool is_boost_compose;
        bool is_no_add_to_memory_manager;
    };

    class ResourceUnit {
        public:
            friend class res::ResourceBinder;
            friend class AsyncResourceManager;
            friend class ResourceMemoryManager;
            friend class ResourceUnitAllocator;
            friend class ResourceUnitManager;
        public:
            enum class Status : u32 {
                Uninitialized = 0,
                Freed         = 1,
                Loading       = 2,
                Loaded        = 3,
                Composing     = 4,
                Composed      = 5,
                PostComposed  = 6,
                Decomposing   = 7,
                Decomposed    = 8,
                Finalizing    = 9,
                Finalized     = 10,
                Error         = 11,
                ComposeNG     = 12,
                PostComposeNG = 13,
                DecomposeNG   = 14,
            };
            enum State : u32 {
                State_IsExternalHeap = 0,
                State_IsTrackedMemoryGlobal = 1,
                State_IsTrackedMemoryActive = 2,
                State_IsHeapAdjusted = 3,
                State_IsUserResourceSize = 3,
                State_IsRetryOnBusy = 4,
                State_IsAllowArchiveReference = 5,

                State_IsNoMemory = 7,
                State_IsNoFile = 8,
                State_IsSizeZero = 9,
                State_IsComposeNG = 10,
                State_IsAllocSizeNG = 11,
                State_IsHeapSizeNG = 13,
                State_IsFileDeviceNG = 14,

                State_IsFinalized = 13,
                State_IsPersistant = 14, /* Managed */
                State_IsCacheUnload = 15,
                State_IsRequireHeapAdjust = 18,
                State_IsFreeableForMemoryManager = 19,
                State_IsTransient = 20,
                State_IsBoostCompose = 21,
                State_IsRequireCompose = 22,
            };
            enum CacheUnloadFlag : u32 {
                CacheUnloadFlag_IsValid = 0,
                CacheUnloadFlag_IsTransientRef = 1,
            };
            enum CacheUnloadNoErrorFlag : u32 {
                CacheUnloadNoErrorFlag_IsValid = 0,
            };
            enum CompressionFlag : u32 {
                CompressionFlag_IsNoAddToMemoryMgr = 0,
                CompressionFlag_IsCompressSzs = 1,
                CompressionFlag_IsCompressZstd = 2,
                CompressionFlag_IsCompressMc = 3,
                CompressionFlag_IsComposable = 4,
                CompressionFlag_IsFrameHeap = 6,
                CompressionFlag_IsCompose = 7,
            };
        public:
            using ResourceUnitTreeNode = vp::util::IntrusiveRedBlackTreeNode<vp::util::HashString<u32>>;
        private:
            vp::util::FixedConnection<u32, BITSIZEOF(u32)> m_state;
            vp::util::FixedConnection<u8, BITSIZEOF(u8)> m_compression_flags;
            vp::util::FixedConnection<u8, BITSIZEOF(u8)> m_cache_unload_flags;
            vp::util::FixedConnection<u8, BITSIZEOF(u8)> m_cache_unload_no_error_flags;
            u8                           m_is_part_of_resource_unit_mgr;
            Status                       m_status;
            s32                          m_reference_count;
            s32                          m_deferred_adjust_count;
            s32                          m_compose_reference_count;
            u32                          m_resource_initialize_guard;
            size_t                       m_user_resource_size;
            s32                          m_file_alignment;

            ResourceUnitManager         *m_resource_unit_manager;
            ResourceUnitTreeNode         m_resource_unit_manager_tree_node;
            MaxPathString                m_file_path;
            FileDeviceBase              *m_file_device;
            const ResourceFactoryBase   *m_resource_factory;
            IComposeInfo                *m_compose_info;
            ResourceBinder               m_archive_binder;
            ArchiveResource             *m_archive_resource;
            Resource                    *m_resource;

            async::AsyncTask             m_load_task;
            async::AsyncTask             m_compose_task;
            async::AsyncTask             m_unload_task;

            mem::Heap                   *m_resource_heap;
            ResourceMemoryManager       *m_memory_manager;
            vp::util::IntrusiveListNode  m_finalize_async_res_mgr_list_node;
            vp::util::IntrusiveListNode  m_memory_manager_node;
            vp::util::IntrusiveListNode  m_memory_manager_free_cache_node;
            sys::Event                   m_status_update_event;
        private:
            void AdjustReferenceCount(s32 adjust_amount);

            constexpr u32 GetReferenceCount() const {
                return m_reference_count + m_cache_unload_flags.GetBit(CacheUnloadFlag_IsTransientRef);
            }
            void IncrementReference();
            void DecrementReference();
            void DecrementComposeReferenceCount();

            void ReleaseTransient();
            void ReleasePersistantReference();

            void SetToBinder(ResourceBinder *resource_binder);
            constexpr void SetBinderStatusFromError(ResourceBinder *binder) const {
                if (m_state.GetBit(State_IsNoFile)) { binder->m_status = ResourceBinder::Status::NoFile; return; }
                if (m_state.GetBit(State_IsSizeZero)) { binder->m_status = ResourceBinder::Status::SizeZero; return; }
                if (m_state.GetBit(State_IsComposeNG)) { binder->m_status = ResourceBinder::Status::ComposeNG; return; }
                if (m_state.GetBit(State_IsAllocSizeNG)) { binder->m_status = ResourceBinder::Status::AllocSizeNG; return; }
                if (m_state.GetBit(State_IsHeapSizeNG)) { binder->m_status = ResourceBinder::Status::HeapSizeNG; return; }
                if (m_state.GetBit(State_IsFileDeviceNG)) { binder->m_status = ResourceBinder::Status::FileDeviceNG; return; }
                if (m_state.GetBit(State_IsNoMemory)) { binder->m_status = ResourceBinder::Status::NoMemory; return; }
                binder->m_status = ResourceBinder::Status::Unknown;
                return;
            }

            void ScheduleClearCache();

            void   FinalizeIfOutOfReference();
            Result FreeToResourceUnitAllocator(awn::async::TaskResultInvokeInfo *result_info);

            void ChangeLoadPriority(async::AsyncQueue *memory_queue, async::AsyncQueue *load_queue, u32 priority);
        public:
            void WaitForLoad();

            bool IsLoading() const;
            constexpr bool IsComposed() const {
                /* Succeed if the resource is post initialization */
                if (m_status == Status::PostComposed) { return true; }
                /* Fail if not loaded */
                if (m_status != Status::Loaded) { return false; }
                /* Fail if no resource */
                if (m_resource == nullptr) { return false; }
                /* Succeed if we don't care about initialization */
                if (!m_state.GetBit(State_IsRequireCompose)) { return true; }
                /* Fail if we are loaded and have a resource, it requires initialization, and it is still initializable */
                if (m_compression_flags.GetBit(CompressionFlag_IsComposable)) { return false; }
                return true;
            }
            bool IsUnloadable() const;
            constexpr bool IsErrorStatus() const { return static_cast<u32>(Status::Error) <= static_cast<u32>(m_status);}
        public:
            constexpr CompressionType GetCompressionType() const {
                if (m_compression_flags.GetBit(CompressionFlag_IsCompressZstd)) { return CompressionType::Zstandard; }
                if (m_compression_flags.GetBit(CompressionFlag_IsCompressSzs)) { return CompressionType::Szs; }
                return CompressionType::None;
            }
            constexpr void SetCompressionType(CompressionType compression_type) {
                constexpr u32 cMask = (1 << CompressionFlag_IsCompressSzs) | (1 << CompressionFlag_IsCompressZstd) | (1 << CompressionFlag_IsCompressMc);
                u32 next = m_compression_flags.GetBitMask(0) & (~cMask);
                switch (compression_type) {
                    case CompressionType::Szs:
                        next |= (1 << CompressionFlag_IsCompressSzs);
                        break;
                    case CompressionType::Zstandard:
                        next |= (1 << CompressionFlag_IsCompressZstd);
                        break;
                    default:
                    break;
                };
                m_compression_flags.SetBitMask(next, 0);
                return;
            }
        public:
            void FreeResourceHeap();

            void RequestHeapAdjust();
            void UpdateMemoryUsage();
            void ReleaseMemoryUsageGlobal();
            void ReleaseMemoryUsageActive();
        public:
            Result HeapAdjust();

            void PrepareSizeImpl(ResourceSizePrepareResult *prepare_result, const ResourceSizePrepareInfo *prepare_info);

            size_t PrepareResourceLoad();
            Result LoadMemory();
            Result LoadMemoryResult(async::TaskResultInvokeInfo *result_info);
            
            Result LoadFile();
            void RequestComposeForAfter();

            Result Compose();

            Result Unload();
            Result ClearCache();
        public:
            void ScheduleCompose(u32 priority);

            Result TryLoadFile(const char *file_path);
            bool ScheduleUnload();
        public:
            bool CheckOrWaitUnloaded(bool is_wait);
            void CancelUnload(AsyncResourceManager *manager);

            void UnregisterFromResourceUnitManager();
            void FinalizeForClearCache();

            bool RecoverFromCache(LoadUserData *load_info);
        public:
            bool ComposeResource(IComposeInfo *info);

            bool PreFinalizeResource();
            void FinalizeResource();

            void FreeResource();
            bool CancelResource();

            constexpr Resource *GetResource() { return m_resource; }
        public:
            ResourceUnit();
            ~ResourceUnit();

            void Initialize(ResourceUnitInfo *unit_info);
    };
}
