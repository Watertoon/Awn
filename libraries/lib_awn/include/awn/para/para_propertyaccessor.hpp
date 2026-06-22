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

namespace awn::para {

    using PropertyString = vp::util::FixedString8<0x100>;

    class IPropertyAccessor {
        public:
            constexpr IPropertyAccessor() {/*...*/}
            constexpr IPropertyAccessor([[maybe_unused]] void *dummy) {/*...*/}
            virtual constexpr ~IPropertyAccessor() {/*...*/}

            virtual bool  LoadByamlIterator(vp::res::ByamlIterator iter) { VP_UNUSED(iter); VP_UNEXPECTED_DEFAULT(); return false; }
#ifdef VP_ENABLE_LIBYAML
            virtual bool LoadYamlIterator(vp::res::YamlIterator iter) { VP_UNUSED(iter); VP_UNEXPECTED_DEFAULT(); return false; }
#endif
            virtual u32   Format(char *out_string, u32 string_length) { VP_UNUSED(out_string, string_length); VP_UNEXPECTED_DEFAULT(); return 0; }

            virtual void  ClearValue() { VP_UNEXPECTED_DEFAULT(); }
            virtual void  SetValue(const void *value_addr) { VP_UNUSED(value_addr); VP_UNEXPECTED_DEFAULT(); }

            virtual constexpr void *GetValueAddress() { return nullptr; }
            virtual constexpr u32 GetValueSize()      { return 0; }
    };

    #define AWN_PARA_PROPERTY_ACCESSOR_CUSTOM_TYPE_TRAITS(class_name) \
        public: \
            static constexpr bool cIsCustomTypeForPropertyAccessor = true

