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

#define AWN_PARA_DECLARE_TYPED_PARAM_AUTOV(class_name_for_mgr, class_name, base_type_name, layout_type_name, ...) VP_FOUR_ARG_VMACRO(class_name_for_mgr, class_name, base_type_name, layout_type_name, AWN_PARA_DECLARE_TYPEDPARAM_AUTO_NAME, __VA_ARGS__)

#define AWN_PARA_DECLARE_TYPED_PARAM_IMPL(class_name) \
    static ::awn::para::TypedParamFactory sTypedParamFactory_##class_name ( class_name ::cClassHash, ::awn::para::CreateTypedParamImpl< class_name >)

namespace awn::para {

    class Composite;
    class TypedParam;
    class Document;

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
            VP_RTTI_DERIVED(TypedParam, Composite);
        protected:
            constexpr TypedParam *GetParent(); /* See document manager */
            constexpr TypedParam *GetParent() const; /* See document manager */
            void SetParentHandle(TypedParam *parent);
            void FreeParent();
            void ReleaseParent();

            void ReleaseHandle();
            bool LoadParent(Document *document, const char *parent_name);

            bool VerifyParent();

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
        protected:
            virtual Result LoopProperties([[maybe_unused]] PropertyLoopFunction *loop_function)  { RESULT_RETURN_SUCCESS; }
            virtual Result LoopEmbeds([[maybe_unused]] EmbeddedLoopFunction *loop_function)      { RESULT_RETURN_SUCCESS; }
            virtual Result LoopComposites([[maybe_unused]] CompositeLoopFunction *loop_function) { RESULT_RETURN_SUCCESS; }
            virtual Result FormatProperties([[maybe_unused]] vp::util::IFunction<Result(PropertyFormatInfo*)> *format_function)  { RESULT_RETURN_SUCCESS; }
            virtual Result FormatComposites([[maybe_unused]] vp::util::IFunction<Result(CompositeFormatInfo*)> *format_function) { RESULT_RETURN_SUCCESS; }
            virtual bool IsParamNonDefault(const vp::util::RuntimeTypeInfo *rtti, u32 param_index) { VP_UNUSED(rtti, param_index); return false; }
    };

    template <typename T>
        requires (TypedParam::CheckRuntimeTypeInfoStatic(T::GetRuntimeTypeInfoStatic()) == true)
    TypedParam *CreateTypedParamImpl(mem::Heap *heap) {
        return new (heap, alignof(T)) T();
    }
} 

