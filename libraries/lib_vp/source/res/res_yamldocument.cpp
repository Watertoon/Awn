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
#include <vp.hpp>

#ifdef VP_ENABLE_LIBYAML
namespace vp::res {

    namespace impl {

        Result ParseYamlValueEvent(yaml_parser_t *parser, yaml_event_t *event) {
            const s32 v_result = ::yaml_parser_parse(parser, event);
            RESULT_RETURN_IF(v_result == 0, ResultParserNG);
            RESULT_RETURN_IF(!impl::IsYamlValueEvent(event), ResultStreamNG);
            RESULT_RETURN_SUCCESS;
        }

        Result YamlBinaryData::ParseYaml(vp::imem::IHeap *heap, yaml_parser_t *parser, yaml_event_t *root_event, bool is_alignment) {
            /* Clear data */
            this->data = nullptr;
            this->size = 0;
            this->alignment = 1;
            /* Ensure root is start of sequence */
            RESULT_RETURN_IF(root_event->type != YAML_MAPPING_START_EVENT, ResultStreamNG);
            [[indeterminate]] yaml_event_t event_storage[2];
            yaml_event_t *key_event = std::addressof(event_storage[0]);
            yaml_event_t *value_event = std::addressof(event_storage[1]);
            bool is_parse_align = false;
            while (::yaml_parser_parse(parser,key_event)) {
                switch(key_event->type) {
                    case YAML_MAPPING_END_EVENT:
                    {
                        RESULT_RETURN_SUCCESS;
                    }
                    case YAML_SCALAR_EVENT:
                    {
                        /* Get key */
                        const char *key = reinterpret_cast<const char*>(key_event->data.scalar.value);
                        RESULT_RETURN_IF(key == nullptr, ResultStreamNG);
                        /* Parse value */
                        const s32 v_result = ::yaml_parser_parse(parser, value_event);
                        RESULT_RETURN_IF(v_result != 1, ResultStreamNG);
                        RESULT_RETURN_IF(value_event->type != YAML_SCALAR_EVENT, ResultStreamNG);
                        /* Parse by key */
                        if (is_alignment && vp::util::StringCompare("Alignment", key)) {
                            RESULT_RETURN_IF(value_event->data.scalar.tag == nullptr || !vp::util::StringCompare(value_event->data.scalar.tag, "!u32"), ResultStreamNG);
                            /* Parse u32 */
                            const char *start = reinterpret_cast<const char*>(value_event->data.scalar.value);
                            RESULT_RETURN_IF(start == nullptr || *start == '\0', ResultValueNG);
                            errno = 0;
                            char *end = nullptr;
                            const unsigned long value = std::strtoul(start, std::addressof(end), 0);
                            RESULT_RETURN_IF(errno != 0, ResultValueNG);
                            RESULT_RETURN_IF(start + value_event->data.scalar.length != end, ResultValueNG);
                            this->alignment = static_cast<u32>(value);
                            RESULT_RETURN_IF(!vp::util::IsAlignedPow2(this->alignment), ResultValueNG)
                            is_parse_align = true;
                        } else if (vp::util::StringCompare("Data", key)) {
                            RESULT_RETURN_IF(is_alignment && !is_parse_align, ResultStreamNG);
                            RESULT_RETURN_IF(value_event->data.scalar.tag == nullptr || !vp::util::StringCompare(value_event->data.scalar.tag, "!str"), ResultStreamNG);
                            /* Parse byte string */
                            RESULT_RETURN_IF((value_event->data.scalar.length & 1) != 0, ResultStreamNG);
                            const size_t byte_count = value_event->data.scalar.length / 2;
                            RESULT_RETURN_IF(byte_count > (1ull << BITSIZEOF(u32)), ResultStreamNG);
                            if (byte_count) {
                                void *data = ::operator new(byte_count, heap, this->alignment);
                                RESULT_RETURN_IF(data == nullptr, ResultNoMemory);
                                auto fail_guard = SCOPE_GUARD {
                                    ::operator delete (data);
                                };
                                RESULT_TRY(ParseByteString(std::addressof(this->size), reinterpret_cast<u8*>(data), byte_count, reinterpret_cast<const char*>(value_event->data.scalar.value), value_event->data.scalar.length));
                                fail_guard.Cancel();
                                this->data = data;
                            }
                        } else {
                            RESULT_RETURN(ResultStreamNG);
                        }
                        break;
                    }
                    default:
                        RESULT_RETURN(ResultStreamNG);
                };
            }
            RESULT_RETURN_SUCCESS;
        }

