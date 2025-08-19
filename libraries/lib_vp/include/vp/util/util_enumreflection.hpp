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

/* Expects enum value labels as variadic arguments */
#define VP_ENUM_TRAITSV(lookup_name, enum_type, ...) VP_CLASS_VMACRO(lookup_name, enum_type, VP_ENUM_TRAITS, __VA_ARGS__)

#define VP_ENUM_TRAITS_IMPL(enum_type) \
    const ::vp::util::EnumDictionaryNode _s##enum_type##DicNode(std::addressof( _c##enum_type##TraitsEnumDictionary ));

namespace vp::util::impl {        
    struct DummyEnumTraits {
        static constexpr bool cIsDummy = true;
    };
}

template <typename T>
    requires (std::is_enum<T>::value)
struct TVpEnumTraits {
    static constexpr bool cIsDummy = true;
    using Traits = vp::util::impl::DummyEnumTraits;
};

/* Must be declared outside namespaces (the enum name must contain it's namespace) */
#define VP_ENUM_TRAITS_AUTO(enum_type) \
    template<> struct TVpEnumTraits< enum_type > { \
        static constexpr bool cIsDummy = false; \
        using Traits = enum_type##Traits; \
    };

namespace vp::util {

    template <typename T>
        requires (std::is_enum<T>::value)
    using TEnumTraits = TVpEnumTraits<T>::Traits;

    class EnumDictionaryBase {
        protected:
            const char *m_class_name;
        public:
            VP_RTTI_BASE(EnumDictionaryBase);
        public:
            constexpr EnumDictionaryBase(const char *class_name) : m_class_name(class_name) {/*...*/}
            virtual constexpr ~EnumDictionaryBase() {/*...*/}

            constexpr ALWAYS_INLINE const char * GetClassName() const {
                return m_class_name;
            }
    };

    template <typename T>
        requires (!std::is_enum<T>::value)
    class EnumDictionary : public EnumDictionaryBase {
        public:
            using KeyMap = vp::util::KeyIndexMap<u32,u16>;
        protected:
            const T *m_value_array;
            const char * const *m_string_array;
            KeyMap m_key_map;
        public:
            VP_RTTI_DERIVED(EnumDictionary, EnumDictionaryBase);
        public:
            constexpr EnumDictionary(const char *class_name, const char * const *string_array, const T *value_array) : EnumDictionaryBase(class_name), m_value_array(value_array), m_string_array(string_array), m_key_map() {/*...*/}
            virtual constexpr ~EnumDictionary() override {/*...*/}

            constexpr bool GetEnumIndexByName(u32 *out_index, const char *enum_value_name) const {
                const u32 hash = HashMurmur3(enum_value_name);
                const u16 index = m_key_map.TryGetIndexByKey(hash);
                if (index == KeyMap::cInvalidEntryIndex) { return false; }
                *out_index = static_cast<u32>(index);
                return true;
            }
            constexpr bool GetEnumIndexByValue(u32 *out_index, T enum_value) const {
                for (u32 i = 0; i < this->GetEnumCount(); ++i) {
                    if (enum_value == m_value_array[i]) { *out_index = i; return true; }
                }
                return false;
            }
            constexpr bool GetEnumValueByName(T *out_value, const char *enum_value_name) const {
                u32 index;
                const bool result = this->GetEnumIndexByName(std::addressof(index), enum_value_name);
                if (!result) { return false; }
                *out_value = static_cast<T>(m_value_array[index]);
                return true;
            }
            constexpr bool GetEnumValueByIndex(T *out_value, u32 index) const {
                VP_ASSERT(index < this->GetEnumCount());
                *out_value = static_cast<T>(m_value_array[index]);
                return true;
            }
            const char *GetEnumNameByIndex(const char *out_value, u32 index) const {
                return m_string_array[index];
            }
            constexpr u32 GetEnumCount() const {
                return m_key_map.GetUsedCount();
            }
    };

