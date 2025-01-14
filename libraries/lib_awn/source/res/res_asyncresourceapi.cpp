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

    Result LoadAsyncResourceSync(Resource **out_resource, const char *path, AsyncResourceLoadInfo *load_info) {
        VP_UNUSED(out_resource, path, load_info); 
        VP_NOT_IMPLEMENTED();
    }

    void SetControlThreadPriority(s32 priority) {
        impl::AsyncResourceManager::GetInstance()->SetControlThreadPriority(priority);
    }

	void SetMemoryThreadPriority(s32 priority) {
        impl::AsyncResourceManager::GetInstance()->SetMemoryThreadPriority(priority);
    }

	void SetLoadThreadsPriority(s32 priority) {
        impl::AsyncResourceManager::GetInstance()->SetLoadThreadsPriority(priority);
    }

	void SetComposeThreadsPriority(s32 priority) {
        impl::AsyncResourceManager::GetInstance()->SetComposeThreadsPriority(priority);
    }

    DecompressorHandle AllocateDecompressorHandle() {
        return impl::AsyncResourceManager::GetInstance()->AllocateDecompressor();
    }

    void FreeDecompressorHandle(DecompressorHandle handle) {
        impl::AsyncResourceManager::GetInstance()->FreeDecompressor(handle);
    }

    IParallelDecompressor *GetParallelDecompressor(DecompressorHandle handle, CompressionType compression_type, s32 priority, sys::CoreMask core_mask) {
        return impl::AsyncResourceManager::GetInstance()->GetParallelDecompressor(handle, compression_type, priority, core_mask);
    }

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