        Result CreateYamlValue(YamlValueBase **out_value, vp::imem::IHeap *heap, YamlDocument *document, yaml_parser_t *parser, yaml_event_t *event) {
            Result last_result = ResultSuccess;
            auto last_error_guard = SCOPE_GUARD {
                document->SetLastError(parser, event, last_result);
            };
            YamlValueBase *value = nullptr;
            switch (event->type) {
                case YAML_MAPPING_START_EVENT:
                {
                    /* Parse mapping data type */
                    const char *tag = reinterpret_cast<const char*>(event->data.mapping_start.tag);
                    RESULT_RETURN_IF(tag == nullptr, last_result = ResultDataTypeNG);
                    const vp::res::ByamlDataType data_type = impl::ParseMappingTagToDataType(tag);
                    RESULT_RETURN_IF(data_type == cInvalidYamlDataType, last_result = ResultDataTypeNG);
                    /* Parse root data type */
                    switch (data_type) {
                        case vp::res::ByamlDataType::Dictionary:
                        case vp::res::ByamlDataType::DictionaryWithRemap:
                        {
                            YamlDictionary *dict = new (heap, alignof(YamlDictionary)) YamlDictionary();
                            RESULT_RETURN_IF(dict == nullptr, ResultNoMemory);
                            dict->SetIsRemap(data_type == vp::res::ByamlDataType::DictionaryWithRemap);
                            last_result = dict->ParseYaml(heap, document, parser, event);
                            if (RESULT_FAILED(last_result)) {
                                last_error_guard.Cancel();
                                RESULT_RETURN(last_result);
                            }
                            value = dict;
                            break;
                        }
                        case vp::res::ByamlDataType::HashArrayU32_1:
                        case vp::res::ByamlDataType::HashArrayU32_2:
                        case vp::res::ByamlDataType::HashArrayU32_3:
                        case vp::res::ByamlDataType::HashArrayU32_4:
                        case vp::res::ByamlDataType::HashArrayU32_5:
                        case vp::res::ByamlDataType::HashArrayU32_6:
                        case vp::res::ByamlDataType::HashArrayU32_7:
                        case vp::res::ByamlDataType::HashArrayU32_8:
                        case vp::res::ByamlDataType::HashArrayU32_9:
                        case vp::res::ByamlDataType::HashArrayU32_10:
                        case vp::res::ByamlDataType::HashArrayU32_11:
                        case vp::res::ByamlDataType::HashArrayU32_12:
                        case vp::res::ByamlDataType::HashArrayU32_13:
                        case vp::res::ByamlDataType::HashArrayU32_14:
                        case vp::res::ByamlDataType::HashArrayU32_15:
                        case vp::res::ByamlDataType::HashArrayU32_16:
                        case vp::res::ByamlDataType::HashArrayWithRemapU32_1:
                        case vp::res::ByamlDataType::HashArrayWithRemapU32_2:
                        case vp::res::ByamlDataType::HashArrayWithRemapU32_3:
                        case vp::res::ByamlDataType::HashArrayWithRemapU32_4:
                        case vp::res::ByamlDataType::HashArrayWithRemapU32_5:
                        case vp::res::ByamlDataType::HashArrayWithRemapU32_6:
                        case vp::res::ByamlDataType::HashArrayWithRemapU32_7:
                        case vp::res::ByamlDataType::HashArrayWithRemapU32_8:
                        case vp::res::ByamlDataType::HashArrayWithRemapU32_9:
                        case vp::res::ByamlDataType::HashArrayWithRemapU32_10:
                        case vp::res::ByamlDataType::HashArrayWithRemapU32_11:
                        case vp::res::ByamlDataType::HashArrayWithRemapU32_12:
                        case vp::res::ByamlDataType::HashArrayWithRemapU32_13:
                        case vp::res::ByamlDataType::HashArrayWithRemapU32_14:
                        case vp::res::ByamlDataType::HashArrayWithRemapU32_15:
                        case vp::res::ByamlDataType::HashArrayWithRemapU32_16:
                        {
                            YamlHashArray *hash_array = new (heap, alignof(YamlHashArray)) YamlHashArray();
                            RESULT_RETURN_IF(hash_array == nullptr, ResultNoMemory);
                            hash_array->SetDataType(data_type);
                            last_result = hash_array->ParseYaml(heap, document, parser, event);
                            if (RESULT_FAILED(last_result)) {
                                last_error_guard.Cancel();
                                RESULT_RETURN(last_result);
                            }
                            value = hash_array;
                            break;
                        }
                        case vp::res::ByamlDataType::BinaryData:
                        {
                            YamlValueBinaryData *bin = new (heap, alignof(YamlValueBinaryData)) YamlValueBinaryData();
                            RESULT_RETURN_IF(bin == nullptr, ResultNoMemory);
                            last_result = bin->ParseYaml(heap, parser, event);
                            RESULT_TRY(last_result);
                            value = bin;
                            break;
                        }
                        case vp::res::ByamlDataType::BinaryDataWithAlignment:
                        {
                            YamlValueBinaryDataWithAlignment *bin = new (heap, alignof(YamlValueBinaryDataWithAlignment)) YamlValueBinaryDataWithAlignment();
                            RESULT_RETURN_IF(bin == nullptr, ResultNoMemory);
                            last_result = bin->ParseYaml(heap, parser, event);
                            RESULT_TRY(last_result);
                            value = bin;
                            break;
                        }
                        default:
                            RESULT_RETURN(last_result = ResultStreamNG);
                    };

                    break;
                }
                case YAML_SEQUENCE_START_EVENT:
                {
                    /* Parse sequence tag */
                    const char *tag = reinterpret_cast<const char*>(event->data.sequence_start.tag);
                    RESULT_RETURN_IF(tag == nullptr, last_result = ResultDataTypeNG);
                    const vp::res::ByamlDataType data_type = impl::ParseSequenceTagToDataType(tag);
                    RESULT_RETURN_IF(data_type == cInvalidYamlDataType, last_result = ResultDataTypeNG);
                    /* Parse array */
                    switch (data_type) {
                        case vp::res::ByamlDataType::Array:
                        case vp::res::ByamlDataType::MonoTypedArray:
                        {
                            YamlArray *array = new (heap, alignof(YamlArray)) YamlArray();
                            RESULT_RETURN_IF(array == nullptr, ResultNoMemory);
                            array->SetIsMonoTyped(data_type == vp::res::ByamlDataType::MonoTypedArray);
                            last_result = array->ParseYaml(heap, document, parser, event);
                            if (RESULT_FAILED(last_result)) {
                                last_error_guard.Cancel();
                                RESULT_RETURN(last_result);
                            }
                            value = array;
                            break;
                        }
                        default:
                            RESULT_RETURN(last_result = ResultStreamNG);
                    };
                    break;
                }
                case YAML_SCALAR_EVENT:
                {
                    /* Parse scalar tag */
                    const char *tag = reinterpret_cast<const char*>(event->data.scalar.tag);
                    RESULT_RETURN_IF(tag == nullptr, last_result = ResultDataTypeNG);
                    const vp::res::ByamlDataType data_type = impl::ParseScalarTagToDataType(tag);
                    /* An empty array or dict may decay to a scalar */
                    if (data_type == cInvalidYamlDataType) {
                        const vp::res::ByamlDataType seq_data_type = impl::ParseSequenceTagToDataType(tag);
                        switch (seq_data_type) {
                            case vp::res::ByamlDataType::Array:
                            case vp::res::ByamlDataType::MonoTypedArray:
                            {
                                YamlArray *array = new (heap, alignof(YamlArray)) YamlArray();
                                RESULT_RETURN_IF(array == nullptr, ResultNoMemory);
                                array->SetIsMonoTyped(data_type == vp::res::ByamlDataType::MonoTypedArray);
                                value = array;
                                break;
                            }
                            default:
                                break;
                        };
                        if (value != nullptr) { break; }
                        const vp::res::ByamlDataType map_data_type = impl::ParseMappingTagToDataType(tag);
                        switch (map_data_type) {
                            case vp::res::ByamlDataType::Dictionary:
                            case vp::res::ByamlDataType::DictionaryWithRemap:
                            {
                                YamlDictionary *dict = new (heap, alignof(YamlDictionary)) YamlDictionary();
                                RESULT_RETURN_IF(dict == nullptr, ResultNoMemory);
                                dict->SetIsRemap(data_type == vp::res::ByamlDataType::DictionaryWithRemap);
                                value = dict;
                                break;
                            }
                            case vp::res::ByamlDataType::HashArrayU32_1:
                            case vp::res::ByamlDataType::HashArrayU32_2:
                            case vp::res::ByamlDataType::HashArrayU32_3:
                            case vp::res::ByamlDataType::HashArrayU32_4:
                            case vp::res::ByamlDataType::HashArrayU32_5:
                            case vp::res::ByamlDataType::HashArrayU32_6:
                            case vp::res::ByamlDataType::HashArrayU32_7:
                            case vp::res::ByamlDataType::HashArrayU32_8:
                            case vp::res::ByamlDataType::HashArrayU32_9:
                            case vp::res::ByamlDataType::HashArrayU32_10:
                            case vp::res::ByamlDataType::HashArrayU32_11:
                            case vp::res::ByamlDataType::HashArrayU32_12:
                            case vp::res::ByamlDataType::HashArrayU32_13:
                            case vp::res::ByamlDataType::HashArrayU32_14:
                            case vp::res::ByamlDataType::HashArrayU32_15:
                            case vp::res::ByamlDataType::HashArrayU32_16:
                            case vp::res::ByamlDataType::HashArrayWithRemapU32_1:
                            case vp::res::ByamlDataType::HashArrayWithRemapU32_2:
                            case vp::res::ByamlDataType::HashArrayWithRemapU32_3:
                            case vp::res::ByamlDataType::HashArrayWithRemapU32_4:
                            case vp::res::ByamlDataType::HashArrayWithRemapU32_5:
                            case vp::res::ByamlDataType::HashArrayWithRemapU32_6:
                            case vp::res::ByamlDataType::HashArrayWithRemapU32_7:
                            case vp::res::ByamlDataType::HashArrayWithRemapU32_8:
                            case vp::res::ByamlDataType::HashArrayWithRemapU32_9:
                            case vp::res::ByamlDataType::HashArrayWithRemapU32_10:
                            case vp::res::ByamlDataType::HashArrayWithRemapU32_11:
                            case vp::res::ByamlDataType::HashArrayWithRemapU32_12:
                            case vp::res::ByamlDataType::HashArrayWithRemapU32_13:
                            case vp::res::ByamlDataType::HashArrayWithRemapU32_14:
                            case vp::res::ByamlDataType::HashArrayWithRemapU32_15:
                            case vp::res::ByamlDataType::HashArrayWithRemapU32_16:
                            {
                                YamlHashArray *hash_array = new (heap, alignof(YamlHashArray)) YamlHashArray();
                                RESULT_RETURN_IF(hash_array == nullptr, ResultNoMemory);
                                hash_array->SetDataType(data_type);
                                value = hash_array;
                                break;
                            }
                            default:
                                break;
                        };
                        RESULT_RETURN_UNLESS(value != nullptr, last_result = ResultDataTypeNG);
                        break;
                    }
                    /* Create root as value */
                    switch (data_type) {
                        case vp::res::ByamlDataType::Null:
                        {
                            YamlValueNull *null = new (heap, alignof(YamlValueNull)) YamlValueNull();
                            RESULT_RETURN_IF(null == nullptr, last_result = ResultNoMemory);
                            value = null;
                            break;
                        }
                        case vp::res::ByamlDataType::Bool:
                        {
                            YamlValueBool *val = new (heap, alignof(YamlValueBool)) YamlValueBool();
                            RESULT_RETURN_IF(val == nullptr, last_result = ResultNoMemory);
                            last_result = val->ParseYaml(heap, parser, event);
                            RESULT_TRY(last_result);
                            value = val;
                            break;
                        }
                        case vp::res::ByamlDataType::S32:
                        {
                            YamlValueS32 *val = new (heap, alignof(YamlValueS32)) YamlValueS32();
                            RESULT_RETURN_IF(val == nullptr, last_result = ResultNoMemory);
                            last_result = val->ParseYaml(heap, parser, event);
                            RESULT_TRY(last_result);
                            value = val;
                            break;
                        }
                        case vp::res::ByamlDataType::U32:
                        {
                            YamlValueU32 *val = new (heap, alignof(YamlValueU32)) YamlValueU32();
                            RESULT_RETURN_IF(val == nullptr, last_result = ResultNoMemory);
                            last_result = val->ParseYaml(heap, parser, event);
                            RESULT_TRY(last_result);
                            value = val;
                            break;
                        }
                        case vp::res::ByamlDataType::S64:
                        {
                            YamlValueS64 *val = new (heap, alignof(YamlValueS64)) YamlValueS64();
                            RESULT_RETURN_IF(val == nullptr, last_result = ResultNoMemory);
                            last_result = val->ParseYaml(heap, parser, event);
                            RESULT_TRY(last_result);
                            value = val;
                            break;
                        }
                        case vp::res::ByamlDataType::U64:
                        {
                            YamlValueU64 *val = new (heap, alignof(YamlValueU64)) YamlValueU64();
                            RESULT_RETURN_IF(val == nullptr, last_result = ResultNoMemory);
                            last_result = val->ParseYaml(heap, parser, event);
                            RESULT_TRY(last_result);
                            value = val;
                            break;
                        }
                        case vp::res::ByamlDataType::F32:
                        {
                            YamlValueFloat *val = new (heap, alignof(YamlValueFloat)) YamlValueFloat();
                            RESULT_RETURN_IF(val == nullptr, last_result = ResultNoMemory);
                            last_result = val->ParseYaml(heap, parser, event);
                            RESULT_TRY(last_result);
                            value = val;
                            break;
                        }
                        case vp::res::ByamlDataType::F64:
                        {
                            YamlValueDouble *val = new (heap, alignof(YamlValueDouble)) YamlValueDouble();
                            RESULT_RETURN_IF(val == nullptr, last_result = ResultNoMemory);
                            last_result = val->ParseYaml(heap, parser, event);
                            RESULT_TRY(last_result);
                            value = val;
                            break;
                        }
                        case vp::res::ByamlDataType::StringIndex:
                        {
                            YamlValueString *val = new (heap, alignof(YamlValueString)) YamlValueString();
                            RESULT_RETURN_IF(val == nullptr, last_result = ResultNoMemory);
                            last_result = val->ParseYaml(heap, parser, event);
                            RESULT_TRY(last_result);
                            value = val;
                            break;
                        }
                        default:
                            RESULT_RETURN(last_result = ResultStreamNG);
                    };
                    break;
                }
                default:
                    break;
            };
            RESULT_RETURN_IF(value == nullptr, last_result = ResultStreamNG);
            *out_value = value;
            last_error_guard.Cancel();
            RESULT_RETURN_SUCCESS;
        }
    }

