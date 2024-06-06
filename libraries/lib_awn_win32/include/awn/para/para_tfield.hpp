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

    template <typename T>
        requires (vp::res::TGetByamlDataTypeValue<T>() != vp::res::ByamlDataType::Null || T::cIsPropertyForTypedParam)
    consteval bool TIsProperty() { return true; }

    template <typename T>
    consteval bool TIsProperty() { return false; }

    template <typename T>
        requires (std::is_pointer<T>::value && TypedParam::CheckRuntimeTypeInfoStatic(std::remove_pointer_t<T>::GetRuntimeTypeInfoStatic()))
    consteval bool TIsEmbed() { return true; }

    template <typename T>
    consteval bool TIsEmbed() { return false; }

    template <typename T>
        requires ((!std::is_pointer<T>::value && TypedParam::CheckRuntimeTypeInfoStatic(std::remove_pointer_t<T>::GetRuntimeTypeInfoStatic()) == false && Composite::CheckRuntimeTypeInfoStatic(T::GetRuntimeTypeInfoStatic())) || (std::is_pointer<T>::value && TypedParam::CheckRuntimeTypeInfoStatic(std::remove_pointer_t<T>::GetRuntimeTypeInfoStatic())))
    consteval bool TIsComposite() { return true; }

    template <typename T>
    consteval bool TIsComposite() { return false; }

    template <typename T>
    consteval bool TIsField() { return TIsProperty<T>() || TIsEmbed<T>() || TIsComposite<T>(); }

    template <typename T>
    constexpr const char *TGetEmbedClassName() {
        return "";
    }

    template <typename T>
        requires (TIsEmbed<T>())
    constexpr const char *TGetEmbedClassName() {
        return T::cClassName();
    }

    template <typename T>
    constexpr const vp::util::RuntimeTypeInfo *TGetCompositeRtti() {
        return nullptr;
    }

    template <typename T>
        requires (TIsEmbed<T>() || TIsComposite<T>())
    constexpr const vp::util::RuntimeTypeInfo *TGetCompositeRtti() {
        return T::GetRuntimeTypeInfoStatic();
    }

    #define AWN_PARA_LAYOUT_TYPE_TRAITS(layout_type_name) \
        static constexpr bool cIsLayoutForTypedParam = true; \
        template <auto T> \
        static constexpr auto TGetPropertyDefault() { return 0; } \
        template <auto T> \
            requires (TIsProperty<vp::util::MemberType<T>>() && std::is_standard_layout<vp::util::MemberType<T>>::value) \
        static constexpr auto TGetPropertyDefault() { \
            const layout_type_name layout = {}; \
            return layout.*T; \
        }

    template <typename T>
    struct PropertyAccessorHelper {
        using Type = IPropertyAccessor;
    };

    template <>
    struct PropertyAccessorHelper<bool> {
        using Type = PropertyAccessor<bool>;
    };

    template <>
    struct PropertyAccessorHelper<s32> {
        using Type = PropertyAccessor<s32>;
    };

    template <>
    struct PropertyAccessorHelper<u32> {
        using Type = PropertyAccessor<u32>;
    };

    template <>
    struct PropertyAccessorHelper<float> {
        using Type = PropertyAccessor<float>;
    };

    template <>
    struct PropertyAccessorHelper<s64> {
        using Type = PropertyAccessor<s64>;
    };

    template <>
    struct PropertyAccessorHelper<u64> {
        using Type = PropertyAccessor<u64>;
    };

    template <>
    struct PropertyAccessorHelper<double> {
        using Type = PropertyAccessor<double>;
    };

    template <>
    struct PropertyAccessorHelper<const char*> {
        using Type = PropertyAccessor<const char*>;
    };

    template <typename T>
    using TPropertyAccessor = PropertyAccessorHelper<T>::Type;

    template <typename T>
    constexpr TPropertyAccessor<T> TGetPropertyAccessor(T *value_address) {
        static_assert(false, "Property type is invalid and not dummyable");
        const IPropertyAccessor accessor{};
        return accessor;
    }
    template <typename T>
        requires (TIsEmbed<T>() || TIsComposite<T>())
    constexpr TPropertyAccessor<T> TGetPropertyAccessor(T *value_address) {
        VP_UNUSED(value_address);
        const IPropertyAccessor accessor{};
        return accessor;
    }
    template <typename T>
        requires (TIsProperty<T>())
    constexpr TPropertyAccessor<T> TGetPropertyAccessor(T *value_address) {
        const TPropertyAccessor<T> accessor(value_address);
        return accessor;
    }

    /* Ensure common cases */
    static_assert(TIsProperty<const char*>()  == true);
    static_assert(TIsEmbed<const char*>()     == false);
    static_assert(TIsComposite<const char*>() == false);
    static_assert(TIsProperty<PropertyMap<const char*>>()  == false);
    static_assert(TIsEmbed<PropertyMap<const char*>>()     == false);
    static_assert(TIsComposite<PropertyMap<const char*>>() == true);
    static_assert(TIsProperty<PropertyArray<const char*>>()  == false);
    static_assert(TIsEmbed<PropertyArray<const char*>>()     == false);
    static_assert(TIsComposite<PropertyArray<const char*>>() == true);

    static_assert(TIsProperty<bool>()  == true);
    static_assert(TIsEmbed<bool>()     == false);
    static_assert(TIsComposite<bool>() == false);
    static_assert(TIsProperty<PropertyMap<bool>>()  == false);
    static_assert(TIsEmbed<PropertyMap<bool>>()     == false);
    static_assert(TIsComposite<PropertyMap<bool>>() == true);
    static_assert(TIsProperty<PropertyArray<bool>>()  == false);
    static_assert(TIsEmbed<PropertyArray<bool>>()     == false);
    static_assert(TIsComposite<PropertyArray<bool>>() == true);

    static_assert(TIsProperty<s32>()  == true);
    static_assert(TIsEmbed<s32>()     == false);
    static_assert(TIsComposite<s32>() == false);
    static_assert(TIsProperty<PropertyMap<s32>>()  == false);
    static_assert(TIsEmbed<PropertyMap<s32>>()     == false);
    static_assert(TIsComposite<PropertyMap<s32>>() == true);
    static_assert(TIsProperty<PropertyArray<s32>>()  == false);
    static_assert(TIsEmbed<PropertyArray<s32>>()     == false);
    static_assert(TIsComposite<PropertyArray<s32>>() == true);

    static_assert(TIsProperty<u32>()  == true);
    static_assert(TIsEmbed<u32>()     == false);
    static_assert(TIsComposite<u32>() == false);
    static_assert(TIsProperty<PropertyMap<u32>>()  == false);
    static_assert(TIsEmbed<PropertyMap<u32>>()     == false);
    static_assert(TIsComposite<PropertyMap<u32>>() == true);
    static_assert(TIsProperty<PropertyArray<u32>>()  == false);
    static_assert(TIsEmbed<PropertyArray<u32>>()     == false);
    static_assert(TIsComposite<PropertyArray<u32>>() == true);

    static_assert(TIsProperty<float>()  == true);
    static_assert(TIsEmbed<float>()     == false);
    static_assert(TIsComposite<float>() == false);
    static_assert(TIsProperty<PropertyMap<float>>()  == false);
    static_assert(TIsEmbed<PropertyMap<float>>()     == false);
    static_assert(TIsComposite<PropertyMap<float>>() == true);
    static_assert(TIsProperty<PropertyArray<float>>()  == false);
    static_assert(TIsEmbed<PropertyArray<float>>()     == false);
    static_assert(TIsComposite<PropertyArray<float>>() == true);

    static_assert(TIsProperty<s64>()  == true);
    static_assert(TIsEmbed<s64>()     == false);
    static_assert(TIsComposite<s64>() == false);
    static_assert(TIsProperty<PropertyMap<s64>>()  == false);
    static_assert(TIsEmbed<PropertyMap<s64>>()     == false);
    static_assert(TIsComposite<PropertyMap<s64>>() == true);
    static_assert(TIsProperty<PropertyArray<s64>>()  == false);
    static_assert(TIsEmbed<PropertyArray<s64>>()     == false);
    static_assert(TIsComposite<PropertyArray<s64>>() == true);

    static_assert(TIsProperty<u64>()  == true);
    static_assert(TIsEmbed<u64>()     == false);
    static_assert(TIsComposite<u64>() == false);
    static_assert(TIsProperty<PropertyMap<u64>>()  == false);
    static_assert(TIsEmbed<PropertyMap<u64>>()     == false);
    static_assert(TIsComposite<PropertyMap<u64>>() == true);
    static_assert(TIsProperty<PropertyArray<u64>>()  == false);
    static_assert(TIsEmbed<PropertyArray<u64>>()     == false);
    static_assert(TIsComposite<PropertyArray<u64>>() == true);

    static_assert(TIsProperty<double>()  == true);
    static_assert(TIsEmbed<double>()     == false);
    static_assert(TIsComposite<double>() == false);
    static_assert(TIsProperty<PropertyMap<double>>()  == false);
    static_assert(TIsEmbed<PropertyMap<double>>()     == false);
    static_assert(TIsComposite<PropertyMap<double>>() == true);
    static_assert(TIsProperty<PropertyArray<double>>()  == false);
    static_assert(TIsEmbed<PropertyArray<double>>()     == false);
    static_assert(TIsComposite<PropertyArray<double>>() == true);

    static_assert(TIsProperty<TypedParam>() == false);
    static_assert(TIsEmbed<TypedParam>() == false);
    static_assert(TIsComposite<TypedParam>() == false);

    static_assert(TIsProperty<TypedParam*>() == false);
    static_assert(TIsEmbed<TypedParam*>() == true);
    static_assert(TIsComposite<TypedParam*>() == true);

    static_assert(TIsProperty<TypedParamMap<TypedParam>>() == false);
    static_assert(TIsEmbed<TypedParamMap<TypedParam>>() == false);
    static_assert(TIsComposite<TypedParamMap<TypedParam>>() == true);

    static_assert(TIsProperty<TypedParamArray<TypedParam>>() == false);
    static_assert(TIsEmbed<TypedParamArray<TypedParam>>() == false);
    static_assert(TIsComposite<TypedParamArray<TypedParam>>() == true);
}
