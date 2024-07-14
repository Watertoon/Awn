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

    class DocumentManager;
    class DocumentBinder;
    class ResourceDocument;

    class Document {
        public:
            friend class DocumentManager;
            friend class DocumentBinder;
            friend class ResourceDocument;
            friend class TypedParam;
        public:
            using DocumentTreeNode = vp::util::IntrusiveRedBlackTreeNode<u64>;
        private:
            DocumentTreeNode    m_tree_node;
            ResourceDocument   *m_resource;
            TypedParam         *m_typed_param;
            res::MaxPathString  m_file_path;
            const char         *m_file_name;
            u64                 m_hash;
            u32                 m_reference_count;
            u32                 m_is_parent_resolved;
        private:
            static Result UnresolveParentRescursiveImpl(CompositeLoopInfo *loop_info, vp::util::IFunction<void(TypedParam*)> *unresolve_function);
        public:
            constexpr Document() : m_tree_node(), m_resource(), m_typed_param(), m_file_path(), m_file_name(), m_hash(), m_reference_count(), m_is_parent_resolved() {/*...*/}
            constexpr Document(const char *file_path, TypedParam *typed_param) : m_tree_node(), m_resource(), m_typed_param(typed_param), m_file_path(file_path), m_file_name(vp::util::AdvancePathNoDirectory(file_path)), m_hash(vp::util::DualHashMurmur64(file_path, typed_param->GetClassName())), m_reference_count(), m_is_parent_resolved() {/*...*/}
            ~Document();

            void UnresolveFreeHandle();

            void ResolveParent();
            void UnresolveReleaseParent();

            constexpr bool IsDummy() const { return m_file_path == "Dummy"; }
    };

    template <typename T>
    ALWAYS_INLINE T *DocumentBinder::GetTypedParam() const {
        static_assert(TypedParam::CheckRuntimeTypeInfoStatic(T::GetRuntimeTypeInfoStatic()));
        if (m_document == nullptr) { return nullptr; }
        TypedParam *typed_param   = m_document->m_typed_param;
        const bool  is_rtti_valid = T::CheckRuntimeTypeInfoStatic(typed_param);
        T          *result        = (is_rtti_valid) ? reinterpret_cast<T*>(typed_param) : nullptr;
        return result;
    }
}
