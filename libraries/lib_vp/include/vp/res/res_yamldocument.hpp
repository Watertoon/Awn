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

namespace vp::res {

    class YamlValueBase;
    class YamlDocument;

    namespace impl {

        static constexpr vp::res::ByamlDataType cInvalidYamlDataType = static_cast<vp::res::ByamlDataType>(0);

        constexpr bool IsYamlValueEvent(yaml_event_t *event) {
            const bool is_value = (event->type == YAML_MAPPING_START_EVENT) || (event->type == YAML_SCALAR_EVENT) || (event->type == YAML_SEQUENCE_START_EVENT);
            return is_value;
        }
        Result ParseYamlValueEvent(yaml_parser_t *parser, yaml_event_t *event);

        Result CreateYamlValue(YamlValueBase **out_value, vp::imem::IHeap *heap, YamlDocument *document, yaml_parser_t *parser, yaml_event_t *event);

        struct YamlBinaryData {
            void *data;
            u32 size;
            u32 alignment;

            void Finalize() {
                if (this->data != nullptr) {
                    ::operator delete(this->data);
                    this->size = 0;
                    this->alignment = 0;
                }
            }

            Result ParseYaml(vp::imem::IHeap *heap, yaml_parser_t *parser, yaml_event_t *root_event, bool is_alignment);
        };
        static constexpr std::meta::info cYamlA0DataTypeArray[] = {
            ^^vp::util::HeapString8,
            ^^YamlBinaryData,
            ^^YamlBinaryData,
        };
        static constexpr std::meta::info cYamlD0DataTypeArray[] = {
            ^^bool,
            ^^s32,
            ^^float,
            ^^u32,
            ^^s64,
            ^^u64,
            ^^double,
        };

        consteval std::meta::info GetYamlValueMetaInfo(vp::res::ByamlDataType data_type) {
            u8 type = static_cast<u8>(data_type);
            if ((type & 0xf0) == 0xa0) {
                //static_assert((type & 0xf) < 3);
                return cYamlA0DataTypeArray[type & 3];
            } else if ((type & 0xf0) == 0xd0) {
                //static_assert((type & 0xf) < 7);
                return cYamlD0DataTypeArray[(type & 7)];
            }
            return ^^u32;
        }
    
        constexpr Result ParseByteString(u32 *out_real_size, u8 *data, size_t max_data, const char *str, size_t str_len) {
            /* Ignore initial whitespace */
            size_t i = 0;
            for (; i < str_len; ++i) {
                const u8 c = static_cast<u8>(str[i]);
                if (std::isspace(c)) { continue; }
                break;
            }
            /* Accept no data */
            if (str_len - i < 2) {
                RESULT_RETURN_IF(str_len - i != 0, ResultStreamNG);
                *out_real_size = 0;
                RESULT_RETURN_SUCCESS;
            }
            /* Remove optional 0x */
            if (str[i] == '0' && (str[i+1] == 'x' || str[i+1] == 'X')) {
                i += 2;
            }
            /* Clear mem */
            std::memset(data, 0, str_len / 2);
            /* Parse hex from reverse start */
            const char *r_end = std::addressof(str[i]);
            size_t b_i = 0;
            for (const char *r_i = std::addressof(str[str_len-1]); r_end <= r_i; --r_i) {
                RESULT_RETURN_IF(b_i/2 == max_data, ResultStreamNG);
                /* Convert to 4-bits */
                const u8 c = static_cast<u8>(*r_i);
                const u8 num = c - static_cast<u8>('0');
                const u8 lh = c - static_cast<u8>('a');
                const u8 uh = c - static_cast<u8>('A');
                const bool is_num = num <= 9;
                const bool is_lh = lh <= 5;
                const bool is_uh = uh <= 5;
                const u8 shift = (b_i & 1) ? 4 : 0;
                u8 value;
                if (is_num) {
                    value = num;
                } else if (is_lh || is_uh) {
                    value = lh + 10;
                } else {
                    RESULT_RETURN(ResultStreamNG);
                }
                /* Combine */
                data[b_i/2] |= (value << shift);
                ++b_i;
            }
            *out_real_size = static_cast<u32>((b_i + (b_i & 1)) / 2);
            RESULT_RETURN_SUCCESS;
        }

