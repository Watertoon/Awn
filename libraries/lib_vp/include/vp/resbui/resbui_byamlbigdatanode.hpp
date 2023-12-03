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

namespace vp::resbui {

    class ByamlNodeBase;

	class ByamlBigDataNodeBase {
        public:
			static constexpr u32 cInvalidOffset = 0xffff'ffff;
		protected:
			u32 m_offset;
		public:
            constexpr  ByamlBigDataNodeBase() : m_offset(cInvalidOffset) {/*...*/}
            virtual ~ByamlBigDataNodeBase() {/*...*/}

            virtual void PushKeys([[maybe_unused]] ByamlStringPoolBuilder *pool_builder) {/*...*/}
            virtual void PushStrings([[maybe_unused]] ByamlStringPoolBuilder *pool_builder) {/*...*/}

            virtual constexpr res::ByamlDataType GetByamlDataType() const { return res::ByamlDataType::Null; }
            virtual void Serialize([[maybe_unused]] uintptr_t *big_data_offset, [[maybe_unused]] uintptr_t *container_offset, [[maybe_unused]] vp::res::ResByaml *head) {/*...*/}

            virtual void CalculateEndOffset([[maybe_unused]] uintptr_t *big_data_offset_iter, [[maybe_unused]] uintptr_t *container_offset_iter, [[maybe_unused]] uintptr_t head) { return; }

            constexpr u32 GetOffset() const { return m_offset; }
	};

    /* ByamlNodeBigData definitions */
    constexpr res::ByamlDataType ByamlNodeBigData::GetByamlDataType() const { return m_big_data_node->GetByamlDataType(); }

    class ByamlBigDataNodeContainerBase : public ByamlBigDataNodeBase {
        public:
            using NodeList = vp::util::IntrusiveListTraits<ByamlNodeBase, &ByamlNodeBase::m_container_node>::List;
        protected:
            u32      m_node_count;
            NodeList m_node_list;
        public:
            constexpr  ByamlBigDataNodeContainerBase() : ByamlBigDataNodeBase(), m_node_count(), m_node_list() {/*...*/}
            virtual ~ByamlBigDataNodeContainerBase() override { this->Finalize(); }
    
            void Finalize() {
                m_node_list.Clear();
            }

            virtual void PushKeys(ByamlStringPoolBuilder *pool_builder) override {
                for (ByamlNodeBase &node : m_node_list) {
                    node.PushKeys(pool_builder);
                }
            }
            virtual void PushStrings([[maybe_unused]] ByamlStringPoolBuilder *pool_builder) override {
                for (ByamlNodeBase &node : m_node_list) {
                    node.PushStrings(pool_builder);
                }
            }
    };

    enum class ByamlArrayDataTypeMode : u8 {
        Auto       = 0,
        MonoTyped  = 1,
        MultiTyped = 2,
    };
    class ByamlBigDataNodeArray : public ByamlBigDataNodeContainerBase {
        protected:
            ByamlArrayDataTypeMode m_data_type_mode;
        private:
            constexpr bool CheckMonoTyped() const {
                if (m_data_type_mode != ByamlArrayDataTypeMode::Auto) { return (m_data_type_mode == ByamlArrayDataTypeMode::MonoTyped); }
                const res::ByamlDataType ref_data_type = (m_node_list.IsEmpty() == false) ? m_node_list.Front().GetByamlDataType() : res::ByamlDataType::Null;
                for (ByamlNodeBase &node : m_node_list) {
                    if (node.GetByamlDataType() != ref_data_type) { return false; }
                }
                return true;
            }
        public:
            constexpr ByamlBigDataNodeArray() : ByamlBigDataNodeContainerBase(), m_data_type_mode(ByamlArrayDataTypeMode::Auto) {/*...*/}
            virtual ~ByamlBigDataNodeArray() override {/*...*/}
    
            void AddNode(ByamlNodeBase *node) {
                m_node_list.PushBack(*node);
                ++m_node_count;
            }
            
