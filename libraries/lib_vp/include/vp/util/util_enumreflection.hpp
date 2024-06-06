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
    const ::vp::res::TFixedValueDictionary<sizeof( enum_name##Traits::cNWStringArray) / sizeof(const char*)> s##enum_name##EnumDictionary{ enum_name##Traits::cClassName, enum_name##Traits::cNWStringArray, enum_name##Traits::cIndexToEnumArray}; \
    const ::vp::res::EnumDictionaryNode s##enum_name##DicNode(std::addressof(s##enum_name##EnumDictionary));

namespace vp::res {

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

    class EnumDictionary {
        protected:
            const char                          *m_class_name;
            const u32                           *m_value_array;
            vp::res::ResNintendoWareDictionary   m_res_dic;
        public:
            constexpr EnumDictionary(const char *class_name, const u32 *value_array) : m_class_name(class_name), m_value_array(value_array), m_res_dic() {/*...*/}
            constexpr ~EnumDictionary() {/*...*/}

            ALWAYS_INLINE bool TryGetValueByName(u32 *out_value, const char *enum_value_name) const {
                const u32 index = m_res_dic.TryGetEntryIndexByKey(enum_value_name);
                if (index == vp::res::ResNintendoWareDictionary::cInvalidEntryIndex) { return false; }
                *out_value = m_value_array[index];
                return true;
            }
            constexpr ALWAYS_INLINE u32 GetEnumCount() const {
                return m_res_dic.node_count;
            }
            constexpr ALWAYS_INLINE const char * GetClassName() const {
                return m_class_name;
            }
    };

    template <size_t Size>
    class TFixedValueDictionary : public EnumDictionary {
        private:
            vp::res::ResNintendoWareDictionary::ResDicNode m_entry_array[Size];
        public:
            TFixedValueDictionary(const char *class_name, const char * const *nw_string_array, const u32 *value_array) : EnumDictionary(class_name, value_array), m_entry_array{} {

                /* Populate entries */
                for (u32 i = 0; i < Size; ++i) {
                    m_entry_array[i].key = nw_string_array[i];
                }

                /* Set res dic meta data */
                m_res_dic.magic         = vp::res::ResNintendoWareDictionary::cMagic;
                m_res_dic.node_count    = Size;
                m_res_dic.root_node.key = "\0\0\0";

                /* Build res dic */
                const bool result = m_res_dic.Build();
                VP_ASSERT(result == true);

                return;
            }
            constexpr ~TFixedValueDictionary() {/*...*/}
    };

    class EnumDictionaryManager;

    class EnumDictionaryNode {
        public:
            friend EnumDictionaryManager;
        private:
            vp::util::IntrusiveRedBlackTreeNode<u64>  m_tree_node;
            const EnumDictionary                     *m_dictionary;
        public:
            EnumDictionaryNode(const EnumDictionary *enum_dictionary);
            constexpr ~EnumDictionaryNode() {/*...*/}
    };

    class EnumDictionaryManager {
        public:
            friend EnumDictionaryNode;
        public:
            using EnumDictionaryTree = vp::util::IntrusiveRedBlackTreeTraits<EnumDictionaryNode, &EnumDictionaryNode::m_tree_node>::Tree;
        private:
            EnumDictionaryTree m_tree;
        private:
            void Insert(EnumDictionaryNode *dic_node) {
                m_tree.Insert(dic_node);
            }
        public:
            constexpr EnumDictionaryManager() : m_tree() {/*...*/}
            constexpr ~EnumDictionaryManager() {/*...*/}

            const EnumDictionary *GetEnumDictionary(const char *class_name) const {
                const u64                 class_hash = vp::util::HashMurmur64(class_name);
                const EnumDictionaryNode *dic_node   = m_tree.Find(class_hash);
                VP_ASSERT(dic_node != nullptr);
                return dic_node->m_dictionary;
            }

            u32 GetEnumValue(const char *class_name, const char *value_string) const {
                const EnumDictionary *enum_dic   = this->GetEnumDictionary(class_name);
                u32                   enum_value = 0;
                const bool            result     = enum_dic->TryGetValueByName(std::addressof(enum_value), value_string);
                VP_ASSERT(result == true);
                return enum_value;
            }
    };

    EnumDictionaryManager *GetEnumDictionaryManager();
}