    Result YamlHashArray::BuildIndex(vp::imem::IHeap *heap) {
        RESULT_RETURN_IF(m_count < 1, ResultSuccess);
        RESULT_TRY(m_array.Initialize(heap, m_count));
        if (this->IsRemap()) {
            for (Node &node : m_list) {
                m_array.PushPointer(std::addressof(node));
            }
        } else {
            for (Node &node : m_dict) {
                m_array.PushPointer(std::addressof(node));
            }
        }
        RESULT_RETURN_SUCCESS;
    }
    Result YamlHashArray::CreateNode(Node **out_node, vp::imem::IHeap *heap, const char *key, size_t len) {
        /* Ensure key */
        RESULT_RETURN_IF(key == nullptr, ResultStreamNG);
        /* Parse key */
        vp::util::HeapArray<u32> key_temp;
        RESULT_TRY(key_temp.Initialize(heap, m_key_size));
        std::memset(key_temp.GetArray(), 0, key_temp.GetCount() * sizeof(u32));
        u32 key_size = 0;
        RESULT_TRY(impl::ParseByteString(std::addressof(key_size), reinterpret_cast<u8*>(key_temp.GetArray()), key_temp.GetCount() * sizeof(u32), key, len));
        RESULT_RETURN_IF(key_size / sizeof(u32) > m_key_size, ResultStreamNG);
        /* Parse key */
        Key temp_key = { std::addressof(key_temp), };
        Node *ref_node = m_dict.Find(temp_key);
        RESULT_RETURN_IF(ref_node != nullptr, ResultKeyCollision);
        /* Create */
        Node *node = new (heap, alignof(Node)) Node();
        RESULT_RETURN_IF(node == nullptr, ResultNoMemory);
        auto fail_guard = SCOPE_GUARD { delete node; };
        node->key_array.SetMemory(key_temp.GetArray(), key_temp.GetCount());
        node->hash_node.SetKey(Key{std::addressof(node->key_array),});
        /* Insert */
        m_dict.Insert(node);
        m_list.PushBack(*node);
        ++m_count;
        *out_node = node;
        fail_guard.Cancel();
        RESULT_RETURN_SUCCESS;
    }