    template <typename T, size_t Size>
    class TFixedValueDictionary final : public EnumDictionary<T> {
        private:
            EnumDictionary<T>::KeyMap::Key m_hash_array[Size * 2];
            u16 m_index_array[Size * 2];
        public:
            constexpr TFixedValueDictionary(const char *class_name, const std::array<const char*, Size> &string_array, const std::array<T, Size> &value_array, const std::array<u32, Size> &hash_array) : EnumDictionary<T>(class_name, string_array.data(), value_array.data()), m_hash_array{}, m_index_array{} {

                /* Populate entries */
                this->EnumDictionary<T>::m_key_map.SetMemory(m_hash_array, m_index_array, Size * 2);
                for (u32 i = 0; i < Size; ++i) {
                    this->EnumDictionary<T>::m_key_map.AddEntry(hash_array[i], i);
                }

                return;
            }
            virtual constexpr ~TFixedValueDictionary() override {/*...*/}
    };

    class EnumDictionaryManager;

    class EnumDictionaryNode {
        public:
            friend EnumDictionaryManager;
        private:
            vp::util::ParentRedBlackTreeNode<EnumDictionaryNode, u64> m_tree_node;
            const EnumDictionaryBase *m_dictionary;
        public:
            EnumDictionaryNode(const EnumDictionaryBase *enum_dictionary);
            constexpr ~EnumDictionaryNode() {/*...*/}
    };

    class EnumDictionaryManager {
        public:
            friend EnumDictionaryNode;
        public:
            using EnumDictionaryTree = vp::util::ParentRedBlackTreeTraits<EnumDictionaryNode, &EnumDictionaryNode::m_tree_node>::Tree;
        private:
            EnumDictionaryTree m_tree;
        private:
            void Insert(EnumDictionaryNode *dic_node) {
                m_tree.Insert(dic_node);
            }
        public:
            constexpr EnumDictionaryManager() : m_tree() {/*...*/}
            constexpr ~EnumDictionaryManager() {/*...*/}

            template <typename T>
            constexpr const EnumDictionary<T> *GetEnumDictionary(const char *class_name) const {
                const u64 class_hash = vp::util::HashMurmur64(class_name);
                const EnumDictionaryNode *dic_node   = m_tree.Find(class_hash);
                if (dic_node == nullptr) { return nullptr; }
                if (!EnumDictionary<T>::CheckRuntimeTypeInfoStatic(dic_node->m_dictionary)) { return nullptr; }
                return reinterpret_cast<const EnumDictionary<T>*>(dic_node->m_dictionary);
            }
    };

    EnumDictionaryManager *GetEnumDictionaryManager();
}


namespace vp::util::impl {

    template <class UnderlyingType>
    struct EnumUnpackHelper {
        const char * const string;
        UnderlyingType value;
    };

    template <class UnderlyingType, size_t Size>
    constexpr std::array<const char*, Size> EnumUnpackString(const EnumUnpackHelper<UnderlyingType> *h_array) {
        std::array<const char*, Size> ret;
        for (u32 i = 0; i < Size; ++i) {
            ret[i] = h_array[i].string;
        }
        return ret;
    }

    template <class UnderlyingType, size_t Size>
    constexpr std::array<UnderlyingType, Size> EnumUnpackValue(const EnumUnpackHelper<UnderlyingType> *h_array) {
        std::array<UnderlyingType, Size> ret;
        for (u32 i = 0; i < Size; ++i) {
            ret[i] = h_array[i].value;
        }
        return ret;
    }

    template <size_t Size>
    constexpr std::array<u32,Size> GenerateEnumValueHashArray(const std::array<const char*,Size> &string_array) {
        std::array<u32,Size> ret;
        for (u32 i = 0; i < Size; ++i) {
            ret[i] = HashMurmur3(string_array[i]);
        }
        return ret;
    }