        constexpr vp::res::ByamlDataType ParseScalarTagToDataType(const char *tag) {
            static constexpr const char *cTagArray[] = {
                "!s32",
                "!float",
                "!u32",
                "!bool",
                "!s64",
                "!u64",
                "!double",
                "!str",
                "!null",
            };
            static constexpr vp::res::ByamlDataType cDataTypeArray[] = {
                vp::res::ByamlDataType::S32,
                vp::res::ByamlDataType::F32,
                vp::res::ByamlDataType::U32,
                vp::res::ByamlDataType::Bool,
                vp::res::ByamlDataType::S64,
                vp::res::ByamlDataType::U64,
                vp::res::ByamlDataType::F64,
                vp::res::ByamlDataType::StringIndex,
                vp::res::ByamlDataType::Null,
            };
            static_assert(sizeof(cTagArray) / sizeof(const char*) == sizeof(cDataTypeArray) / sizeof(vp::res::ByamlDataType));
            for (u32 i = 0; i < sizeof(cTagArray) / sizeof(const char*); ++i) {
                if (vp::util::StringCompare(tag,cTagArray[i])) { return cDataTypeArray[i]; }
            }
            return cInvalidYamlDataType;
        }
        constexpr vp::res::ByamlDataType ParseMappingTagToDataType(const char *tag) {
            static constexpr const char *cTagArray[] = {
                "!dict",
                "!bin",
                "!binalign",
                "!hash_1",
                "!hash_2",
                "!dictremap",
                "!hashremap_1",
                "!hashremap_2",
                "!hash_3",
                "!hash_4",
                "!hash_5",
                "!hash_6",
                "!hash_7",
                "!hash_8",
                "!hash_9",
                "!hash_10",
                "!hash_11",
                "!hash_12",
                "!hash_13",
                "!hash_14",
                "!hash_15",
                "!hash_16",
                "!hashremap_3",
                "!hashremap_4",
                "!hashremap_5",
                "!hashremap_6",
                "!hashremap_7",
                "!hashremap_8",
                "!hashremap_9",
                "!hashremap_10",
                "!hashremap_11",
                "!hashremap_12",
                "!hashremap_13",
                "!hashremap_14",
                "!hashremap_15",
                "!hashremap_16",
            };
            static constexpr vp::res::ByamlDataType cDataTypeArray[] = {
                vp::res::ByamlDataType::Dictionary,
                vp::res::ByamlDataType::BinaryData,
                vp::res::ByamlDataType::BinaryDataWithAlignment,
                vp::res::ByamlDataType::HashArrayU32_1,
                vp::res::ByamlDataType::HashArrayU32_2,
                vp::res::ByamlDataType::DictionaryWithRemap,
                vp::res::ByamlDataType::HashArrayWithRemapU32_1,
                vp::res::ByamlDataType::HashArrayWithRemapU32_2,
                vp::res::ByamlDataType::HashArrayU32_3,
                vp::res::ByamlDataType::HashArrayU32_4,
                vp::res::ByamlDataType::HashArrayU32_5,
                vp::res::ByamlDataType::HashArrayU32_6,
                vp::res::ByamlDataType::HashArrayU32_7,
                vp::res::ByamlDataType::HashArrayU32_8,
                vp::res::ByamlDataType::HashArrayU32_9,
                vp::res::ByamlDataType::HashArrayU32_10,
                vp::res::ByamlDataType::HashArrayU32_11,
                vp::res::ByamlDataType::HashArrayU32_12,
                vp::res::ByamlDataType::HashArrayU32_13,
                vp::res::ByamlDataType::HashArrayU32_14,
                vp::res::ByamlDataType::HashArrayU32_15,
                vp::res::ByamlDataType::HashArrayU32_16,
                vp::res::ByamlDataType::HashArrayWithRemapU32_3,
                vp::res::ByamlDataType::HashArrayWithRemapU32_4,
                vp::res::ByamlDataType::HashArrayWithRemapU32_5,
                vp::res::ByamlDataType::HashArrayWithRemapU32_6,
                vp::res::ByamlDataType::HashArrayWithRemapU32_7,
                vp::res::ByamlDataType::HashArrayWithRemapU32_8,
                vp::res::ByamlDataType::HashArrayWithRemapU32_9,
                vp::res::ByamlDataType::HashArrayWithRemapU32_10,
                vp::res::ByamlDataType::HashArrayWithRemapU32_11,
                vp::res::ByamlDataType::HashArrayWithRemapU32_12,
                vp::res::ByamlDataType::HashArrayWithRemapU32_13,
                vp::res::ByamlDataType::HashArrayWithRemapU32_14,
                vp::res::ByamlDataType::HashArrayWithRemapU32_15,
                vp::res::ByamlDataType::HashArrayWithRemapU32_16,
            };
            static_assert(sizeof(cTagArray) / sizeof(const char*) == sizeof(cDataTypeArray) / sizeof(vp::res::ByamlDataType));
            for (u32 i = 0; i < sizeof(cTagArray) / sizeof(const char*); ++i) {
                if (vp::util::StringCompare(tag,cTagArray[i])) { return cDataTypeArray[i]; }
            }
            return cInvalidYamlDataType;
        }
        constexpr vp::res::ByamlDataType ParseSequenceTagToDataType(const char *tag) {
            static constexpr const char *cTagArray[] = {
                "!array",
                "!arraymono",
            };
            static constexpr vp::res::ByamlDataType cDataTypeArray[] = {
                vp::res::ByamlDataType::Array,
                vp::res::ByamlDataType::MonoTypedArray,
            };
            static_assert(sizeof(cTagArray) / sizeof(const char*) == sizeof(cDataTypeArray) / sizeof(vp::res::ByamlDataType));
            for (u32 i = 0; i < sizeof(cTagArray) / sizeof(const char*); ++i) {
                if (vp::util::StringCompare(tag,cTagArray[i])) { return cDataTypeArray[i]; }
            }
            return cInvalidYamlDataType;
        }
    }