    void YamlHashArray::DeleteNode(Node *node) {
        if (node == nullptr) { return; }
        if (node->hash_node.IsLinked()) {
            m_dict.Remove(node);
        }
        node->remap_node.Unlink();
        node->key_array.Finalize();
        if (node->value != nullptr) {
            node->value->Finalize();
            delete node->value;
        }
        --m_count;
        delete node;
        return;
    }

    void YamlHashArray::Finalize() {
        auto iter = m_list.begin();
        const auto end = m_list.end();
        while (iter != end) {
            Node *node = std::addressof(*iter);
            ++iter;
            this->DeleteNode(node);
        }
        m_array.Finalize();
        return;
    }

    Result YamlHashArray::ParseYaml(vp::imem::IHeap *heap, YamlDocument *document, yaml_parser_t *parser, yaml_event_t *root_event) {
        yaml_event_t event_storage[2] = {};
        yaml_event_t *key_event = std::addressof(event_storage[0]);
        yaml_event_t *value_event = std::addressof(event_storage[1]);
        yaml_event_t *last_event = root_event;
        vp::Result last_result = ResultSuccess;
        auto fail_guard0 = SCOPE_GUARD { document->SetLastError(parser, last_event, last_result); ::yaml_event_delete(key_event); ::yaml_event_delete(value_event); };
        /* Parse key loop */
        RESULT_RETURN_IF(root_event->type != YAML_MAPPING_START_EVENT, last_result = ResultStreamNG);
        while (::yaml_parser_parse(parser, key_event)) {
            if (key_event->type == YAML_MAPPING_END_EVENT) {
                RESULT_TRY(this->BuildIndex(heap));
                fail_guard0.Cancel();
                RESULT_RETURN_SUCCESS;
            }
            last_event = key_event;
            RESULT_RETURN_IF(key_event->type != YAML_SCALAR_EVENT, last_result = ResultStreamNG);
            /* Create dictionary node */
            const char *key = reinterpret_cast<const char*>(key_event->data.scalar.value);
            Node *node = nullptr;
            const vp::Result result1 = this->CreateNode(std::addressof(node), heap, key, key_event->data.scalar.length);
            RESULT_TRY(last_result = result1);
            /* Parse value */
            const vp::Result result0 = impl::ParseYamlValueEvent(parser, value_event);
            RESULT_TRY(last_result = result0);
            auto fail_guard1 = SCOPE_GUARD { this->DeleteNode(node); };
            last_event = value_event;
            const vp::Result result2 = impl::CreateYamlValue(std::addressof(node->value), heap, document, parser, value_event);
            if (RESULT_FAILED(result2)) {
                last_result = result2;
                fail_guard0.Cancel();
                RESULT_RETURN(last_result);
            }
            fail_guard1.Cancel();
            ::yaml_event_delete(value_event);
            ::yaml_event_delete(key_event);
        }
        last_event = root_event;
        RESULT_RETURN(last_result = ResultStreamNG);
    }

