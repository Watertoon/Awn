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

    class IPropertyAccessor {
        public:
            using PropertyString = vp::util::FixedString<0x100>;
        private:
        public:
            constexpr IPropertyAccessor() {/*...*/}
            constexpr IPropertyAccessor([[maybe_unused]] void *dummy) {/*...*/}
            virtual constexpr ~IPropertyAccessor() {/*...*/}

            virtual bool  LoadByamlData(vp::res::ByamlIterator iter, vp::res::ByamlData data) { VP_UNUSED(iter, data); VP_ASSERT(false); }
            virtual u32   Format(PropertyString *property_string) { VP_UNUSED(property_string); VP_ASSERT(false); }

            virtual void  ClearValue() { VP_ASSERT(false); }
            virtual void  SetValue(const void *value_addr) { VP_UNUSED(value_addr); VP_ASSERT(false); }

            virtual constexpr void *GetValueAddress() { return nullptr; }
            virtual constexpr u32 GetValueSize()      { return 0; }
    };

    template <typename T>
        requires (vp::res::TGetByamlDataTypeValue<T>() != vp::res::ByamlDataType::Null)
    class PropertyAccessor : public IPropertyAccessor {
        private:
            T *m_value_address;
        public:
            constexpr PropertyAccessor(T *value_addr) : IPropertyAccessor(), m_value_address(value_addr) {/*...*/}
            virtual constexpr ~PropertyAccessor() override {/*...*/}

            virtual bool LoadByamlData(vp::res::ByamlIterator iter, vp::res::ByamlData data) override {
                return iter.TryGetValueByData(m_value_address, data);
            }
            virtual u32 Format(PropertyString *property_string) override {
                VP_UNUSED(property_string);
                return 0;
            }
            virtual void ClearValue()                     override { *m_value_address = T{}; }
            virtual void SetValue(const void *value_addr) override { *m_value_address = *reinterpret_cast<const T*>(value_addr); }

            virtual constexpr void *GetValueAddress() override { return m_value_address; }
            virtual constexpr u32   GetValueSize()    override { return sizeof(T); }
    };
}
