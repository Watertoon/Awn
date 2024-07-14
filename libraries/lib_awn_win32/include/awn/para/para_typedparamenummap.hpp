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

    struct TypedParamEnumEntry {
        TypedParam *typed_param;
        u8          is_non_default;
    };

    class TypedParamEnumMapBase : public Composite {
        public:
            enum class ResolveType : u32 {
                None                 = 0,
                Parent               = 1,
                ParentAndChildParent = 2,
            };
        public:
            using EntryArray = vp::util::HeapArray<TypedParamEnumEntry>;
        protected:
            EntryArray              m_typed_param_array;
            TypedParamEnumMapBase  *m_parent;
            const char *const      *m_enum_array;
            u32                     m_enum_count;
            u32                     m_resolve_type;
        public:
            VP_RTTI_DERIVED(TypedParamEnumMapBase, Composite);
        public:
            void InitializeTypedParam(mem::Heap *heap, Document *document, u32 index, u64 hash);
        public:
            constexpr TypedParamEnumMapBase(const char *const*enum_array, u32 enum_count) : Composite(), m_typed_param_array(), m_parent(), m_enum_array(enum_array), m_enum_count(enum_count), m_resolve_type(2) {/*...*/}
            virtual constexpr ~TypedParamEnumMapBase() {/*...*/}

            constexpr ALWAYS_INLINE EntryArray::Iterator begin() {
                return m_typed_param_array.begin();
            }
            constexpr ALWAYS_INLINE EntryArray::Iterator end() {
                return m_typed_param_array.end();
            }

            virtual void InitializeBgyml(mem::Heap *heap, Document *document, vp::res::ByamlIterator byaml_iterator) override;
            virtual void InitializeDefaults(mem::Heap *heap, bool is_unlink_parent) override;

            virtual void Finalize() override;

            virtual void ResolveParent(Document *document, TypedParam *parent_typed_param, const char *field_name) override;

            virtual Result FindComposite(FindCompositeContext *out_find_info, const char *search_string) override;

            virtual u64 GetBaseClassHash() const = 0;
            virtual TypedParam *CreateTypedParam(mem::Heap *heap, Document *document, u64 class_hash) const = 0;
    };

    template <typename E, typename T>
		requires (!vp::util::TEnumTraitsAuto<E>::cIsDummy && TypedParam::CheckRuntimeTypeInfoStatic(T::GetRuntimeTypeInfoStatic()) == true)
    class TypedParamEnumMap : public TypedParamEnumMapBase {
        public:
            using EnumTraits = vp::util::TEnumTraitsAuto<E>::Traits;
        public:
            VP_RTTI_DERIVED(TypedParamEnumMap, TypedParamEnumMapBase);
        public:
            constexpr TypedParamEnumMap() : TypedParamEnumMapBase(EnumTraits::cIndexToStringArray, EnumTraits::cEnumCount) {/*...*/}
            virtual constexpr ~TypedParamEnumMap() {/*...*/}

            T *GetTypedParamBase(E enum_value) {
                const u32 index = EnumTraits::GetEnumIndexByValue(enum_value);
                TypedParamEnumEntry *entry = std::addressof(m_typed_param_array[index]);
                TypedParamEnumMapBase *parent;
                while (!entry->is_non_default) {
                    parent = m_parent;
                    if (parent == nullptr) { break; }
                    entry = std::addressof(parent->m_typed_param_array[index]);
                }
                return reinterpret_cast<T*>(entry->typed_param);
            }

            template <typename O>
                requires (T::CheckRuntimeTypeInfoStatic(O::GetRuntimeTypeInfoStatic()))
            O *GetTypedParam(E enum_value) {
                TypedParam *base = this->GetTypedParamBase(enum_value);
                if (!O::CheckRuntimeTypeInfoStatic(base->GetRuntimeTypeInfo())) { return nullptr; }
                return reinterpret_cast<O*>(base);
            }

            virtual u64 GetBaseClassHash() const override { return T::GetClassHashStatic(); }
            virtual TypedParam *CreateTypedParam(mem::Heap *heap, [[maybe_unused]] Document *document, u64 class_hash) const override {
                TypedParam *typed_param = para::CreateTypedParam(heap, class_hash);
                VP_ASSERT(T::CheckRuntimeTypeInfoStatic(typed_param->GetRuntimeTypeInfo()));
                return typed_param;
            }
    };
}
