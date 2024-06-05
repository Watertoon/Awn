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

namespace awn::para {
	
    class IDocumentManagerCallback {
        public:
            constexpr IDocumentManagerCallback() {/*...*/}
            virtual ~IDocumentManagerCallback() {/*...*/}
            
            virtual Document *TryLoadDocument(const char *file_name, const char *class_name, bool is_allow_dummy) { VP_UNUSED(file_name, class_name, is_allow_dummy); return nullptr; }
            virtual void DestructDocument(Document *document)  { VP_UNUSED(document); }
            virtual void OnBindDocument(Document *document)    { VP_UNUSED(document); }
            virtual bool OnLoadDocument(Document *document) { VP_UNUSED(document); return true; }
            virtual bool OnReleaseDocument(Document *document) { VP_UNUSED(document); return true; }
            virtual constexpr bool IsResolveLoad(Document *document, const char *path) { VP_UNUSED(document, path); return true; }
    };

    struct DocumentManagerInfo {
        u32                       typed_param_parent_count;
        u32                       remove_queue_count;
        IDocumentManagerCallback *document_manager_callback;
    };

    struct TypedParamParentHandle {
        TypedParam *typed_param;
        u32         free_count;
    };

    class DocumentManager {
        public:
            friend class Document;
            friend class DocumentBinder;
            friend class TypedParam;
        public:
            using DocumentTree    = vp::util::IntrusiveRedBlackTreeTraits<Document, &Document::m_tree_node>::Tree;
            using HandleAllocator = vp::util::AtomicIndexAllocator<u16>;
        private:
            IDocumentManagerCallback                    *m_callback;
            ResourceFactoryDocument                      m_resource_factory;
            DocumentTree                                 m_document_tree;
            vp::util::HeapArray<TypedParamParentHandle>  m_typed_param_parent_handle_array;
            HandleAllocator                              m_typed_param_parent_handle_index_allocator;
            vp::util::AtomicRingBuffer<u64>              m_remove_queue;
            sys::ServiceCriticalSection                  m_bind_cs;
            sys::ServiceCriticalSection                  m_load_cs;
        public:
            AWN_SINGLETON_TRAITS(DocumentManager);
        protected:
            void RemoveDocumentFromTree(Document *document);

            Document *TryLoadDocumentImpl(const char *file_name, const char *class_name, u64 hash, bool is_allow_dummy);
            void OnLoadDocumentImpl(Document *document);

            void ReleaseDocumentImpl(Document *document);

            u16 AcquireTypedParamHandle(TypedParam *typed_param);

            constexpr TypedParam *GetParentByHandle(const u16 handle, const u32 free_index) {
                return ((handle != HandleAllocator::cInvalidHandle) & (m_typed_param_parent_handle_array[handle].free_count == free_index)) ? m_typed_param_parent_handle_array[handle].typed_param : nullptr;
            }
            constexpr TypedParam *GetParentByHandle(const u16 handle, const u32 free_index) const {
                return ((handle != HandleAllocator::cInvalidHandle) & (m_typed_param_parent_handle_array[handle].free_count == free_index)) ? m_typed_param_parent_handle_array[handle].typed_param : nullptr;
            }

            constexpr u32 GetFreeCountByHandle(const u16 handle) { return m_typed_param_parent_handle_array[handle].free_count; }

            void ReleaseTypedParamHandle(u16 handle);
        public:
            constexpr ALWAYS_INLINE DocumentManager() : m_callback(), m_resource_factory(), m_document_tree(), m_typed_param_parent_handle_array(), m_typed_param_parent_handle_index_allocator(), m_remove_queue(), m_bind_cs(), m_load_cs() {/*...*/}
            constexpr ALWAYS_INLINE ~DocumentManager() {/*...*/}

            void Initialize(mem::Heap *heap, const DocumentManagerInfo *mgr_info);

            void Calculate();

            constexpr ResourceFactoryDocument *GetResourceFactory() { return std::addressof(m_resource_factory); }
    };

    constexpr TypedParam::TypedParam() : m_parent_name(""), m_parent_document_binder(), m_parent_free_count(0), m_parent_handle(DocumentManager::HandleAllocator::cInvalidHandle), m_handle(DocumentManager::HandleAllocator::cInvalidHandle) {/*...*/}

    constexpr TypedParam *TypedParam::GetParent() {
        if (m_parent_document_binder.IsValid() == false) { return nullptr; }
        DocumentManager *manager = DocumentManager::GetInstance();
        return manager->GetParentByHandle(m_parent_handle, m_parent_free_count);
    }
    constexpr TypedParam *TypedParam::GetParent() const {
        if (m_parent_document_binder.IsValid() == false) { return nullptr; }
        DocumentManager *manager = DocumentManager::GetInstance();
        return manager->GetParentByHandle(m_parent_handle, m_parent_free_count);
    }
}
