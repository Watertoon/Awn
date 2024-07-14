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

	class TypedParamArrayBase : public Composite {
        public:
            using PointerArray = vp::util::PointerArray<TypedParam>;
		private:
            PointerArray         m_array;
            TypedParamArrayBase *m_parent;
            u32                  m_is_allow_parent;
            u32                  m_typed_param_max;
        public:                 
            VP_RTTI_DERIVED(TypedParamArrayBase, Composite);
        private:
            void FinalizeTypedParamImpl(u32 base, u32 count);
            bool InitializeTypedParam(mem::Heap *heap, Document *document, u32 index, u64 class_hash, bool is_no_inherit_previous_parent);
		public:
            constexpr TypedParamArrayBase() : m_array(), m_parent(), m_is_allow_parent(), m_typed_param_max() {/*...*/}
            virtual ~TypedParamArrayBase() override {/*...*/}

            ALWAYS_INLINE PointerArray::Iterator begin() {
                return m_array.begin();
            }
            ALWAYS_INLINE PointerArray::Iterator end() {
                return m_array.end();
            }

            virtual void InitializeBgyml(mem::Heap *heap, Document *document, vp::res::ByamlIterator byaml_iterator) override;
            virtual void InitializeDefaults(mem::Heap *heap, bool is_unlink_parent) override;

            virtual void Finalize() override;

            virtual void ResolveParent(Document *document, TypedParam *parent_typed_param, const char *field_name) override;

            virtual Result FindComposite(FindCompositeContext *out_find_info, const char *search_string) override;

            virtual u64 GetBaseClassHash() const = 0;
            virtual TypedParam *CreateTypedParam(mem::Heap *heap, Document *document, u64 class_hash) const = 0;
	};

	template <typename T>
		requires (TypedParam::CheckRuntimeTypeInfoStatic(T::GetRuntimeTypeInfoStatic()) == true)
	class TypedParamArray : public TypedParamArrayBase {
        public:
            VP_RTTI_DERIVED(TypedParamArray, Composite);
        public:
            constexpr TypedParamArray() : TypedParamArrayBase() {/*...*/}
            virtual ~TypedParamArray() override {/*...*/}

            virtual u64 GetBaseClassHash() const override { return T::GetClassHashStatic(); }
            virtual TypedParam *CreateTypedParam(mem::Heap *heap, [[maybe_unused]] Document *document, u64 class_hash) const override {
                TypedParam *typed_param = para::CreateTypedParam(heap, class_hash);
                VP_ASSERT(T::CheckRuntimeTypeInfoStatic(typed_param->GetRuntimeTypeInfo()) == true);
                return typed_param;
            }
    };
}