    Result YamlArray::BuildIndex(vp::imem::IHeap *heap, vp::res::ByamlDataType mono_type) {
        RESULT_RETURN_IF(m_count < 1, ResultSuccess);
        RESULT_TRY(m_array.Initialize(heap, m_count));
        if (mono_type != impl::cInvalidYamlDataType) {
            switch (mono_type) {
                case vp::res::ByamlDataType::S32:
                {
                    s32 *array = new (heap, alignof(s32)) s32[m_count];
                    RESULT_RETURN_IF(array == nullptr, ResultNoMemory);
                    for (Node &node : m_list) {
                        reinterpret_cast<const YamlValue<vp::res::ByamlDataType::S32>*>(node.value)->GetValue(array);
                        ++array;
                    }
                    m_mono_array_type = mono_type;
                    break;
                }
                case vp::res::ByamlDataType::U32:
                {
                    u32 *array = new (heap, alignof(u32)) u32[m_count];
                    RESULT_RETURN_IF(array == nullptr, ResultNoMemory);
                    for (Node &node : m_list) {
                        reinterpret_cast<const YamlValue<vp::res::ByamlDataType::U32>*>(node.value)->GetValue(array);
                        ++array;
                    }
                    m_mono_array_type = mono_type;
                    break;
                }
                case vp::res::ByamlDataType::F32:
                {
                    float *array = new (heap, alignof(float)) float[m_count];
                    RESULT_RETURN_IF(array == nullptr, ResultNoMemory);
                    for (Node &node : m_list) {
                        reinterpret_cast<const YamlValue<vp::res::ByamlDataType::F32>*>(node.value)->GetValue(array);
                        ++array;
                    }
                    m_mono_array_type = mono_type;
                    break;
                }
                default:
                    break;
            };
        }
        for (Node &node : m_list) {
            m_array.PushPointer(std::addressof(node));
        }
        RESULT_RETURN_SUCCESS;
    }
    Result YamlArray::CreateNode(Node **out_node, vp::imem::IHeap *heap) {
        /* Create */
        Node *node = new (heap, alignof(Node)) Node();
        RESULT_RETURN_IF(node == nullptr, ResultNoMemory);
        /* Insert */
        m_list.PushBack(*node);
        ++m_count;
        *out_node = node;
        RESULT_RETURN_SUCCESS;
    }