    template <size_t Size>
    constexpr bool ValidateEnumHashArray(const std::array<u32,Size> &hash_array) {
        /* Ensure no 0 hash or duplicate */
        for (u32 i = 0; i < hash_array.size(); ++i) {
            if (hash_array[i] == 0) { return false; }
            for (u32 j = i+1; j < hash_array.size(); ++j) {
                if (hash_array[i] == hash_array[j]) { return false; }
            }
        }
        return true;
    }
}

#define VP_ENUM_TRAITS_START(lookup_name, enum_type) \
    struct enum_type##Traits { \
        using EnumType = enum_type ; \
        using UnderlyingType = std::underlying_type_t<EnumType>; \
        static constexpr bool cIsDummy = false; \
        static constexpr const char * cClassName = TOSTRING( lookup_name ); \
        static constexpr const ::vp::util::impl::EnumUnpackHelper<UnderlyingType> cPackArray[] = {

#define VP_ENUM_TRAITS_PACK_ENTRY(_arg) \
            { TOSTRING( _arg ) , static_cast<UnderlyingType>(EnumType:: _arg ), },

#define VP_ENUM_TRAITS_END(enum_type) \
        }; \
        static constexpr const u32 cEnumCount = sizeof(cPackArray) / sizeof(::vp::util::impl::EnumUnpackHelper<UnderlyingType>); \
        static constexpr const std::array<const char*,cEnumCount> cIndexToStringArray = ::vp::util::impl::EnumUnpackString<UnderlyingType, cEnumCount>(cPackArray); \
        static constexpr const std::array<UnderlyingType,cEnumCount> cIndexToEnumArray = ::vp::util::impl::EnumUnpackValue<UnderlyingType, cEnumCount>(cPackArray); \
        static constexpr const std::array<u32,cEnumCount> cHashToIndexArray = ::vp::util::impl::GenerateEnumValueHashArray<cEnumCount>(cIndexToStringArray); \
        static_assert(::vp::util::impl::ValidateEnumHashArray<cEnumCount>(cHashToIndexArray)); \
        \
        static constexpr u32 GetIndexByEnumValue(EnumType enum_value) { u32 i = 0; for (; i < cEnumCount; ++i) { if (static_cast<EnumType>(cIndexToEnumArray[i]) == enum_value) { break; }} VP_ASSERT(i < cEnumCount); return i; } \
        static constexpr const char *GetStringByIndex(u32 index) { VP_ASSERT(index < cEnumCount); return cIndexToStringArray[index]; } \
        static constexpr const char *GetStringByValue(EnumType enum_value) { return cIndexToStringArray[GetIndexByEnumValue(enum_value)]; } \
        static constexpr bool GetValueByString(EnumType *out_value, const char *enum_string); \
        static constexpr bool GetValueByString(UnderlyingType *out_value, const char *enum_string); \
        static constexpr EnumType GetValueByIndex(u32 index) { VP_ASSERT(index < cEnumCount); return static_cast<EnumType>(cIndexToEnumArray[index]); } \
    }; \
    static constexpr const ::vp::util::TFixedValueDictionary<typename enum_type##Traits::UnderlyingType, enum_type##Traits::cEnumCount> _c##enum_type##TraitsEnumDictionary( enum_type##Traits::cClassName, enum_type##Traits::cIndexToStringArray, enum_type##Traits::cIndexToEnumArray, enum_type##Traits::cHashToIndexArray); \
    constexpr bool enum_type##Traits::GetValueByString(EnumType *out_value, const char *enum_string) { \
        UnderlyingType value = 0; \
        const bool result = _c##enum_type##TraitsEnumDictionary.GetEnumValueByName(std::addressof(value), enum_string); \
        *out_value = static_cast<EnumType>(value); \
        return result; \
    } \
    constexpr bool enum_type##Traits::GetValueByString(UnderlyingType *out_value, const char *enum_string) { \
        const bool result = _c##enum_type##TraitsEnumDictionary.GetEnumValueByName(out_value, enum_string); \
        return result; \
    }
