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

/* Expects enum values to be provided as (value_string, value_u32) pairs */
#define VP_ENUM_TRAITSV(namespace, enum_name, ...) VP_CLASS_VMACRO(namespace, enum_name, VP_ENUM_TRAITS, __VA_ARGS__)

#define VP_ENUM_TRAITS_IMPL(namespace, enum_name) \
    constexpr const ::vp::util::TFixedValueDictionary< enum_name##Traits::UnderlyingType, sizeof( enum_name##Traits::cNWStringArray) / sizeof(const char*)> c##enum_name##EnumDictionary( enum_name##Traits::cClassName, enum_name##Traits::cNWStringArray, enum_name##Traits::cIndexToEnumArray); \
    const ::vp::util::EnumDictionaryNode s##enum_name##DicNode(std::addressof( c##enum_name##EnumDictionary ));

namespace vp::util::impl {        
    struct DummyEnumTraits {
        static constexpr bool cIsDummy = true;
    };
}

template <typename T>
    requires (std::is_enum<T>::value)
struct TVpEnumTraitsAuto {
    static constexpr bool cIsDummy = true;
    using Traits = vp::util::impl::DummyEnumTraits;
};

/* Must be declared outside namespaces (the enum name must contain it's namespace) */
#define VP_ENUM_TRAITS_AUTO(enum_name) \
    template<> struct TVpEnumTraitsAuto< enum_name > { \
        static constexpr bool cIsDummy = false; \
        using Traits = enum_name##Traits; \
    };

namespace vp::util {

    template <typename T>
        requires (std::is_enum<T>::value)
    using TEnumTraitsAuto = TVpEnumTraitsAuto<T>;

    template <size_t NamespaceLength, size_t NameLength>
    struct TFixedNamespaceString {
        static constexpr size_t cStringLength = (NamespaceLength != 0) ? NamespaceLength + NameLength + 2 : NameLength;

        char nw_string[cStringLength + 1];

        constexpr TFixedNamespaceString(const char *namespace_string, const char *string) : nw_string() {

            for (u32 i = 0; i < NamespaceLength; ++i) {
                nw_string[i] = namespace_string[i];
            }
            if (NamespaceLength != 0) {
                nw_string[NamespaceLength + 0] = ':';
                nw_string[NamespaceLength + 1] = ':';
            }
            for (u32 i = 0; i < NameLength; ++i) {
                nw_string[i + NamespaceLength + 2] = string[i];
            }

            nw_string[cStringLength] = '\0';
        }
        constexpr ~TFixedNamespaceString() {/*...*/}

        constexpr const char *GetString() const { return nw_string; }
    };

    template <size_t StringLength>
    struct TFixedNintendoWareString {
        static constexpr size_t cStringLength = StringLength;

        char nw_string[cStringLength + 3];

        constexpr TFixedNintendoWareString(const char *string) : nw_string() {

            nw_string[0] = cStringLength;
            nw_string[1] = (cStringLength >> 0x8) & 0xff;
            
            for (u32 i = 0; i < cStringLength; ++i) {
                nw_string[i + 2] = string[i];
            }
            nw_string[cStringLength + 2] = '\0';
        }
        constexpr ~TFixedNintendoWareString() {/*...*/}

        constexpr const char *GetString() const { return nw_string; }
    };

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
        protected:
            const T                             *m_value_array;
            vp::res::ResNintendoWareDictionary   m_res_dic;
        public:
            VP_RTTI_DERIVED(EnumDictionary, EnumDictionaryBase);
        public:
            constexpr EnumDictionary(const char *class_name, const T *value_array) : EnumDictionaryBase(class_name), m_value_array(value_array), m_res_dic() {/*...*/}
            virtual constexpr ~EnumDictionary() override {/*...*/}

            constexpr u32 GetEnumIndexByName(const char *enum_value_name) const {
                const u32 index = m_res_dic.TryGetEntryIndexByKey(enum_value_name);
                return index;
            }
            constexpr bool GetEnumValueByName(T *out_value, const char *enum_value_name) const {
                const u32 index = this->GetEnumIndexByName(enum_value_name);
                if (index == vp::res::ResNintendoWareDictionary::cInvalidEntryIndex) { return false; }
                *out_value = static_cast<T>(m_value_array[index]);
                return true;
            }
            constexpr bool GetEnumValueByIndex(T *out_value, u32 index) const {
                VP_ASSERT(index < this->GetEnumCount());
                *out_value = static_cast<T>(m_value_array[index]);
                return true;
            }
            constexpr ALWAYS_INLINE u32 GetEnumCount() const {
                return m_res_dic.node_count;
            }
    };

    template <typename T, size_t Size>
    class TFixedValueDictionary final : public EnumDictionary<T> {
        private:
            vp::res::ResNintendoWareDictionary::ResDicNode m_entry_array[Size];
        public:
            constexpr TFixedValueDictionary(const char *class_name, const char * const *nw_string_array, const T *value_array) : EnumDictionary<T>(class_name, value_array), m_entry_array{} {

                /* Populate entries */
                for (u32 i = 0; i < Size; ++i) {
                    m_entry_array[i].key = nw_string_array[i];
                }

                /* Set res dic meta data */
                this->EnumDictionary<T>::m_res_dic.magic         = vp::res::ResNintendoWareDictionary::cMagic;
                this->EnumDictionary<T>::m_res_dic.node_count    = Size;
                this->EnumDictionary<T>::m_res_dic.root_node.key = "\0\0\0";

                /* Build res dic */
                const bool result = this->EnumDictionary<T>::m_res_dic.TBuild(m_entry_array);
                VP_ASSERT(result == true);

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
                const u64                 class_hash = vp::util::HashMurmur64(class_name);
                const EnumDictionaryNode *dic_node   = m_tree.Find(class_hash);
                if (dic_node == nullptr) { return nullptr; }
                if (!EnumDictionary<T>::CheckRuntimeTypeInfoStatic(dic_node->m_dictionary)) { return nullptr; }
                return reinterpret_cast<const EnumDictionary<T>*>(dic_node->m_dictionary);
            }
    };

    EnumDictionaryManager *GetEnumDictionaryManager();
}

