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

    using PropertyString = vp::util::FixedString<0x100>;

    class IPropertyAccessor {
        public:
            constexpr IPropertyAccessor() {/*...*/}
            constexpr IPropertyAccessor([[maybe_unused]] void *dummy) {/*...*/}
            virtual constexpr ~IPropertyAccessor() {/*...*/}

            virtual bool  LoadByamlData(vp::res::ByamlIterator iter, vp::res::ByamlData data) { VP_UNUSED(iter, data); VP_ASSERT(false); return false; }
            virtual u32   Format(char *out_string, u32 string_length) { VP_UNUSED(out_string, string_length); VP_ASSERT(false); return 0; }

            virtual void  ClearValue() { VP_ASSERT(false); }
            virtual void  SetValue(const void *value_addr) { VP_UNUSED(value_addr); VP_ASSERT(false); }

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
            ALWAYS_INLINE bool LoadByamlDataImpl(vp::res::ByamlIterator iter, vp::res::ByamlData data) { VP_UNUSED(iter, data); return false; }
            ALWAYS_INLINE bool LoadByamlDataImpl(vp::res::ByamlIterator iter, vp::res::ByamlData data) requires (vp::res::TGetByamlDataTypeValue<T>() != vp::res::ByamlDataType::Null) {
                return iter.TryGetValueByData<T>(m_value_address, data);
            }
            ALWAYS_INLINE bool LoadByamlDataImpl(vp::res::ByamlIterator iter, vp::res::ByamlData data) requires (T::cIsVectorType && T::cVectorCount <= 4) {

                vp::res::ByamlIterator sub_iter;
                const bool result0 = iter.TryGetIteratorByData(std::addressof(sub_iter), data);
                if (!result0) { return false; }
                if constexpr (0 < T::cVectorCount) {
                    const bool result = sub_iter.TryGetValueByKey<typename T::VectorType>(m_value_address->array + 0, "X");
                    if (!result) { return false; }
                }
                if constexpr (1 < T::cVectorCount) {
                    const bool result = sub_iter.TryGetValueByKey<typename T::VectorType>(m_value_address->array + 1, "Y");
                    if (!result) { return false; }
                }
                if constexpr (2 < T::cVectorCount) {
                    const bool result = sub_iter.TryGetValueByKey<typename T::VectorType>(m_value_address->array + 2, "Z");
                    if (!result) { return false; }
                }
                if constexpr (3 < T::cVectorCount) {
                    const bool result = sub_iter.TryGetValueByKey<typename T::VectorType>(m_value_address->array + 3, "W");
                    if (!result) { return false; }
                }

                return true;
            }
            ALWAYS_INLINE bool LoadByamlDataImpl(vp::res::ByamlIterator iter, vp::res::ByamlData data) requires (!vp::util::TEnumTraits<T>::cIsDummy) {
                const char *string = "";
                const bool result = iter.TryGetStringByData(std::addressof(string), data);
                if (!result) { return false; }
                return vp::util::TEnumTraits<T>::Traits::GetEnumValueByString(reinterpret_cast<vp::util::TEnumTraits<T>::Traits::UnderlyingType*>(m_value_address), string);
            }
            ALWAYS_INLINE bool LoadByamlDataImpl(vp::res::ByamlIterator iter, vp::res::ByamlData data) requires (T::IsCustomTypeForPropertyAccessor) {
                return m_value_address->LoadByamlData(iter, data);
            }
        public:
            constexpr PropertyAccessor(T *value_addr) : IPropertyAccessor(), m_value_address(value_addr) {/*...*/}
            virtual constexpr ~PropertyAccessor() override {/*...*/}

            virtual bool LoadByamlData(vp::res::ByamlIterator iter, vp::res::ByamlData data) override {
                return this->LoadByamlDataImpl(iter, data);
            }

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
