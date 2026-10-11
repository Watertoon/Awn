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

#define AWN_PARA_TYPED_PARAM_LAYOUT_TRAITS(layout_type_name) \
    static constexpr bool cIsLayoutForTypedParam = true;

#define AWN_PARA_DECLARE_TYPED_PARAM(class_name_for_mgr, class_name, base_type_name, layout_type_name) \
    class class_name : public base_type_name { \
        public: \
            using BaseType   = base_type_name ; \
            using LayoutType = layout_type_name ; \
        public: \
             static_assert(LayoutType::cIsLayoutForTypedParam); \
             static_assert(::awn::para::impl::VerifyLayoutFields<LayoutType>()); \
        public: \
            static constexpr inline const char *cClassName = TOSTRING( class_name_for_mgr ); \
            static constexpr inline u64 cClassHash = vp::util::HashMurmur64(cClassName); \
            static constexpr inline u32 cLayoutCount = std::meta::nonstatic_data_members_of(^^LayoutType, std::meta::access_context::current()).size(); \
            static constexpr inline u32 cFieldCount = BaseType::cFieldCount + cLayoutCount; \
        public: \
            VP_RTTI_DERIVED( class_name ); \
        private: \
            LayoutType m_layout; \
            u8 m_is_non_default_array[cLayoutCount]; \
        protected: \
            template <FieldNameString Name> \
            static consteval bool IsValidFieldName() { \
                return BaseType::IsValidFieldName<Name>() || ::awn::para::impl::IsValidFieldName<LayoutType,Name>(); \
            } \
            template <FieldNameString Name> \
            static consteval std::meta::info GetFieldInfoByName() { \
                if constexpr (BaseType::IsValidFieldName<Name>()) { \
                    return BaseType::GetFieldInfoByName<Name>(); \
                } else { \
                    static_assert(::awn::para::impl::IsValidFieldName<LayoutType,Name>()); \
                    u32 i = 0; \
                    for (std::meta::info &info : std::meta::nonstatic_data_members_of(^^LayoutType, std::meta::access_context::current())) { \
                        if (vp::util::StringCompare(Name.GetString(), std::meta::identifier_of(info).data())) { return info; } \
                        ++i; \
                    } \
                    VP_UNEXPECTED_DEFAULT(); \
                } \
            } \
        public: \
            template <typename T, FieldNameString Name, u32 PropType> \
            const T *GetPointerImpl() const { \
                if constexpr (BaseType::IsValidFieldName<Name>()) { \
                    return this->BaseType::GetPointerImpl<T,Name,PropType>(); \
                } else { \
                    constexpr u32 cFieldId = ::awn::para::impl::GetFieldIndexByName<LayoutType,Name>(); \
                    constexpr std::meta::info e = std::meta::nonstatic_data_members_of(^^LayoutType, std::meta::access_context::current())[cFieldId]; \
                    using MemberType = [:std::meta::type_of(e):]; \
                    if constexpr (PropType == 0) { \
                        static_assert(::awn::para::impl::TIsProperty<MemberType>()); \
                    } else if constexpr (PropType == 1) { \
                        static_assert(::awn::para::impl::TIsComposite<MemberType>()); \
                    } else { \
                        static_assert(::awn::para::impl::TIsEmbed<MemberType>()); \
                    } \
                    if (m_is_non_default_array[cFieldId] & 1) { \
                        return std::addressof(this->m_layout.[:e:]); \
                    } \
                    const class_name *out = this; \
                    const ::awn::para::TypedParam *parent_iter; \
                    while (parent_iter = out->GetParent(), parent_iter != nullptr) { \
                        if (!this->CheckRuntimeTypeInfoStatic(parent_iter) || (out = reinterpret_cast<const class_name *>(parent_iter), (out->m_is_non_default_array[cFieldId] & 1))) { \
                            break; \
                        } \
                    } \
                    return std::addressof(out->m_layout.[:e:]); \
                } \
            } \
            template <FieldNameString Name> \
            auto GetProperty() const { \
                using MemberType = [:std::meta::type_of(GetFieldInfoByName<Name>()):]; \
                return *this->GetPointerImpl<MemberType,Name,0>(); \
            } \
            template <FieldNameString Name> \
            auto GetComposite() const { \
                using MemberType = [:std::meta::type_of(GetFieldInfoByName<Name>()):]; \
                return this->GetPointerImpl<MemberType,Name,1>(); \
            } \
            template <FieldNameString Name> \
            auto GetEmbed() const { \
                using MemberType = [:std::meta::type_of(GetFieldInfoByName<Name>()):]; \
                return *this->GetPointerImpl<MemberType,Name,2>(); \
            } \
        public: \
            constexpr class_name () : BaseType(), m_layout(), m_is_non_default_array{} {/*...*/} \
            virtual ~class_name () override {/*...*/} \
            virtual constexpr const char *GetClassName() override { return cClassName; } \
        public: \
            virtual Result LoopProperties(PropertyLoopFunction *loop_function) override { \
                RESULT_TRY(this->BaseType::LoopProperties(loop_function)); \
                const ::vp::Result result = this->TypedParam::LoopPropertiesImpl<LayoutType,0,cLayoutCount>(std::addressof(m_layout), m_is_non_default_array, loop_function); \
                RESULT_TRY(result); \
                RESULT_RETURN_SUCCESS; \
            } \
            virtual Result LoopEmbeds(EmbeddedLoopFunction *loop_function) override { \
                RESULT_TRY(this->BaseType::LoopEmbeds(loop_function)); \
                const ::vp::Result result = this->TypedParam::LoopEmbedsImpl<LayoutType,0,cLayoutCount>(std::addressof(m_layout), m_is_non_default_array, loop_function); \
                RESULT_TRY(result); \
                RESULT_RETURN_SUCCESS; \
            } \
            virtual Result LoopComposites(CompositeLoopFunction *loop_function) override { \
                RESULT_TRY(this->BaseType::LoopComposites(loop_function)); \
                const ::vp::Result result = this->TypedParam::LoopCompositesImpl<LayoutType,0,cLayoutCount>(std::addressof(m_layout), m_is_non_default_array, loop_function); \
                RESULT_TRY(result); \
                RESULT_RETURN_SUCCESS; \
            } \
            virtual bool IsFieldNonDefault(const vp::util::RuntimeTypeInfo *rtti, u32 index) override { \
                if (this->CheckRuntimeTypeInfoStatic(rtti) && index < cLayoutCount) { return m_is_non_default_array[index]; } \
                return this->BaseType::IsFieldNonDefault(rtti, index); \
            } \
    }

