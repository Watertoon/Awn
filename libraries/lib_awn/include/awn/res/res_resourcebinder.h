#pragma once

namespace awn::res {

    class ResourceBinder;

    struct AsyncResourceLoadInfo {
        union {
            u32 options;
            struct {
                u32 priority                   : 4;
                u32 is_allow_loose_files       : 1 = true;
                u32 is_allow_archive_reference : 1 = true;
                u32 is_allow_compression       : 1 = true;
                u32 is_require_heap_adjust     : 1;
                u32 is_transient               : 1;
                u32 is_persistant              : 1;
                u32 is_cache_on_unload         : 1 = true;
                u32 is_compose                 : 1;
                u32 compression_type           : 3;
                u32 resource_heap_type         : 2 = 1;
                u32 reserve0                   : 15;
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
            constexpr T *GetResource() const {
                Resource *resource = this->GetResourceDirect();
                return (T::CheckRuntimeTypeInfoStatic(resource)) ? reinterpret_cast<T*>(resource) : nullptr;
            }

            Result ReferenceBinderAsync(ResourceBinder *binder_to_reference);
            Result ReferenceBinderSync(ResourceBinder *binder_to_reference);
            Result ReferenceLocalArchiveSync();

            Resource *GetResourceDirect() const;

            constexpr Status GetStatus() const { return m_status; }
            constexpr bool IsErrorStatus() const { return static_cast<u32>(Status::UnknownError) <= static_cast<u32>(m_status); }

            constexpr bool IsLoadGuard() const { return m_load_guard; }
            constexpr bool IsCompleteGuard() const { return m_complete_guard; }

            constexpr bool IsReferenceBinder() const { return m_type == Type::Reference; }

            bool IsResourceInitialized() const;
            bool IsInLoad() const;
            bool IsLoaded() const;
            bool IsFailed() const;
    };
}