    class YamlValueBase {
        public:
            VP_RTTI_BASE(YamlValueBase);
        public:
            constexpr YamlValueBase() {/*...*/}
            virtual constexpr ~YamlValueBase() {/*...*/}

            virtual void Finalize() {/*...*/}

            virtual vp::res::ByamlDataType GetDataType() const { return vp::res::ByamlDataType::Null; }
    };

    template <vp::res::ByamlDataType DataType>
    class YamlValue : public YamlValueBase {
        public:
            static consteval std::meta::info GetValueMetaInfo() {
                return impl::GetYamlValueMetaInfo(DataType);
            }
        public:
            using ValueType = [:GetValueMetaInfo():];
        private:
            ValueType m_value;
        public:
            VP_RTTI_DERIVED(YamlValue, YamlValueBase);
        public:
            constexpr YamlValue() : m_value() {/*...*/}
            virtual ~YamlValue() override {/*...*/}

            virtual void Finalize() override {
                if constexpr (DataType == vp::res::ByamlDataType::BinaryData || DataType == vp::res::ByamlDataType::BinaryDataWithAlignment) {
                    m_value.Finalize();
                } else if constexpr (DataType == vp::res::ByamlDataType::StringIndex) {
                    m_value.Finalize();
                }
            }

            Result ParseYaml(vp::imem::IHeap *heap, yaml_parser_t *parser, yaml_event_t *event) {
                if constexpr (DataType == vp::res::ByamlDataType::Null) {
                    /* Ignore value */
                    RESULT_RETURN_SUCCESS;
                } else {
                    const char *start = reinterpret_cast<const char*>(event->data.scalar.value);
                    if constexpr (DataType == vp::res::ByamlDataType::S32) {
                        RESULT_RETURN_IF(start == nullptr || *start == '\0', ResultValueNG);
                        /* Parse s32 */
                        errno = 0;
                        char *end = nullptr;
                        const long value = std::strtol(start, std::addressof(end), 0);
                        RESULT_RETURN_IF(errno != 0, ResultValueNG);
                        RESULT_RETURN_IF(start + event->data.scalar.length != end, ResultValueNG);
                        m_value = static_cast<s32>(value);
                    } else if constexpr (DataType == vp::res::ByamlDataType::U32) {
                        RESULT_RETURN_IF(start == nullptr || *start == '\0', ResultValueNG);
                        /* Parse u32 */
                        errno = 0;
                        char *end = nullptr;
                        const unsigned long value = std::strtoul(start, std::addressof(end), 0);
                        RESULT_RETURN_IF(errno != 0, ResultValueNG);
                        RESULT_RETURN_IF(start + event->data.scalar.length != end, ResultValueNG);
                        m_value = static_cast<u32>(value);
                    } else if constexpr (DataType == vp::res::ByamlDataType::S64) {
                        RESULT_RETURN_IF(start == nullptr || *start == '\0', ResultValueNG);
                        /* Parse s64 */
                        errno = 0;
                        char *end = nullptr;
                        const long long value = std::strtoll(start, std::addressof(end), 0);
                        RESULT_RETURN_IF(errno != 0, ResultValueNG);
                        RESULT_RETURN_IF(start + event->data.scalar.length != end, ResultValueNG);
                        m_value = static_cast<s64>(value);
                    } else if constexpr (DataType == vp::res::ByamlDataType::U64) {
                        RESULT_RETURN_IF(start == nullptr || *start == '\0', ResultValueNG);
                        /* Parse u64 */
                        errno = 0;
                        char *end = nullptr;
                        const unsigned long long value = std::strtoull(start, std::addressof(end), 0);
                        RESULT_RETURN_IF(errno != 0, ResultValueNG);
                        RESULT_RETURN_IF(start + event->data.scalar.length != end, ResultValueNG);
                        m_value = static_cast<u64>(value);
                    } else if constexpr (DataType == vp::res::ByamlDataType::F32) {
                        RESULT_RETURN_IF(start == nullptr || *start == '\0', ResultValueNG);
                        /* Parse float */
                        errno = 0;
                        char *end = nullptr;
                        const float value = std::strtof(start, std::addressof(end));
                        RESULT_RETURN_IF(errno != 0, ResultValueNG);
                        RESULT_RETURN_IF(start + event->data.scalar.length != end, ResultValueNG);
                        m_value = value;
                    } else if constexpr (DataType == vp::res::ByamlDataType::F64) {
                        RESULT_RETURN_IF(start == nullptr || *start == '\0', ResultValueNG);
                        /* Parse double */
                        errno = 0;
                        char *end = nullptr;
                        const double value = std::strtod(start, std::addressof(end));
                        RESULT_RETURN_IF(errno != 0, ResultValueNG);
                        RESULT_RETURN_IF(start + event->data.scalar.length != end, ResultValueNG);
                        m_value = value;
                    } else if constexpr (DataType == vp::res::ByamlDataType::Bool) {
                        RESULT_RETURN_IF(start == nullptr || *start == '\0', ResultValueNG);
                        RESULT_RETURN_IF(event->data.scalar.length != 4 && event->data.scalar.length != 5, ResultValueNG);
                        /* Parse bool */
                        const bool is_true = (std::memcmp(event->data.scalar.value, "true", 4) == 0);
                        if (!is_true) {
                            RESULT_RETURN_IF(std::memcmp(event->data.scalar.value, "false", 5) != 0, ResultValueNG);
                        }
                        m_value = is_true;
                    } else if constexpr (DataType == vp::res::ByamlDataType::StringIndex) {
                        RESULT_TRY(m_value.Initialize(heap, event->data.scalar.length + 1));
                        if (start != nullptr) {                            
                            m_value.SetString(start, event->data.scalar.length);
                        }
                    } else if constexpr (DataType == vp::res::ByamlDataType::BinaryData || DataType == vp::res::ByamlDataType::BinaryDataWithAlignment) {
                        RESULT_TRY(m_value.ParseYaml(heap, parser, event, DataType == vp::res::ByamlDataType::BinaryDataWithAlignment));
                    }
                    RESULT_RETURN_SUCCESS;
                }
            }
        public:
            //virtual bool IsBigData() const { return vp::res::IsBigDataType(DataType); }
            virtual vp::res::ByamlDataType GetDataType() const override { return DataType; }
            