#define AWN_PARA_DECLARE_TYPED_PARAM_IMPL(class_name) \
    static ::awn::para::impl::TypedParamFactory sTypedParamFactory_##class_name ( class_name ::cClassHash, ::awn::para::impl::CreateTypedParamImpl< class_name >)

namespace awn::para {

    static constexpr u32 cMaxTypedParamFieldNameLength = 0x100;
    using FieldNameString = vp::util::FixedString8<cMaxTypedParamFieldNameLength>;

    namespace impl {

        template <typename T>
             requires (T::cFieldOrderArray)
        consteval u32 VerifyFieldOrderArray() {
            if (!std::is_same_v<u32,std::remove_all_extents_t<decltype(T::cFieldOrderArray)>>) { return false; }
            const u32 l_count = std::meta::nonstatic_data_members_of(^^T, std::meta::access_context::current()).size();
            if (sizeof(T::cFieldOrderArray) / sizeof(u32) != l_count) { return false; }
            for (u32 l_id = 0; l_id < l_count; ++l_id) {
                bool is_found = false;
                for (u32 f_id = 0; f_id < l_count; ++f_id) {
                    if (T::cFieldOrderArray[l_id] == f_id) {
                        if (is_found) { return false; }
                        is_found = true;
                    }
                }
                if (!is_found) { return false; }
            }
            return true;
        }
        template <typename T>
        consteval bool VerifyFieldOrderArray() {
            return true;
        }

