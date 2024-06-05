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

namespace awn::para {

    Document *ResourceDocument::CreateDocument(mem::Heap *heap, const char *file_path, const char *class_name, void *byaml) {

        /* Set current thread heap*/
        mem::ScopedCurrentThreadHeap l(heap);

        /* Initialize byaml iter */
        vp::res::ByamlIterator byaml_iter(byaml);

        /* Create typed param */
        const u64 hash = vp::util::HashMurmur64(class_name);
        TypedParam *typed_param = para::CreateTypedParam(heap, hash);
        if (typed_param == nullptr) { return nullptr; }

        /* Create document with our typed param and file path */
        Document *document = new (heap, alignof(Document)) Document(file_path, typed_param);

        /* Initialize typed param with bgyml */
        typed_param->InitializeBgyml(heap, document, byaml_iter);

        return document;
    }

    Result ResourceDocument::Initialize(mem::Heap *heap, mem::Heap *gpu_heap, res::ResourceUserContext *user_context, void *file, size_t file_size) {

        /* Unused */
        VP_UNUSED(gpu_heap, file_size);

        /* Get user context */
        if (user_context == nullptr || ResourceDocument::CreateInfo::CheckRuntimeTypeInfoStatic(user_context) == false) { RESULT_RETURN_SUCCESS; }

        /* Create document */
        CreateInfo *create_info = reinterpret_cast<CreateInfo*>(user_context);
        Document *document = CreateDocument(heap, create_info->m_file_name, create_info->m_class_name, file);
        VP_ASSERT(document != nullptr);

        /* Set references between document and resource */
        m_document = document;
        document->m_resource = this;

        RESULT_RETURN_SUCCESS;
    }

    void ResourceDocument::Finalize() {
        if (m_document == nullptr) { return; }
        delete m_document;
        m_document = nullptr;
    }

    Result ResourceDocument::PreFinalize() {
        m_document->UnresolveFreeHandle();
        RESULT_RETURN_SUCCESS;
    }
}
