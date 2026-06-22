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
#define VP_DECLARE_ENUM_DICTIONARY(lookup_name, enum_type) \
    static constexpr const ::vp::util::EnumDictionary<typename ::vp::util::TEnumTraits<enum_type>::UnderlyingType> _c##enum_type##TraitsEnumDictionary( TOSTRING( lookup_name ), ::vp::util::TEnumTraits<enum_type>::cIndexToHashArray.data(), ::vp::util::TEnumTraits<enum_type>::cIndexToStringArray.data(), ::vp::util::TEnumTraits<enum_type>::cIndexToEnumArray.data(), ::vp::util::TEnumTraits<enum_type>::cEnumCount); \

#define VP_DECLARE_ENUM_DICTIONARY_IMPL(enum_type) \
    const ::vp::util::EnumDictionaryNode _s##enum_type##DicNode(std::addressof( _c##enum_type##TraitsEnumDictionary ));

namespace vp::util {
    
    namespace impl {

        template <bool IsDummy, typename T>
        struct _EnumUnderlyingTypeHelper;

        template <typename T>
        struct _EnumUnderlyingTypeHelper<false, T> {
            using UnderlyingType = int;
        };
        template <typename T>
        struct _EnumUnderlyingTypeHelper<true, T> {
            using UnderlyingType = std::underlying_type_t<T>;
        };

        template <typename T>
        using EnumUnderlyingType = _EnumUnderlyingTypeHelper<std::is_enum_v<T>,T>::UnderlyingType;

        template <typename T>
            requires (std::is_enum_v<T>)
        consteval u32 CountEnumFields() {
            return std::meta::enumerators_of(^^T).size();
        }

        template <typename T>
        consteval u32 CountEnumFields() {
            return 0;
        }

        template <typename T>
            requires (std::is_enum_v<T>)
        consteval std::array<const char*, CountEnumFields<T>()> BuildEnumFieldNameArray() {
            std::array<const char*,CountEnumFields<T>()> arr;
            u32 i = 0;
            for (std::meta::info inf : std::meta::enumerators_of(^^T)) {
                arr[i] = std::meta::identifier_of(inf).data();
                ++i;
            }
            return arr;
        }

        template <typename T>
        consteval std::array<const char*, CountEnumFields<T>()> BuildEnumFieldNameArray() {
            return {};
        }

        template <typename T>
            requires (std::is_enum_v<T>)
        consteval std::array<T, CountEnumFields<T>()> BuildEnumFieldValueArray() {
            std::array<T,CountEnumFields<T>()> arr;
            u32 i = 0;
            for (std::meta::info inf : std::meta::enumerators_of(^^T)) {
                arr[i] = static_cast<T>(std::meta::extract<T>(inf));
                ++i;
            }
            return arr;
        }
        template <typename T>
        consteval std::array<T, CountEnumFields<T>()> BuildEnumFieldValueArray() {
            return {};
        }

