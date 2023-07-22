#include <vp.hpp>

namespace vp::res {

    struct BigDataNode {
        vp::util::IntrusiveRedBlackTreeNode<u32> node;
    };

    struct ByamlBigDataCache {
        using BigDataMap = vp::util::IntrusiveRedBlackTreeTraits<BigDataNode, &BigDataNode::node>::Tree;

        vp::imem::IHeap *heap;
        BigDataMap       map;

        bool Cache(u32 data) {

            /* Check if data is in cache */
            if (map.Find(data) != nullptr) { return false; }

            /* Allocate a new cache node  */
            BigDataNode *node = new (heap, alignof(BigDataNode)) BigDataNode();
            VP_ASSERT(node != nullptr);

            /* Set key to byaml data offset */
            node->node.SetKey(data);

            /* Push to tree map */
            map.Insert(node);

            return true;
        }

        void FreeAll() {

            /* Free all nodes */
            BigDataNode *iter = map.Start();
            while (iter != nullptr) {
                BigDataNode *node = iter;
                iter = map.GetNext(iter);
                map.Remove(node);
                ::operator delete (node, heap, alignof(BigDataNode));
            }

            return;
        }
    };

    u32 ResByaml::SwapEndianData(ByamlDataType data_type, u32 data, ByamlBigDataCache *big_data_cache) {

        /* Swap data by type */
        const u32 swapped_data = vp::util::SwapEndian(data);
        switch(data_type) {
            case ByamlDataType::Bool:
            case ByamlDataType::S32:
            case ByamlDataType::F32:
            case ByamlDataType::U32:
            case ByamlDataType::StringIndex:
            {
                return swapped_data;
            }
            case ByamlDataType::HashArrayU32_1:
            case ByamlDataType::HashArrayU32_2:
            case ByamlDataType::HashArrayU32_3:
            case ByamlDataType::HashArrayU32_4:
            case ByamlDataType::HashArrayU32_5:
            case ByamlDataType::HashArrayU32_6:
            case ByamlDataType::HashArrayU32_7:
            case ByamlDataType::HashArrayU32_8:
            case ByamlDataType::HashArrayU32_9:
            case ByamlDataType::HashArrayU32_10:
            case ByamlDataType::HashArrayU32_11:
            case ByamlDataType::HashArrayU32_12:
            case ByamlDataType::HashArrayU32_13:
            case ByamlDataType::HashArrayU32_14:
            case ByamlDataType::HashArrayU32_15:
            case ByamlDataType::HashArrayU32_16:
            case ByamlDataType::HashArrayWithRemapU32_1:
            case ByamlDataType::HashArrayWithRemapU32_2:
            case ByamlDataType::HashArrayWithRemapU32_3:
            case ByamlDataType::HashArrayWithRemapU32_4:
            case ByamlDataType::HashArrayWithRemapU32_5:
            case ByamlDataType::HashArrayWithRemapU32_6:
            case ByamlDataType::HashArrayWithRemapU32_7:
            case ByamlDataType::HashArrayWithRemapU32_8:
            case ByamlDataType::HashArrayWithRemapU32_9:
            case ByamlDataType::HashArrayWithRemapU32_10:
            case ByamlDataType::HashArrayWithRemapU32_11:
            case ByamlDataType::HashArrayWithRemapU32_12:
            case ByamlDataType::HashArrayWithRemapU32_13:
            case ByamlDataType::HashArrayWithRemapU32_14:
            case ByamlDataType::HashArrayWithRemapU32_15:
            case ByamlDataType::HashArrayWithRemapU32_16:
            case ByamlDataType::Dictionary:
            case ByamlDataType::DictionaryWithRemap:
            case ByamlDataType::Array:
            case ByamlDataType::MonoTypedArray:
            case ByamlDataType::BinaryDataWithAlignment:
            case ByamlDataType::BinaryData:
            case ByamlDataType::S64:
            case ByamlDataType::F64:
            case ByamlDataType::U64:
            {
                /* Cache big data offset */
                const bool is_big_data_swapped = big_data_cache->Cache(swapped_data);

                /* Swap big data */
                if (is_big_data_swapped == true) { 
                    this->SwapEndianForBigData(data_type, swapped_data, big_data_cache);
                }

                return swapped_data;
            }
            default:
                break;
        }

        return 0;
    }