            void GetValue(ValueType *data) const {
                *data = m_value;
            }
    };

    using YamlValueNull = YamlValue<vp::res::ByamlDataType::Null>;
    using YamlValueBool = YamlValue<vp::res::ByamlDataType::Bool>;
    using YamlValueS32 = YamlValue<vp::res::ByamlDataType::S32>;
    using YamlValueFloat = YamlValue<vp::res::ByamlDataType::F32>;
    using YamlValueU32 = YamlValue<vp::res::ByamlDataType::U32>;
    using YamlValueS64 = YamlValue<vp::res::ByamlDataType::S64>;
    using YamlValueU64 = YamlValue<vp::res::ByamlDataType::U64>;
    using YamlValueDouble = YamlValue<vp::res::ByamlDataType::F64>;
    using YamlValueBinaryData = YamlValue<vp::res::ByamlDataType::BinaryData>;
    using YamlValueBinaryDataWithAlignment = YamlValue<vp::res::ByamlDataType::BinaryDataWithAlignment>;
    using YamlValueString = YamlValue<vp::res::ByamlDataType::StringIndex>;

    class YamlHashArray;
    class YamlDictionary;
    class YamlArray;

    class YamlContainerBase : public YamlValueBase {
        private:
        public:
            VP_RTTI_DERIVED(YamlValue, YamlValueBase);
        public:
            constexpr YamlContainerBase() : YamlValueBase() {/*...*/}
            constexpr ~YamlContainerBase() {/*...*/}