        template <size_t Size>
        constexpr std::array<u32,Size> GenerateEnumValueHashArray(const std::array<const char*,Size> &string_array) {
            std::array<u32,Size> ret;
            for (u32 i = 0; i < Size; ++i) {
                ret[i] = vp::util::HashMurmur3(string_array[i]);
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

    template <typename T>
        requires (std::is_enum<T>::value)
    struct TEnumTraits {
        using EnumType = T;
        using UnderlyingType = impl::EnumUnderlyingType<T>;
        static constexpr u32 cInvalidEntryIndex = 0xffff'ffff;
        static constexpr const char *cClassName = std::meta::display_string_of(^^T).data();
        static constexpr const u32 cEnumCount = impl::CountEnumFields<EnumType>();
        static constexpr bool cIsDummy = (cEnumCount == 0);
        static constexpr const std::array<const char*,cEnumCount> cIndexToStringArray = impl::BuildEnumFieldNameArray<T>();
        static constexpr const std::array<EnumType,cEnumCount> cIndexToEnumArray = impl::BuildEnumFieldValueArray<T>();
        static constexpr const std::array<u32,cEnumCount> cIndexToHashArray = impl::GenerateEnumValueHashArray<cEnumCount>(cIndexToStringArray);
        static_assert(impl::ValidateEnumHashArray<cEnumCount>(cIndexToHashArray));
        static constexpr u32 GetIndexByValue(EnumType enum_value) {
            u32 i = 0;
            for (; i < cEnumCount; ++i) {
                if (cIndexToEnumArray[i] == enum_value) { return i; }
            }
            return cInvalidEntryIndex;
        }
        static constexpr const char *GetStringByIndex(u32 index) { return (index < cEnumCount) ? cIndexToStringArray[index] : nullptr; }
        static constexpr const char *GetStringByValue(EnumType enum_value) { return cIndexToStringArray[GetIndexByValue(enum_value)]; }
        static constexpr bool GetValueByString(EnumType *out_value, const char *enum_string) {
            const u32 hash = vp::util::HashMurmur3(enum_string);
            return GetValueByHash(out_value, hash);
        }
        static constexpr bool GetValueByString(UnderlyingType *out_value, const char *enum_string) {
            const u32 hash = vp::util::HashMurmur3(enum_string);
            return GetValueByHash(out_value, hash);
        }
        static constexpr bool GetValueByHash(EnumType *out_value, u32 enum_hash) {
            for (u32 i = 0; i < cEnumCount; ++i) {
                if (cIndexToHashArray[i] == enum_hash) {
                    *out_value = cIndexToEnumArray[i];
                    return true;
                }
            }
            return false;
        }
        static constexpr bool GetValueByHash(UnderlyingType *out_value, u32 enum_hash) {
            for (u32 i = 0; i < cEnumCount; ++i) {
                if (cIndexToHashArray[i] == enum_hash) {
                    *out_value = static_cast<UnderlyingType>(cIndexToEnumArray[i]);
                    return true;
                }
            }
            return false;
        }
        static constexpr EnumType GetValueByIndex(u32 index) { VP_ASSERT(index < cEnumCount); return cIndexToEnumArray[index]; }
    };

    enum class EnumType : u32 {
        S8,
        S16,
        S32,
        S64,
        U8,
        U16,
        U32,
        U64,
        Bool,
    };

    constexpr bool IsSigned(EnumType enum_type) {
        return static_cast<u32>(enum_type) < 4;
    }
    constexpr bool IsUnsigned(EnumType enum_type) {
        return static_cast<u32>(enum_type) - 4 < 4;
    }

    class EnumDictionaryBase {
        public:
            static constexpr u32 cInvalidEntryIndex = 0xffff'ffff;
        public:
            EnumType m_enum_type;
            u32 m_enum_count;
            const char *m_class_name;
            const u32 *m_hash_array;
            const char * const *m_string_array;
        public:
            constexpr EnumDictionaryBase(const char *class_name, const u32 *hash_array, const char * const *string_array, u32 enum_count, EnumType enum_type) : m_enum_type(enum_type), m_enum_count(enum_count), m_class_name(class_name), m_hash_array(hash_array), m_string_array(string_array) {/*...*/}
            virtual constexpr ~EnumDictionaryBase() {/*...*/}

            constexpr u32 GetEnumIndexByName(const char *enum_value_name) const {
                const u32 hash = vp::util::HashMurmur3(enum_value_name);
                return this->GetEnumIndexByHash(hash);
            }
            constexpr u32 GetEnumIndexByHash(u32 enum_value_hash) const {
                const u32 *hash_array = m_hash_array;
                for (u32 i = 0; i < m_enum_count; ++i) {
                    if (enum_value_hash == hash_array[i]) { return i; }
                }
                return cInvalidEntryIndex;
            }
            const char *GetEnumNameByIndex(u32 index) const {
                return m_string_array[index];
            }
            constexpr u32 GetEnumCount() const {
                return m_enum_count;
            }
            constexpr const char * GetClassName() const {
                return m_class_name;
            }
            constexpr EnumType GetEnumType() const {
                return m_enum_type;
            }
    };

    template <typename T>
        requires (!std::is_enum<T>::value && std::is_integral_v<T> && sizeof(T) <= 8 && vp::util::IsAlignedPow2(sizeof(T)))
    class EnumDictionary : public EnumDictionaryBase {
        public:
            /* Pun to closest integral type */
            static constexpr EnumType cEnumType = (std::is_same_v<std::remove_cv_t<T>, bool>) ? EnumType::Bool : static_cast<EnumType>(vp::util::CountRightZeroBits(sizeof(T)) + std::is_unsigned_v<T> * 4);
        public:
            const T *m_value_array;
        public:
            constexpr EnumDictionary(const char *class_name, const u32 *hash_array, const char * const *string_array, const T *value_array, u32 enum_count) : EnumDictionaryBase(class_name, hash_array, string_array, enum_count, cEnumType), m_value_array(value_array) {/*...*/}
            virtual constexpr ~EnumDictionary() override {/*...*/}

            constexpr bool GetEnumIndexByValue(u32 *out_index, T enum_value) const {
                for (u32 i = 0; i < this->GetEnumCount(); ++i) {
                    if (enum_value == m_value_array[i]) { *out_index = i; return true; }
                }
                return false;
            }
            constexpr bool GetEnumValueByName(T *out_value, const char *enum_value_name) const {
                const u32 index = this->GetEnumIndexByName(enum_value_name);
                if (index == cInvalidEntryIndex) { return false; }
                *out_value = static_cast<T>(m_value_array[index]);
                return true;
            }
            constexpr bool GetEnumValueByHash(T *out_value, u32 enum_value_hash) const {
                const u32 index = this->GetEnumIndexByHash(enum_value_hash);
                if (index == cInvalidEntryIndex) { return false; }
                *out_value = static_cast<T>(m_value_array[index]);
                return true;
            }
            constexpr bool GetEnumValueByIndex(T *out_value, u32 index) const {
                VP_ASSERT(index < this->GetEnumCount());
                *out_value = static_cast<T>(m_value_array[index]);
                return true;
            }
    };

    template <typename T>
    bool GetAndCastEnumValueByIndex(T *out_value, const EnumDictionaryBase *base, u32 value_id) {
        if (base->GetEnumCount() <= value_id) { return false; }
        switch (base->GetEnumType()) {
            case EnumType::S8:
            {
                *out_value = static_cast<T>(reinterpret_cast<const EnumDictionary<s8>*>(base)->m_value_array[value_id]);
                return true;
            }
            case EnumType::U8:
            {
                *out_value = static_cast<T>(reinterpret_cast<const EnumDictionary<u8>*>(base)->m_value_array[value_id]);
                return true;
            }
            case EnumType::S16:
            {
                *out_value = static_cast<T>(reinterpret_cast<const EnumDictionary<s16>*>(base)->m_value_array[value_id]);
                return true;
            }
            case EnumType::U16:
            {
                *out_value = static_cast<T>(reinterpret_cast<const EnumDictionary<u16>*>(base)->m_value_array[value_id]);
                return true;
            }
            case EnumType::S32:
            {
                *out_value = static_cast<T>(reinterpret_cast<const EnumDictionary<s32>*>(base)->m_value_array[value_id]);
                return true;
            }
            case EnumType::U32:
            {
                *out_value = static_cast<T>(reinterpret_cast<const EnumDictionary<u32>*>(base)->m_value_array[value_id]);
                return true;
            }
            case EnumType::S64:
            {
                *out_value = static_cast<T>(reinterpret_cast<const EnumDictionary<s64>*>(base)->m_value_array[value_id]);
                return true;
            }
            case EnumType::U64:
            {
                *out_value = static_cast<T>(reinterpret_cast<const EnumDictionary<u64>*>(base)->m_value_array[value_id]);
                return true;
            }
            case EnumType::Bool:
            {
                *out_value = static_cast<T>(reinterpret_cast<const EnumDictionary<bool>*>(base)->m_value_array[value_id]);
                return true;
            }
            default:
                break;
        };
        return false;
    }

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
            constexpr void Insert(EnumDictionaryNode *dic_node) {
                m_tree.Insert(dic_node);
            }
        public:
            constexpr EnumDictionaryManager() : m_tree() {/*...*/}
            constexpr ~EnumDictionaryManager() {/*...*/}

            constexpr const EnumDictionaryBase *GetEnumDictionaryBase(const char *class_name) const {
                const u64 class_hash = vp::util::HashMurmur64(class_name);
                const EnumDictionaryNode *dic_node = m_tree.Find(class_hash);
                return (dic_node == nullptr) ? nullptr : dic_node->m_dictionary;
            }
            template <typename T>
                requires (!std::is_enum<T>::value && std::is_integral_v<T> && sizeof(T) <= 8 && vp::util::IsAlignedPow2(sizeof(T)))
            constexpr const EnumDictionary<T> *GetEnumDictionary(const char *class_name) const {
                const EnumDictionaryBase *dic_node = this->GetEnumDictionaryBase(class_name);
                if (dic_node == nullptr) { return nullptr; }
                static constexpr EnumType cEnumType = static_cast<EnumType>((sizeof(T) / 2) + std::is_unsigned_v<T> * 4);
                if (dic_node->GetEnumType() != cEnumType) { return nullptr; }
                return reinterpret_cast<const EnumDictionary<T>*>(dic_node);
            }
    };

    const EnumDictionaryManager *GetEnumDictionaryManager();

    ALWAYS_INLINE const EnumDictionaryBase *FindEnumDictionaryBase(const char *class_name) {
        const EnumDictionaryManager *mgr = GetEnumDictionaryManager();
        return mgr->GetEnumDictionaryBase(class_name);
    }
    template <typename T>
        requires (!std::is_enum<T>::value && std::is_integral_v<T> && sizeof(T) <= 8 && vp::util::IsAlignedPow2(sizeof(T)))
    ALWAYS_INLINE const EnumDictionary<T> *FindEnumDictionary(const char *class_name) {
        const EnumDictionaryManager *mgr = GetEnumDictionaryManager();
        return mgr->GetEnumDictionary<T>(class_name);
    }
}
