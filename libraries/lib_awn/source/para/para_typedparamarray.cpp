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

    void TypedParamArrayBase::FinalizeTypedParamImpl(u32 base, u32 count) {

        /* Delete typed param array */
        for (u32 i = base; i < count; ++i) {
            TypedParam *typed_param = m_array[i];
            if (typed_param == nullptr) { continue; }
            typed_param->Finalize();
            delete typed_param;
        }

        return;
    }

    bool TypedParamArrayBase::InitializeTypedParam(mem::Heap *heap, Document *document, u32 index, u64 class_hash, bool is_no_inherit_previous_parent) {

        /* Create Typed Param Impl */
        const u32   ptr_count = m_array.GetUsedCount();
        TypedParam *typed_param = nullptr;
        if (index < ptr_count) {

            /* Recreate typed param if it exists */
            TypedParam *last_typed_param = m_array[index];
            if (last_typed_param != nullptr) {

                /* Only recreate different class */
                if (class_hash == last_typed_param->GetClassHash()) { return false; }

                /* Create new class */
                typed_param = this->CreateTypedParam(heap, document, class_hash);

                /* Inherit parent if necessary */
                if (is_no_inherit_previous_parent == false) { last_typed_param->SetParentTo(typed_param); }

                /* Delete last typed param */
                last_typed_param->Finalize();
                delete last_typed_param;
            } else {
                /* Make new typed param */
                typed_param = this->CreateTypedParam(heap, document, class_hash);
            }
        } else {
            typed_param = this->CreateTypedParam(heap, document, class_hash);
            if (m_array.GetUsedCount() < index) { m_array.SetUsedCount(index + 1); }
        }

        /* Set typed param */
        m_array.SetPointer(index, typed_param);

        return true;
    }

    void TypedParamArrayBase::InitializeBgyml(mem::Heap *heap, Document *document, vp::res::ByamlIterator byaml_iterator) {

        /* Ensure container iterator */
        VP_ASSERT(vp::res::IsContainerType(static_cast<vp::res::ByamlDataType>(byaml_iterator.GetDataType())) == true);

        /* Get array count */
        const u32 real_count = (0 < m_typed_param_max) ? m_typed_param_max : byaml_iterator.GetDataCount();

        /* Finalize array if necessary */
        if (m_array.IsValid()) {

            /* Finalize typed params over max */
            const u32 array_count = m_array.GetUsedCount();
            if (real_count < array_count) {
                this->FinalizeTypedParamImpl(real_count, array_count);
            }

            /* Adjust count */
            m_array.SetUsedCount(real_count);

        } else {

            /* Initialize array */
            RESULT_DISCARD(m_array.Initialize(heap, real_count));
        }

        /* Create typed params */
        const u64 base_class_hash = this->GetBaseClassHash();
        for (u32 i = 0; i < real_count; ++i) {
            vp::res::ByamlIterator sub_iterator;
            const bool result = byaml_iterator.TryGetIteratorByIndex(std::addressof(sub_iterator), i);
            const char *real_class_name = "";
            const u64   real_class_hash = (result == true && sub_iterator.TryGetStringByKey(std::addressof(real_class_name), "$type") == true) ? vp::util::HashMurmur64(real_class_name) : base_class_hash;
            this->InitializeTypedParam(heap, document, i, real_class_hash, true);
        }

        /* Initialize typed params */
        for (u32 i = 0; i < real_count; ++i) {
            vp::res::ByamlIterator sub_iterator;
            const bool result = byaml_iterator.TryGetIteratorByIndex(std::addressof(sub_iterator), i);
            if (result == true) {
                m_array[i]->InitializeBgyml(heap, document, sub_iterator);
            } else {
                m_array[i]->InitializeDefaults(heap, true);
            }
        }

        return;
    }

    void TypedParamArrayBase::InitializeDefaults(mem::Heap *heap, bool is_unlink_parent) {

        /* Handle finalization, initialization, and reinitialization */
        const u32 default_count = m_typed_param_max;
        if (default_count < 1) {

            /* Finalize all typed params */
            this->FinalizeTypedParamImpl(0, m_array.GetUsedCount());

            /* Unlink parent if necessary*/
            if (is_unlink_parent) { m_parent = nullptr; }
        } else if (m_array.IsValid() == false) {

            /* Initialize default array */
            RESULT_DISCARD(m_array.Initialize(heap, default_count));
            
            /* Create typed params */
            const u64 class_hash = this->GetBaseClassHash();
            for (u32 i = 0; i < default_count; ++i) {
                TypedParam *typed_param = this->CreateTypedParam(heap, nullptr, class_hash);
                m_array.PushPointer(typed_param);
                typed_param->InitializeDefaults(heap, is_unlink_parent);
            }
        } else {

            /* Reinitialize TypedParams to default */
            const u64 class_hash = this->GetBaseClassHash();
            for (u32 i = 0; i < default_count; ++i) {
                this->InitializeTypedParam(heap, nullptr, i, class_hash, is_unlink_parent);
                m_array[i]->InitializeDefaults(heap, is_unlink_parent);
            }
        }
        
        return;
    }

    void TypedParamArrayBase::Finalize() { 
        const u32 count = m_array.GetUsedCount();
        this->FinalizeTypedParamImpl(0, count);
        m_array.Finalize();
    }

    void TypedParamArrayBase::ResolveParent(Document *document, TypedParam *parent_typed_param, const char *field_name) {

        /* Resolve parent */
        if (m_is_allow_parent == true) {

            /* Unset parent and ensure parent typed param */
            m_parent = nullptr;
            if (parent_typed_param != nullptr) {                        

                /* Find parent composite */
                FindCompositeContext context = {};
                const Result result = parent_typed_param->ResolveComposite(std::addressof(context), field_name);

                /* Set composite if compatible type */
                if (result == ResultFoundComposite && TypedParamArrayBase::CheckRuntimeTypeInfoStatic(context.composite->GetRuntimeTypeInfo()) == true) {
                    m_parent = reinterpret_cast<TypedParamArrayBase*>(context.composite);
                }
            }
        }

        /* Resolve array parents */
        for (TypedParam *&typed_param : m_array) {
            typed_param->ResolveParent(document, nullptr, "");
        }

        return;
    }

    Result TypedParamArrayBase::FindComposite(FindCompositeContext *out_find_info, const char *search_string) {

        /* Return this if no string */
        if (search_string[0] == '\0') {
            out_find_info->composite = this;
            RESULT_RETURN(ResultFoundComposite);
        }

        /* Search array */
        const char *next_search = "";
        const u32   count       = m_array.GetUsedCount();
        for (u32 i = 0; i < count; ++i) {

            /* Try advance search string on element */
            vp::util::FixedString8<0x20> element_string;
            element_string.Format("%d", i);
            const bool is_advance = AdvanceCompositeSearchString(std::addressof(next_search), search_string, element_string.GetString());
            if (is_advance == false) { continue; }

            const Result result = m_array[i]->FindComposite(out_find_info, next_search);
            if (result == ResultFoundComposite) { RESULT_RETURN(ResultFoundComposite); }
        }

        RESULT_RETURN_SUCCESS;
    }
}