    void YamlArray::DeleteNode(Node *node) {
        if (node == nullptr) { return; }
        node->list_node.Unlink();
        if (node->value != nullptr) {
            node->value->Finalize();
            delete node->value;
        }
        --m_count;
        delete node;
        return;
    }

    void YamlArray::Finalize() {
        auto iter = m_list.begin();
        const auto end = m_list.end();
        while (iter != end) {
            Node *node = std::addressof(*iter);
            ++iter;
            this->DeleteNode(node);
        }
        m_array.Finalize();
        switch (m_mono_array_type) {
            case vp::res::ByamlDataType::S32:
            {
                if (m_mono_array != nullptr) {
                    delete [] reinterpret_cast<s32*>(m_mono_array);
                    m_mono_array = nullptr;
                }
                break;
            }
            case vp::res::ByamlDataType::U32:
            {
                if (m_mono_array != nullptr) {
                    delete [] reinterpret_cast<u32*>(m_mono_array);
                    m_mono_array = nullptr;
                }
                break;
            }
            case vp::res::ByamlDataType::F32:
            {
                if (m_mono_array != nullptr) {
                    delete [] reinterpret_cast<float*>(m_mono_array);
                    m_mono_array = nullptr;
                }
                break;
            }
            default:
                break;
        };
        return;
    }

