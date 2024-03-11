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

    void ThreadLocalArchiveManager::TlsDestructor(void *arg) {
        
        /* Cast to archive binder */
        LocalArchiveBinder *local_archive_binder = reinterpret_cast<LocalArchiveBinder*>(arg);

        /* Force tree clear if no archive */
        ThreadLocalArchiveManager *archive_mgr = AsyncResourceManager::GetInstance()->GetThreadLocalArchiveManager();
        if (arg == nullptr) {
            vp::util::InterlockedExchange(std::addressof(archive_mgr->m_is_trigger_clear_tree), 1u);
            return;
        }

        /* Lookup thread in tree by archive */
        auto tree = archive_mgr->m_archive_tree_allocator.GetTree();
        for (ArchiveTreeNode &node : *tree) {
            if (node.m_local_archive_binder != local_archive_binder) { continue; }

            if (local_archive_binder->reference_count != 0 || local_archive_binder->resource_binder.IsLoadGuard() == true) {
                local_archive_binder->resource_binder.Finalize();
                vp::util::InterlockedExchange(std::addressof(local_archive_binder->reference_count), 0u);
            }

            tree->Remove(std::addressof(node));

            sys::SetTlsData(archive_mgr->m_tls_slot, nullptr);

            archive_mgr->m_free_archive_ring_buffer.Insert(local_archive_binder);

            break;
        }

        return;
    }

    void ThreadLocalArchiveManager::Initialize(mem::Heap *heap, u32 max_thread_count) {

        /* Allocate tls slot */
        m_tls_slot = sys::AllocateTlsSlot(TlsDestructor);

        /* Initialize arrays */
        m_archive_tree_allocator.Initialize(heap, max_thread_count);
        m_free_archive_ring_buffer.Initialize(heap, max_thread_count);
        m_archive_binder_array.Initialize(heap, max_thread_count);
        m_registered_thread_ring.Initialize(heap, max_thread_count);

        /* Push archives to free ring */
        for (u32 i = 0; i < max_thread_count; ++i) {
            m_free_archive_ring_buffer.Insert(std::addressof(m_archive_binder_array[i]));
        }

        return;
    }
    void ThreadLocalArchiveManager::Finalize() {
        m_free_archive_ring_buffer.Finalize();
        m_archive_tree_allocator.Finalize();
        m_archive_binder_array.Finalize();
        m_registered_thread_ring.Finalize();
        if (m_tls_slot != sys::cInvalidTlsSlot) {                    
            sys::FreeTlsSlot(m_tls_slot);
            m_tls_slot = sys::cInvalidTlsSlot;
        }
    }

    void ThreadLocalArchiveManager::Calculate() {

        /* Check trigger */
        const u32 last = vp::util::InterlockedExchange(std::addressof(m_is_trigger_clear_tree), 0u);
        if (last == false) { return; }

        /* Build transient thread ring */
        m_registered_thread_ring.Clear();
        auto tree = m_archive_tree_allocator.GetTree();
        for (ArchiveTreeNode &node : *tree) {
            m_registered_thread_ring.Insert(node.m_tree_node.GetKey());
        }

        /* Force unregister invalid threads */
        for (sys::ThreadBase *&thread : m_registered_thread_ring) {
            if (sys::IsThreadValid(thread) == true) { continue; }

            ArchiveTreeNode    *node                 = m_archive_tree_allocator.Find(thread);
            LocalArchiveBinder *local_archive_binder = node->m_local_archive_binder;
            VP_ASSERT(local_archive_binder != nullptr);

            if (local_archive_binder->reference_count != 0 || local_archive_binder->resource_binder.IsLoadGuard() == true) {
                local_archive_binder->resource_binder.Finalize();
                vp::util::InterlockedExchange(std::addressof(local_archive_binder->reference_count), 0u);
            }

            tree->Remove(node);

            sys::SetTlsData(m_tls_slot, nullptr);

            m_free_archive_ring_buffer.Insert(local_archive_binder);
        }
        m_registered_thread_ring.Clear();

        return;
    }

    LocalArchiveBinder *ThreadLocalArchiveManager::GetThreadLocalArchive() {

        /* Lookup local archive by tls */
        sys::ThreadBase *thread = sys::GetCurrentThread();
        LocalArchiveBinder *local_archive = reinterpret_cast<LocalArchiveBinder*>(thread->GetTlsData(m_tls_slot));

        /* Register current thread if it's not registered */
        if (local_archive == nullptr) {
            local_archive = this->RegisterThread(thread);
            if (local_archive == nullptr) { return nullptr; }
        }

        return (local_archive->resource_binder.IsResourceInitialized() == true) ? local_archive: nullptr;
    }

    ResourceBinder *ThreadLocalArchiveManager::GetThreadLocalArchiveBinder() {

        /* Lookup local archive by tls */
        sys::ThreadBase *thread = sys::GetCurrentThread();
        LocalArchiveBinder *local_archive = reinterpret_cast<LocalArchiveBinder*>(thread->GetTlsData(m_tls_slot));

        /* Register current thread if it's not registered */
        if (local_archive == nullptr) {
            local_archive = this->RegisterThread(thread);
            if (local_archive == nullptr) { return nullptr; }
        }

        return (local_archive->resource_binder.IsResourceInitialized() == true) ? std::addressof(local_archive->resource_binder) : nullptr;
    }

    LocalArchiveBinder *ThreadLocalArchiveManager::RegisterThread(sys::ThreadBase *thread) {
        
        /* Lock tree */
        std::scoped_lock l(m_cs);
       
        /* Check if another thread registered this thread */
        ArchiveTreeNode    *archive_node  = m_archive_tree_allocator.Find(thread);  
        LocalArchiveBinder *local_archive = (archive_node != nullptr) ? archive_node->m_local_archive_binder : nullptr;   

        /* Update tls */
        if (local_archive != nullptr) { thread->SetTlsData(m_tls_slot, local_archive); return local_archive; }                     

        /* Allocate a new local archive */
        local_archive = m_free_archive_ring_buffer.RemoveFront();
        if (local_archive == nullptr) { return nullptr; }
        ArchiveTreeNode *tree_node = m_archive_tree_allocator.Allocate(thread, local_archive);
        if (tree_node == nullptr) { return nullptr; }

        return local_archive;
    }

    void ThreadLocalArchiveManager::UnregisterThread(sys::ThreadBase *thread) {

        /* Lock tree */
        std::scoped_lock l(m_cs);

        /* Check if another thread registered this thread */
        ArchiveTreeNode *tree_node = m_archive_tree_allocator.Find(thread);   
        if (tree_node == nullptr) { return; }

        /* Free slot, archive binder, tree node */
        thread->SetTlsData(m_tls_slot, nullptr);
        m_free_archive_ring_buffer.Insert(tree_node->m_local_archive_binder);
        m_archive_tree_allocator.Free(tree_node);

        return;
    }
    void ThreadLocalArchiveManager::UnregisterCurrentThread() {

        /* Lock tree */
        std::scoped_lock l(m_cs);

        /* Check if another thread registered this thread */
        sys::ThreadBase *thread    = sys::GetCurrentThread();
        ArchiveTreeNode *tree_node = m_archive_tree_allocator.Find(thread);   
        if (tree_node == nullptr) { return; }

        /* Free slot, archive binder, tree node */
        m_archive_tree_allocator.Free(tree_node);
        thread->SetTlsData(m_tls_slot, nullptr);
        m_free_archive_ring_buffer.Insert(tree_node->m_local_archive_binder);

        return;
    }

    bool ThreadLocalArchiveManager::SetThreadLocalArchive(ResourceBinder *binder_to_reference) {

        /* Check if archive is already loaded */
        LocalArchiveBinder *local_archive_binder = this->GetThreadLocalArchive();
        if (local_archive_binder == nullptr || local_archive_binder->resource_binder.IsResourceInitialized() == true) { return false; }

        /* Reference binder */
        return local_archive_binder->resource_binder.ReferenceBinderSync(binder_to_reference) == ResultSuccess;
    }

    bool ThreadLocalArchiveManager::IsThreadLocalArchiveInReference() {
        LocalArchiveBinder *local_archive_binder = this->GetThreadLocalArchive();
        return local_archive_binder != nullptr && 0 < local_archive_binder->reference_count;
    }
    
    void ThreadLocalArchiveManager::ReferenceThreadLocalArchive() {
        LocalArchiveBinder *local_archive_binder = this->GetThreadLocalArchive();
        if (local_archive_binder == nullptr) { return; }
        vp::util::InterlockedIncrement(std::addressof(local_archive_binder->reference_count));
        return;
    }
    void ThreadLocalArchiveManager::ReleaseThreadLocalArchive() {

        LocalArchiveBinder *local_archive_binder = reinterpret_cast<LocalArchiveBinder*>(sys::GetTlsData(m_tls_slot));
        if (local_archive_binder == nullptr) { return; }
        local_archive_binder->resource_binder.Finalize();

        return;
    }

    ScopedThreadLocalArchive::ScopedThreadLocalArchive(ResourceBinder *archive_binder) : m_last_binder() {
        ThreadLocalArchiveManager *manager       = AsyncResourceManager::GetInstance()->GetThreadLocalArchiveManager();
        LocalArchiveBinder *local_archive_binder = manager->GetThreadLocalArchive();
        if (local_archive_binder != nullptr) {
            m_last_binder.ReferenceBinderSync(std::addressof(local_archive_binder->resource_binder));
            local_archive_binder->resource_binder.Finalize();
        }
        manager->SetThreadLocalArchive(archive_binder);
    }
    ScopedThreadLocalArchive::~ScopedThreadLocalArchive() {
        ThreadLocalArchiveManager *manager = AsyncResourceManager::GetInstance()->GetThreadLocalArchiveManager();
        LocalArchiveBinder *local_archive_binder = manager->GetThreadLocalArchive();
        if (local_archive_binder != nullptr) {
            local_archive_binder->resource_binder.Finalize();
        }
        manager->SetThreadLocalArchive(std::addressof(m_last_binder));
    }
}