    template <typename T>
    class PropertyAccessor : public IPropertyAccessor {
        private:
            T *m_value_address;
        public:
            ALWAYS_INLINE bool LoadByamlIterImpl(vp::res::ByamlIterator iter) { VP_UNUSED(iter); return false; }
            ALWAYS_INLINE bool LoadByamlIterImpl(vp::res::ByamlIterator iter) requires (vp::res::TGetByamlDataTypeValue<T>() != vp::res::ByamlDataType::Null) {
                return iter.TryGetValueByIterator<T>(m_value_address);
            }
            ALWAYS_INLINE bool LoadByamlIterImpl(vp::res::ByamlIterator iter) requires (T::cIsVectorType && T::cCount <= 4) {

                if (!iter.IsValid()) { return false; }
                if constexpr (0 < T::cCount) {
                    const bool result = iter.TryGetValueByKey<typename T::UnderlyingType>(std::addressof((*m_value_address)[0]), "X");
                    if (!result) { return false; }
                }
                if constexpr (1 < T::cCount) {
                    const bool result = iter.TryGetValueByKey<typename T::UnderlyingType>(std::addressof((*m_value_address)[1]), "Y");
                    if (!result) { return false; }
                }
                if constexpr (2 < T::cCount) {
                    const bool result = iter.TryGetValueByKey<typename T::UnderlyingType>(std::addressof((*m_value_address)[2]), "Z");
                    if (!result) { return false; }
                }
                if constexpr (3 < T::cCount) {
                    const bool result = iter.TryGetValueByKey<typename T::UnderlyingType>(std::addressof((*m_value_address)[3]), "W");
                    if (!result) { return false; }
                }

                return true;
            }
            ALWAYS_INLINE bool LoadByamlIterImpl(vp::res::ByamlIterator iter) requires (!vp::util::TEnumTraits<T>::cIsDummy) {
                u32 hash = 0;
                const bool is_u32 = iter.TryGetU32ByIterator(std::addressof(hash));
                if (is_u32) {
                    return vp::util::TEnumTraits<T>::Traits::GetEnumValueByHash(reinterpret_cast<vp::util::TEnumTraits<T>::Traits::UnderlyingType*>(m_value_address), hash);
                }
                const char *string = "";
                const bool is_str = iter.TryGetStringByIterator(std::addressof(string));
                if (is_str) {
                    return vp::util::TEnumTraits<T>::Traits::GetEnumValueByString(reinterpret_cast<vp::util::TEnumTraits<T>::Traits::UnderlyingType*>(m_value_address), string);
                }
                return false;
            }
            ALWAYS_INLINE bool LoadByamlIterImpl(vp::res::ByamlIterator iter) requires (T::IsCustomTypeForPropertyAccessor) {
                return m_value_address->LoadByamlIterator(iter);
            }
#ifdef VP_ENABLE_LIBYAML
        public:
            ALWAYS_INLINE bool LoadYamlIterImpl(vp::res::YamlIterator iter) { VP_UNUSED(iter); return false; }
            ALWAYS_INLINE bool LoadYamlIterImpl(vp::res::YamlIterator iter) requires (vp::res::TGetByamlDataTypeValue<T>() != vp::res::ByamlDataType::Null) {
                return iter.TryGetValueByIterator<T>(m_value_address);
            }
            ALWAYS_INLINE bool LoadYamlIterImpl(vp::res::YamlIterator iter) requires (T::cIsVectorType && T::cCount <= 4) {

                if (!iter.IsValid()) { return false; }
                if constexpr (0 < T::cCount) {
                    const bool result = iter.TryGetValueByKey<typename T::UnderlyingType>(std::addressof((*m_value_address)[0]), "X");
                    if (!result) { return false; }
                }
                if constexpr (1 < T::cCount) {
                    const bool result = iter.TryGetValueByKey<typename T::UnderlyingType>(std::addressof((*m_value_address)[1]), "Y");
                    if (!result) { return false; }
                }
                if constexpr (2 < T::cCount) {
                    const bool result = iter.TryGetValueByKey<typename T::UnderlyingType>(std::addressof((*m_value_address)[2]), "Z");
                    if (!result) { return false; }
                }
                if constexpr (3 < T::cCount) {
                    const bool result = iter.TryGetValueByKey<typename T::UnderlyingType>(std::addressof((*m_value_address)[3]), "W");
                    if (!result) { return false; }
                }

                return true;
            }
            ALWAYS_INLINE bool LoadYamlIterImpl(vp::res::YamlIterator iter) requires (!vp::util::TEnumTraits<T>::cIsDummy) {
                u32 hash = 0;
                const bool is_u32 = iter.TryGetU32ByIterator(std::addressof(hash));
                if (is_u32) {
                    return vp::util::TEnumTraits<T>::Traits::GetEnumValueByHash(reinterpret_cast<vp::util::TEnumTraits<T>::Traits::UnderlyingType*>(m_value_address), hash);
                }
                const char *string = "";
                const bool is_str = iter.TryGetStringByIterator(std::addressof(string));
                if (is_str) {
                    return vp::util::TEnumTraits<T>::Traits::GetEnumValueByString(reinterpret_cast<vp::util::TEnumTraits<T>::Traits::UnderlyingType*>(m_value_address), string);
                }
                return false;
            }
            ALWAYS_INLINE bool LoadYamlIterImpl(vp::res::YamlIterator iter) requires (T::IsCustomTypeForPropertyAccessor) {
                return m_value_address->LoadByamlIterator(iter);
            }
#endif
        public:
            constexpr PropertyAccessor(T *value_addr) : IPropertyAccessor(), m_value_address(value_addr) {/*...*/}
            virtual constexpr ~PropertyAccessor() override {/*...*/}

            virtual bool LoadByamlIterator(vp::res::ByamlIterator iter) override {
                return this->LoadByamlIterImpl(iter);
            }
#ifdef VP_ENABLE_LIBYAML
            virtual bool LoadYamlIterator(vp::res::YamlIterator iter) override {
                return this->LoadYamlIterImpl(iter);
            }
#endif

            virtual u32 Format(char *out_string, u32 string_length) override {
                VP_UNUSED(out_string, string_length);
                return 0;
            }
            virtual void ClearValue()                     override { *m_value_address = T{}; }
            virtual void SetValue(const void *value_addr) override { *m_value_address = *reinterpret_cast<const T*>(value_addr); }

            virtual constexpr void *GetValueAddress() override { return m_value_address; }
            virtual constexpr u32   GetValueSize()    override { return sizeof(T); }
    };
}