            virtual u32 GetDataCount() const { return 0; }
            virtual bool GetValueByIndex(YamlValueBase **out_value, u32 index) {
                VP_UNUSED(out_value, index);
                return false;
            }
    };

    class YamlHashArray : public YamlContainerBase {
        public:
            struct Key {
                vp::util::HeapArray<u32> *key_array;

                std::strong_ordering operator<=>(const Key &rhs) const {
                    VP_ASSERT(this->key_array->GetCount() == rhs.key_array->GetCount());
                    for (u32 i = this->key_array->GetCount()-1; i < this->key_array->GetCount(); --i) {
                        if ((*this->key_array)[i] < (*rhs.key_array)[i]) { return std::strong_ordering::less; }
                        if ((*this->key_array)[i] > (*rhs.key_array)[i]) { return std::strong_ordering::greater; }
                    }
                    return std::strong_ordering::equal;
                }
            };
            struct Node {                
                vp::util::HeapArray<u32> key_array;
                vp::util::IntrusiveRedBlackTreeNode<Key> hash_node;
                vp::util::IntrusiveListNode remap_node;
                YamlValueBase *value;
            };
        private:
            using Dict = vp::util::IntrusiveRedBlackTreeTraits<Node, &Node::hash_node>::Tree;
            using RemapList = vp::util::IntrusiveListTraits<Node, &Node::remap_node>::List;
            using Array = vp::util::PointerArray<Node>;
        private:
            Dict m_dict;
            RemapList m_list;
            Array m_array;
            u32 m_count;
            u8 m_key_size;
            bool m_is_remap;
        public:
            VP_RTTI_DERIVED(YamlHashArray, YamlContainerBase);
        private:
            Result CreateNode(Node **out_node, vp::imem::IHeap *heap, const char *key, size_t len);
            void DeleteNode(Node *node);
            Result BuildIndex(vp::imem::IHeap *heap);
        public:
            constexpr YamlHashArray() : YamlContainerBase(), m_dict(), m_list(), m_array(), m_count(), m_key_size(1), m_is_remap() {/*...*/}
            constexpr ~YamlHashArray() {/*...*/}

            constexpr void SetDataType(vp::res::ByamlDataType data_type) {
                VP_ASSERT(vp::res::IsHashArrayType(data_type));
                const u32 type = static_cast<u32>(data_type);
                m_key_size = (type & 0xf) + 1;
                m_is_remap = (type & 0x30) == 0x30;
            }

            virtual void Finalize() override;
            Result ParseYaml(vp::imem::IHeap *heap, YamlDocument *document, yaml_parser_t *parser, yaml_event_t *root_event);
        public:
            virtual vp::res::ByamlDataType GetDataType() const override { return (!m_is_remap) ? static_cast<vp::res::ByamlDataType>(0x20 | (m_key_size-1)) : static_cast<vp::res::ByamlDataType>(0x30 | (m_key_size-1)); }
            virtual u32 GetDataCount() const override { return m_count; }
            constexpr u32 GetKeySize() {
                return m_key_size;
            }
            constexpr bool IsRemap() const { return m_is_remap; }

