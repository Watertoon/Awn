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

    bool TypedParamMapBase::VerifyMap(vp::res::ByamlIterator iter) {

        /* Check same count */
        const u32 data_count = iter.GetDataCount();
        const u32 tree_count = m_used_count;
        if (tree_count != data_count) { return false; }

        /* Check same */
        for (u32 i = 0; i < data_count; ++i) {

            /* Get key and iter */
            vp::res::ByamlIterator  sub_iter;
            const char             *key     = "";
            const bool              result0 = iter.TryGetIteratorAndKeyByIndex(std::addressof(sub_iter), std::addressof(key), i);
            if (result0 == false) { continue; }

            /* Find node by key */
            const u64          key_hash = vp::util::HashMurmur64(key);
            TypedParamMapNode *node     = m_tree.Find(key_hash);
            if (node == nullptr)              { return false; }
            if (node->typed_param == nullptr) { return false; }

            /* Ensure same type */
            const char *type_name      = "";
            const bool  result1        = iter.TryGetStringByKey(std::addressof(type_name), "$type");
            const u64   type_hash      = (result1 == true) ? vp::util::HashMurmur64(type_name) : this->GetBaseClassHash();
            const char *prev_type_name = node->typed_param->GetClassName();
            const u64   prev_type_hash = vp::util::HashMurmur64(prev_type_name);
            if (type_hash != prev_type_hash) { return false; }
        }

        return true;
    }

    void TypedParamMapBase::AllocateMap(mem::Heap *heap, vp::res::ByamlIterator iter) {
        
        /* Get real count */
        const u32 data_count = iter.GetDataCount();
        u32       real_count = 0;
        for (u32 i = 0; i < data_count; ++i) {
            vp::res::ByamlIterator  sub_iter;
            const char             *key     = "";
            const bool              result0 = iter.TryGetIteratorAndKeyByIndex(std::addressof(sub_iter), std::addressof(key), i);
            real_count += result0;
        }

        /* Delete tree if valid */
        if (m_tree.IsValid() == true) {
            for (TypedParamMapNode &map_node : m_tree) {
                if (map_node.typed_param == nullptr) { continue; }
                map_node.typed_param->Finalize();
                delete map_node.typed_param;
            }
            m_tree.Finalize();
            m_used_count = 0;
            m_max_count = 0;
        }

        /* Initialize tree */
        m_tree.Initialize(heap, real_count);
        m_max_count = real_count;

        return;
    }
    
    void TypedParamMapBase::InitializeBgyml(mem::Heap *heap, Document *document, vp::res::ByamlIterator byaml_iterator) {

        /* Ensure dictionary iterator */
        VP_ASSERT(vp::res::IsDictionaryType(static_cast<vp::res::ByamlDataType>(byaml_iterator.GetDataType())) == true);

        /* Check if map changed */
        const bool result0 = this->VerifyMap(byaml_iterator);

        /* Reintialize typed params if same */
        const u32 data_count = byaml_iterator.GetDataCount();
        if (result0 == true) {

            for (u32 i = 0; i < data_count; ++i) {

                /* Get sub iter */
                vp::res::ByamlIterator  sub_iter;
                const char             *key     = "";
                const bool              result1 = byaml_iterator.TryGetIteratorAndKeyByIndex(std::addressof(sub_iter), std::addressof(key), i);
                if (result1 == false) { continue; }

                /* Get node by key */
                const u64          key_hash = vp::util::HashMurmur64(key);
                TypedParamMapNode *node     = m_tree.Find(key_hash);
                node->typed_param->InitializeBgyml(heap, document, sub_iter);
            }

            return;
        }

        /* Initialize tree map */
        this->AllocateMap(heap, byaml_iterator);

        /* Read data */
        for (u32 i = 0; i < data_count; ++i) {

            /* Get sub iter */
            vp::res::ByamlIterator  sub_iter;
            const char             *key     = "";
            const bool              result0 = byaml_iterator.TryGetIteratorAndKeyByIndex(std::addressof(sub_iter), std::addressof(key), i);
            if (result0 == false) { continue; }

            /* Get key hash */
            const u64 key_hash = vp::util::HashMurmur64(key);

            /* Allocate map node */
            TypedParamMapNode *new_node = m_tree.Allocate(key_hash);
            ++m_used_count;

            /* Set original key */
            new_node->key = key;

            /* Get type hash */
            const char *type_name      = "";
            const bool  result2        = sub_iter.TryGetStringByKey(std::addressof(type_name), "$type");
            const u64   type_hash      = (result2 == true) ? vp::util::HashMurmur64(type_name) : this->GetBaseClassHash();

            /* Try allocate typed param */
            TypedParam *typed_param = this->CreateTypedParam(heap, document, type_hash);

            /* Fallback to default key */
            if (typed_param == nullptr) {
                typed_param = this->CreateTypedParam(heap, document, this->GetBaseClassHash());
            }

            /* Initialize typed param */
            new_node->typed_param = typed_param;
            typed_param->InitializeBgyml(heap, document, sub_iter);
        }

        return;
    }

    void TypedParamMapBase::InitializeDefaults([[maybe_unused]] mem::Heap *heap, bool is_force_unlink_parents) {

        /* Delete tree if valid */
        if (m_tree.IsValid() == true) {
            for (TypedParamMapNode &map_node : m_tree) {
                if (map_node.typed_param == nullptr) { continue; }
                map_node.typed_param->Finalize();
                delete map_node.typed_param;
            }
            m_tree.Finalize();
            m_used_count = 0;
            m_max_count = 0;
        }

        /* Unlink parent */
        if (is_force_unlink_parents == true) { m_parent = nullptr; }

        return;
    }

    void TypedParamMapBase::Finalize() {

        /* Delete tree if valid */
        if (m_tree.IsValid() == true) {
            for (TypedParamMapNode &map_node : m_tree) {
                if (map_node.typed_param == nullptr) { continue; }
                map_node.typed_param->Finalize();
                delete map_node.typed_param;
            }
            m_tree.Finalize();
            m_used_count = 0;
            m_max_count = 0;
        }

        return;
    }

    void TypedParamMapBase::ResolveParent(Document *document, TypedParam *parent_typed_param, const char *field_name) {

        /* Loop children for resolve parent */
        if (m_resolve_type == static_cast<u32>(ResolveType::ParentAndChildParent)) {
            for (TypedParamMapNode &map_node : m_tree) {
                vp::util::FixedString8<0x100> field_string;
                field_string.Format("%s{%s}", field_name, map_node.key);
                map_node.typed_param->ResolveParent(document, parent_typed_param, field_string.GetString());
            }
        } else {
            for (TypedParamMapNode &map_node : m_tree) {
                map_node.typed_param->ResolveParent(document, nullptr, "");
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
        m_parent = (TypedParamMapBase::CheckRuntimeTypeInfoStatic(context.composite->GetRuntimeTypeInfo()) == true) ? reinterpret_cast<TypedParamMapBase*>(context.composite) : nullptr;

        return;
    }

    Result TypedParamMapBase::FindComposite(FindCompositeContext *out_find_info, const char *search_string) {

        /* Return this if no string */
        if (search_string[0] == '\0') {
            out_find_info->composite = this;
            RESULT_RETURN(ResultFoundComposite);
        }

        /* Search array */
        const char *next_search = "";
        for (TypedParamMapNode &map_node : m_tree) {

            /* Try advance search string on element */
            const bool is_advance = AdvanceCompositeSearchString(std::addressof(next_search), search_string, map_node.key);
            if (is_advance == false) { continue; }

            const Result result = map_node.typed_param->FindComposite(out_find_info, next_search);
            if (result == ResultFoundComposite) { RESULT_RETURN(ResultFoundComposite); }
        }

        RESULT_RETURN_SUCCESS;
    }

    
}
