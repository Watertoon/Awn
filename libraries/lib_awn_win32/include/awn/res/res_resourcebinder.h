#pragma once

namespace awn::res {

    class ResourceUnit;
    class AsyncResourceManager;
    struct AsyncResourceLoadInfo;

    class ResourceBinder {
        public:
            friend class ResourceUnit;
            friend class AsyncResourceManager;
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
        private:
            union {
                u32 m_state_mask;
                struct {
                    u32 m_is_initialized : 1;
                    u32 m_load_guard     : 1;
                    u32 m_is_finalize    : 1;
                    u32 m_complete_guard : 1;
                    u32 m_reserve0       : 28;
                };
            };
            Status                   m_status;
            ResourceUnit            *m_resource_unit;
            async::AsyncTaskWatcher  m_watcher;
        private:
            constexpr bool IsErrorStatus() const {
                return static_cast<u32>(Status::UnknownError) <= static_cast<u32>(m_status);
            }

            void HandleMemoryAllocationError() {/*...*/}
            void HandleResourceUnitError();

            void InitializeResource(ResourceUserContext *resource_user_context);
        public:
            constexpr  ResourceBinder() {/*...*/}
            ~ResourceBinder() { this->Finalize(); }

            void Finalize();

            Result TryLoadSync(const char *file_path, const AsyncResourceLoadInfo *bind_info);
            Result TryLoadAsync(const char *file_path, const AsyncResourceLoadInfo *bind_info);

            Result ReferenceBinderAsync(ResourceBinder *binder_to_reference);
            Result ReferenceBinderSync(ResourceBinder *binder_to_reference);

            Result ReferenceLocalArchiveSync();

            void WaitForLoad();
            bool Complete(ResourceUserContext *resource_user_context);

            template <typename T>
                requires (Resource::CheckRuntimeTypeInfoStatic(T::GetRuntimeTypeInfoStatic()) == true)
            constexpr T *GetResource() const {
                Resource *resource = this->GetResourceDirect();
                return (T::CheckRuntimeTypeInfoStatic(resource) == true) ? reinterpret_cast<T*>(resource) : nullptr;
            }

            constexpr Resource *GetResourceDirect() const;

            constexpr bool IsLoadGuard()     const { return m_load_guard; }
            constexpr bool IsCompleteGuard() const { return m_complete_guard; }

            constexpr bool IsResourceInitialized() const;
            constexpr bool IsInLoad() const;
            constexpr bool IsLoaded() const;
            constexpr bool IsFailed() const;
    };
}
