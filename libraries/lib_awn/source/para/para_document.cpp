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

    Document::~Document() {
        if (m_typed_param != nullptr) {
            m_typed_param->Finalize();
            delete m_typed_param;
            m_typed_param = nullptr;
        }
        impl::DocumentManager::GetInstance()->RemoveDocumentFromTree(this);
    }

    void Document::UnresolveFreeHandle() {

        /* Ensure typed param */
        if (m_typed_param == nullptr) { return; }

        /* Release handle callback */
        const auto destruct_release_handle_lambda = [=](TypedParam *typed_param) {
            typed_param->ReleaseHandle();
            RESULT_RETURN_SUCCESS;
        };
        auto destruct_release_handle_func = vp::util::MakeLambdaFunction<void(TypedParam*)>(destruct_release_handle_lambda);

        /* Loop all composites for unresolve parent */
        const auto release_lambda = [&](CompositeLoopInfo *loop_info) {
            return UnresolveParentRescursiveImpl(loop_info, std::addressof(destruct_release_handle_func));
        };
        auto release_func = vp::util::MakeLambdaFunction<Result(CompositeLoopInfo*)>(release_lambda);
        RESULT_DISCARD(m_typed_param->LoopComposites(std::addressof(release_func)));

        return;
    }

    void Document::ResolveParent() {

        /* Resolve guard */
        if (m_is_parent_resolved) { return; }

        /* Resolve parent */
        m_typed_param->ResolveParent(this, nullptr, "");

        /* Set resolve guard */
        m_is_parent_resolved = true;

        return;
    }

    void Document::UnresolveReleaseParent() {

        /* Resolve guard */
        if (!m_is_parent_resolved) { return; }

        /* Release parent callback */
        const auto destruct_release_parent_lambda = [=](TypedParam *typed_param) {
            typed_param->ReleaseParent();
        };
        auto destruct_release_parent_func = vp::util::MakeLambdaFunction<void(TypedParam*)>(destruct_release_parent_lambda);

        /* Loop all composites for unresolve parent */
        const auto destruct_lambda = [&](CompositeLoopInfo *loop_info) {
            return UnresolveParentRescursiveImpl(loop_info, std::addressof(destruct_release_parent_func));
        };
        auto destruct_func = vp::util::MakeLambdaFunction<Result(CompositeLoopInfo*)>(destruct_lambda);
        RESULT_DISCARD(m_typed_param->LoopComposites(std::addressof(destruct_func)));

        /* Set guard */
        m_is_parent_resolved = false;

        return;
    }
    
    Result Document::UnresolveParentRescursiveImpl(CompositeLoopInfo *loop_info, vp::util::IFunction<void(TypedParam*)> *unresolve_function) {
        
        /* Nothing to do if default */
        if (loop_info->is_non_default == 0) { RESULT_RETURN_SUCCESS; }

        /* Create recursive destruct function */
        const auto destruct_lambda = [&](CompositeLoopInfo *loop_info) {
            return UnresolveParentRescursiveImpl(loop_info, unresolve_function);
        };
        auto destruct_func = vp::util::MakeLambdaFunction<Result(CompositeLoopInfo*)>(destruct_lambda);

        /* Handle typed param */
        Composite *composite      = loop_info->composite;
        const auto composite_rtti = composite->GetRuntimeTypeInfo();
        if (TypedParam::CheckRuntimeTypeInfoStatic(composite_rtti)) {
            TypedParam *typed_param = reinterpret_cast<TypedParam*>(composite);
            unresolve_function->Invoke(typed_param);
            RESULT_DISCARD(typed_param->LoopComposites(std::addressof(destruct_func)));
            RESULT_RETURN_SUCCESS;
        }
        
        /* Handle typed param array */
        if (TypedParamArrayBase::CheckRuntimeTypeInfoStatic(composite_rtti)) {
            TypedParamArrayBase *array = reinterpret_cast<TypedParamArrayBase*>(composite);
            for (TypedParam *&typed_param : *array) {
                unresolve_function->Invoke(typed_param);
                RESULT_DISCARD(typed_param->LoopComposites(std::addressof(destruct_func)));
            }
        }

        /* Handle typed param map */
        if (TypedParamMapBase::CheckRuntimeTypeInfoStatic(composite_rtti)) {
            TypedParamMapBase *map = reinterpret_cast<TypedParamMapBase*>(composite);
            for (TypedParamMapNode &map_node : *map) {
                unresolve_function->Invoke(map_node.typed_param);
                RESULT_DISCARD(map_node.typed_param->LoopComposites(std::addressof(destruct_func)));
            }
        }
        
        /* Handle typed param enum map */
        if (TypedParamEnumMapBase::CheckRuntimeTypeInfoStatic(composite_rtti)) {
            TypedParamEnumMapBase *enum_map = reinterpret_cast<TypedParamEnumMapBase*>(composite);
            for (TypedParamEnumEntry &enum_entry : *enum_map) {
                unresolve_function->Invoke(enum_entry.typed_param);
                RESULT_DISCARD(enum_entry.typed_param->LoopComposites(std::addressof(destruct_func)));
            }
        }

        RESULT_RETURN_SUCCESS;
    }
}