            void RemoveNode(ByamlNodeBase *node) {
                node->m_container_node.Unlink();
                --m_node_count;
            }

            virtual constexpr res::ByamlDataType GetByamlDataType() const override { return (this->CheckMonoTyped() == true) ? res::ByamlDataType::MonoTypedArray : res::ByamlDataType::Array; }
            virtual void Serialize(uintptr_t *big_data_offset, uintptr_t *container_offset, vp::res::ResByaml *head) override  {

                /* Set offset */
                m_offset = static_cast<u32>(*container_offset - reinterpret_cast<uintptr_t>(head));

                /* Calculate sizes */
                const bool is_mono_typed        = this->CheckMonoTyped();
                const u32 data_type_size        = (is_mono_typed == false) ? vp::util::AlignUp(sizeof(vp::res::ByamlDataType) * m_node_count, alignof(u32))  : vp::util::AlignUp(sizeof(vp::res::ByamlDataType), alignof(u32));
                const u32 next_container_offset = sizeof(vp::res::ResByamlContainer) + data_type_size + sizeof(u32) * m_node_count;

                /* Get container regions */
                vp::res::ResByamlContainer *container             = reinterpret_cast<vp::res::ResByamlContainer*>(*container_offset);
                u8                         *data_type_array       = reinterpret_cast<u8*>(*container_offset + sizeof(vp::res::ResByamlContainer));
                void                       *value_array           = reinterpret_cast<void*>(*container_offset + sizeof(vp::res::ResByamlContainer) + data_type_size);

                /* Advance container offset*/
                *container_offset = *container_offset + next_container_offset;

                /* Write container header */
                container->data_type = static_cast<u8>(this->GetByamlDataType());
                container->count     = m_node_count;
    
                if (is_mono_typed == true) {

                    /* Write mono type */
                    if (m_node_list.IsEmpty() == false) {
                        *data_type_array = static_cast<u8>(m_node_list.Front().GetByamlDataType());
                    }

                    /* Stream out mono-typed container */
                    u32 i = 0;
                    for (ByamlNodeBase &node : m_node_list) {

                        /* Serialize big data */
                        if (ByamlNodeBigData::CheckRuntimeTypeInfo(std::addressof(node)) == true) {
                            reinterpret_cast<ByamlNodeBigData*>(std::addressof(node))->SerializeForBigData(big_data_offset, container_offset, head);
                        }

                        /* Serialize */
                        node.Serialize(reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(value_array) + i), head);
                        ++i;
                    }

                } else {

                    /* Stream out multi-typed container */
                    u32 i = 0;
                    for (ByamlNodeBase &node : m_node_list) {

                        /* Serialize data type */
                        data_type_array[i] = static_cast<u8>(node.GetByamlDataType());

                        /* Serialize big data */
                        if (ByamlNodeBigData::CheckRuntimeTypeInfo(std::addressof(node)) == true) {
                            reinterpret_cast<ByamlNodeBigData*>(std::addressof(node))->SerializeForBigData(big_data_offset, container_offset, head);
                        }

                        /* Serialize */
                        node.Serialize(reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(value_array) + i), head);
                        ++i;
                    }

                }