            bool GetNodeByIndex(Node **out_value, u32 index) {
                if (m_count <= index) { return false; }
                *out_value = m_array[index];
                return true;
            }
            bool GetNodeByHash(Node **out_value, const u32 *hash) {
                vp::util::HeapArray<u32> h_array;
                h_array.SetMemory(const_cast<u32*>(hash), m_key_size);
                Node *node = m_dict.Find(Key{std::addressof(h_array),});
                if (node == nullptr) { return false; }
                *out_value = node;
                return true;
            }
            virtual bool GetValueByIndex(YamlValueBase **out_value, u32 index) override {
                Node *node;
                if (!this->GetNodeByIndex(std::addressof(node), index)) { return false; }
                *out_value = node->value;
                return true;
            }
            bool GetValueByHash(YamlValueBase **out_value, const u32 *hash) {
                Node *node;
                if (!this->GetNodeByHash(std::addressof(node), hash)) { return false; }
                *out_value = node->value;
                return true;
            }
            bool GetHashByIndex(u32 *out_hash, u32 index) {
                Node *node;
                if (!this->GetNodeByIndex(std::addressof(node), index)) { return false; }
                std::memcpy(out_hash, node->key_array.GetArray(), node->key_array.GetCount() * sizeof(u32));
                return true;
            }
    };

    class YamlArray : public YamlContainerBase {
        public:
            struct Node {
                vp::util::IntrusiveListNode list_node;
                YamlValueBase *value;
            };
        public:
            using ArrayList = vp::util::IntrusiveListTraits<Node, &Node::list_node>::List;
        public:
            enum class MonoArrayType {
                None,
                S32,
                Float,
                U32,
            };
            using Array = vp::util::PointerArray<Node>;
        private:
            ArrayList m_list;
            Array m_array;
            void *m_mono_array;
            u32 m_count;
            vp::res::ByamlDataType m_mono_array_type;
            bool m_is_monotyped;
        public:
            VP_RTTI_DERIVED(YamlArray, YamlContainerBase);
        private:
            Result CreateNode(Node **out_node, vp::imem::IHeap *heap);
            void DeleteNode(Node *node);
            Result BuildIndex(vp::imem::IHeap *heap, vp::res::ByamlDataType mono_data_type);
        public:
            constexpr YamlArray() : YamlContainerBase(), m_list(), m_array(), m_mono_array(), m_count(), m_mono_array_type(impl::cInvalidYamlDataType), m_is_monotyped() {/*...*/}
            constexpr ~YamlArray() {/*...*/}

            constexpr void SetIsMonoTyped(bool is_monotype) { m_is_monotyped = is_monotype; }

            virtual void Finalize() override;
            Result ParseYaml(vp::imem::IHeap *heap, YamlDocument *document, yaml_parser_t *parser, yaml_event_t *root_event);
        public:
            virtual vp::res::ByamlDataType GetDataType() const override { return (!m_is_monotyped) ? vp::res::ByamlDataType::Array : vp::res::ByamlDataType::MonoTypedArray; }
            virtual u32 GetDataCount() const override { return m_array.GetUsedCount(); }
            constexpr bool IsMonoTyped() const { return m_is_monotyped; }
            constexpr vp::res::ByamlDataType GetMonoArrayType() const {
                return m_mono_array_type;
            }
            constexpr const void *GetMonoValueArray() const {
                return m_mono_array;
            }

            constexpr bool GetNodeByIndex(Node **out_value, u32 index) {
                if (m_count <= index) { return false; }
                *out_value = m_array[index];
                return true;
            }
            virtual bool GetValueByIndex(YamlValueBase **out_value, u32 index) override {
                Node *node;
                if (!this->GetNodeByIndex(std::addressof(node), index)) { return false; }
                *out_value = node->value;
                return true;
            }
    };
    class YamlDictionary : public YamlContainerBase {
        public:
            struct Key {
                vp::util::HeapString8 *key;