        template <typename T>
             requires (T::cFieldOrderArray)
        consteval u32 GetFieldIndexByLayoutIndex(u32 i) requires (T::cFieldOrderArray) {
            return T::cFieldOrderArray[i];
        }

        template <typename T>
        consteval u32 GetFieldIndexByLayoutIndex(u32 i) {
            return i;
        }

        template <typename T, FieldNameString Name>
        consteval bool IsValidFieldName() {
            for (std::meta::info &info : std::meta::nonstatic_data_members_of(^^T, std::meta::access_context::current())) {
                if (vp::util::StringCompare(Name.GetString(), std::meta::identifier_of(info).data())) { return true; }
            }
            return false;
        }

        template <typename T, FieldNameString Name>
        consteval u32 GetFieldIndexByName() {
            static_assert(IsValidFieldName<T,Name>());
            u32 i = 0;
            for (std::meta::info &info : std::meta::nonstatic_data_members_of(^^T, std::meta::access_context::current())) {
                if (vp::util::StringCompare(Name.GetString(), std::meta::identifier_of(info).data())) { return i; }
                ++i;
            }
            return 0xffff'ffff;
        }

        template <typename T>
        consteval u32 VerifyFieldNames() {
            for (std::meta::info &info : std::meta::nonstatic_data_members_of(^^T, std::meta::access_context::current())) {
                if (cMaxTypedParamFieldNameLength <= std::meta::identifier_of(info).size()) { return false; }
            }
            return true;
        }

        template <typename T>
        consteval u32 VerifyIsValidFieldTypes();

        template <typename T>
        consteval u32 VerifyLayoutFields() {
            if (!VerifyIsValidFieldTypes<T>()) { return false; }
            if (!VerifyFieldNames<T>()) { return false; }
            if (!VerifyFieldOrderArray<T>()) { return false; }
            return true;
        }
    }

    class Composite;
    class TypedParam;      
    class Document;

    TypedParam *CreateTypedParam(mem::Heap *heap, u64 hash);

    struct FindCompositeContext {
        TypedParam *parent;
        Composite  *composite;
        const char *found_name;
        bool        is_non_default;
    };

    class Composite {
        public:
            VP_RTTI_BASE(Composite);
        public:
            constexpr Composite() {/*...*/}
            virtual constexpr ~Composite() {/*...*/}

            virtual void InitializeBgyml(mem::Heap *heap, Document *document, vp::res::ByamlIterator byaml_iterator) = 0;
#ifdef VP_ENABLE_LIBYAML
            virtual void InitializeYaml(mem::Heap *heap, Document *document, vp::res::YamlIterator yaml_iterator) = 0;
#endif
            virtual void InitializeDefaults(mem::Heap *heap, bool is_force_unlink_parents) = 0;
            virtual void Finalize() {/*...*/}

            virtual Result FindComposite(FindCompositeContext *out_find_info, const char *composite_name);

            virtual void ResolveParent(Document *document, TypedParam *parent_typed_param, const char *field_name) = 0;
    };

    struct PropertyLoopInfo {
        const char                      *name;
        const vp::util::RuntimeTypeInfo *typed_param_rtti;
        u8                              *is_non_default;
        u32                              field_index;
        TypedParam                      *typed_param;
        IPropertyAccessor               *property_accessor;
        const void                      *default_value;
    };
    
    struct EmbeddedLoopInfo {
        const char                       *name;
        const vp::util::RuntimeTypeInfo  *typed_param_rtti;
        u8                               *is_non_default;
        u32                               field_index;
        TypedParam                       *typed_param;
        const char                       *embedded_class_name;
        const vp::util::RuntimeTypeInfo  *embedded_rtti;
        TypedParam                      **embedded_typed_param;
    };
    