#define VP_ENUM_TRAITS_START(namespace, enum_name) \
    struct enum_name##Traits { \
        using EnumType = namespace :: enum_name; \
        using UnderlyingType = std::underlying_type< namespace :: enum_name >::type; \
        static constexpr bool cIsDummy = false; \
        static constexpr const char *cClassNameSafe = TOSTRING( enum_name ); \
        static constexpr const char *cNamespaceSafe = TOSTRING( namespace ); \
        static constexpr const ::vp::util::TFixedNamespaceString<vp::util::TStringLength( TOSTRING( namespace ) ,0xffff'ffff), vp::util::TStringLength( TOSTRING( enum_name ) ,0xffff'ffff)> cNamespaceClass{cNamespaceSafe, cClassNameSafe}; \
        static constexpr const char *cClassName = cNamespaceClass.GetString();

#define VP_ENUM_TRAITS_END(namespace, enum_name) \
        }; \
        static constexpr u32 cEnumCount = sizeof(cIndexToEnumArray) / sizeof(u32); \
        static constexpr u32 GetIndexByEnumValue(EnumType enum_value) { u32 i = 0; for (; i < cEnumCount; ++i) { if (static_cast<EnumType>(cIndexToEnumArray[i]) == enum_value) { break; }} VP_ASSERT(i < cEnumCount); return i; } \
        static constexpr const char *GetStringByIndex(u32 index) { VP_ASSERT(index < cEnumCount); return cIndexToStringArray[index]; } \
        static constexpr const char *GetStringByEnumValue(EnumType enum_value) { return cIndexToStringArray[GetIndexByEnumValue(enum_value)]; } \
        static constexpr bool GetEnumValueByString(EnumType *out_value, const char *enum_string); \
        static constexpr bool GetEnumValueByString(UnderlyingType *out_value, const char *enum_string); \
        static constexpr EnumType GetEnumValueByIndex(u32 index) { VP_ASSERT(index < cEnumCount); return static_cast<EnumType>(cIndexToEnumArray[index]); } \
        static_assert(sizeof(cNWStringArray) / sizeof(const char*) == sizeof(cIndexToEnumArray) / sizeof(UnderlyingType)); \
    }; \
    extern const ::vp::util::TFixedValueDictionary< enum_name##Traits::UnderlyingType, sizeof( enum_name##Traits::cNWStringArray) / sizeof(const char*)> c##enum_name##EnumDictionary; \
    extern const ::vp::util::EnumDictionaryNode s##enum_name##DicNode; \
    constexpr bool enum_name##Traits::GetEnumValueByString(EnumType *out_value, const char *enum_string) { \
        if (std::is_constant_evaluated() == true) { \
            for (u32 i = 0; i < cEnumCount; ++i) { if (::vp::util::TStringCompare(enum_string, cIndexToStringArray[i], 0xffff'ffff) == true) { *out_value = static_cast<EnumType>(cIndexToEnumArray[i]); return true; } } \
            return false; \
        } else { \
            UnderlyingType value = 0; \
            const bool result = c##enum_name##EnumDictionary.GetEnumValueByName(std::addressof(value), enum_string); \
            *out_value = static_cast<EnumType>(value); \
            return result; \
        } \
    } \
    constexpr bool enum_name##Traits::GetEnumValueByString(UnderlyingType *out_value, const char *enum_string) { \
        if (std::is_constant_evaluated() == true) { \
            for (u32 i = 0; i < cEnumCount; ++i) { if (::vp::util::TStringCompare(enum_string, cIndexToStringArray[i], 0xffff'ffff) == true) { *out_value = static_cast<UnderlyingType>(cIndexToEnumArray[i]); return true; } } \
            return false; \
        } else { \
            const bool result = c##enum_name##EnumDictionary.GetEnumValueByName(out_value, enum_string); \
            return result; \
        } \
    }

#define VP_ENUM_TRAITS_NW_STR(_arg, _num) \
    static constexpr const char *cString##_num = TOSTRING( _arg ); \
    static constexpr const ::vp::util::TFixedNintendoWareString<vp::util::TStringLength( TOSTRING( _arg ) ,0xffff'ffff)>  cNWString##_num { cString##_num };
