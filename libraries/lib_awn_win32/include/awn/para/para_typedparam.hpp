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

    #define AWN_PARA_DECLARE_TYPED_PARAM_AUTOV(class_name_for_mgr, class_name, base_type_name, layout_type_name, ...) VP_FOUR_ARG_VMACRO(class_name_for_mgr, class_name, base_type_name, layout_type_name, AWN_PARA_DECLARE_TYPEDPARAM_AUTO_NAME, __VA_ARGS__)

    #define AWN_PARA_DECLARE_TYPED_PARAM_IMPL(class_name) \
        static TypedParamFactory sTypedParamFactory_##class_name ( class_name ::cClassHash, CreateTypedParamImpl< class_name >)
}
