#include <vp.hpp>

namespace vp::resbui {

    void ByamlBigDataNodeContainerBase::Finalize() {
        m_node_list.Clear();
    }

    void ByamlBigDataNodeContainerBase::PushKeys(ByamlStringPoolBuilder *pool_builder) {
        for (ByamlNodeBase &node : m_node_list) {
            node.PushKeys(pool_builder);
        }
    }

    void ByamlBigDataNodeContainerBase::PushStrings([[maybe_unused]] ByamlStringPoolBuilder *pool_builder) {
        for (ByamlNodeBase &node : m_node_list) {
            node.PushStrings(pool_builder);
        }
    }

    void ByamlBigDataNodeContainerBase::CalculateBigDataOffset(uintptr_t *big_data_offset_iter, uintptr_t head) {
        /* Calculate sub nodes */
        for (ByamlNodeBase &node : m_node_list) {
            ByamlNodeBigData *big_data = vp::util::DynamicCast<ByamlNodeBigData*>(std::addressof(node));
            if (big_data == nullptr) { continue; }
            big_data->CalculateBigDataOffsets(big_data_offset_iter, head);
        }
        return;
    }
    void ByamlBigDataNodeContainerBase::CalculateContainerOffset(uintptr_t *container_offset_iter, uintptr_t head) {
        /* Calculate sub nodes */
        for (ByamlNodeBase &node : m_node_list) {
            ByamlNodeBigData *big_data = vp::util::DynamicCast<ByamlNodeBigData*>(std::addressof(node));
            if (big_data == nullptr) { continue; }
            big_data->CalculateContainerOffsets(container_offset_iter, head);
        }
        return;
    }

    void ByamlBigDataNodeArray::AddNode(ByamlNodeBase *node) {
        m_node_list.PushBack(*node);
        ++m_node_count;
    }
    
    void ByamlBigDataNodeArray::RemoveNode(ByamlNodeBase *node) {
        node->m_container_node.Unlink();
        --m_node_count;
    }