#define AWN_TYPED_PARAM_AUTO_START(class_name_for_mgr, class_name, base_type_name, layout_type_name) \
    class class_name : public base_type_name { \
        public: \
            using BaseType   = base_type_name ; \
            using LayoutType = layout_type_name ; \
        public: \
             static_assert( layout_type_name ::cIsLayoutForTypedParam == true); \
        public: \
            static constexpr inline const char *cClassName         = TOSTRING( class_name_for_mgr ); \
            static constexpr inline u64         cClassHash         = vp::util::HashMurmur64(cClassName); \
            static constexpr inline const char *cLayoutNameArray[] = {
#define AWN_TYPED_PARAM_AUTO_MID0(class_name, base_type_name) \
            }; \
            static constexpr inline u32         cLayoutCount       = sizeof(cLayoutNameArray) / sizeof(const char*); \
            static constexpr inline u32         cFieldCount        = BaseType::cFieldCount + cLayoutCount; \
        public: \
            VP_RTTI_DERIVED( class_name , base_type_name ); \
        private: \
            LayoutType m_layout; \
            u8         m_is_non_default_array[cLayoutCount]; \
        public: \
             static constexpr inline auto cIsNonDefaultArrayMember = & class_name ::m_is_non_default_array;
#define AWN_PARA_FIELD_MEMBER_IMPL(field_name, field_index) \
    static constexpr inline auto cFieldMember##field_index = &LayoutType:: field_name ; \
    using FieldType##field_index = vp::util::MemberType< cFieldMember##field_index > ; \
    static_assert(::awn::para::TIsField< FieldType##field_index >() == true) ;
#define AWN_PARA_DEFAULT_IMPL(field_index) \
    static constexpr inline auto cDefault##field_index = LayoutType::TGetPropertyDefault< cFieldMember##field_index >(); 
#define AWN_TYPED_PARAM_AUTO_MID1(class_name) \
        public: \
            template <typename T> constexpr const T *GetPointerImpl(const auto &value_offset, u32 index) const { \
                if ((m_is_non_default_array[index] & 1) == true) { \
                    return std::addressof(this->m_layout.*value_offset); \
                } \
                const class_name *out = this; \
                const ::awn::para::TypedParam *parent_iter; \
                while (parent_iter = out->GetParent(), parent_iter != nullptr) { \
                    if (!CheckRuntimeTypeInfoStatic(parent_iter) || (out = reinterpret_cast<const class_name *>(parent_iter), (out->m_is_non_default_array[index] & 1))) { \
                        break; \
                    } \
                } \
                return std::addressof(out->m_layout.*value_offset); \
            }
#define AWN_PARA_GET_FIELD_IMPL(field_name, field_index) \
            constexpr const FieldType##field_index *Get##field_name () const { \
                return this->GetPointerImpl< FieldType##field_index >(cFieldMember##field_index , field_index ); \
            }
#define AWN_TYPED_PARAM_AUTO_MID2(class_name) \
        public: \
            constexpr class_name () : BaseType(), m_layout(), m_is_non_default_array{} {/*...*/} \
            virtual ~class_name () override {/*...*/} \
            virtual constexpr const char *GetClassName() override { return cClassName; } \
        public: \
            virtual Result LoopProperties(PropertyLoopFunction *loop_function) override { \
                this->BaseType::LoopProperties(loop_function);
#define AWN_PARA_LOOP_PROPERTY_IMPL(field_index)  \
                if constexpr (::awn::para::TIsProperty< FieldType##field_index >()) { \
                    ::awn::para::TPropertyAccessor< FieldType##field_index > accessor = ::awn::para::TGetPropertyAccessor< FieldType##field_index >(std::addressof(m_layout.*cFieldMember##field_index )); \
                    const Result result = this->LoopPropertyImpl(loop_function, std::addressof(accessor), cLayoutNameArray[ field_index ], GetRuntimeTypeInfoStatic(), std::addressof(m_is_non_default_array[ field_index ]), field_index , std::addressof( cDefault##field_index )); \
                    RESULT_THROW_ON_FAILED(result); \
                } 
#define AWN_TYPED_PARAM_AUTO_MID3() \
                RESULT_RETURN_SUCCESS; \
            } \
            virtual Result LoopEmbeds(EmbeddedLoopFunction *loop_function) override { \
                this->BaseType::LoopEmbeds(loop_function);
#define AWN_PARA_LOOP_EMBED_IMPL(field_index) \
                if constexpr (::awn::para::TIsEmbed< FieldType##field_index >()) { \
                    const Result result = this->LoopEmbeddedImpl(loop_function, cLayoutNameArray[ field_index ], GetRuntimeTypeInfoStatic(), std::addressof(m_is_non_default_array[ field_index ]), field_index , ::awn::para::TGetEmbedClassName< FieldType##field_index >(), ::awn::para::TGetCompositeRtti <FieldType##field_index >(), std::addressof( m_layout.*cFieldMember##field_index )); \
                    RESULT_THROW_ON_FAILED(result); \
                }
#define AWN_TYPED_PARAM_AUTO_MID4() \
                RESULT_RETURN_SUCCESS; \
            } \
            virtual Result LoopComposites(CompositeLoopFunction *loop_function) override { \
                this->BaseType::LoopComposites(loop_function);
#define AWN_PARA_LOOP_COMPOSITE_IMPL(field_index) \
                if constexpr (::awn::para::TIsComposite< FieldType##field_index >()) { \
                    const Result result = this->LoopCompositeImpl(loop_function, cLayoutNameArray[ field_index ], GetRuntimeTypeInfoStatic(), std::addressof(m_is_non_default_array[ field_index ]), {0}, std::addressof(m_layout.*cFieldMember##field_index )); \
                    RESULT_THROW_ON_FAILED(result); \
                }
#define AWN_TYPED_PARAM_AUTO_END() \
                RESULT_RETURN_SUCCESS; \
            } \
            virtual bool IsParamNonDefault(const vp::util::RuntimeTypeInfo *rtti, u32 index) override { \
                if (this->CheckRuntimeTypeInfoStatic(rtti) == true && index < cLayoutCount) { return m_is_non_default_array[index]; } \
                return this->BaseType::IsParamNonDefault(rtti, index); \
            } \
    };
