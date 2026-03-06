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

    void TypedParamEnumMapBase::InitializeTypedParam(mem::Heap *heap, Document *document, u32 index, u64 hash) {

        /* Remove last typed param if exists and class changed */
        TypedParamEnumEntry *entry            = std::addressof(m_typed_param_array[index]);
        TypedParam          *last_typed_param = entry->typed_param;
        if (last_typed_param != nullptr) {
            if (last_typed_param->GetClassHash() == hash) { return; }
            last_typed_param->Finalize();
            delete last_typed_param;
        }

        /* Allocate new typed param */
        TypedParam *new_typed_param = this->CreateTypedParam(heap, document, hash);
        VP_ASSERT(new_typed_param != nullptr);
        entry->typed_param = new_typed_param;

        return;
    }

    void TypedParamEnumMapBase::InitializeBgyml(mem::Heap *heap, Document *document, vp::res::ByamlIterator byaml_iterator) {

        /* Ensure container iterator */
        VP_ASSERT(vp::res::IsDictionaryType(static_cast<vp::res::ByamlDataType>(byaml_iterator.GetDataType())));

        /* Initialize array if necessary */
        const u32 count = m_enum_count;
        if (!m_typed_param_array.IsValid()) {
            RESULT_ABORT_UNLESS(m_typed_param_array.Initialize(heap, count));
        }

        /* Initialize typed params */
        for (u32 i = 0; i < count; ++i) {
            vp::res::ByamlIterator sub_iter;
            const bool result0 = byaml_iterator.TryGetIteratorByKey(std::addressof(sub_iter), m_enum_array[i]);
            if (result0) {

                /* Get type name */
                const char *type_name  = "";
                const bool  result1    = sub_iter.TryGetStringByKey(std::addressof(type_name), "$type");
                const u64   class_hash = (result1) ? vp::util::HashMurmur64(type_name) : this->GetBaseClassHash();

                /* Load bgyml */
                this->InitializeTypedParam(heap, document, i, class_hash);
                TypedParamEnumEntry *entry = std::addressof(m_typed_param_array[i]);
                entry->typed_param->InitializeBgyml(heap, document, sub_iter);
                entry->is_non_default = true;
                
            } else {

                /* Initialize default */
                this->InitializeTypedParam(heap, document, i, this->GetBaseClassHash());
                TypedParamEnumEntry *entry = std::addressof(m_typed_param_array[i]);
                entry->typed_param->InitializeDefaults(heap, false);
                entry->is_non_default = false;
            }
        }

        return;
    }

    void TypedParamEnumMapBase::InitializeDefaults(mem::Heap *heap, bool is_unlink_parent) {

        /* Initialize array if necessary */
        const u32 count = m_enum_count;
        if (!m_typed_param_array.IsValid()) {
            RESULT_DISCARD(m_typed_param_array.Initialize(heap, count));
        }

        /* Initialize typed params */
        for (u32 i = 0; i < count; ++i) {

            /* Initialize default */
            this->InitializeTypedParam(heap, nullptr, i, this->GetBaseClassHash());
            TypedParamEnumEntry *entry = std::addressof(m_typed_param_array[i]);
            entry->typed_param->InitializeDefaults(heap, is_unlink_parent);
            entry->is_non_default = false;
        }
        
        return;
    }

    void TypedParamEnumMapBase::Finalize() { 
        for (TypedParamEnumEntry &entry : m_typed_param_array) {
            if (entry.typed_param == nullptr) { continue; }
            entry.typed_param->Finalize();
            delete entry.typed_param;
        }
        m_typed_param_array.Finalize();
    }

    void TypedParamEnumMapBase::ResolveParent(Document *document, TypedParam *parent_typed_param, const char *field_name) {

        /* Loop children for resolve parent */
        const u32 count = m_enum_count;
        if (m_resolve_type == static_cast<u32>(ResolveType::ParentAndChildParent)) {
            for (u32 i = 0; i < count; ++i) {
                vp::util::FixedString8<0x100> field_string;
                field_string.Format("%s{%s}", field_name, m_enum_array[i]);
                m_typed_param_array[i].typed_param->ResolveParent(document, parent_typed_param, field_string.GetString());
            }
        } else {
            for (u32 i = 0; i < count; ++i) {
                m_typed_param_array[i].typed_param->ResolveParent(document, nullptr, "");
            }
        }

        /* Exit on no resolve */
        if (m_resolve_type == 0) { return; }

        /* Unset parent and ensure parent typed param */
        m_parent = nullptr;
        if (parent_typed_param == nullptr) { return; }

        /* Find parent composite */
        FindCompositeContext context = {};
        const Result result = parent_typed_param->ResolveComposite(std::addressof(context), field_name);
        if (result != ResultFoundComposite) { return; }

        /* Set composite if compatible type */
        m_parent = (TypedParamEnumMapBase::CheckRuntimeTypeInfoStatic(context.composite->GetRuntimeTypeInfo()) == true) ? reinterpret_cast<TypedParamEnumMapBase*>(context.composite) : nullptr;

        return;
    }

    Result TypedParamEnumMapBase::FindComposite(FindCompositeContext *out_find_info, const char *search_string) {

        /* Return this if no string */
        if (search_string[0] == '\0') {
            out_find_info->composite = this;
            RESULT_RETURN(ResultFoundComposite);
        }

        /* Search array */
        const char *next_search = "";
        const u32 count = m_enum_count;
        for (u32 i = 0; i < count; ++i) {

            /* Try advance search string on element */
            const bool is_advance = AdvanceCompositeSearchString(std::addressof(next_search), search_string, m_enum_array[i]);
            if (is_advance == false) { continue; }

            const Result result = m_typed_param_array[i].typed_param->FindComposite(out_find_info, next_search);
            if (result == ResultFoundComposite) { RESULT_RETURN(ResultFoundComposite); }
        }

        RESULT_RETURN_SUCCESS;
    }
}