    Result YamlArray::ParseYaml(vp::imem::IHeap *heap, YamlDocument *document, yaml_parser_t *parser, yaml_event_t *root_event) {
        yaml_event_t *last_event = root_event;
        vp::Result last_result = ResultSuccess;
        yaml_event_t event_storage[1] = {};
        yaml_event_t *value_event = std::addressof(event_storage[0]);
        auto fail_guard0 = SCOPE_GUARD { document->SetLastError(parser, last_event, last_result); ::yaml_event_delete(value_event); };
        /* Parse key loop */
        RESULT_RETURN_IF(root_event->type != YAML_SEQUENCE_START_EVENT, last_result = ResultStreamNG);
        vp::res::ByamlDataType mono_type = impl::cInvalidYamlDataType;
        bool is_mono = true;
        while (::yaml_parser_parse(parser, value_event)) {
            last_event = value_event;
            if (value_event->type == YAML_SEQUENCE_END_EVENT) {
                RESULT_TRY(this->BuildIndex(heap, mono_type));
                fail_guard0.Cancel();
                RESULT_RETURN_SUCCESS;
            }
            /* Create array node */
            Node *node = nullptr;
            const vp::Result result0 = this->CreateNode(std::addressof(node), heap);
            RESULT_TRY(last_result = result0);
            auto fail_guard1 = SCOPE_GUARD { this->DeleteNode(node); };
            const vp::Result result1 = impl::CreateYamlValue(std::addressof(node->value), heap, document, parser, value_event);
            RESULT_TRY(last_result = result1);
            if (is_mono) {                
                if (mono_type == impl::cInvalidYamlDataType) {
                    mono_type = node->value->GetDataType();
                } else if (mono_type != node->value->GetDataType()) {
                    mono_type = impl::cInvalidYamlDataType;
                    is_mono = false;
                    RESULT_RETURN_IF(m_is_monotyped, last_result = ResultStreamNG);
                }
            }
            fail_guard1.Cancel();
            ::yaml_event_delete(value_event);
        }
        last_event = root_event;
        RESULT_RETURN(last_result = ResultStreamNG);
    }

    Result YamlDictionary::BuildIndex(vp::imem::IHeap *heap) {
        RESULT_RETURN_IF(m_count < 1, ResultSuccess);
        RESULT_TRY(m_array.Initialize(heap, m_count));
        if (this->IsRemap()) {
            for (Node &node : m_list) {
                m_array.PushPointer(std::addressof(node));
            }
        } else {
            for (Node &node : m_dict) {
                m_array.PushPointer(std::addressof(node));
            }
        }
        RESULT_RETURN_SUCCESS;
    }
    Result YamlDictionary::CreateNode(Node **out_node, vp::imem::IHeap *heap, const char *key, size_t len) {
        /* Ensure key */
        RESULT_RETURN_IF(key == nullptr, ResultStreamNG);
        RESULT_RETURN_IF(len < 1, ResultStreamNG);
        /* Ensure no collision */
        vp::util::HeapString8 key_str;
        RESULT_TRY(key_str.Initialize(heap, len + 1));
        auto fail_guard0 = SCOPE_GUARD { key_str.Finalize(); };
        key_str.SetString(key, len);
        Node *ref_node = m_dict.Find(Key{std::addressof(key_str)});
        RESULT_RETURN_IF(ref_node != nullptr, ResultKeyCollision);
        /* Create */
        Node *node = new (heap, alignof(Node)) Node();
        RESULT_RETURN_IF(node == nullptr, ResultNoMemory);
        fail_guard0.Cancel();
        auto fail_guard1 = SCOPE_GUARD { this->DeleteNode(node); };
        node->key.SetMemoryDirect(key_str.GetString(), key_str.GetMemorySize(), key_str.GetLength());
        node->key_node.SetKey(Key{std::addressof(node->key)});
        /* Insert */
        m_dict.Insert(node);
        m_list.PushBack(*node);
        ++m_count;
        *out_node = node;
        fail_guard1.Cancel();
        RESULT_RETURN_SUCCESS;
    }
    void YamlDictionary::DeleteNode(Node *node) {
        if (node == nullptr) { return; }
        if (node->key_node.IsLinked()) {
            m_dict.Remove(node);
        }
        node->remap_node.Unlink();
        node->key.Finalize();
        if (node->value != nullptr) {
            node->value->Finalize();
            delete node->value;
        }
        --m_count;
        delete node;
        return;
    }

    void YamlDictionary::Finalize() {
        auto iter = m_list.begin();
        const auto end = m_list.end();
        while (iter != end) {
            Node *node = std::addressof(*iter);
            ++iter;
            this->DeleteNode(node);
        }
        m_array.Finalize();
        return;
    }

