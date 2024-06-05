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

    struct TypedParamMapNode {
        vp::util::IntrusiveRedBlackTreeNode<u64>  node;
        const char                               *key;
        TypedParam                               *typed_param;
    };

    class TypedParamMapBase : public Composite {
        public:
            enum class ResolveType : u32 {
                None                 = 0,
                Parent               = 1,
                ParentAndChildParent = 2,
            };
        public:
            using TypedParamTree = vp::util::IntrusiveRedBlackTreeAllocator<vp::util::IntrusiveRedBlackTreeTraits<TypedParamMapNode, &TypedParamMapNode::node>>;
        private:
            TypedParamTree     m_tree;
            u32                m_used_count;
            u32                m_max_count;
            u32                m_resolve_type;
            TypedParamMapBase *m_parent;
        public:
            VP_RTTI_DERIVED(TypedParamMapBase, Composite);
        private:
            bool VerifyMap(vp::res::ByamlIterator iter);
            void AllocateMap(mem::Heap *heap, vp::res::ByamlIterator iter);
        public:
            constexpr TypedParamMapBase() : m_tree(), m_resolve_type(2), m_parent() {/*...*/}
            virtual constexpr ~TypedParamMapBase() override {/*...*/}

            ALWAYS_INLINE auto begin() {
                return m_tree.begin();
            }
            ALWAYS_INLINE auto end() {
                return m_tree.end();
            }

            virtual void InitializeBgyml(mem::Heap *heap, Document *document, vp::res::ByamlIterator byaml_iterator) override;
            virtual void InitializeDefaults(mem::Heap *heap, bool is_force_unlink_parents) override;

            virtual void Finalize() override;

            virtual void ResolveParent(Document *document, TypedParam *parent_typed_param, const char *field_name) override;

            virtual Result FindComposite(FindCompositeContext *out_find_info, const char *search_string) override;

            virtual u64 GetBaseClassHash() const = 0;
            virtual TypedParam *CreateTypedParam(mem::Heap *heap, Document *document, u64 class_hash) const = 0;
    };

    template <typename T>
		requires (TypedParam::CheckRuntimeTypeInfoStatic(T::GetRuntimeTypeInfoStatic()) == true)
    class TypedParamMap : public TypedParamMapBase {
        public:
            VP_RTTI_DERIVED(TypedParamMap, Composite);
        public:
            constexpr TypedParamMap() : TypedParamMapBase() {/*...*/}
            virtual constexpr ~TypedParamMap() override {/*...*/}

            virtual u64 GetBaseClassHash() const override { return T::GetClassHash(); }
            virtual TypedParam *CreateTypedParam(mem::Heap *heap, [[maybe_unused]] Document *document, u64 class_hash) const override {
                TypedParam *typed_param = para::CreateTypedParam(heap, class_hash);
                VP_ASSERT(T::CheckRuntimeTypeInfoStatic(typed_param->GetRuntimeTypeInfo()) == true);
                return typed_param;
            }
    };
}