                return;
            }

            virtual void CalculateEndOffset(uintptr_t *big_data_offset_iter, uintptr_t *container_offset_iter, uintptr_t head) override {

                /* Update container size */
                const u32 data_type_size = (this->CheckMonoTyped() == false) ? vp::util::AlignUp(sizeof(vp::res::ByamlDataType) * m_node_count, alignof(u32))  : vp::util::AlignUp(sizeof(vp::res::ByamlDataType), alignof(u32));
                *container_offset_iter = vp::util::AlignUp(*container_offset_iter + sizeof(vp::res::ResByamlContainer) + data_type_size + sizeof(u32) * m_node_count, alignof(u32));

                /* Calculate sub nodes */
                for (ByamlNodeBase &node : m_node_list) {
                    if (ByamlNodeBigData::CheckRuntimeTypeInfo(std::addressof(node)) == false) { continue; }
                    reinterpret_cast<ByamlNodeBigData*>(std::addressof(node))->CalculateBigDataEndOffsets(big_data_offset_iter, container_offset_iter, head);
                }

                return;
            }
    };

    class ByamlBigDataNodeDictionary : public ByamlBigDataNodeContainerBase {
        private:
            bool m_is_index_remap;
        public:
            constexpr ByamlBigDataNodeDictionary() : ByamlBigDataNodeContainerBase(), m_is_index_remap(false) {/*...*/}
            virtual ~ByamlBigDataNodeDictionary() override {/*...*/}

            bool AddNode(ByamlNodeBase *node, const char *key, u32 index = ByamlNodeBase::cInvalidIndex) {

                /* Key check */
                if (key == nullptr || *key == '\0') { return false; }

                /* Set key and index */
                node->m_key.SetString(key);
                node->m_index = index;

                /* Sorted insertion into list */
                ByamlNodeBase *link_node = nullptr;
                for (ByamlNodeBase &node_iter : m_node_list) {
                    const s32 result = ::strcmp(node_iter.m_key.GetString(), key);
                    if (result == 0)  { return false; }
                    if (result == -1) { continue; }
                    link_node = std::addressof(node_iter);
                    break;
                }

                /* Link node */
                if (link_node == nullptr) {
                    m_node_list.PushBack(*node);
                } else {
                    link_node->m_container_node.LinkPrev(std::addressof(node->m_container_node));
                }

                ++m_node_count;

                return true;
            }

            void RemoveNode(ByamlNodeBase *node) {
                node->m_container_node.Unlink();
                --m_node_count;
            }

            constexpr void SetIsIndexRemap(bool is_index_remap) { m_is_index_remap = is_index_remap; }

            virtual constexpr res::ByamlDataType GetByamlDataType() const override { return (m_is_index_remap == true) ? res::ByamlDataType::DictionaryWithRemap : res::ByamlDataType::Dictionary; }
            virtual void Serialize(uintptr_t *big_data_offset, uintptr_t *container_offset, vp::res::ResByaml *head) override  {

                /* Set offset */
                m_offset = static_cast<u32>(*container_offset - reinterpret_cast<uintptr_t>(head));

                /* Calculate sizes */
                const u32 remap_size            = (m_is_index_remap == false) ? 0 : sizeof(u32) * m_node_count;
                const u32 next_container_offset = sizeof(vp::res::ResByamlContainer) + remap_size + sizeof(vp::res::ResByamlDictionaryPair) * m_node_count;

                /* Get container regions */
                vp::res::ResByamlContainer      *container   = reinterpret_cast<vp::res::ResByamlContainer*>(*container_offset);
                vp::res::ResByamlDictionaryPair *pair_array  = reinterpret_cast<vp::res::ResByamlDictionaryPair*>(*container_offset + sizeof(vp::res::ResByamlContainer));
                u32                             *remap_array = reinterpret_cast<u32*>(*container_offset + sizeof(vp::res::ResByamlContainer) + sizeof(vp::res::ResByamlDictionaryPair) * m_node_count);

                /* Advance container offset*/
                *container_offset = *container_offset + next_container_offset;

                /* Write container header */
                container->data_type = static_cast<u8>(this->GetByamlDataType());
                container->count     = m_node_count;

                if (m_is_index_remap == true) {

                    /* Stream out remapped container */
                    u32 i = 0;
                    for (ByamlNodeBase &node : m_node_list) {

                        /* Serialize big data */
                        if (ByamlNodeBigData::CheckRuntimeTypeInfo(std::addressof(node)) == true) {
                            reinterpret_cast<ByamlNodeBigData*>(std::addressof(node))->SerializeForBigData(big_data_offset, container_offset, head);
                        }

                        /* Write data type */
                        pair_array[i].data_type = static_cast<u8>(node.GetByamlDataType());

                        /* Write key */
                        pair_array[i].key_index = node.GetKey()->GetKeyIndex();

                        /* Serialize */
                        node.Serialize(reinterpret_cast<void*>(std::addressof(pair_array[i].u32_value)), head);
                        
                        /* Write index */
                        remap_array[i] = node.GetIndex();

                        ++i;
                    }

                } else {

                    /* Stream out container */
                    u32 i = 0;
                    for (ByamlNodeBase &node : m_node_list) {

                        /* Serialize big data */
                        if (ByamlNodeBigData::CheckRuntimeTypeInfo(std::addressof(node)) == true) {
                            reinterpret_cast<ByamlNodeBigData*>(std::addressof(node))->SerializeForBigData(big_data_offset, container_offset, head);
                        }

                        /* Write data type */
                        pair_array[i].data_type = static_cast<u8>(node.GetByamlDataType());

                        /* Write key */
                        pair_array[i].key_index = node.GetKey()->GetKeyIndex();

                        /* Serialize */
                        node.Serialize(reinterpret_cast<void*>(std::addressof(pair_array[i].u32_value)), head);

                        ++i;
                    }

                }

                return;
            }

            virtual void CalculateEndOffset(uintptr_t *big_data_offset_iter, uintptr_t *container_offset_iter, uintptr_t head) override {

                /* Update container size */
                const u32 remap_size   = (m_is_index_remap == false) ? 0 : sizeof(u32) * m_node_count;
                *container_offset_iter = vp::util::AlignUp(*container_offset_iter + sizeof(vp::res::ResByamlContainer) + remap_size + sizeof(vp::res::ResByamlDictionaryPair) * m_node_count, alignof(u32));

                /* Calculate sub nodes */
                for (ByamlNodeBase &node : m_node_list) {
                    if (ByamlNodeBigData::CheckRuntimeTypeInfo(std::addressof(node)) == false) { continue; }
                    reinterpret_cast<ByamlNodeBigData*>(std::addressof(node))->CalculateBigDataEndOffsets(big_data_offset_iter, container_offset_iter, head);
                }

                return;
            }
    };

    class ByamlBigDataNodeHashArray : public ByamlBigDataNodeContainerBase {
        private:
            bool m_is_index_remap;
            u32  m_key_size;
        public:
            constexpr ByamlBigDataNodeHashArray() : ByamlBigDataNodeContainerBase(), m_is_index_remap(false), m_key_size(sizeof(u32)) {/*...*/}
            virtual ~ByamlBigDataNodeHashArray() override {/*...*/}

            bool AddNode(ByamlNodeBase *node, u64 hash, u32 index = ByamlNodeBase::cInvalidIndex) {
 
                /* Set key and index */
                node->m_hash  = hash;
                node->m_index = index;

                /* Sorted insertion into list */
                ByamlNodeBase *link_node = nullptr;
                for (ByamlNodeBase &node_iter : m_node_list) {
                    if (node_iter.m_hash == hash)  { return false; }
                    if (node_iter.m_hash < hash) { continue; }
                    link_node = std::addressof(node_iter);
                    break;
                }

                /* Link node */
                if (link_node == nullptr) {
                    m_node_list.PushBack(*node);
                } else {
                    link_node->m_container_node.LinkPrev(std::addressof(node->m_container_node));
                }

                ++m_node_count;

                return true;
            }

            void RemoveNode(ByamlNodeBase *node) {
                node->m_container_node.Unlink();
                --m_node_count;
            }

            constexpr void SetIsIndexRemap(bool is_index_remap) { m_is_index_remap = is_index_remap; }
            constexpr void SetKeySize(u32 key_size) { m_key_size = key_size; }

            virtual constexpr res::ByamlDataType GetByamlDataType() const override { 
                const u32 sub_type = ((m_key_size / sizeof(u32)) - 1);
                return (m_is_index_remap == true) ? static_cast<res::ByamlDataType>(static_cast<u8>(res::ByamlDataType::HashArrayWithRemapU32_1) | sub_type) : static_cast<res::ByamlDataType>(static_cast<u8>(res::ByamlDataType::HashArrayU32_1) | sub_type); 
            }
            virtual void Serialize(uintptr_t *big_data_offset, uintptr_t *container_offset, vp::res::ResByaml *head) override  {

                /* Set offset */
                m_offset = static_cast<u32>(*container_offset - reinterpret_cast<uintptr_t>(head));

                /* Calculate sizes */
                const u32 remap_size            = (m_is_index_remap == false) ? 0 : sizeof(u32) * m_node_count;
                const u32 next_container_offset = sizeof(vp::res::ResByamlContainer) + (m_key_size + sizeof(u32)) * m_node_count + sizeof(u8) * m_node_count + remap_size;

                /* Get container regions */
                vp::res::ResByamlContainer *container       = reinterpret_cast<vp::res::ResByamlContainer*>(*container_offset);
                void                       *pair_array      = reinterpret_cast<void*>(*container_offset + sizeof(vp::res::ResByamlContainer));
                u8                         *data_type_array = reinterpret_cast<u8*>(*container_offset + sizeof(vp::res::ResByamlContainer) + (m_key_size + sizeof(u32)) * m_node_count);
                u32                        *remap_array     = reinterpret_cast<u32*>(*container_offset + sizeof(vp::res::ResByamlContainer) + (m_key_size + sizeof(u32)) * m_node_count + sizeof(u8) * m_node_count);

                /* Advance container offset*/
                *container_offset = *container_offset + next_container_offset;

                /* Write container header */
                container->data_type = static_cast<u8>(this->GetByamlDataType());
                container->count     = m_node_count;

                if (m_is_index_remap == true) {

                    /* Stream out remapped container */
                    u32 i = 0;
                    for (ByamlNodeBase &node : m_node_list) {

                        /* Serialize big data */
                        if (ByamlNodeBigData::CheckRuntimeTypeInfo(std::addressof(node)) == true) {
                            reinterpret_cast<ByamlNodeBigData*>(std::addressof(node))->SerializeForBigData(big_data_offset, container_offset, head);
                        }

                        /* Calculate pair offsets */
                        void *key = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(pair_array) + (m_key_size + sizeof(u32)) * i);
                        void *data = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(pair_array) + (m_key_size + sizeof(u32)) * i + m_key_size);

                        /* Write key TODO; large key support */
                        u64 node_key = node.GetHash();
                        ::memcpy(key, std::addressof(node_key), m_key_size);

                        /* Serialize */
                        node.Serialize(data, head);

                        /* Write data type */
                        data_type_array[i] = static_cast<u8>(node.GetByamlDataType());

                        /* Write index */
                        remap_array[i] = node.GetIndex();

                        ++i;
                    }

                } else {

                    /* Stream out container */
                    u32 i = 0;
                    for (ByamlNodeBase &node : m_node_list) {

                        /* Serialize big data */
                        if (ByamlNodeBigData::CheckRuntimeTypeInfo(std::addressof(node)) == true) {
                            reinterpret_cast<ByamlNodeBigData*>(std::addressof(node))->SerializeForBigData(big_data_offset, container_offset, head);
                        }

                        /* Calculate pair offsets */
                        void *key = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(pair_array) + (m_key_size + sizeof(u32)) * i);
                        void *data = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(pair_array) + (m_key_size + sizeof(u32)) * i + m_key_size);

                        /* Write key TODO; large key support */
                        u64 node_key = node.GetHash();
                        ::memcpy(key, std::addressof(node_key), m_key_size);

                        /* Serialize */
                        node.Serialize(data, head);

                        /* Write data type */
                        data_type_array[i] = static_cast<u8>(node.GetByamlDataType());

                        ++i;
                    }

                }

                return;
            }

            virtual void CalculateEndOffset(uintptr_t *big_data_offset_iter, uintptr_t *container_offset_iter, uintptr_t head) override {

                /* Update container size */
                const u32 remap_size   = (m_is_index_remap == false) ? 0 : sizeof(u32) * m_node_count;
                *container_offset_iter = vp::util::AlignUp(*container_offset_iter + sizeof(vp::res::ResByamlContainer) + remap_size + (m_key_size + sizeof(u32)) * m_node_count + sizeof(u8) * m_node_count, alignof(u32));

                /* Calculate sub nodes */
                for (ByamlNodeBase &node : m_node_list) {
                    if (ByamlNodeBigData::CheckRuntimeTypeInfo(std::addressof(node)) == false) { continue; }
                    reinterpret_cast<ByamlNodeBigData*>(std::addressof(node))->CalculateBigDataEndOffsets(big_data_offset_iter, container_offset_iter, head);
                }

                return;
            }
    };

    class ByamlBigDataNodeU64 : public ByamlBigDataNodeBase {
        private:
            u64 m_value;
        public:
            constexpr ByamlBigDataNodeU64() : ByamlBigDataNodeBase(), m_value() {/*...*/}
            constexpr ByamlBigDataNodeU64(u64 value) : ByamlBigDataNodeBase(), m_value(value) {/*...*/}
            virtual ~ByamlBigDataNodeU64() override {/*...*/}

            constexpr void SetValue(u64 value) { m_value = value; }

            virtual constexpr res::ByamlDataType GetByamlDataType() const override { return res::ByamlDataType::U64; }
            virtual void Serialize(uintptr_t *big_data_offset, [[maybe_unused]] uintptr_t *container_offset, [[maybe_unused]] vp::res::ResByaml *head) override {

                /* Write big data */
                *reinterpret_cast<u64*>(*big_data_offset) = m_value;

                /* Advance big data offset */
                *big_data_offset = vp::util::AlignUp(*big_data_offset + sizeof(u64), alignof(u64));

                return;
            }
            
            virtual void CalculateEndOffset(uintptr_t *big_data_offset_iter, [[maybe_unused]] uintptr_t *container_offset_iter, [[maybe_unused]] uintptr_t head) override {

                /* Update big data size */
                *big_data_offset_iter = vp::util::AlignUp(*big_data_offset_iter + sizeof(u64), alignof(u64));

                return;
            }
    };

    class ByamlBigDataNodeS64 : public ByamlBigDataNodeBase {
        private:
            s64 m_value;
        public:
            constexpr ByamlBigDataNodeS64() : ByamlBigDataNodeBase(), m_value() {/*...*/}
            constexpr ByamlBigDataNodeS64(s64 value) : ByamlBigDataNodeBase(), m_value(value) {/*...*/}
            virtual ~ByamlBigDataNodeS64() override {/*...*/}

            constexpr void SetValue(s64 value) { m_value = value; }

            virtual constexpr res::ByamlDataType GetByamlDataType() const override { return res::ByamlDataType::S64; }
            virtual void Serialize(uintptr_t *big_data_offset, [[maybe_unused]] uintptr_t *container_offset, [[maybe_unused]] vp::res::ResByaml *head) override {

                /* Write big data */
                *reinterpret_cast<s64*>(*big_data_offset) = m_value;

                /* Advance big data offset */
                *big_data_offset = vp::util::AlignUp(*big_data_offset + sizeof(s64), alignof(s64));

                return;
            }
            
            virtual void CalculateEndOffset(uintptr_t *big_data_offset_iter, [[maybe_unused]] uintptr_t *container_offset_iter, [[maybe_unused]] uintptr_t head) override {

                /* Update big data size */
                *big_data_offset_iter = vp::util::AlignUp(*big_data_offset_iter + sizeof(s64), alignof(s64));

                return;
            }
    };

    class ByamlBigDataNodeDouble : public ByamlBigDataNodeBase {
        private:
            double m_value;
        public:
            constexpr ByamlBigDataNodeDouble() : ByamlBigDataNodeBase(), m_value() {/*...*/}
            constexpr ByamlBigDataNodeDouble(double value) : ByamlBigDataNodeBase(), m_value(value) {/*...*/}
            virtual ~ByamlBigDataNodeDouble() override {/*...*/}

            constexpr void SetValue(double value) { m_value = value; }

            virtual constexpr res::ByamlDataType GetByamlDataType() const override { return res::ByamlDataType::F64; }
            virtual void Serialize(uintptr_t *big_data_offset, [[maybe_unused]] uintptr_t *container_offset, [[maybe_unused]] vp::res::ResByaml *head) override {

                /* Write big data */
                *reinterpret_cast<double*>(*big_data_offset) = m_value;

                /* Advance big data offset */
                *big_data_offset = vp::util::AlignUp(*big_data_offset + sizeof(double), alignof(double));

                return;
            }
            
            virtual void CalculateEndOffset(uintptr_t *big_data_offset_iter, [[maybe_unused]] uintptr_t *container_offset_iter, [[maybe_unused]] uintptr_t head) override {

                /* Update big data size */
                *big_data_offset_iter = vp::util::AlignUp(*big_data_offset_iter + sizeof(double), alignof(double));

                return;
            }
    };

    class ByamlBigDataNodeBinary : public ByamlBigDataNodeBase {
        public:
            static constexpr u32 cInvalidAlignment = 0xffff'ffff;
        private:
            void *m_data;
            u32   m_data_size;
            u32   m_data_alignment;
        public:
            constexpr ByamlBigDataNodeBinary() : ByamlBigDataNodeBase(), m_data(), m_data_size(0), m_data_alignment(cInvalidAlignment) {/*...*/}
            virtual ~ByamlBigDataNodeBinary() override {/*...*/}

            constexpr void SetBinaryData(void *data, u32 size, u32 alignment = cInvalidAlignment) {
                m_data           = data;
                m_data_size      = size;
                m_data_alignment = alignment;
            }

            virtual constexpr res::ByamlDataType GetByamlDataType() const override { return (m_data_alignment == cInvalidAlignment) ? res::ByamlDataType::BinaryData : res::ByamlDataType::BinaryDataWithAlignment; }
            virtual void Serialize(uintptr_t *big_data_offset, [[maybe_unused]] uintptr_t *container_offset, vp::res::ResByaml *head) override {

                /* Nothing to write if no data */
                if (m_data == nullptr || m_data_size == 0) { return; }

                /* Write alignment */
                u32  *data_array = reinterpret_cast<u32*>(*big_data_offset);
                void *data_base  = reinterpret_cast<void*>(*big_data_offset + sizeof(u32));
                if (m_data_alignment != cInvalidAlignment) {

                    /* Adjust data start */
                    data_array = reinterpret_cast<u32*>(vp::util::AlignUp(*big_data_offset + sizeof(u32) * 2, m_data_alignment) - (sizeof(u32) * 2));

                    /* Adjust data base */
                    data_base = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(data_array) + sizeof(u32) * 2);

                    /* Write alignment */
                    data_array[1] = m_data_alignment;
                }

                /* Set offset */
                m_offset = static_cast<u32>(reinterpret_cast<uintptr_t>(data_array) - reinterpret_cast<uintptr_t>(head));

                /* Write data_size */
                data_array[0] = m_data_size;

                /* Copy data */
                ::memcpy(data_base, m_data, m_data_size);

                /* Advance big data offset */
                *big_data_offset = reinterpret_cast<uintptr_t>(data_base) + m_data_size;

                return;
            }

            virtual void CalculateEndOffset(uintptr_t *big_data_offset_iter, [[maybe_unused]] uintptr_t *container_offset_iter, [[maybe_unused]] uintptr_t head) override {

                /* No size if no data */
                if (m_data == nullptr || m_data_size == 0) { return; }

                /* Update big data size */
                if (m_data_alignment != cInvalidAlignment) {
                    *big_data_offset_iter = vp::util::AlignUp(*big_data_offset_iter + sizeof(u32) * 2, m_data_alignment) + m_data_size;
                } else {
                    *big_data_offset_iter = *big_data_offset_iter + sizeof(u32) + m_data_size;
                }

                return;
            }
    };
}
