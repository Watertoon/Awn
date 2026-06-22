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

    class YamlEmitter {
        public:
            struct Mapping {
                vp::res::ByamlDataType data_type;
                vp::res::ByamlDataType mono_data_type;
                s32 mapping_id;
            };
            static constexpr const char *cHashArrayTags[0x10] = {
                "!hash_1",
                "!hash_2",
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
            };
            static constexpr const char *cHashArrayRemapTags[0x10] = {
                "!hashremap_1",
                "!hashremap_2",
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
        public:
            using SerializeFunction = vp::util::IFunction<Result(const u8*,size_t)>;
            using MappingFrame = vp::util::FixedFrameArray<Mapping,0x40>;
        private:
            yaml_document_t m_document;
            MappingFrame m_mapping_frame;
            bool m_root_guard;
        public:
            constexpr YamlEmitter() : m_document(), m_mapping_frame(), m_root_guard() {/*...*/}
            constexpr ~YamlEmitter() {/*...*/}

            void Initialize() {
                this->Finalize();
                const s32 is_init = ::yaml_document_initialize(std::addressof(m_document), nullptr, nullptr, nullptr, false, false);
                VP_ASSERT(is_init == 1);
            }
            void Finalize() {
                ::yaml_document_delete(std::addressof(m_document));
                m_mapping_frame.Clear();
                m_root_guard = false;
            }

            void Serialize(SerializeFunction *func) {
                yaml_emitter_t emitter_data;
                yaml_emitter_t *emitter = std::addressof(emitter_data);
                ::yaml_emitter_initialize(emitter);
                auto stream_lambda = [](void *data, unsigned char *buffer, size_t size) -> s32 {
                    SerializeFunction *func = reinterpret_cast<SerializeFunction*>(data);
                    if (RESULT_FAILED(func->Invoke(buffer, size))) { return 0; }
                    return 1;
                };
                ::yaml_emitter_set_output(emitter, stream_lambda, func);
                ::yaml_emitter_set_indent(emitter, 4);
                ::yaml_emitter_set_encoding(emitter, YAML_UTF8_ENCODING);
                ::yaml_emitter_set_unicode(emitter, true);
                ::yaml_emitter_open(emitter);
                ::yaml_emitter_dump(emitter, std::addressof(m_document));
                ::yaml_emitter_close(emitter);
                return;
            }
            s32 BeginDictionaryImpl(bool is_remap = false) {
                VP_ASSERT(m_root_guard);
                Mapping *mapping = m_mapping_frame.Allocate();
                VP_ASSERT(mapping != nullptr);
                /* Add */
                const yaml_char_t *tag = (!is_remap) ? reinterpret_cast<const yaml_char_t*>("!dict") : reinterpret_cast<const yaml_char_t*>("!dictremap");
                const s32 mapping_id = ::yaml_document_add_mapping(std::addressof(m_document), tag, YAML_ANY_MAPPING_STYLE);
                VP_ASSERT(mapping_id != 0);
                mapping->data_type = (!is_remap) ? vp::res::ByamlDataType::Dictionary : vp::res::ByamlDataType::DictionaryWithRemap;
                mapping->mapping_id = mapping_id;
                return mapping_id;
            }
            s32 BeginHashArrayImpl(u32 key_size, bool is_remap = false) {
                VP_ASSERT(m_root_guard);
                VP_ASSERT(0 < key_size);
                Mapping *mapping = m_mapping_frame.Allocate();
                VP_ASSERT(mapping != nullptr);
                /* Add */
                const u32 type_base = (!is_remap) ? 0x20 : 0x30;
                const u32 key_low = ((key_size-1) & 0xf);
                const u32 type = type_base | key_low;
                const yaml_char_t *tag = (!is_remap) ? reinterpret_cast<const yaml_char_t*>(cHashArrayTags[key_low]) : reinterpret_cast<const yaml_char_t*>(cHashArrayRemapTags[key_low]);
                const s32 mapping_id = ::yaml_document_add_mapping(std::addressof(m_document), tag, YAML_ANY_MAPPING_STYLE);
                VP_ASSERT(mapping_id != 0);
                mapping->data_type = static_cast<vp::res::ByamlDataType>(type);
                mapping->mapping_id = mapping_id;
                return mapping_id;
            }
            s32 BeginArrayImpl(bool is_monotyped = false, vp::res::ByamlDataType mono_type = vp::res::ByamlDataType::Null) {
                VP_ASSERT(m_root_guard);
                Mapping *mapping = m_mapping_frame.Allocate();
                VP_ASSERT(mapping != nullptr);
                /* Add */
                const yaml_char_t *tag = (!is_monotyped) ? reinterpret_cast<const yaml_char_t*>("!array") : reinterpret_cast<const yaml_char_t*>("!arraymono");
                const s32 mapping_id = ::yaml_document_add_sequence(std::addressof(m_document), tag, YAML_ANY_SEQUENCE_STYLE);
                VP_ASSERT(mapping_id != 0);
                mapping->data_type = (!is_monotyped) ? vp::res::ByamlDataType::Array : vp::res::ByamlDataType::MonoTypedArray;
                mapping->mapping_id = mapping_id;
                mapping->mono_data_type = mono_type;
                return mapping_id;
            }
        public:
            Result BeginDictionary(bool is_remap) {
                if (!m_mapping_frame.IsEmpty()) {
                    VP_ASSERT(m_root_guard);
                    Mapping mapping = m_mapping_frame.PeekBack();
                    VP_ASSERT(vp::res::IsArrayType(mapping.data_type));
                    const s32 value_id = this->BeginDictionaryImpl(is_remap);
                    const s32 result0 = ::yaml_document_append_sequence_item(std::addressof(m_document), mapping.mapping_id, value_id);
                    VP_ASSERT(result0 != 0);
                } else {
                    m_root_guard = true;
                    const s32 value_id = this->BeginDictionaryImpl(is_remap);
                    VP_ASSERT(value_id != 0);
                }
                RESULT_RETURN_SUCCESS;
            }
            Result BeginDictionaryByKey(const char *key, bool is_remap) {
                VP_ASSERT(m_root_guard);
                Mapping mapping = m_mapping_frame.PeekBack();
                VP_ASSERT(vp::res::IsDictionaryType(mapping.data_type));
                const s32 key_id = this->AddKey(key);
                const s32 value_id = this->BeginDictionaryImpl(is_remap);
                const s32 result0 = ::yaml_document_append_mapping_pair(std::addressof(m_document), mapping.mapping_id, key_id, value_id);
                VP_ASSERT(result0 != 0);
                RESULT_RETURN_SUCCESS;
            }
            Result BeginDictionaryByHash(const u32 *hash, bool is_remap) {
                VP_ASSERT(m_root_guard);
                Mapping mapping = m_mapping_frame.PeekBack();
                VP_ASSERT(vp::res::IsHashArrayType(mapping.data_type));
                const s32 hash_key = this->AddHash(mapping.data_type, hash);
                const s32 value_id = this->BeginDictionaryImpl(is_remap);
                const s32 result0 = ::yaml_document_append_mapping_pair(std::addressof(m_document), mapping.mapping_id, hash_key, value_id);
                VP_ASSERT(result0 != 0);
                RESULT_RETURN_SUCCESS;
            }
            void EndDictionary() {
                m_mapping_frame.RemoveBack();
                return;
            }

            Result BeginArray(bool is_monotyped) {
                if (!m_mapping_frame.IsEmpty()) {
                    VP_ASSERT(m_root_guard);
                    Mapping mapping = m_mapping_frame.PeekBack();
                    VP_ASSERT(vp::res::IsArrayType(mapping.data_type));
                    const s32 value_id = this->BeginArrayImpl(is_monotyped);
                    const s32 result0 = ::yaml_document_append_sequence_item(std::addressof(m_document), mapping.mapping_id, value_id);
                    VP_ASSERT(result0 != 0);
                } else {
                    m_root_guard = true;
                    const s32 value_id = this->BeginArrayImpl(is_monotyped);
                    VP_ASSERT(value_id != 0);
                }
                RESULT_RETURN_SUCCESS;
            }
            Result BeginArrayByKey(const char *key, bool is_monotyped) {
                VP_ASSERT(m_root_guard);
                Mapping mapping = m_mapping_frame.PeekBack();
                VP_ASSERT(vp::res::IsDictionaryType(mapping.data_type));
                const s32 key_id = this->AddKey(key);
                const s32 value_id = this->BeginArrayImpl(is_monotyped);
                const s32 result0 = ::yaml_document_append_mapping_pair(std::addressof(m_document), mapping.mapping_id, key_id, value_id);
                VP_ASSERT(result0 != 0);
                RESULT_RETURN_SUCCESS;
            }
            Result BeginArrayByHash(const u32 *hash, bool is_monotyped) {
                VP_ASSERT(m_root_guard);
                Mapping mapping = m_mapping_frame.PeekBack();
                VP_ASSERT(vp::res::IsHashArrayType(mapping.data_type));
                const s32 hash_key = this->AddHash(mapping.data_type, hash);
                const s32 value_id = this->BeginArrayImpl(is_monotyped);
                const s32 result0 = ::yaml_document_append_mapping_pair(std::addressof(m_document), mapping.mapping_id, hash_key, value_id);
                VP_ASSERT(result0 != 0);
                RESULT_RETURN_SUCCESS;
            }
            void EndArray() {
                m_mapping_frame.RemoveBack();
                return;
            }

            Result BeginHashArray(u32 key_size, bool is_remap) {
                if (!m_mapping_frame.IsEmpty()) {
                    VP_ASSERT(m_root_guard);
                    Mapping mapping = m_mapping_frame.PeekBack();
                    VP_ASSERT(vp::res::IsArrayType(mapping.data_type));
                    const s32 value_id = this->BeginHashArrayImpl(key_size, is_remap);
                    const s32 result0 = ::yaml_document_append_sequence_item(std::addressof(m_document), mapping.mapping_id, value_id);
                    VP_ASSERT(result0 != 0);
                } else {
                    m_root_guard = true;
                    const s32 value_id = this->BeginHashArrayImpl(key_size, is_remap);
                    VP_ASSERT(value_id != 0);
                }
                RESULT_RETURN_SUCCESS;
            }
            Result BeginHashArrayByKey(const char *key, u32 key_size, bool is_remap) {
                VP_ASSERT(m_root_guard);
                Mapping mapping = m_mapping_frame.PeekBack();
                VP_ASSERT(vp::res::IsDictionaryType(mapping.data_type));
                const s32 key_id = this->AddKey(key);
                const s32 value_id = this->BeginHashArrayImpl(key_size, is_remap);
                const s32 result0 = ::yaml_document_append_mapping_pair(std::addressof(m_document), mapping.mapping_id, key_id, value_id);
                VP_ASSERT(result0 != 0);
                RESULT_RETURN_SUCCESS;
            }
            Result BeginHashArrayByHash(const u32 *hash, u32 key_size, bool is_remap) {
                VP_ASSERT(m_root_guard);
                Mapping mapping = m_mapping_frame.PeekBack();
                VP_ASSERT(vp::res::IsHashArrayType(mapping.data_type));
                const s32 hash_key = this->AddHash(mapping.data_type, hash);
                const s32 value_id = this->BeginHashArrayImpl(key_size, is_remap);
                const s32 result0 = ::yaml_document_append_mapping_pair(std::addressof(m_document), mapping.mapping_id, hash_key, value_id);
                VP_ASSERT(result0 != 0);
                RESULT_RETURN_SUCCESS;
            }
            void EndHashArray() {
                m_mapping_frame.RemoveBack();
                return;
            }
        private:
            static constexpr void FixupFloatingPointString(char *string, u32 length, u32 sig_fig_count) {
                u32 sig_add = 0;
                u32 last_sig_fig = 0;
                u32 sig_fig = 0;
                u32 dec_pos = 0;
                bool is_hit_decimal   = false;
                bool is_hit_first_sig = false;
                bool is_hit_sig_fig = false;
                for (u32 i = 0; i < length; ++i) {

                    /* Null and sign check */
                    if (string[i] == '\0') { break; }
                    if (string[i] == '-') { continue; }

                    /* Track if we hit a decimal point */
                    if (string[i] == '.') {
                        is_hit_decimal = true; 
                        dec_pos = i;
                        /* Complete if are out of sig fig */
                        if (is_hit_sig_fig) { break; }
                        continue; 
                    }

                    /* Count trailing zeroes */
                    ++sig_add;
                    if (string[i] == '0') {
                        /* Continue if we are leading zeroes */
                        if (!is_hit_first_sig) { continue; }
                        /* Continue if we haven't hit the sig limit */
                        if (sig_fig + sig_add < sig_fig_count) { continue; }
                        /* Complete at the last sig fig or the decimal pos (highest) */
                        if (is_hit_decimal) { last_sig_fig = (last_sig_fig < dec_pos) ? dec_pos + 1 : last_sig_fig; }
                        is_hit_sig_fig = true;
                        string[last_sig_fig + 1] = '\0';
                        break;
                    } else if (!is_hit_first_sig) {
                        /* Signal we are now counting trailing zeroes */
                        is_hit_first_sig = true; 
                        sig_add = 1; 
                    }

                    /* Countup sig fig */
                    sig_fig += sig_add;
                    sig_add  = 0;
                    last_sig_fig = i;

                    if (sig_fig_count <= sig_fig) { is_hit_sig_fig = true; }
                    if (!is_hit_sig_fig || !is_hit_decimal) { continue; }

                    string[i + 1] = '\0';
                    break;
                }
                if (!is_hit_sig_fig && is_hit_decimal) { string[dec_pos + 2] = '\0'; }

                return;
            }

            s32 AddKey(const char *key) {
                const yaml_char_t *value = reinterpret_cast<const yaml_char_t*>(key);
                const u32 len = vp::util::StringLength(key);
                return ::yaml_document_add_scalar(std::addressof(m_document), nullptr, value, len, YAML_ANY_SCALAR_STYLE);
            }
            s32 AddHash(vp::res::ByamlDataType data_type, const u32 *key) {
                const u32 key_size = (static_cast<u32>(data_type) & 0xf) + 1;
                vp::util::FixedString8<0x100> str;
                str.Format("0x");
                for (u32 i = key_size-1; i < key_size; --i) {
                    str.AppendFormat("%08x", key[i]);
                }
                const yaml_char_t *val = reinterpret_cast<const yaml_char_t*>(str.GetString());
                return ::yaml_document_add_scalar(std::addressof(m_document), nullptr, val, str.GetLength(), YAML_ANY_SCALAR_STYLE);
            }
            template <auto DataType>
            s32 AddValueImpl(auto value) {
                if constexpr (DataType == vp::res::ByamlDataType::Null) {
                    constexpr const char cTag[] = "!null";
                    const yaml_char_t *tag = reinterpret_cast<const yaml_char_t*>(cTag);
                    const yaml_char_t *val = reinterpret_cast<const yaml_char_t*>("");
                    return ::yaml_document_add_scalar(std::addressof(m_document), tag, val, 0, YAML_ANY_SCALAR_STYLE);
                } else if constexpr (DataType == vp::res::ByamlDataType::Bool) {
                    constexpr const char cTag[] = "!bool";
                    const yaml_char_t *tag = reinterpret_cast<const yaml_char_t*>(cTag);
                    const yaml_char_t *val = (value) ? reinterpret_cast<const yaml_char_t*>("true") : reinterpret_cast<const yaml_char_t*>("false");
                    const u32 len = (value) ? sizeof("true")-1 : sizeof("false")-1;
                    return ::yaml_document_add_scalar(std::addressof(m_document), tag, val, len, YAML_ANY_SCALAR_STYLE);
                } else if constexpr (DataType == vp::res::ByamlDataType::S32) {
                    constexpr const char cTag[] = "!s32";
                    const yaml_char_t *tag = reinterpret_cast<const yaml_char_t*>(cTag);
                    vp::util::FixedString8<0x40> str;
                    str.Format("%d",value);
                    const yaml_char_t *val = reinterpret_cast<const yaml_char_t*>(str.GetString());
                    return ::yaml_document_add_scalar(std::addressof(m_document), tag, val, str.GetLength(), YAML_ANY_SCALAR_STYLE);
                } else if constexpr (DataType == vp::res::ByamlDataType::U32) {
                    constexpr const char cTag[] = "!u32";
                    const yaml_char_t *tag = reinterpret_cast<const yaml_char_t*>(cTag);
                    vp::util::FixedString8<0x40> str;
                    str.Format("%u",value);
                    const yaml_char_t *val = reinterpret_cast<const yaml_char_t*>(str.GetString());
                    return ::yaml_document_add_scalar(std::addressof(m_document), tag, val, str.GetLength(), YAML_ANY_SCALAR_STYLE);
                } else if constexpr (DataType == vp::res::ByamlDataType::F32) {
                    constexpr const char cTag[] = "!float";
                    const yaml_char_t *tag = reinterpret_cast<const yaml_char_t*>(cTag);
                    vp::util::FixedString8<0x80> str;
                    str.Format("%.43f",value);
                    FixupFloatingPointString(str.GetString(),str.GetLength(),9);
                    str.RecomputeLength();
                    const yaml_char_t *val = reinterpret_cast<const yaml_char_t*>(str.GetString());
                    return ::yaml_document_add_scalar(std::addressof(m_document), tag, val, str.GetLength(), YAML_ANY_SCALAR_STYLE);
                } else if constexpr (DataType == vp::res::ByamlDataType::S64) {
                    constexpr const char cTag[] = "!s64";
                    const yaml_char_t *tag = reinterpret_cast<const yaml_char_t*>(cTag);
                    vp::util::FixedString8<0x40> str;
                    str.Format("%lld",value);
                    const yaml_char_t *val = reinterpret_cast<const yaml_char_t*>(str.GetString());
                    return ::yaml_document_add_scalar(std::addressof(m_document), tag, val, str.GetLength(), YAML_ANY_SCALAR_STYLE);
                } else if constexpr (DataType == vp::res::ByamlDataType::U64) {
                    constexpr const char cTag[] = "!u64";
                    const yaml_char_t *tag = reinterpret_cast<const yaml_char_t*>(cTag);
                    vp::util::FixedString8<0x40> str;
                    str.Format("%llu",value);
                    const yaml_char_t *val = reinterpret_cast<const yaml_char_t*>(str.GetString());
                    return ::yaml_document_add_scalar(std::addressof(m_document), tag, val, str.GetLength(), YAML_ANY_SCALAR_STYLE);
                } else if constexpr (DataType == vp::res::ByamlDataType::F64) {
                    constexpr const char cTag[] = "!double";
                    const yaml_char_t *tag = reinterpret_cast<const yaml_char_t*>(cTag);
                    vp::util::FixedString8<0x400> str;
                    str.Format("%.322f",value);
                    FixupFloatingPointString(str.GetString(),str.GetLength(),15);
                    str.RecomputeLength();
                    const yaml_char_t *val = reinterpret_cast<const yaml_char_t*>(str.GetString());
                    return ::yaml_document_add_scalar(std::addressof(m_document), tag, val, str.GetLength(), YAML_ANY_SCALAR_STYLE);
                } else if constexpr (DataType == vp::res::ByamlDataType::BinaryData || DataType == vp::res::ByamlDataType::BinaryDataWithAlignment) {
                    constexpr const char *cTag = (DataType == vp::res::ByamlDataType::BinaryData) ? "!bin" : "!binalign";
                    const yaml_char_t *tag = reinterpret_cast<const yaml_char_t*>(cTag);
                    vp::util::HeapString8 str;
                    if (0 < value.size) {
                        RESULT_ABORT_UNLESS(str.Initialize(nullptr, value.size * 2 + 3));
                        str.Format("0x");
                        const u8 *data = reinterpret_cast<const u8*>(value.data);
                        const auto append_lambda = [&str](u8 val) {
                            char c;
                            if (val <= 9) {
                                c = static_cast<char>('0' + val);
                            } else if (val <= 15) {
                                c = static_cast<char>('a' + (val - 10));
                            } else {
                                VP_UNEXPECTED_DEFAULT();
                            }
                            str.AppendChar(c);
                        };
                        for (u32 i = value.size-1; i < value.size; --i) {
                            append_lambda((data[i] >> 0x4) & 0x0f);
                            append_lambda((data[i] >> 0x0) & 0x0f);
                        }
                    }
                    const s32 bin_mapping = ::yaml_document_add_mapping(std::addressof(m_document), tag, YAML_ANY_MAPPING_STYLE);
                    VP_ASSERT(bin_mapping != 0);
                    if (DataType == vp::res::ByamlDataType::BinaryDataWithAlignment) {
                        const s32 akey_scalar = this->AddKey("Alignment");
                        VP_ASSERT(akey_scalar != 0);
                        const s32 abin_scalar = this->AddValueImpl<vp::res::ByamlDataType::U32>(value.alignment);
                        VP_ASSERT(abin_scalar != 0);
                        const s32 result1 = ::yaml_document_append_mapping_pair(std::addressof(m_document), bin_mapping, akey_scalar, abin_scalar);
                        VP_ASSERT(result1 != 0);
                    }
                    const s32 key_scalar = this->AddKey("Data");
                    VP_ASSERT(key_scalar != 0);
                    const s32 bin_scalar = this->AddValueImpl<vp::res::ByamlDataType::StringIndex>(std::addressof(str));
                    VP_ASSERT(bin_scalar != 0);
                    const s32 result0 = ::yaml_document_append_mapping_pair(std::addressof(m_document), bin_mapping, key_scalar, bin_scalar);
                    VP_ASSERT(result0 != 0);
                    if (0 < value.size) {
                        str.Finalize();
                    }
                    return bin_mapping;
                } else if constexpr (DataType == vp::res::ByamlDataType::StringIndex) {
                    constexpr const char cTag[] = "!str";
                    const yaml_char_t *tag = reinterpret_cast<const yaml_char_t*>(cTag);
                    const char *str;
                    u32 len;
                    vp::util::GetUnderlyingString(std::addressof(str), std::addressof(len), value);
                    const yaml_char_t *val = reinterpret_cast<const yaml_char_t*>(str);
                    return ::yaml_document_add_scalar(std::addressof(m_document), tag, val, len, YAML_ANY_SCALAR_STYLE);
                } else if constexpr (DataType == vp::res::ByamlDataType::Dictionary || DataType == vp::res::ByamlDataType::DictionaryWithRemap) {
                    const yaml_char_t *tag = (DataType == vp::res::ByamlDataType::Dictionary) ? reinterpret_cast<const yaml_char_t*>("!dict") : reinterpret_cast<const yaml_char_t*>("!dictremap");
                    return ::yaml_document_add_mapping(std::addressof(m_document), tag, YAML_ANY_MAPPING_STYLE);
                } else if constexpr (DataType == vp::res::ByamlDataType::Array || DataType == vp::res::ByamlDataType::MonoTypedArray) {
                    const yaml_char_t *tag = (DataType == vp::res::ByamlDataType::Array) ? reinterpret_cast<const yaml_char_t*>("!array") : reinterpret_cast<const yaml_char_t*>("!arraymono");
                    return ::yaml_document_add_sequence(std::addressof(m_document), tag, YAML_ANY_SEQUENCE_STYLE);
                } else if constexpr (vp::res::IsHashArrayType(DataType)) {
                    const bool is_remap = ((static_cast<u32>(DataType) & 0x30) == 0x30);
                    const u32 key_low = (static_cast<u32>(DataType) & 0xf);
                    const yaml_char_t *tag = (!is_remap) ? reinterpret_cast<const yaml_char_t*>(cHashArrayTags[key_low]) : reinterpret_cast<const yaml_char_t*>(cHashArrayRemapTags[key_low]);
                    return ::yaml_document_add_mapping(std::addressof(m_document), tag, YAML_ANY_MAPPING_STYLE);
                }
                return 0;
            }
            template <auto DataType>
            Result AddDictionaryImpl(auto value, const char *key) {
                VP_ASSERT(m_root_guard);
                const Mapping mapping = m_mapping_frame.PeekBack();
                VP_ASSERT(vp::res::IsDictionaryType(mapping.data_type));
                const s32 key_id = this->AddKey(key);
                const s32 value_id = this->AddValueImpl<DataType>(value);
                const s32 result0 = ::yaml_document_append_mapping_pair(std::addressof(m_document), mapping.mapping_id, key_id, value_id);
                VP_ASSERT(result0 != 0);
                RESULT_RETURN_SUCCESS;
            }
            template <auto DataType>
            Result AddArrayImpl(auto value) {
                if (!m_mapping_frame.IsEmpty()) {
                    VP_ASSERT(m_root_guard);
                    const Mapping mapping = m_mapping_frame.PeekBack();
                    VP_ASSERT(vp::res::IsArrayType(mapping.data_type));
                    const s32 value_id = this->AddValueImpl<DataType>(value);
                    const s32 result0 = ::yaml_document_append_sequence_item(std::addressof(m_document), mapping.mapping_id, value_id);
                    VP_ASSERT(result0 != 0);
                } else {
                    m_root_guard = true;
                    this->AddValueImpl<DataType>(value);
                }
                RESULT_RETURN_SUCCESS;
            }
            template <auto DataType>
            Result AddHashArrayImpl(auto value, const u32 *hash) {
                VP_ASSERT(m_root_guard);
                const Mapping mapping = m_mapping_frame.PeekBack();
                VP_ASSERT(vp::res::IsHashArrayType(mapping.data_type));
                const s32 hash_id = this->AddHash(mapping.data_type, hash);
                const s32 value_id = this->AddValueImpl<DataType>(value);
                const s32 result0 = ::yaml_document_append_mapping_pair(std::addressof(m_document), mapping.mapping_id, hash_id, value_id);
                VP_ASSERT(result0 != 0);
                RESULT_RETURN_SUCCESS;
            }
        public:

            template <typename T>
                requires (vp::res::TGetByamlDataTypeValue<T>() != vp::res::ByamlDataType::Null)
            Result AddValue(T value, const char *key, const u32 *hash) {
                constexpr vp::res::ByamlDataType data_type = vp::res::TGetByamlDataTypeValue<T>();
                if (key != nullptr) {
                    if constexpr (data_type == vp::res::ByamlDataType::Bool) {
                        return this->AddBoolByKey(value, key);
                    } else if constexpr (data_type == vp::res::ByamlDataType::S32) {
                        return this->AddS32ByKey(value, key);
                    } else if constexpr (data_type == vp::res::ByamlDataType::F32) {
                        return this->AddFloatByKey(value, key);
                    } else if constexpr (data_type == vp::res::ByamlDataType::U32) {
                        return this->AddU32ByKey(value, key);
                    } else if constexpr (data_type == vp::res::ByamlDataType::S64) {
                        return this->AddS64ByKey(value, key);
                    } else if constexpr (data_type == vp::res::ByamlDataType::U64) {
                        return this->AddU64ByKey(value, key);
                    } else if constexpr (data_type == vp::res::ByamlDataType::F64) {
                        return this->AddDoubleByKey(value, key);
                    } else if constexpr (data_type == vp::res::ByamlDataType::StringIndex) {
                        return this->AddStringByKey(value, key);
                    }
                } else if (hash != nullptr) {
                    if constexpr (data_type == vp::res::ByamlDataType::Bool) {
                        return this->AddBoolByHash(value, hash);
                    } else if constexpr (data_type == vp::res::ByamlDataType::S32) {
                        return this->AddS32ByHash(value, hash);
                    } else if constexpr (data_type == vp::res::ByamlDataType::F32) {
                        return this->AddFloatByHash(value, hash);
                    } else if constexpr (data_type == vp::res::ByamlDataType::U32) {
                        return this->AddU32ByHash(value, hash);
                    } else if constexpr (data_type == vp::res::ByamlDataType::S64) {
                        return this->AddS64ByHash(value, hash);
                    } else if constexpr (data_type == vp::res::ByamlDataType::U64) {
                        return this->AddU64ByHash(value, hash);
                    } else if constexpr (data_type == vp::res::ByamlDataType::F64) {
                        return this->AddDoubleByHash(value, hash);
                    } else if constexpr (data_type == vp::res::ByamlDataType::StringIndex) {
                        return this->AddStringByHash(value, hash);
                    }
                } else {
                    if constexpr (data_type == vp::res::ByamlDataType::Bool) {
                        return this->AddBool(value);
                    } else if constexpr (data_type == vp::res::ByamlDataType::S32) {
                        return this->AddS32(value);
                    } else if constexpr (data_type == vp::res::ByamlDataType::F32) {
                        return this->AddFloat(value);
                    } else if constexpr (data_type == vp::res::ByamlDataType::U32) {
                        return this->AddU32(value);
                    } else if constexpr (data_type == vp::res::ByamlDataType::S64) {
                        return this->AddS64(value);
                    } else if constexpr (data_type == vp::res::ByamlDataType::U64) {
                        return this->AddU64(value);
                    } else if constexpr (data_type == vp::res::ByamlDataType::F64) {
                        return this->AddDouble(value);
                    } else if constexpr (data_type == vp::res::ByamlDataType::StringIndex) {
                        return this->AddString(value);
                    }
                }
                RESULT_RETURN_SUCCESS;
            }
        public:
            /* Root or array */
            Result AddNull() { RESULT_RETURN(this->AddArrayImpl<vp::res::ByamlDataType::Null>(0)); }
            Result AddBool(bool value) { RESULT_RETURN(this->AddArrayImpl<vp::res::ByamlDataType::Bool>(value)); }
            Result AddS32(s32 value) { RESULT_RETURN(this->AddArrayImpl<vp::res::ByamlDataType::S32>(value)); }
            Result AddU32(u32 value) { RESULT_RETURN(this->AddArrayImpl<vp::res::ByamlDataType::U32>(value)); }
            Result AddFloat(float value) { RESULT_RETURN(this->AddArrayImpl<vp::res::ByamlDataType::F32>(value)); }
            Result AddU64(u64 value) { RESULT_RETURN(this->AddArrayImpl<vp::res::ByamlDataType::U64>(value)); }
            Result AddS64(s64 value) { RESULT_RETURN(this->AddArrayImpl<vp::res::ByamlDataType::S64>(value)); }
            Result AddDouble(double value) { RESULT_RETURN(this->AddArrayImpl<vp::res::ByamlDataType::F64>(value)); }
            Result AddBinaryData(impl::YamlBinaryData value) { RESULT_RETURN(this->AddArrayImpl<vp::res::ByamlDataType::BinaryData>(value)); }
            Result AddBinaryDataWithAlignment(impl::YamlBinaryData value) { RESULT_RETURN(this->AddArrayImpl<vp::res::ByamlDataType::BinaryDataWithAlignment>(value)); }
            Result AddString(const char *value) { RESULT_RETURN(this->AddArrayImpl<vp::res::ByamlDataType::StringIndex>(value)); }
        public:
            /* Dictionary */
            Result AddNullByKey(const char *key) { RESULT_RETURN(this->AddDictionaryImpl<vp::res::ByamlDataType::Null>(0, key)); }
            Result AddBoolByKey(bool value, const char *key) { RESULT_RETURN(this->AddDictionaryImpl<vp::res::ByamlDataType::Bool>(value, key)); }
            Result AddS32ByKey(s32 value, const char *key) { RESULT_RETURN(this->AddDictionaryImpl<vp::res::ByamlDataType::S32>(value, key)); }
            Result AddU32ByKey(u32 value, const char *key) { RESULT_RETURN(this->AddDictionaryImpl<vp::res::ByamlDataType::U32>(value, key)); }
            Result AddFloatByKey(float value, const char *key) { RESULT_RETURN(this->AddDictionaryImpl<vp::res::ByamlDataType::F32>(value, key)); }
            Result AddU64ByKey(u64 value, const char *key) { RESULT_RETURN(this->AddDictionaryImpl<vp::res::ByamlDataType::U64>(value, key)); }
            Result AddS64ByKey(s64 value, const char *key) { RESULT_RETURN(this->AddDictionaryImpl<vp::res::ByamlDataType::S64>(value, key)); }
            Result AddDoubleByKey(double value, const char *key) { RESULT_RETURN(this->AddDictionaryImpl<vp::res::ByamlDataType::F64>(value, key)); }
            Result AddBinaryDataByKey(impl::YamlBinaryData value, const char *key) { RESULT_RETURN(this->AddDictionaryImpl<vp::res::ByamlDataType::BinaryData>(value, key)); }
            Result AddBinaryDataWithAlignmentByKey(impl::YamlBinaryData value, const char *key) { RESULT_RETURN(this->AddDictionaryImpl<vp::res::ByamlDataType::BinaryDataWithAlignment>(value, key)); }
            Result AddStringByKey(const char *value, const char *key) { RESULT_RETURN(this->AddDictionaryImpl<vp::res::ByamlDataType::StringIndex>(value, key)); }
        public:
            /* HashArray */
            Result AddNullByHash(const u32 *hash) { RESULT_RETURN(this->AddHashArrayImpl<vp::res::ByamlDataType::Null>(0, hash)); }
            Result AddBoolByHash(bool value, const u32 *hash) { RESULT_RETURN(this->AddHashArrayImpl<vp::res::ByamlDataType::Bool>(value, hash)); }
            Result AddS32ByHash(s32 value, const u32 *hash) { RESULT_RETURN(this->AddHashArrayImpl<vp::res::ByamlDataType::S32>(value, hash)); }
            Result AddU32ByHash(u32 value, const u32 *hash) { RESULT_RETURN(this->AddHashArrayImpl<vp::res::ByamlDataType::U32>(value, hash)); }
            Result AddFloatByHash(float value, const u32 *hash) { RESULT_RETURN(this->AddHashArrayImpl<vp::res::ByamlDataType::F32>(value, hash)); }
            Result AddU64ByHash(u64 value, const u32 *hash) { RESULT_RETURN(this->AddHashArrayImpl<vp::res::ByamlDataType::U64>(value, hash)); }
            Result AddS64ByHash(s64 value, const u32 *hash) { RESULT_RETURN(this->AddHashArrayImpl<vp::res::ByamlDataType::S64>(value, hash)); }
            Result AddDoubleByHash(double value, const u32 *hash) { RESULT_RETURN(this->AddHashArrayImpl<vp::res::ByamlDataType::F64>(value, hash)); }
            Result AddBinaryDataByHash(impl::YamlBinaryData value, const u32 *hash) { RESULT_RETURN(this->AddHashArrayImpl<vp::res::ByamlDataType::BinaryData>(value, hash)); }
            Result AddBinaryDataWithAlignmentByHash(impl::YamlBinaryData value, const u32 *hash) { RESULT_RETURN(this->AddHashArrayImpl<vp::res::ByamlDataType::BinaryDataWithAlignment>(value, hash)); }
            Result AddStringByHash(const char *value, const u32 *hash) { RESULT_RETURN(this->AddHashArrayImpl<vp::res::ByamlDataType::StringIndex>(value, hash)); }
    };
}