    void ByamlBigDataNodeArray::Serialize(uintptr_t *big_data_offset, uintptr_t *container_offset, vp::res::ResByaml *head) {

        /* Calculate sizes */
        const bool is_mono_typed        = this->CheckMonoTyped();
        const u32 data_type_size        = (!is_mono_typed) ? vp::util::AlignUp(sizeof(vp::res::ByamlDataType) * m_node_count, alignof(u32)) : vp::util::AlignUp(sizeof(vp::res::ByamlDataType), alignof(u32));
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
        if (head->IsReverseEndian()) { container->SwapEndian(); }

        if (is_mono_typed) {

            /* Write mono type */
            if (!m_node_list.IsEmpty()) {
                *data_type_array = static_cast<u8>(m_node_list.Front().GetByamlDataType());
            }

            /* Stream out mono-typed container */
            u32 i = 0;
            for (ByamlNodeBase &node : m_node_list) {

                /* Serialize big data */
                if (ByamlNodeBigData::CheckRuntimeTypeInfoStatic(std::addressof(node))) {
                    reinterpret_cast<ByamlNodeBigData*>(std::addressof(node))->SerializeForBigData(big_data_offset, container_offset, head);
                }

                /* Serialize */
                node.Serialize(reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(value_array) + i * sizeof(u32)), head);
                ++i;
            }

        } else {

            /* Stream out multi-typed container */
            u32 i = 0;
            for (ByamlNodeBase &node : m_node_list) {

                /* Serialize data type */
                data_type_array[i] = static_cast<u8>(node.GetByamlDataType());

                /* Serialize big data */
                if (ByamlNodeBigData::CheckRuntimeTypeInfoStatic(std::addressof(node))) {
                    reinterpret_cast<ByamlNodeBigData*>(std::addressof(node))->SerializeForBigData(big_data_offset, container_offset, head);
                }

                /* Serialize */
                node.Serialize(reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(value_array) + i * sizeof(u32)), head);
                ++i;
            }

        }

        return;
    }

    void ByamlBigDataNodeArray::CalculateContainerOffset(uintptr_t *container_offset_iter, uintptr_t head) {
        m_cache.container_offset = static_cast<u32>(*container_offset_iter);
        /* Update container size */
        const u32 data_type_size = (!this->CheckMonoTyped()) ? vp::util::AlignUp(sizeof(vp::res::ByamlDataType) * m_node_count, alignof(u32))  : vp::util::AlignUp(sizeof(vp::res::ByamlDataType), alignof(u32));
        *container_offset_iter = vp::util::AlignUp(*container_offset_iter + sizeof(vp::res::ResByamlContainer) + data_type_size + sizeof(u32) * m_node_count, alignof(u32));
        this->ByamlBigDataNodeContainerBase::CalculateContainerOffset(container_offset_iter, head);
        return;
    }
    
    bool ByamlBigDataNodeDictionary::AddNode(ByamlNodeBase *node, const char *key, u32 index) {

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

    void ByamlBigDataNodeDictionary::RemoveNode(ByamlNodeBase *node) {
        node->m_container_node.Unlink();
        --m_node_count;
    }

    void ByamlBigDataNodeDictionary::Serialize(uintptr_t *big_data_offset, uintptr_t *container_offset, vp::res::ResByaml *head) {

        /* Calculate sizes */
        const u32 remap_size            = this->CalculateRemapSize();
        const u32 next_container_offset = sizeof(vp::res::ResByamlContainer) + remap_size + sizeof(vp::res::ResByamlDictionaryPair) * m_node_count;

        /* Get container regions */
        vp::res::ResByamlContainer      *container   = reinterpret_cast<vp::res::ResByamlContainer*>(*container_offset);
        vp::res::ResByamlDictionaryPair *pair_array  = reinterpret_cast<vp::res::ResByamlDictionaryPair*>(*container_offset + sizeof(vp::res::ResByamlContainer));
        void                            *remap_array = reinterpret_cast<u32*>(*container_offset + sizeof(vp::res::ResByamlContainer) + sizeof(vp::res::ResByamlDictionaryPair) * m_node_count);

        /* Advance container offset*/
        *container_offset = *container_offset + next_container_offset;

        /* Write container header */
        container->data_type = static_cast<u8>(this->GetByamlDataType());
        container->count     = m_node_count;
        if (head->IsReverseEndian()) { container->SwapEndian(); }

        if (m_is_index_remap) {

            /* Stream out remapped container */
            u32 remap_size = (m_node_count < 0x100) ? 1 : (m_node_count < 0x10000) ? 2 : 3;
            u32 i = 0;
            for (ByamlNodeBase &node : m_node_list) {

                /* Serialize big data */
                if (ByamlNodeBigData::CheckRuntimeTypeInfoStatic(std::addressof(node))) {
                    reinterpret_cast<ByamlNodeBigData*>(std::addressof(node))->SerializeForBigData(big_data_offset, container_offset, head);
                }

                /* Write data type */
                pair_array[i].data_type = static_cast<u8>(node.GetByamlDataType());

                /* Write key */
                const u32 key_index = node.GetKey()->GetKeyIndex();
                pair_array[i].key_index = (!head->IsReverseEndian()) ? key_index : vp::util::SwapEndian24(key_index);

                /* Serialize */
                node.Serialize(reinterpret_cast<void*>(std::addressof(pair_array[i].u32_value)), head);

                /* Write index */
                if (remap_size == 1) {
                    const u8 remap_index = static_cast<u8>(node.GetIndex());
                    reinterpret_cast<u8*>(remap_array)[i] = remap_index;
                } else if (remap_size == 2) {
                    const u16 remap_index = static_cast<u16>(node.GetIndex());
                    reinterpret_cast<u16*>(remap_array)[i] = (!head->IsReverseEndian()) ? remap_index : vp::util::SwapEndian(remap_index);
                } else if (remap_size == 3) {
                    const u32 remap_index = static_cast<u32>(node.GetIndex());
                    reinterpret_cast<u32*>(remap_array)[i] = (!head->IsReverseEndian()) ? remap_index : vp::util::SwapEndian(remap_index);
                }

                ++i;
            }

        } else {

            /* Stream out container */
            u32 i = 0;
            for (ByamlNodeBase &node : m_node_list) {

                /* Serialize big data */
                if (ByamlNodeBigData::CheckRuntimeTypeInfoStatic(std::addressof(node))) {
                    reinterpret_cast<ByamlNodeBigData*>(std::addressof(node))->SerializeForBigData(big_data_offset, container_offset, head);
                }

                /* Write data type */
                pair_array[i].data_type = static_cast<u8>(node.GetByamlDataType());

                /* Write key */
                const u32 key_index = node.GetKey()->GetKeyIndex();
                pair_array[i].key_index = (!head->IsReverseEndian()) ? key_index : vp::util::SwapEndian24(key_index);

                /* Serialize */
                node.Serialize(reinterpret_cast<void*>(std::addressof(pair_array[i].u32_value)), head);

                ++i;
            }

        }

        return;
    }

    void ByamlBigDataNodeDictionary::CalculateContainerOffset(uintptr_t *container_offset_iter, uintptr_t head) {
        m_cache.container_offset = static_cast<u32>(*container_offset_iter);
        /* Update container size */
        const u32 remap_size   = this->CalculateRemapSize();
        *container_offset_iter = vp::util::AlignUp(*container_offset_iter + sizeof(vp::res::ResByamlContainer) + remap_size + sizeof(vp::res::ResByamlDictionaryPair) * m_node_count, alignof(u32));
        this->ByamlBigDataNodeContainerBase::CalculateContainerOffset(container_offset_iter, head);
        return;
    }
    
    bool ByamlBigDataNodeHashArray::AddNode(ByamlNodeBase *node, u64 hash, u32 index) {

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

    void ByamlBigDataNodeHashArray::RemoveNode(ByamlNodeBase *node) {
        node->m_container_node.Unlink();
        --m_node_count;
    }

    void ByamlBigDataNodeHashArray::Serialize(uintptr_t *big_data_offset, uintptr_t *container_offset, vp::res::ResByaml *head) {

        /* Calculate sizes */
        const u32 remap_size            = this->CalculateRemapSize();
        const u32 next_container_offset = sizeof(vp::res::ResByamlContainer) + (m_key_size + sizeof(u32)) * m_node_count + vp::util::AlignUp(sizeof(u8) * m_node_count, alignof(u32)) + remap_size;

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
        if (head->IsReverseEndian()) { container->SwapEndian(); }

        if (m_is_index_remap) {

            /* Stream out remapped container */
            u32 remap_size = (m_node_count < 0x100) ? 1 : (m_node_count < 0x10000) ? 2 : 3;
            u32 i = 0;
            for (ByamlNodeBase &node : m_node_list) {

                /* Serialize big data */
                if (ByamlNodeBigData::CheckRuntimeTypeInfoStatic(std::addressof(node))) {
                    reinterpret_cast<ByamlNodeBigData*>(std::addressof(node))->SerializeForBigData(big_data_offset, container_offset, head);
                }

                /* Calculate pair offsets */
                void *key = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(pair_array) + (m_key_size + sizeof(u32)) * i);
                void *data = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(pair_array) + (m_key_size + sizeof(u32)) * i + m_key_size);

                /* Write key TODO; large key support */
                u64 node_key = node.GetHash();
                std::memcpy(key, std::addressof(node_key), m_key_size);

                /* Serialize */
                node.Serialize(data, head);

                /* Write data type */
                data_type_array[i] = static_cast<u8>(node.GetByamlDataType());

                /* Write remap index */
                if (remap_size == 1) {
                    const u8 remap_index = static_cast<u8>(node.GetIndex());
                    reinterpret_cast<u8*>(remap_array)[i] = remap_index;
                } else if (remap_size == 2) {
                    const u16 remap_index = static_cast<u16>(node.GetIndex());
                    reinterpret_cast<u16*>(remap_array)[i] = (!head->IsReverseEndian()) ? remap_index : vp::util::SwapEndian(remap_index);
                } else if (remap_size == 3) {
                    const u32 remap_index = static_cast<u32>(node.GetIndex());
                    reinterpret_cast<u32*>(remap_array)[i] = (!head->IsReverseEndian()) ? remap_index : vp::util::SwapEndian(remap_index);
                }
                ++i;
            }
        } else {

            /* Stream out container */
            u32 i = 0;
            for (ByamlNodeBase &node : m_node_list) {

                /* Serialize big data */
                if (ByamlNodeBigData::CheckRuntimeTypeInfoStatic(std::addressof(node))) {
                    reinterpret_cast<ByamlNodeBigData*>(std::addressof(node))->SerializeForBigData(big_data_offset, container_offset, head);
                }

                /* Calculate pair offsets */
                void *key = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(pair_array) + (m_key_size + sizeof(u32)) * i);
                void *data = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(pair_array) + (m_key_size + sizeof(u32)) * i + m_key_size);

                /* Write key TODO; large key support */
                u64 node_key = node.GetHash();
                std::memcpy(key, std::addressof(node_key), m_key_size);

                /* Serialize */
                node.Serialize(data, head);

                /* Write data type */
                data_type_array[i] = static_cast<u8>(node.GetByamlDataType());

                ++i;
            }
        }

        return;
    }

    void ByamlBigDataNodeHashArray::CalculateContainerOffset(uintptr_t *container_offset_iter, uintptr_t head) {
        m_cache.container_offset = static_cast<u32>(*container_offset_iter);
        /* Update container size */
        const u32 remap_size   = this->CalculateRemapSize();
        *container_offset_iter = vp::util::AlignUp(*container_offset_iter + sizeof(vp::res::ResByamlContainer) + remap_size + (m_key_size + sizeof(u32)) * m_node_count + sizeof(u8) * m_node_count, alignof(u32));
        this->ByamlBigDataNodeContainerBase::CalculateContainerOffset(container_offset_iter, head);
        return;
    }
    
    void ByamlBigDataNodeU64::Serialize(uintptr_t *big_data_offset, [[maybe_unused]] uintptr_t *container_offset, [[maybe_unused]] vp::res::ResByaml *head) {

        /* Write big data */
        *reinterpret_cast<u64*>(*big_data_offset) = (!head->IsReverseEndian()) ? m_value : vp::util::SwapEndian(m_value);

        /* Advance big data offset */
        *big_data_offset = *big_data_offset + sizeof(u64);

        return;
    }

    void ByamlBigDataNodeU64::CalculateBigDataOffset(uintptr_t *big_data_offset_iter, [[maybe_unused]] uintptr_t head) {
        m_cache.big_data_offset = static_cast<u32>(*big_data_offset_iter);
        *big_data_offset_iter = *big_data_offset_iter + sizeof(u64);
        return;
    }

    void ByamlBigDataNodeS64::Serialize(uintptr_t *big_data_offset, [[maybe_unused]] uintptr_t *container_offset, [[maybe_unused]] vp::res::ResByaml *head) {

        /* Write big data */
        *reinterpret_cast<s64*>(*big_data_offset) =  (!head->IsReverseEndian()) ? m_value : vp::util::SwapEndian(m_value);

        /* Advance big data offset */
        *big_data_offset = *big_data_offset + sizeof(s64);

        return;
    }
    
    void ByamlBigDataNodeS64::CalculateBigDataOffset(uintptr_t *big_data_offset_iter, [[maybe_unused]] uintptr_t head) {
        m_cache.big_data_offset = static_cast<u32>(*big_data_offset_iter);
        *big_data_offset_iter = *big_data_offset_iter + sizeof(s64);
        return;
    }

    void ByamlBigDataNodeF64::Serialize(uintptr_t *big_data_offset, [[maybe_unused]] uintptr_t *container_offset, vp::res::ResByaml *head) {

        /* Write big data */
        *reinterpret_cast<double*>(*big_data_offset) = (!head->IsReverseEndian()) ? m_value : vp::util::SwapEndian(m_value);

        /* Advance big data offset */
        *big_data_offset = *big_data_offset + sizeof(double);

        return;
    }

    void ByamlBigDataNodeF64::CalculateBigDataOffset(uintptr_t *big_data_offset_iter, [[maybe_unused]] uintptr_t head) {
        m_cache.big_data_offset = static_cast<u32>(*big_data_offset_iter);
        *big_data_offset_iter = *big_data_offset_iter + sizeof(double);
        return;
    }

    void ByamlBigDataNodeBinary::Serialize(uintptr_t *big_data_offset, [[maybe_unused]] uintptr_t *container_offset, vp::res::ResByaml *head) {

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
            data_array[1] = (!head->IsReverseEndian()) ? m_data_alignment : vp::util::SwapEndian(m_data_alignment);
        }

        /* Write data_size */
        data_array[0] = (!head->IsReverseEndian()) ? m_data_size : vp::util::SwapEndian(m_data_size);

        /* Copy data */
        std::memcpy(data_base, m_data, m_data_size);

        /* Advance big data offset */
        *big_data_offset = reinterpret_cast<uintptr_t>(data_base) + m_data_size;

        return;
    }

    void ByamlBigDataNodeBinary::CalculateBigDataOffset(uintptr_t *big_data_offset_iter, [[maybe_unused]] uintptr_t head) {
        
        /* No size if no data */
        if (m_data == nullptr || m_data_size == 0) { return; }

        uintptr_t iter = *big_data_offset_iter;
        if (m_data_alignment != cInvalidAlignment) {
            uintptr_t base = vp::util::AlignUp(*big_data_offset_iter + sizeof(u32) * 2, m_data_alignment) - (sizeof(u32) * 2);
            m_cache.big_data_offset = static_cast<u32>(base);
            *big_data_offset_iter = base + m_data_size;
        } else {
            m_cache.big_data_offset = iter;
            *big_data_offset_iter = iter + sizeof(u32) + m_data_size;
        }

        return;
    }
}