    void ResByaml::SwapEndianForBigData(ByamlDataType data_type, u32 data, ByamlBigDataCache *big_data_cache) {

        const u32 r_offset = (this->IsReverseEndian() == false) ? data : vp::util::SwapEndian(data);

        void *big_data = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(this) + r_offset);
        switch (data_type) {
            case ByamlDataType::HashArrayU32_1:
            case ByamlDataType::HashArrayU32_2:
            case ByamlDataType::HashArrayU32_3:
            case ByamlDataType::HashArrayU32_4:
            case ByamlDataType::HashArrayU32_5:
            case ByamlDataType::HashArrayU32_6:
            case ByamlDataType::HashArrayU32_7:
            case ByamlDataType::HashArrayU32_8:
            case ByamlDataType::HashArrayU32_9:
            case ByamlDataType::HashArrayU32_10:
            case ByamlDataType::HashArrayU32_11:
            case ByamlDataType::HashArrayU32_12:
            case ByamlDataType::HashArrayU32_13:
            case ByamlDataType::HashArrayU32_14:
            case ByamlDataType::HashArrayU32_15:
            case ByamlDataType::HashArrayU32_16:
            case ByamlDataType::HashArrayWithRemapU32_1:
            case ByamlDataType::HashArrayWithRemapU32_2:
            case ByamlDataType::HashArrayWithRemapU32_3:
            case ByamlDataType::HashArrayWithRemapU32_4:
            case ByamlDataType::HashArrayWithRemapU32_5:
            case ByamlDataType::HashArrayWithRemapU32_6:
            case ByamlDataType::HashArrayWithRemapU32_7:
            case ByamlDataType::HashArrayWithRemapU32_8:
            case ByamlDataType::HashArrayWithRemapU32_9:
            case ByamlDataType::HashArrayWithRemapU32_10:
            case ByamlDataType::HashArrayWithRemapU32_11:
            case ByamlDataType::HashArrayWithRemapU32_12:
            case ByamlDataType::HashArrayWithRemapU32_13:
            case ByamlDataType::HashArrayWithRemapU32_14:
            case ByamlDataType::HashArrayWithRemapU32_15:
            case ByamlDataType::HashArrayWithRemapU32_16:
            case ByamlDataType::Dictionary:
            case ByamlDataType::DictionaryWithRemap:
            case ByamlDataType::Array:
            case ByamlDataType::MonoTypedArray:
            {
                ResByamlContainer *container = reinterpret_cast<ResByamlContainer*>(big_data);
                this->SwapEndianDataContainer(container, big_data_cache);
                break;
            }
            {
            case ByamlDataType::BinaryDataWithAlignment:
                *(reinterpret_cast<u32*>(reinterpret_cast<uintptr_t>(big_data) + sizeof(u32))) = vp::util::SwapEndian(*(reinterpret_cast<u32*>(reinterpret_cast<uintptr_t>(big_data) + sizeof(u32))));
                [[fallthrough]];
            case ByamlDataType::BinaryData:
                *reinterpret_cast<u32*>(big_data) = vp::util::SwapEndian(*reinterpret_cast<u32*>(big_data));
                break;
            }
            case ByamlDataType::S64:
            case ByamlDataType::F64:
            case ByamlDataType::U64:
            {
                *reinterpret_cast<u64*>(big_data) = vp::util::SwapEndian(*reinterpret_cast<u64*>(big_data));
                break;
            }
            default:
                break;
        }
        return;
    }

    void ResByaml::SwapEndianHashArray(ResByamlContainer *container, ByamlBigDataCache *big_data_cache) {

        /* Larger than 64-bit keys are currently unsupported */
        const ByamlDataType container_type = static_cast<ByamlDataType>(container->data_type);
        if (static_cast<u32>(ByamlDataType::HashArrayU32_2) < (static_cast<u32>(container_type) & 0x2f)) { return; }

        /* Swap table */
        const u32 r_count     = (this->IsReverseEndian() == false) ? static_cast<u32>(container->count) : vp::util::SwapEndian24(static_cast<u32>(container->count));
        const u32 hash_size   = sizeof(u32) * ((static_cast<u32>(container_type) & 0xf) + 1);
        const u32 stride      = hash_size + sizeof(u32);
        void *content_base    = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(container) + sizeof(ResByamlContainer));
        u8   *data_type_array = reinterpret_cast<u8*>(reinterpret_cast<uintptr_t>(container) + sizeof(ResByamlContainer) + stride * r_count);
        u32  *remap_array     = reinterpret_cast<u32*>(reinterpret_cast<uintptr_t>(container) + sizeof(ResByamlContainer) + stride * r_count + vp::util::AlignUp(sizeof(u8) * r_count, alignof(u32)));
        for (u32 i = 0; i < r_count; ++i) {

            /* Swap hash */
            void *hash_offset = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(content_base) + stride * i);
            if (hash_size == sizeof(u32)) {
                *reinterpret_cast<u32*>(hash_offset) = vp::util::SwapEndian(*reinterpret_cast<u32*>(hash_offset));
            } else if (hash_size == sizeof(u64)) {
                *reinterpret_cast<u64*>(hash_offset) = vp::util::SwapEndian(*reinterpret_cast<u64*>(hash_offset));
            }

            /* Swap data */
            u32 *data_offset = reinterpret_cast<u32*>(reinterpret_cast<uintptr_t>(hash_offset) + hash_size);
            *data_offset     = this->SwapEndianData(static_cast<ByamlDataType>(data_type_array[i]), *data_offset, big_data_cache);

            /* Swap index */
            if ((static_cast<u32>(container_type) & 0x10) != 0) {
                remap_array[i] = vp::util::SwapEndian(remap_array[i]);
            }
        }

        return;
    }
    void ResByaml::SwapEndianDictionary(ResByamlContainer *container, ByamlBigDataCache *big_data_cache) {

        /* Swap table */
        const u32 r_count                      = (this->IsReverseEndian() == false) ? static_cast<u32>(container->count) : vp::util::SwapEndian24(static_cast<u32>(container->count));
        const ByamlDataType     container_type = static_cast<ByamlDataType>(container->data_type);
        ResByamlDictionaryPair *pair_array     = reinterpret_cast<ResByamlDictionaryPair*>(reinterpret_cast<uintptr_t>(container) + sizeof(ResByamlContainer));
        u32                    *remap_array    = reinterpret_cast<u32*>(reinterpret_cast<uintptr_t>(container) + sizeof(ResByamlContainer) + sizeof(ResByamlDictionaryPair) * r_count);
        for (u32 i = 0; i < r_count; ++i) {

            /* Swap key index */
            pair_array[i].key_index = vp::util::SwapEndian24(pair_array[i].key_index);

            /* Swap data */
            pair_array[i].u32_value = this->SwapEndianData(static_cast<ByamlDataType>(pair_array[i].data_type), pair_array[i].u32_value, big_data_cache);

            /* Swap index */
            if (container_type == ByamlDataType::DictionaryWithRemap) {
                remap_array[i] = vp::util::SwapEndian(remap_array[i]);
            }
        }

        return;
    }

    void ResByaml::SwapEndianArray(ResByamlContainer *container, ByamlBigDataCache *big_data_cache) {

        /* Swap table */
        const u32 r_count                  = (this->IsReverseEndian() == false) ? static_cast<u32>(container->count) : vp::util::SwapEndian24(static_cast<u32>(container->count));
        const ByamlDataType container_type = static_cast<ByamlDataType>(container->data_type);
        u8  *data_type_array               = reinterpret_cast<u8*>(reinterpret_cast<uintptr_t>(container) + sizeof(ResByamlContainer));
        u32 *data_array                    = (container_type == ByamlDataType::Array) ? reinterpret_cast<u32*>(reinterpret_cast<uintptr_t>(container) + sizeof(ResByamlContainer) + vp::util::AlignUp(sizeof(u8) * r_count, alignof(u32))) : reinterpret_cast<u32*>(reinterpret_cast<uintptr_t>(container) + sizeof(ResByamlContainer) + vp::util::AlignUp(sizeof(u8), alignof(u32)));
        for (u32 i = 0; i < r_count; ++i) {

            /* Read data type */
            const ByamlDataType data_type = (container_type == ByamlDataType::Array) ? static_cast<ByamlDataType>(data_type_array[i]) : static_cast<ByamlDataType>(*data_type_array);

            /* Swap data */
            data_array[i] = this->SwapEndianData(data_type, data_array[i], big_data_cache);
        }

        return;
    }

    void ResByaml::SwapEndianDataContainer(ResByamlContainer *container, ByamlBigDataCache *big_data_cache) {

        /* Swap header endian */
        container->SwapEndian();

        /* Swap container contents  */
        switch (static_cast<ByamlDataType>(container->data_type)) {
            case ByamlDataType::HashArrayU32_1:
            case ByamlDataType::HashArrayU32_2:
            case ByamlDataType::HashArrayU32_3:
            case ByamlDataType::HashArrayU32_4:
            case ByamlDataType::HashArrayU32_5:
            case ByamlDataType::HashArrayU32_6:
            case ByamlDataType::HashArrayU32_7:
            case ByamlDataType::HashArrayU32_8:
            case ByamlDataType::HashArrayU32_9:
            case ByamlDataType::HashArrayU32_10:
            case ByamlDataType::HashArrayU32_11:
            case ByamlDataType::HashArrayU32_12:
            case ByamlDataType::HashArrayU32_13:
            case ByamlDataType::HashArrayU32_14:
            case ByamlDataType::HashArrayU32_15:
            case ByamlDataType::HashArrayU32_16:
            case ByamlDataType::HashArrayWithRemapU32_1:
            case ByamlDataType::HashArrayWithRemapU32_2:
            case ByamlDataType::HashArrayWithRemapU32_3:
            case ByamlDataType::HashArrayWithRemapU32_4:
            case ByamlDataType::HashArrayWithRemapU32_5:
            case ByamlDataType::HashArrayWithRemapU32_6:
            case ByamlDataType::HashArrayWithRemapU32_7:
            case ByamlDataType::HashArrayWithRemapU32_8:
            case ByamlDataType::HashArrayWithRemapU32_9:
            case ByamlDataType::HashArrayWithRemapU32_10:
            case ByamlDataType::HashArrayWithRemapU32_11:
            case ByamlDataType::HashArrayWithRemapU32_12:
            case ByamlDataType::HashArrayWithRemapU32_13:
            case ByamlDataType::HashArrayWithRemapU32_14:
            case ByamlDataType::HashArrayWithRemapU32_15:
            case ByamlDataType::HashArrayWithRemapU32_16:
            {
                this->SwapEndianHashArray(container, big_data_cache);
                break;
            }
            case ByamlDataType::Dictionary:
            case ByamlDataType::DictionaryWithRemap:
            {
                this->SwapEndianDictionary(container, big_data_cache);
                break;
            }
            case ByamlDataType::Array:
            case ByamlDataType::MonoTypedArray:
            {
                this->SwapEndianArray(container, big_data_cache);
                break;
            }
            default:
                break;
        }

        return;
    }

    void ResByaml::SwapRootContainerEndian(u32 table_offset, ByamlBigDataCache *big_data_cache) {

        /* Null table check */
        if (table_offset == 0) { return; }

        /* Cache big data offset */
        const bool is_already_swapped = big_data_cache->Cache(table_offset);
        if (is_already_swapped == false) { return; }

        /* Get root dictionary */
        ResByamlContainer *root_dic = reinterpret_cast<vp::res::ResByamlContainer*>(reinterpret_cast<uintptr_t>(this) + table_offset);
        if (IsContainerType(static_cast<ByamlDataType>(root_dic->data_type)) == false) { return; }

        /* Swap endian */
        this->SwapEndianDataContainer(root_dic, big_data_cache);

        return;
    }

    void ResByaml::SwapStringTableEndian(u32 table_offset, ByamlBigDataCache *big_data_cache) {

        /* Null table check */
        if (table_offset == 0) { return; }

        /* Cache big data offset */
        const bool is_already_swapped = big_data_cache->Cache(table_offset);
        if (is_already_swapped == false) { return; }

        /* Get table */
        ResByamlContainer *string_table = reinterpret_cast<vp::res::ResByamlContainer*>(reinterpret_cast<uintptr_t>(this) + table_offset);
        if (static_cast<ByamlDataType>(string_table->data_type) != ByamlDataType::KeyTable) { return; }

        /* Swap endian */
        string_table->SwapEndian();
        const u32 string_count = (this->IsReverseEndian() == false) ? string_table->count : vp::util::SwapEndian24(string_table->count);
        const u32 index_count = string_count + 1;
        u32 *index_array = reinterpret_cast<u32*>(reinterpret_cast<uintptr_t>(string_table) + sizeof(ResByamlContainer));
        for (u32 i = 0; i < index_count; ++i) {
            index_array[i] = vp::util::SwapEndian(index_array[i]);
        }

        return;
    }

    void ResByaml::SwapKeyTableEndian(u32 table_offset, ByamlBigDataCache *big_data_cache) {

        /* Null table check */
        if (table_offset == 0) { return; }

        /* Cache big data offset */
        const bool is_already_swapped = big_data_cache->Cache(table_offset);
        if (is_already_swapped == false) { return; }

        /* Get table */
        ResByamlContainer *key_table = reinterpret_cast<vp::res::ResByamlContainer*>(reinterpret_cast<uintptr_t>(this) + table_offset);
        if (static_cast<ByamlDataType>(key_table->data_type) != ByamlDataType::KeyTable) { return; }

        /* Swap endian */
        key_table->SwapEndian();
        const u32 string_count = (this->IsReverseEndian() == false) ? key_table->count : vp::util::SwapEndian24(key_table->count);
        const u32 index_count = string_count + 1;
        u32 *index_array = reinterpret_cast<u32*>(reinterpret_cast<uintptr_t>(key_table) + sizeof(ResByamlContainer));
        for (u32 i = 0; i < index_count; ++i) {
            index_array[i] = vp::util::SwapEndian(index_array[i]);
        }

        return;
    }

    void ResByaml::SwapByamlEndian(vp::imem::IHeap *heap) {

        /* Initialize big data cache */
        ByamlBigDataCache big_data_cache = {
            .heap = heap
        };

        /* Swap header */
        magic               = vp::util::SwapEndian(magic);
        version             = vp::util::SwapEndian(version);
        key_table_offset    = vp::util::SwapEndian(key_table_offset);
        string_table_offset = vp::util::SwapEndian(string_table_offset);
        data_offset         = vp::util::SwapEndian(data_offset);

        /* Swap table endians */
        const u32 r_key_table_offset = (this->IsReverseEndian() == false) ? key_table_offset : vp::util::SwapEndian(key_table_offset);
        this->SwapKeyTableEndian(r_key_table_offset, std::addressof(big_data_cache));

        const u32 r_string_table_offset = (this->IsReverseEndian() == false) ? string_table_offset : vp::util::SwapEndian(string_table_offset);
        this->SwapStringTableEndian(r_string_table_offset, std::addressof(big_data_cache));

        const u32 r_data_offset = (this->IsReverseEndian() == false) ? data_offset : vp::util::SwapEndian(data_offset);
        this->SwapRootContainerEndian(r_data_offset, std::addressof(big_data_cache));

        /* Free cache */
        big_data_cache.FreeAll();

        return;
    }
}
