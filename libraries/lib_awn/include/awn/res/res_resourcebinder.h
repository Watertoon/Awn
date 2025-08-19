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

    enum class ResourceHeapType : u32 {
        FrameHeap = 0,
        ExpHeap   = 1,
    };

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
                u32 is_persistant              : 1;
                u32 is_cache_on_unload         : 1;
                u32 is_compose                 : 1;
                u32 compression_type           : 3;
                u32 resource_heap_type         : 2;
                u32 reserve0                   : 15;
            };
        };
        s32                        file_alignment;
        size_t                     resource_size;
        mem::Heap                 *resource_heap;
        const ResourceFactoryBase *resource_factory;
        FileDeviceBase            *file_device;
        ResourceBinder            *archive_binder;
        ResourceUserContext       *resource_user_context;

        constexpr void SetDefaults() {
            this->options = 0;
            this->priority = 4;
            this->is_allow_loose_files = true;
            this->is_allow_archive_reference = true;
            this->is_allow_compression = true;
            this->is_cache_on_unload = true;
            this->resource_heap_type = static_cast<u32>(ResourceHeapType::FrameHeap);
            this->file_alignment = 0;
            this->resource_size = 0;
            this->resource_heap = nullptr;
            this->resource_factory = nullptr;
            this->file_device = nullptr;
            this->archive_binder = nullptr;
            this->resource_user_context = nullptr;
        }
        constexpr void SetPriority(u32 priority) {
            this->priority = priority;
        }
        constexpr void SetIsAllowLooseFiles(bool is_allow_loose_files) {
            this->is_allow_loose_files = is_allow_loose_files;
        }
        constexpr void SetIsAllowArchiveReference(bool is_allow_archive_reference) {
            this->is_allow_archive_reference = is_allow_archive_reference;
        }
        constexpr void SetIsAllowCompression(bool is_allow_compression) {
            this->is_allow_compression = is_allow_compression;
        }
        constexpr void SetIsForceHeapAdjust(bool is_require_heap_adjust) {
            this->is_require_heap_adjust = is_require_heap_adjust;
        }
        constexpr void SetIsTransient(bool is_transient) {
            this->is_transient = is_transient;
        }
        constexpr void SetIsPersistant(bool is_persistant) {
            this->is_persistant = is_persistant;
        }
        constexpr void SetIsCacheOnUnload(bool is_cache_on_unload) {
            this->is_cache_on_unload = is_cache_on_unload;
        }
        constexpr void SetIsCompose(bool is_compose) {
            this->is_compose = is_compose;
        }
        constexpr void SetCompressionType(CompressionType compression_type) {
            this->compression_type = static_cast<u32>(compression_type);
        }
        constexpr void SetResourceHeapType(ResourceHeapType resource_heap_type) {
            this->resource_heap_type = static_cast<u32>(resource_heap_type);
        }
        constexpr void SetFileAlignment(u32 alignment) {
            this->file_alignment = alignment;
        }
        constexpr void SetResourceHeapSize(size_t resource_heap_size) {
            this->resource_size = resource_heap_size;
        }
        constexpr void SetResourceHeap(mem::Heap *resource_heap) {
            this->resource_heap = resource_heap;
        }
        constexpr void SetResourceFactory(const ResourceFactoryBase *resource_factory) {
            this->resource_factory = resource_factory;
        }
        constexpr void SetFileDevice(FileDeviceBase *file_device) {
            this->file_device = file_device;
        }
        constexpr void SetArchiveBinder(ResourceBinder *archive_binder) {
            this->archive_binder = archive_binder;
        }
        constexpr void SetResourceUserContext(ResourceUserContext *resource_user_context) {
            this->resource_user_context = resource_user_context;
        }
    };

    namespace impl {        
        class ResourceUnit;
        class AsyncResourceManager;
    }

    class ResourceBinder {
        public:
            friend class impl::ResourceUnit;
            friend class impl::AsyncResourceManager;
        public:
            enum class Status : u32 {
                Uninitialized               = 0,
                ResourceInitialized         = 1,
                Referenced                  = 2,
                Reclaimed                   = 3,
                InLoad                      = 4,
                /* Error statuses */
                UnknownError                = 5,
                RequiresReprocess           = 6,
                FileNotFound                = 7,
                FailedToGetDecompressedSize = 8,
                FailedToInitializeResource  = 9,
                InvalidUserResourceSize     = 10,
                InvalidResourceSize         = 11,
                FileNotAvailable            = 12,
                MemoryAllocationFailure     = 13,
                NoResourceUnitOnFinalize    = 14,
            };
            enum class Type : u8 {
                Uninitialized = 0,
                Load = 1,
                Reference = 3,
            };
        private:
            union {
                u8 m_state_mask;
                struct {
                    u8 m_is_initialized : 1;
                    u8 m_load_guard     : 1;
                    u8 m_is_finalize    : 1;
                    u8 m_complete_guard : 1;
                    u8 m_reserve0       : 4;
                };
            };
            Type m_type;
            Status m_status;
            impl::ResourceUnit *m_resource_unit;
            async::AsyncTaskWatcher m_watcher;
        private:
            void HandleMemoryAllocationError() {/*...*/}
            void HandleResourceUnitError();

            void InitializeResource(ResourceUserContext *resource_user_context);
            Resource *GetResourceDirect() const;
        public:
            constexpr  ResourceBinder() : m_state_mask(), m_type(Type::Uninitialized), m_status(Status::Uninitialized), m_resource_unit(), m_watcher() {/*...*/}
            ~ResourceBinder() { this->Release(); }

            void Release();
            void ReleaseSync();

            Result TryLoadSync(const char *file_path, const AsyncResourceLoadInfo *bind_info);

            Result TryLoadAsync(const char *file_path, const AsyncResourceLoadInfo *bind_info);
            void WaitForLoad();
            bool Complete(ResourceUserContext *resource_user_context);

            template <typename T>
                requires (Resource::CheckRuntimeTypeInfoStatic(T::GetRuntimeTypeInfoStatic()))
            T *GetResource() const {
                Resource *resource = this->GetResourceDirect();
                if constexpr (T::GetRuntimeTypeInfoStatic() == Resource::GetRuntimeTypeInfoStatic()) {
                    return resource;
                } else {
                    return (T::CheckRuntimeTypeInfoStatic(resource)) ? reinterpret_cast<T*>(resource) : nullptr;
                }
            }
        public:
            Result ReferenceBinderAsync(ResourceBinder *binder_to_reference);
            Result ReferenceBinderSync(ResourceBinder *binder_to_reference);
            Result ReferenceLocalArchiveSync();
        public:
            constexpr Status GetStatus() const { return m_status; }
            constexpr bool IsErrorStatus() const { return static_cast<u32>(Status::UnknownError) <= static_cast<u32>(m_status); }

            constexpr bool IsLoadGuard() const { return m_load_guard; }
            constexpr bool IsCompleteGuard() const { return m_complete_guard; }

            constexpr bool IsReferenceBinder() const { return m_type == Type::Reference; }

            bool IsLoading() const;
            bool IsLoaded() const;
            bool IsFailed() const;
            bool IsCompleted() const;
    };
}
