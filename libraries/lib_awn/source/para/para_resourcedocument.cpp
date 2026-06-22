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

    Document *ResourceDocument::CreateDocument(mem::Heap *heap, const char *file_path, const char *class_name, void *file, size_t size, bool is_yaml) {

        /* Set current thread heap*/
        mem::ScopedCurrentThreadHeap l(heap);

        /* Create typed param */
        const u64 hash = vp::util::HashMurmur64(class_name);
        TypedParam *typed_param = para::CreateTypedParam(heap, hash);
        if (typed_param == nullptr) { return nullptr; }

        /* Create document with our typed param and file path */
        Document *document = new (heap, alignof(Document)) Document(file_path, typed_param);

        /* Initialize typed param with bgyml or yaml */
#ifdef VP_ENABLE_LIBYAML
        if (is_yaml) {
            m_yaml_doc = new (heap, alignof(vp::res::YamlDocument)) vp::res::YamlDocument();
            RESULT_ABORT_UNLESS(m_yaml_doc->ParseYaml(heap, reinterpret_cast<const char*>(file), size));
            vp::res::YamlIterator yaml_iter(m_yaml_doc);
            typed_param->InitializeYaml(heap, document, yaml_iter);
        } else {
#endif
            vp::res::ByamlIterator byaml_iter(file);            
            typed_param->InitializeBgyml(heap, document, byaml_iter);
#ifdef VP_ENABLE_LIBYAML
        }
#endif

        return document;
    }

    Result ResourceDocument::Compose(mem::Heap *heap, res::IComposeInfo *info, void *file, size_t file_size) {

        /* Create document */
        CreateInfo *create_info = vp::util::DynamicCast<CreateInfo*>(info);
        if (create_info == nullptr) { RESULT_RETURN_SUCCESS; }
        Document *document = CreateDocument(heap, create_info->m_file_name, create_info->m_class_name, file, file_size, create_info->m_is_yaml);
        VP_ASSERT(document != nullptr);

        /* Set references between document and resource */
        m_document = document;
        document->m_resource = this;

        RESULT_RETURN_SUCCESS;
    }

    void ResourceDocument::Finalize() {
        if (m_document != nullptr) {
            delete m_document;
            m_document = nullptr;
        }
#ifdef VP_ENABLE_LIBYAML
        if (m_yaml_doc != nullptr) {
            m_yaml_doc->Finalize();
            delete m_yaml_doc;
            m_yaml_doc = nullptr;
        }
#endif
    }

    Result ResourceDocument::PreFinalize() {
        m_document->UnresolveFreeHandle();
        RESULT_RETURN_SUCCESS;
    }
}