    Result YamlDictionary::ParseYaml(vp::imem::IHeap *heap, YamlDocument *document, yaml_parser_t *parser, yaml_event_t *root_event) {
        yaml_event_t event_storage[2] = {};
        yaml_event_t *key_event = std::addressof(event_storage[0]);
        yaml_event_t *value_event = std::addressof(event_storage[1]);
        yaml_event_t *last_event = root_event;
        vp::Result last_result = ResultSuccess;
        auto fail_guard0 = SCOPE_GUARD { document->SetLastError(parser, last_event, last_result); ::yaml_event_delete(key_event); ::yaml_event_delete(value_event); };
        /* Parse key loop */
        RESULT_RETURN_IF(root_event->type != YAML_MAPPING_START_EVENT, ResultStreamNG);
        while (::yaml_parser_parse(parser, key_event)) {
            if (key_event->type == YAML_MAPPING_END_EVENT) {
                RESULT_TRY(this->BuildIndex(heap));
                fail_guard0.Cancel();
                RESULT_RETURN_SUCCESS;
            }
            last_event = key_event;
            RESULT_RETURN_IF(key_event->type != YAML_SCALAR_EVENT, last_result = ResultStreamNG);
            /* Create dictionary node */
            const char *key = reinterpret_cast<const char*>(key_event->data.scalar.value);
            Node *node = nullptr;
            const vp::Result result1 = this->CreateNode(std::addressof(node), heap, key, key_event->data.scalar.length);
            RESULT_TRY(last_result = result1);
            /* Parse value */
            const vp::Result result0 = impl::ParseYamlValueEvent(parser, value_event);
            RESULT_TRY(last_result = result0);
            last_event = value_event;
            auto fail_guard1 = SCOPE_GUARD { this->DeleteNode(node); };
            const vp::Result result2 = impl::CreateYamlValue(std::addressof(node->value), heap, document, parser, value_event);
            if (RESULT_FAILED(result2)) {
                last_result = result2;
                fail_guard0.Cancel();
                RESULT_RETURN(last_result);
            }
            fail_guard1.Cancel();
            ::yaml_event_delete(value_event);
            ::yaml_event_delete(key_event);
        }
        last_event = root_event;
        RESULT_RETURN(last_result = ResultStreamNG);
    }

    void YamlDocument::SetLastError(yaml_parser_t *parser, yaml_event_t *event, Result result) {
        VP_UNUSED(parser, result);
        m_error.Format("(Line %lld, Col: %lld) Error occurred\n", event->start_mark.line+1, event->start_mark.column+1);
    }

    void YamlDocument::Finalize() {
        if (m_root != nullptr) {
            m_root->Finalize();
            delete m_root;
            m_root = nullptr;
            m_root_type = impl::cInvalidYamlDataType;
        }
    }

    Result YamlDocument::ParseYaml(vp::imem::IHeap *heap, const char *yaml, size_t length) {

        /* Setup parser */
        [[indeterminate]] yaml_parser_t parser_storage;
        yaml_parser_t *parser = std::addressof(parser_storage);
        const s32 is_init = ::yaml_parser_initialize(parser);
        RESULT_RETURN_IF(is_init == 0, ResultParserNG);
        ON_SCOPE_EXIT { ::yaml_parser_delete(parser); };
        ::yaml_parser_set_input_string(parser, reinterpret_cast<const u8*>(yaml), length);
        ::yaml_parser_set_encoding(parser, YAML_UTF8_ENCODING);
        /* Parse yaml */
        [[indeterminate]] yaml_event_t event_storage;
        yaml_event_t *event = std::addressof(event_storage);
        auto fail_guard = SCOPE_GUARD { this->Finalize(); };
        Result last_result = ResultSuccess;
        auto set_last_error_guard = SCOPE_GUARD { this->SetLastError(parser, event, last_result); ::yaml_event_delete(event); };
        bool is_in_document = false;
        while (::yaml_parser_parse(parser,event)) {
            if (event->type == YAML_DOCUMENT_END_EVENT) { break; }
            if (event->type == YAML_STREAM_END_EVENT) { break; }
            switch (event->type) {
                case YAML_STREAM_START_EVENT:
                {
                    /* Ensure we are utf 8 */
                    RESULT_RETURN_IF(event->data.stream_start.encoding != YAML_UTF8_ENCODING, last_result = ResultEncodingNG);
                    break;
                }
                case YAML_DOCUMENT_START_EVENT:
                {
                    /* Require explicit start */
                    RESULT_RETURN_IF(event->data.document_start.implicit, last_result = ResultStreamNG);
                    is_in_document = true;
                    break;
                }
                case YAML_MAPPING_START_EVENT:
                case YAML_SEQUENCE_START_EVENT:
                case YAML_SCALAR_EVENT:
                {
                    RESULT_RETURN_UNLESS(is_in_document, last_result = ResultStreamNG);
                    RESULT_RETURN_UNLESS(m_root_type == impl::cInvalidYamlDataType, last_result = ResultStreamNG);
                    /* Parse root data type */
                    YamlValueBase *value = nullptr;
                    const vp::Result result0 = impl::CreateYamlValue(std::addressof(value), heap, this, parser, event);
                    if (RESULT_FAILED(result0)) {
                        set_last_error_guard.Cancel();
                        RESULT_RETURN(result0);
                    }
                    m_root_type = value->GetDataType();
                    m_root = value;
                    break;
                }
                default:
                    break;
            };
            ::yaml_event_delete(event);
        }
        ::yaml_event_delete(event);
        set_last_error_guard.Cancel();
        fail_guard.Cancel();
        RESULT_RETURN_SUCCESS;
    }
}
#endif
