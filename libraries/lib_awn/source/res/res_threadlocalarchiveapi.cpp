#include <awn.hpp>

namespace awn::res {

	void SetDefaultArchive(ResourceBinder *archive_binder) {
        impl::AsyncResourceManager::GetInstance()->SetDefaultArchive(archive_binder);
    }

    void ReferenceThreadLocalArchive() {
        impl::AsyncResourceManager::GetInstance()->GetThreadLocalArchiveManager()->ReferenceThreadLocalArchive();
    }

    void ReleaseThreadLocalArchive() {
        impl::AsyncResourceManager::GetInstance()->GetThreadLocalArchiveManager()->ReleaseThreadLocalArchive();
    }

    ScopedThreadLocalArchive::ScopedThreadLocalArchive(ResourceBinder *archive_binder) : m_last_binder() {
        impl::ThreadLocalArchiveManager *manager = impl::AsyncResourceManager::GetInstance()->GetThreadLocalArchiveManager();
        impl::LocalArchiveBinder *local_archive_binder = manager->GetThreadLocalArchive();
        if (local_archive_binder != nullptr) {
            m_last_binder.ReferenceBinderSync(std::addressof(local_archive_binder->resource_binder));
            local_archive_binder->resource_binder.Release();
        }
        manager->SetThreadLocalArchive(archive_binder);
    }

    void ScopedThreadLocalArchive::SetArchive(ResourceBinder *archive_binder) {
        impl::ThreadLocalArchiveManager *manager = impl::AsyncResourceManager::GetInstance()->GetThreadLocalArchiveManager();
        impl::LocalArchiveBinder *local_archive_binder = manager->GetThreadLocalArchive();
        if (local_archive_binder != nullptr) {
            m_last_binder.ReferenceBinderSync(std::addressof(local_archive_binder->resource_binder));
            local_archive_binder->resource_binder.Release();
        }
        manager->SetThreadLocalArchive(archive_binder);
    }

    ScopedThreadLocalArchive::~ScopedThreadLocalArchive() {
        impl::ThreadLocalArchiveManager *manager = impl::AsyncResourceManager::GetInstance()->GetThreadLocalArchiveManager();
        impl::LocalArchiveBinder *local_archive_binder = manager->GetThreadLocalArchive();
        if (local_archive_binder != nullptr) {
            local_archive_binder->resource_binder.Release();
        }
        manager->SetThreadLocalArchive(std::addressof(m_last_binder));
    }

    ScopedThreadLocalArchiveReference::ScopedThreadLocalArchiveReference() {
        impl::ThreadLocalArchiveManager *manager = impl::AsyncResourceManager::GetInstance()->GetThreadLocalArchiveManager();
        manager->ReferenceThreadLocalArchive();
    }

    ScopedThreadLocalArchiveReference::~ScopedThreadLocalArchiveReference() {
        impl::ThreadLocalArchiveManager *manager = impl::AsyncResourceManager::GetInstance()->GetThreadLocalArchiveManager();
        manager->ReleaseThreadLocalArchive();
    }
}