    struct CompositeLoopInfo {
        const char                      *name;
        const vp::util::RuntimeTypeInfo *typed_param_rtti;
        u8                              *is_non_default;
        u32                              field_index;
        TypedParam                      *typed_param;
        Composite                       *composite;
    };

    struct PropertyFormatInfo;
    struct CompositeFormatInfo;

    class TypedParam : public Composite {
        public:
            friend class DocumentManager;
            friend class Document;
        public:
            struct DummyLayout {
                AWN_PARA_TYPED_PARAM_LAYOUT_TRAITS(DummyLayout);
            };
            using LayoutType = DummyLayout;
        public:
            using PropertyLoopFunction  = vp::util::IFunction<Result(PropertyLoopInfo*)>;
            using EmbeddedLoopFunction  = vp::util::IFunction<Result(EmbeddedLoopInfo*)>;
            using CompositeLoopFunction = vp::util::IFunction<Result(CompositeLoopInfo*)>;
        public:
            static constexpr inline const char *cClassName   = "awn__para__TypedParam";
            static constexpr inline u64         cClassHash   = vp::util::HashMurmur64(cClassName);
            static constexpr inline u32         cLayoutCount = 0;
            static constexpr inline u32         cFieldCount  = 0;
        private:
            const char     *m_parent_name;
            DocumentBinder  m_parent_document_binder;
            u32             m_parent_free_count;
            u16             m_parent_handle;
            u16             m_handle;
        public:
            VP_RTTI_DERIVED(TypedParam);
        protected:
            template <FieldNameString Name> 
            static consteval bool IsValidFieldName() { return false; }
            template <FieldNameString Name>
            static consteval std::meta::info GetFieldInfoByName() {
                VP_UNEXPECTED_DEFAULT();
            }
            template <FieldNameString Name, u32 PropType>
            constexpr const [:std::meta::type_of(GetFieldInfoByName<Name>()):] *GetPointerImpl() const {
                VP_UNEXPECTED_DEFAULT();
            }
        protected:
            constexpr TypedParam *GetParent(); /* See document manager */
            constexpr TypedParam *GetParent() const; /* See document manager */
            void SetParentHandle(TypedParam *parent);
            void FreeParent();
            void ReleaseParent();

            void ReleaseHandle();
            bool LoadParent(Document *document, const char *parent_name);

            bool VerifyParent();
        protected:
            /* Declares in para_tfield.hpp */
            template <typename T, u32 i, u32 End>
            Result LoopPropertiesImpl(T *layout, u8 *is_non_default_array, PropertyLoopFunction *loop_function);
            template <typename T, u32 i, u32 End>
            Result LoopCompositesImpl(T *layout, u8 *is_non_default_array, CompositeLoopFunction *loop_function);
            template <typename T, u32 i, u32 End>
            Result LoopEmbedsImpl(T *layout, u8 *is_non_default_array, EmbeddedLoopFunction *loop_function);

            ALWAYS_INLINE Result LoopPropertyImpl(PropertyLoopFunction *loop_function, IPropertyAccessor *property_accessor, const char *layout_name, const vp::util::RuntimeTypeInfo *typed_param_rtti, u8 *is_non_default, u32 field_index, const void *default_value) {

                PropertyLoopInfo loop_context = { 
                    .name              = layout_name,
                    .typed_param_rtti  = typed_param_rtti,
                    .is_non_default    = is_non_default,
                    .field_index       = field_index,
                    .typed_param       = this,
                    .property_accessor = property_accessor,
                    .default_value     = default_value, 
                }; 
                const Result result = loop_function->Invoke(std::addressof(loop_context));
                return result;
            }

