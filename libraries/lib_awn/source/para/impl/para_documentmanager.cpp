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

namespace awn::para::impl {

    AWN_SINGLETON_TRAITS_IMPL(DocumentManager);

    void DocumentManager::RemoveDocumentFromTree(Document *document) {
        m_document_tree.Remove(document);
    }

    Document *DocumentManager::TryLoadDocumentImpl(const char *file_name, const char *class_name, u64 hash, bool is_allow_dummy) {

        /* Lock tree for load */
        std::scoped_lock l(m_load_cs);

        /* Find document reference */
        Document *document = m_document_tree.Find(hash);

        /* Try create new document if necessary */
        if (document == nullptr) {
            document = m_app_impl->TryLoadDocument(file_name, class_name, is_allow_dummy);
            if (document == nullptr) { return nullptr; }
            document->m_tree_node.SetKey(hash);
            m_document_tree.Insert(document);
        }

        /* Resolve parent document */
        if (!document->m_is_parent_resolved) {
            document->m_typed_param->ResolveParent(document, nullptr, "");
            document->m_is_parent_resolved = true;
        }

        return document;
    }

    void DocumentManager::ReleaseDocumentImpl(Document *document) {

        /* Release callback */
        const bool is_release = m_app_impl->OnReleaseDocument(document);
        if (!is_release) { return; }

        /* Push remove request */
        const u64 hash = vp::util::DualHashMurmur64(document->m_file_path.GetString(), document->m_typed_param->GetClassName());
        m_remove_queue.PushBack(hash);

        return;
    }
    void DocumentManager::OnLoadDocumentImpl(Document *document) {
        m_app_impl->OnLoadDocument(document);
    }

    u16 DocumentManager::AcquireTypedParamHandle(TypedParam *typed_param) {

        const u32 new_handle = m_typed_param_parent_handle_index_allocator.Allocate();
        VP_ASSERT(new_handle != HandleAllocator::cInvalidHandle);
        
        m_typed_param_parent_handle_array[new_handle].typed_param  = typed_param;

        return new_handle;
    }

    void DocumentManager::ReleaseTypedParamHandle(u16 handle) {

        m_typed_param_parent_handle_array[handle].typed_param  = nullptr;
        m_typed_param_parent_handle_array[handle].free_count  += 1;

        m_typed_param_parent_handle_index_allocator.Free(handle);

        return;
    }

    void DocumentManager::Initialize(mem::Heap *heap, const DocumentManagerInfo *mgr_info) {
        m_app_impl = mgr_info->app_impl;
        m_typed_param_parent_handle_array.Initialize(heap, mgr_info->typed_param_parent_count);
        m_typed_param_parent_handle_index_allocator.Initialize(heap, mgr_info->typed_param_parent_count);
        m_remove_queue.Initialize(heap, mgr_info->remove_queue_count);
    }

    void DocumentManager::Finalize() {
        m_typed_param_parent_handle_array.Finalize();
        m_typed_param_parent_handle_index_allocator.Finalize();
        m_remove_queue.Finalize();
        m_app_impl = nullptr;
    }

    void DocumentManager::Calculate() {

        /* Attempt to lock manager */
        const bool result0 = m_bind_cs.TryEnter();
        if (!result0) { return; }
        ON_SCOPE_EXIT { m_bind_cs.Leave(); };
        const bool result1 = m_load_cs.TryEnter();
        if (!result1) { return; }
        ON_SCOPE_EXIT {m_load_cs.Leave(); };

        /* Process garbage */
        for (;;) {

            /* Pop ring buffer element */
            const u64 remove_key = m_remove_queue.RemoveFront();
            if (remove_key == 0) { break; }

            /* Get document and ensure it's out of reference */
            Document *document = m_document_tree.Find(remove_key);
            if (document == nullptr || 0 < document->m_reference_count) { continue; }

            /* Remove document from tree */
            this->RemoveDocumentFromTree(document);

            /* Release composites */
            document->UnresolveReleaseParent();

            /* Destruct callback */
            m_app_impl->DestructDocument(document);
        }

        return;
    }
}