                std::strong_ordering operator<=>(const Key &rhs) const {
                    const s32 result = *this->key <=> *rhs.key;
                    std::strong_ordering strong_order0 = (result < 0) ? std::strong_ordering::less : std::strong_ordering::equal;
                    std::strong_ordering strong_order1 = (result > 0) ? std::strong_ordering::greater : strong_order0;
                    return strong_order1;
                }
            };
            struct Node {
                vp::util::HeapString8 key;
                vp::util::IntrusiveRedBlackTreeNode<Key> key_node;
                vp::util::IntrusiveListNode remap_node;
                YamlValueBase *value;
            };
        public:
            using Dict = vp::util::IntrusiveRedBlackTreeTraits<Node, &Node::key_node>::Tree;
            using RemapList = vp::util::IntrusiveListTraits<Node, &Node::remap_node>::List;
            using Array = vp::util::PointerArray<Node>;
        private:
            Dict m_dict;
            RemapList m_list;
            Array m_array;
            u32 m_count;
            bool m_is_remap;
        public:
            VP_RTTI_DERIVED(YamlDictionary, YamlContainerBase);
        private:
            Result CreateNode(Node **out_node, vp::imem::IHeap *heap, const char *key, size_t len);
            void DeleteNode(Node *node);
            Result BuildIndex(vp::imem::IHeap *heap);
        public:
            constexpr YamlDictionary() : m_dict(), m_list(), m_array(), m_count(), m_is_remap() {/*...*/}
            virtual ~YamlDictionary() {/*...*/}

            constexpr void SetIsRemap(bool is_remap) { m_is_remap = is_remap; }

            virtual void Finalize() override;
            Result ParseYaml(vp::imem::IHeap *heap, YamlDocument *document, yaml_parser_t *parser, yaml_event_t *root_event);
        public:
            virtual vp::res::ByamlDataType GetDataType() const override { return (!m_is_remap) ? vp::res::ByamlDataType::Dictionary : vp::res::ByamlDataType::DictionaryWithRemap; }
            virtual u32 GetDataCount() const override { return m_count; }
            constexpr bool IsRemap() const { return m_is_remap; }

            bool GetNodeByIndex(Node **out_value, u32 index) {
                if (m_count <= index) { return false; }
                *out_value = m_array[index];
                return true;
            }
            bool GetNodeByKey(Node **out_value, const char *key) {
                const u32 len = vp::util::StringLength(key);
                vp::util::HeapString8 str;
                str.SetMemoryDirect(const_cast<char*>(key), len+1, len);
                Node *node = m_dict.Find(Key{std::addressof(str)});
                if (node == nullptr) { return false; }
                *out_value = node;
                return true;
            }
            virtual bool GetValueByIndex(YamlValueBase **out_value, u32 index) override {
                Node *node;
                if (!this->GetNodeByIndex(std::addressof(node), index)) { return false; }
                *out_value = node->value;
                return true;
            }
            bool GetValueByKey(YamlValueBase **out_value, const char *key) {
                Node *node;
                if (!this->GetNodeByKey(std::addressof(node), key)) { return false; }
                *out_value = node->value;
                return true;
            }
            bool GetKeyByIndex(const char **out_string, u32 index) {
                Node *node;
                if (!this->GetNodeByIndex(std::addressof(node), index)) { return false; }
                *out_string = node->key.GetString();
                return true;
            }

            constexpr bool RemapIndexByKey(u32 *out_id, const char *key) {
                u32 i = 0;
                for (Node *&node : m_array) {
                    ON_SCOPE_EXIT {++i;};
                    if (vp::util::StringCompare(node->key.GetString(), key)) {
                        *out_id = i;
                        return true;
                    }
                }
                return false;
            }
    };

    class YamlDocument {
        private:
            vp::res::ByamlDataType m_root_type;
            YamlValueBase *m_root;
            vp::util::FixedString8<0x100> m_error;
        public:
            void SetLastError(yaml_parser_t *parser, yaml_event_t *event, Result result);
        public:
            constexpr YamlDocument() : m_root_type(impl::cInvalidYamlDataType), m_root(), m_error() {/*...*/}
            constexpr ~YamlDocument() {/*...*/}

            void Finalize();
            Result ParseYaml(vp::imem::IHeap *heap, const char *yaml, size_t length);

            constexpr const char *GetLastError() const { return m_error.GetString(); }
            constexpr YamlValueBase *GetRoot() { return m_root; }
    };
}