            ALWAYS_INLINE Result LoopEmbeddedImpl(EmbeddedLoopFunction *loop_function, const char *layout_name, const vp::util::RuntimeTypeInfo *typed_param_rtti, u8 *is_non_default, u32 field_index, const char *embedded_class_name, const vp::util::RuntimeTypeInfo *embedded_rtti, void *embedded_typed_param) {

                EmbeddedLoopInfo loop_context = {
                    .name                 = layout_name,
                    .typed_param_rtti     = typed_param_rtti,
                    .is_non_default       = is_non_default,
                    .field_index          = field_index,
                    .typed_param          = this,
                    .embedded_class_name  = embedded_class_name,
                    .embedded_rtti        = embedded_rtti,
                    .embedded_typed_param = reinterpret_cast<TypedParam**>(embedded_typed_param),
                }; 
                const Result result = loop_function->Invoke(std::addressof(loop_context));
                return result;
            }

            ALWAYS_INLINE Result LoopCompositeImpl(CompositeLoopFunction *loop_function, const char *layout_name, const vp::util::RuntimeTypeInfo *typed_param_rtti, u8 *is_non_default, u32 field_index, void *composite) {

                CompositeLoopInfo loop_context = { 
                    .name             = layout_name,
                    .typed_param_rtti = typed_param_rtti,
                    .is_non_default   = is_non_default,
                    .field_index      = field_index,
                    .typed_param      = this,
                    .composite        = reinterpret_cast<Composite*>(composite),
                };
                const Result result = loop_function->Invoke(std::addressof(loop_context));
                return result;
            }
        public:
            void SetParentTo(TypedParam *typed_param);
            Result ResolveComposite(FindCompositeContext *out_find_info, const char *field_name);
            virtual void ResolveParent(Document *document, TypedParam *parent_typed_param, const char *field_name) override;
        public:
            constexpr TypedParam(); /* See document manager */
            virtual ~TypedParam() override {/*...*/}

            virtual void InitializeBgyml(mem::Heap *heap, Document *document, vp::res::ByamlIterator byaml_iterator) override;
#ifdef VP_ENABLE_LIBYAML
            virtual void InitializeYaml(mem::Heap *heap, Document *document, vp::res::YamlIterator yaml_iterator) override;
#endif
            virtual void InitializeDefaults(mem::Heap *heap, bool is_force_unlink_parents) override;

            virtual void Finalize() override;

            virtual Result FindComposite(FindCompositeContext *out_find_info, const char *search_string) override;

            virtual constexpr const char *GetClassName() { return cClassName; }
            static constexpr u64 GetClassHashStatic() {
                const u64 hash = vp::util::HashMurmur64(cClassName);
                return hash; 
            }
            constexpr u64 GetClassHash() {
                const u64 hash = vp::util::HashMurmur64(this->GetClassName());
                return hash; 
            }
        public:
            virtual Result LoopProperties([[maybe_unused]] PropertyLoopFunction *loop_function)  { RESULT_RETURN_SUCCESS; }
            virtual Result LoopEmbeds([[maybe_unused]] EmbeddedLoopFunction *loop_function)      { RESULT_RETURN_SUCCESS; }
            virtual Result LoopComposites([[maybe_unused]] CompositeLoopFunction *loop_function) { RESULT_RETURN_SUCCESS; }
            virtual Result FormatProperties([[maybe_unused]] vp::util::IFunction<Result(PropertyFormatInfo*)> *format_function)  { RESULT_RETURN_SUCCESS; }
            virtual Result FormatComposites([[maybe_unused]] vp::util::IFunction<Result(CompositeFormatInfo*)> *format_function) { RESULT_RETURN_SUCCESS; }
            virtual bool IsFieldNonDefault(const vp::util::RuntimeTypeInfo *rtti, u32 field_index) { VP_UNUSED(rtti, field_index); return false; }
    };

    namespace impl {

        template <typename T>
            requires (TypedParam::CheckRuntimeTypeInfoStatic(T::GetRuntimeTypeInfoStatic()))
        TypedParam *CreateTypedParamImpl(mem::Heap *heap) {
            return new (heap, alignof(T)) T();
        }
    }
}
