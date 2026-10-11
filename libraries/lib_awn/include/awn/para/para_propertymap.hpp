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

    class PropertyMapBase : public Composite {
        public:
            VP_RTTI_DERIVED(PropertyMapBase);
        public:
            constexpr PropertyMapBase() {/*...*/}
            virtual constexpr ~PropertyMapBase() override {/*...*/}
    };

    template <typename T>
    class PropertyMap : public PropertyMapBase {
        public:
            struct MapNode {
                vp::util::IntrusiveRedBlackTreeNode<u64>  node;
                T                                         value;
                const char                               *key;
            };
        public:
            using TreeAllocator = vp::util::IntrusiveRedBlackTreeAllocator<vp::util::IntrusiveRedBlackTreeTraits<MapNode, &MapNode::node>>;
        private:
            TreeAllocator   m_map;
            u32             m_is_allow_parent;
            PropertyMap<T> *m_parent;
        public:
            VP_RTTI_DERIVED(PropertyMap);
        public:
            constexpr PropertyMap() : PropertyMapBase(), m_map(), m_is_allow_parent(), m_parent() {/*...*/}
            virtual constexpr ~PropertyMap() override {/*...*/}

            virtual void InitializeBgyml(mem::Heap *heap, [[maybe_unused]] Document *document, vp::res::ByamlIterator byaml_iterator) override {

                /* Ensure dictionary iterator */
                VP_ASSERT(vp::res::IsDictionaryType(static_cast<vp::res::ByamlDataType>(byaml_iterator.GetDataType())) == true);

                /* Ensure same size if fixed array type */
                const u32 count = byaml_iterator.GetDataCount();
                if (count < 1) { return; }

                /* Get real count */
                vp::res::ByamlIterator iter = {};
                const char *key  = "";
                u32 real_count = 0;
                for (u32 i = 0; i < count; ++i) {
                    real_count = real_count + byaml_iterator.TryGetIteratorAndKeyByIndex(std::addressof(iter), std::addressof(key), i);
                }

                /* Create array */
                RESULT_ABORT_UNLESS(m_map.Initialize(heap, real_count));

                /* Parse data */
                for (u32 i = 0; i < count; ++i) {

                    /* Get data and key */
                    const bool result = byaml_iterator.TryGetIteratorAndKeyByIndex(std::addressof(iter), std::addressof(key), i);
                    if (result == false) { continue; }

                    /* Hash key */
                    const u64 hash = vp::util::HashMurmur64(key);

                    /* Allocate map node */
                    MapNode *node = m_map.Allocate(hash);
                    node->key     = key;

                    /* Get value by data */
                    const bool value_result = iter.TryGetValueByIterator(std::addressof(node->value));
                    VP_ASSERT(value_result == true);
                }

                return;
            }
#ifdef VP_ENABLE_LIBYAML
            virtual void InitializeYaml(mem::Heap *heap, [[maybe_unused]] Document *document, vp::res::YamlIterator yaml_iterator) override {

                /* Ensure dictionary iterator */
                VP_ASSERT(vp::res::IsDictionaryType(static_cast<vp::res::ByamlDataType>(yaml_iterator.GetDataType())));

                /* Ensure same size if fixed array type */
                const u32 count = yaml_iterator.GetDataCount();
                if (count < 1) { return; }

                /* Get real count */
                vp::res::YamlIterator iter = {};
                const char *key  = "";
                u32 real_count = 0;
                for (u32 i = 0; i < count; ++i) {
                    real_count = real_count + yaml_iterator.TryGetIteratorAndKeyByIndex(std::addressof(iter), std::addressof(key), i);
                }

                /* Create array */
                RESULT_ABORT_UNLESS(m_map.Initialize(heap, real_count));

                /* Parse data */
                for (u32 i = 0; i < count; ++i) {

                    /* Get data and key */
                    const bool result = yaml_iterator.TryGetIteratorAndKeyByIndex(std::addressof(iter), std::addressof(key), i);
                    if (result == false) { continue; }

                    /* Hash key */
                    const u64 hash = vp::util::HashMurmur64(key);

                    /* Allocate map node */
                    MapNode *node = m_map.Allocate(hash);
                    node->key = key;

                    /* Get value by data */
                    const bool value_result = iter.TryGetValueByIterator(std::addressof(node->value));
                    VP_ASSERT(value_result == true);
                }

                return;
            }
#endif

            virtual void InitializeDefaults(mem::Heap *heap, bool is_force_unlink_parents) override { VP_UNUSED(heap, is_force_unlink_parents); m_map.Finalize(); }
            virtual void Finalize() override { m_map.Finalize(); }

            virtual void ResolveParent([[maybe_unused]] Document *document, TypedParam *parent_typed_param, const char *field_name) override {

                /* Nothing to do if parenting is disabled */
                if (m_is_allow_parent == 0) { return; }

                /* Unset parent and ensure parent typed param */
                m_parent = nullptr;
                if (parent_typed_param == nullptr) { return; }

                /* Find parent composite */
                FindCompositeContext context = {};
                const Result result = parent_typed_param->ResolveComposite(std::addressof(context), field_name);
                if (result != ResultFoundComposite) { return; }

                /* Set composite if compatible type */
                m_parent = (PropertyMap<T>::CheckRuntimeTypeInfoStatic(context.composite->GetRuntimeTypeInfo())) ? reinterpret_cast<PropertyMap<T>*>(context.composite) : nullptr;

                return;
            }
    };
}
