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

    AWN_PARA_DECLARE_TYPED_PARAM_IMPL(TypedParam);

    Result Composite::FindComposite(FindCompositeContext *out_find_info, const char *composite_name) {
        const u8 cur_char = *composite_name;
        const bool is_this = cur_char == '\0';
        out_find_info->composite = (is_this) ? this : nullptr;
        RESULT_RETURN_IF(!is_this, ResultNoComposite);
        RESULT_RETURN_SUCCESS;
    }

    void TypedParam::SetParentHandle(TypedParam *parent) {
        
        /* Allocate parent handle in parent if necessary */
        impl::DocumentManager *manager = impl::DocumentManager::GetInstance();
        u16 last_handle;
        u16 new_handle = manager->AcquireTypedParamHandle(parent);
        if (!vp::util::InterlockedCompareExchange(std::addressof(last_handle), std::addressof(parent->m_handle), new_handle, impl::DocumentManager::HandleAllocator::cInvalidHandle)) { manager->ReleaseTypedParamHandle(new_handle); new_handle = last_handle; }

        /* Set parent handle */
        m_parent_handle     = new_handle;
        m_parent_free_count = manager->GetFreeCountByHandle(new_handle);

        return;
    }

    void TypedParam::FreeParent() {
        m_parent_document_binder.Release();

        const u16 handle = vp::util::InterlockedExchange(std::addressof(m_parent_handle), impl::DocumentManager::HandleAllocator::cInvalidHandle);
        if (handle == impl::DocumentManager::HandleAllocator::cInvalidHandle) { return; }

        impl::DocumentManager::GetInstance()->ReleaseTypedParamHandle(handle);

        return;
    }

    void TypedParam::ReleaseParent() {
        m_parent_document_binder.Release();
        m_parent_handle = 0;
    }
    
    void TypedParam::ReleaseHandle() {

        const u16 handle = vp::util::InterlockedExchange(std::addressof(m_handle), impl::DocumentManager::HandleAllocator::cInvalidHandle);
        if (handle == impl::DocumentManager::HandleAllocator::cInvalidHandle) { return; }

        impl::DocumentManager::GetInstance()->ReleaseTypedParamHandle(handle); 
    }

    bool TypedParam::LoadParent(Document *document, const char *parent_name) {

        /* Copy path to stack */
        res::MaxPathString parent_path(parent_name);

        /* Check for ? */
        const char *load_path  = parent_path.GetString();
        const char  first_char = *load_path;

        /* Remove ? */
        if (first_char == '?') {
            load_path = load_path + 1;
        }

        /* Resolve parent directory */
        if (first_char == '.') {
            FormatDirectoryUsingParentPath(std::addressof(parent_path), document->m_file_path.GetString(), parent_name);
            load_path = parent_path.GetString();
        }

        /* Try load document */
        m_parent_document_binder.LoadDocumentForResolve(load_path);

        /* Get document */
        Document *parent_doc = m_parent_document_binder.m_document;
        if (parent_doc == nullptr) { return false; }

        /* Set parent handle to parent document typed param */
        this->SetParentHandle(parent_doc->m_typed_param);

        return true;
    }

    bool TypedParam::VerifyParent() {

        /* Get parent */
        TypedParam *parent_param = this->GetParent();
        if (parent_param == nullptr) { return false; }

        /* Ensure we are a derivitive of the parent */
        return vp::util::CheckRuntimeTypeInfo(parent_param->GetRuntimeTypeInfo(), this->GetRuntimeTypeInfo());
    }

    void TypedParam::SetParentTo(TypedParam *typed_param) {
        typed_param->m_parent_name            = m_parent_name;
        typed_param->m_parent_document_binder = m_parent_document_binder;
        typed_param->m_parent_handle          = m_parent_handle;
        typed_param->m_parent_free_count      = m_parent_free_count;
    }

    Result TypedParam::ResolveComposite(FindCompositeContext *out_find_info, const char *field_name) {

        /* Find parent composite */
        TypedParam *parent = this->GetParent();
        while (parent != nullptr) {

            FindCompositeContext sub_context = {};
            const Result result = parent->FindComposite(std::addressof(sub_context), field_name);
            if (result == ResultFoundComposite) {
                *out_find_info = sub_context;
                RESULT_RETURN(result);
            }

            parent = parent->GetParent();
        }

        RESULT_RETURN_SUCCESS;
    }

    void TypedParam::ResolveParent(Document *document, TypedParam *parent_typed_param, const char *field_name) {

        /* Resolve parent composites on exit */
        const char *field_name_rec = field_name;
        ON_SCOPE_EXIT {

            /* Resolve parent composites */
            const auto resolve_composite_parent = [this, document, parent_typed_param, field_name_rec](CompositeLoopInfo *loop_info) {

                /* Nothing to do if default */
                if ((*loop_info->is_non_default & 1) == false) { RESULT_RETURN_SUCCESS; }

                /* Get next parent param */
                TypedParam *next_parent_param = (*parent_typed_param->m_parent_name == '\0') ? this : parent_typed_param;

                /* Get next field name */
                const char *next_field_name = field_name_rec;
                res::MaxPathString sub_field_name;
                if (*field_name_rec == '\0') {
                    sub_field_name.Format("%s.%s", field_name_rec, loop_info->name);
                    next_field_name = sub_field_name.GetString();
                }

                /* Resolve composite parent */
                loop_info->composite->ResolveParent(document, next_parent_param, next_field_name);

                RESULT_RETURN_SUCCESS;
            };
            auto resolve_composite_func = vp::util::MakeLargeLambdaFunction<Result(CompositeLoopInfo*)>(resolve_composite_parent);
            this->LoopComposites(std::addressof(resolve_composite_func));
        };

        /* Check if no parent name */
        if (*m_parent_name == '\0') {

            /* Release any parent */
            this->ReleaseParent();

            /* Nothing to resolve if no override parent */
            if (parent_typed_param == nullptr) { return; }

            /* Loop all parents */
            TypedParam *parent = parent_typed_param->GetParent();
            while (parent != nullptr) {

                /* Search for composite in provided parent  */
                FindCompositeContext find_result = {};
                const Result result = this->TypedParam::FindComposite(std::addressof(find_result), field_name_rec);

                /* Finish if found */
                if (RESULT_SUCCEEDED(result) && find_result.is_non_default != 0) {

                    /* Set parent handle to found parent composite */
                    if (find_result.parent != nullptr) {                                
                        this->SetParentHandle(find_result.parent);
                    }

                    /* Set parent document to parent typed param */
                    m_parent_document_binder = parent_typed_param->m_parent_document_binder;

                    break;
                }

                parent = this->GetParent();
            }

            return;
        }

        /* Get doc ref count */
        const u32 doc_ref_count = (document != nullptr) ? document->m_reference_count : 0;

        /* Check if any parent name */
        bool result = true;
        if (!vp::util::StringCompare(m_parent_name, "*")) {

            /* Release parent */
            this->ReleaseParent();

            /* Load sub document */
            result = this->LoadParent(document, m_parent_name);
            if (result == false) { this->ReleaseParent(); }
        }

        /* Ensure we are a derivitive of the parent type */
        VP_ASSERT(!result || this->VerifyParent());

        /* Ensure document reference is not cyclic */
        VP_ASSERT(document == nullptr || doc_ref_count == document->m_reference_count);

        /* Stub parent tp and rec fields */
        parent_typed_param = nullptr;
        field_name_rec = "";

        return;
    }
    
    void TypedParam::InitializeBgyml(mem::Heap *heap, Document *document, vp::res::ByamlIterator byaml_iterator) {

        /* Integrity checks */
        VP_ASSERT(vp::res::IsDictionaryType(static_cast<vp::res::ByamlDataType>(byaml_iterator.GetDataType())));

        /* Load properties */
        const auto prop_loop_lambda = [=](PropertyLoopInfo *prop_info) {

            /* Get iterator to data */
            vp::res::ByamlData prop_data;
            const bool has_data = byaml_iterator.TryGetByamlDataByKey(std::addressof(prop_data), prop_info->name);

            /* Set default if no data */
            if (!has_data) {
                *prop_info->is_non_default &= 0xfe;
                prop_info->property_accessor->SetValue(prop_info->default_value);
                RESULT_RETURN_SUCCESS;
            }

            /* Set from prop accessor */
            const bool is_set = prop_info->property_accessor->LoadByamlData(byaml_iterator, prop_data);
            VP_ASSERT(is_set);

            /* Set as non default */
            *prop_info->is_non_default |= 1;

            RESULT_RETURN_SUCCESS;
        };
        auto prop_loop_func = vp::util::MakeLambdaFunction<Result(PropertyLoopInfo*)>(prop_loop_lambda);
        this->LoopProperties(std::addressof(prop_loop_func));

        /* Load embeds */
        const auto embed_loop_lambda = [=](EmbeddedLoopInfo *embed_info) {

            /* Get iterator to data */
            vp::res::ByamlIterator embed_iter;
            const bool has_data = byaml_iterator.TryGetIteratorByKey(std::addressof(embed_iter), embed_info->name);

            /* Get class name */
            const char *type_name = embed_info->embedded_class_name;
            if (has_data) {
                embed_iter.TryGetStringByKey(std::addressof(type_name), "$type");
            }

            /* Recreate the embed with a derived type if the derived type changed */
            TypedParam *typed_param = *embed_info->embedded_typed_param;
            if (typed_param == nullptr || !vp::util::StringCompare(typed_param->GetClassName(), type_name)) {
                if (typed_param != nullptr) {
                    typed_param->Finalize();
                    delete typed_param;
                }
                typed_param = CreateTypedParam(heap, vp::util::HashMurmur64(type_name));
            }

            /* Replace with default if incompatible rtti */
            if (!vp::util::CheckRuntimeTypeInfo(embed_info->embedded_rtti, typed_param->GetRuntimeTypeInfo())) {
                typed_param->Finalize();
                delete typed_param;
                typed_param = CreateTypedParam(heap, vp::util::HashMurmur64(embed_info->embedded_class_name));
            }

            *embed_info->embedded_typed_param = typed_param;

            RESULT_RETURN_SUCCESS;
        };
        auto embed_loop_func = vp::util::MakeLambdaFunction<Result(EmbeddedLoopInfo*)>(embed_loop_lambda);
        this->LoopEmbeds(std::addressof(embed_loop_func));

        /* Load composites */
        const auto comp_loop_lambda = [&](CompositeLoopInfo *comp_info) {

            /* Get iterator to data */
            vp::res::ByamlIterator comp_iter;
            const bool has_data = byaml_iterator.TryGetIteratorByKey(std::addressof(comp_iter), comp_info->name);

            /* Initialize defaults if no data */
            if (!has_data) {

                comp_info->composite->InitializeDefaults(heap, true);

                /* Unset non-default set as initialized default */
                *comp_info->is_non_default = (*comp_info->is_non_default & 0xfe) | 2;
                RESULT_RETURN_SUCCESS;
            }

            /* Initialize with byaml iter */
            comp_info->composite->InitializeBgyml(heap, document, comp_iter);
            *comp_info->is_non_default = (*comp_info->is_non_default & 0xfd) | 1;

            RESULT_RETURN_SUCCESS;
        };
        auto comp_loop_func = vp::util::MakeLambdaFunction<Result(CompositeLoopInfo*)>(comp_loop_lambda);
        this->LoopComposites(std::addressof(comp_loop_func));

        /* Get parent path */
        byaml_iterator.TryGetStringByKey(std::addressof(m_parent_name), "$parent");

        return;
    }

    void TypedParam::InitializeDefaults(mem::Heap *heap, bool is_force_unlink_parents) {

        /* Set default properties */
        const auto prop_loop_lambda = [=](PropertyLoopInfo *prop_info) {
            *prop_info->is_non_default &= 0xfe;
            prop_info->property_accessor->SetValue(prop_info->default_value);
            RESULT_RETURN_SUCCESS;
        };
        auto prop_loop_func = vp::util::MakeLambdaFunction<Result(PropertyLoopInfo*)>(prop_loop_lambda);
        this->LoopProperties(std::addressof(prop_loop_func));

        /* Set default embeds */
        const auto embed_loop_lambda = [=](EmbeddedLoopInfo *embed_info) {

            /* Recreate embed if different class */
            TypedParam *typed_param = *embed_info->embedded_typed_param;
            if (typed_param == nullptr || ::strcmp(typed_param->GetClassName(), embed_info->embedded_class_name) != 0) {
                if (typed_param != nullptr) {
                    typed_param->Finalize();
                    delete typed_param;
                }
                typed_param = CreateTypedParam(heap, vp::util::HashMurmur64(embed_info->embedded_class_name));
                *embed_info->embedded_typed_param = typed_param;
            }

            RESULT_RETURN_SUCCESS;
        };
        auto embed_loop_func = vp::util::MakeLambdaFunction<Result(EmbeddedLoopInfo*)>(embed_loop_lambda);
        this->LoopEmbeds(std::addressof(embed_loop_func));

        /* Set default composites */
        const auto comp_loop_lambda = [=](CompositeLoopInfo *comp_info) {

            /* Initialize defaults */
            comp_info->composite->InitializeDefaults(heap, is_force_unlink_parents);

            /* Unset non-default set as initialized default */
            *comp_info->is_non_default = (*comp_info->is_non_default & 0xfe) | 2;

            RESULT_RETURN_SUCCESS;
        };
        auto comp_loop_func = vp::util::MakeLambdaFunction<Result(CompositeLoopInfo*)>(comp_loop_lambda);
        this->LoopComposites(std::addressof(comp_loop_func));

        /* Complete if not unlinking parents */
        if (!is_force_unlink_parents) { return; }

        /* Release parent */
        this->ReleaseParent();
        m_parent_name   = "";

        return;
    }

    void TypedParam::Finalize() {

        /* Reset composites */
        const auto comp_loop_lambda = [=](CompositeLoopInfo *comp_info) {

            /* Finalize composite */
            comp_info->composite->Finalize();

            RESULT_RETURN_SUCCESS;
        };
        auto comp_loop_func = vp::util::MakeLambdaFunction<Result(CompositeLoopInfo*)>(comp_loop_lambda);
        this->LoopComposites(std::addressof(comp_loop_func));

        /* Reset embeds */
        const auto embed_loop_lambda = [=](EmbeddedLoopInfo *embed_info) {

            /* Free embed */
            TypedParam *typed_param = *embed_info->embedded_typed_param;
            if (typed_param != nullptr) {
                delete typed_param;
                *embed_info->embedded_typed_param = nullptr;
            }

            RESULT_RETURN_SUCCESS;
        };
        auto embed_loop_func = vp::util::MakeLambdaFunction<Result(EmbeddedLoopInfo*)>(embed_loop_lambda);
        this->LoopEmbeds(std::addressof(embed_loop_func));

        /* Release parent */
        this->ReleaseParent();

        /* Release handle */
        this->ReleaseHandle();

        return;
    }

    Result TypedParam::FindComposite(FindCompositeContext *out_find_info, const char *search_string) {

        /* Set parent */
        out_find_info->parent = this;

        /* Set composite as this if out of search string */
        if (*search_string == '\0') {
            out_find_info->composite = this;
            RESULT_RETURN_SUCCESS;
        }

        /* Search composites */
        const auto comp_loop_lambda = [=](CompositeLoopInfo *comp_info) {

            /* Advance composite search string */
            const char *next_search_string = "";
            const bool is_next = AdvanceCompositeSearchString(std::addressof(next_search_string), search_string, comp_info->name);
            if (!is_next) { RESULT_RETURN_SUCCESS; }

            /* Search for composite */
            const Result result = comp_info->composite->FindComposite(out_find_info, next_search_string);

            if (result == vp::ResultSuccess && comp_info->composite == out_find_info->composite) {
                out_find_info->found_name     = comp_info->name;                
                out_find_info->is_non_default = ((*comp_info->is_non_default & 1) == 1);

                RESULT_RETURN(ResultFoundComposite);
            }

            RESULT_RETURN_SUCCESS;
        };
        auto comp_loop_func = vp::util::MakeLambdaFunction<Result(CompositeLoopInfo*)>(comp_loop_lambda);
        const Result result = this->LoopComposites(std::addressof(comp_loop_func));

        if (result == ResultFoundComposite) { RESULT_RETURN_SUCCESS; }

        RESULT_RETURN(ResultNoComposite);
    }
}
