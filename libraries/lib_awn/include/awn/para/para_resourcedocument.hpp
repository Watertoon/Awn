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

    class ResourceDocument : public res::Resource {
        public:
            class CreateInfo : public res::ResourceUserContext {
                public:
                    const char *m_file_name;
                    const char *m_class_name;
                public:
                    VP_RTTI_DERIVED(CreateInfo, res::ResourceUserContext);
                public:
                    constexpr CreateInfo(const char *file_name, const char *class_name) : ResourceUserContext(), m_file_name(file_name), m_class_name(class_name) {/*...*/}
                    virtual ~CreateInfo() override {/*...*/}
            };
        public:
            static Document *CreateDocument(mem::Heap *heap, const char *file_path, const char *class_name, void *byaml);
        private:
            Document                                    *m_document;
            vp::util::TypedStorage<res::ResourceBinder>  m_resource_binder_bgyml;
        public:
            VP_RTTI_DERIVED(ResourceDocument, res::Resource);
        public:
            constexpr ResourceDocument() : Resource(), m_document(), m_resource_binder_bgyml() {/*...*/}
            virtual ~ResourceDocument() override {/*...*/}

            virtual constexpr bool IsRequireInitializeOnCreate() const override { return true; }

            virtual Result Initialize(mem::Heap *heap, mem::Heap *gpu_heap, res::ResourceUserContext *user_context, void *file, size_t file_size) override;
            virtual void Finalize() override;
            virtual Result PreFinalize() override;

            void ReferenceBinder(res::ResourceBinder *binder) {
                vp::util::ConstructAt(m_resource_binder_bgyml);
                vp::util::GetReference(m_resource_binder_bgyml).ReferenceBinderSync(binder);
            }

            constexpr Document *GetDocument() { return m_document; }
    };

    class ResourceFactoryDocument : public res::ResourceFactoryBase {
        public:
            constexpr ResourceFactoryDocument() : ResourceFactoryBase("bgyml") {/*...*/}
            constexpr ~ResourceFactoryDocument() {/*...*/}

            virtual ResourceDocument *AllocateResource(mem::Heap *heap, s32 alignment) {
                return new (heap, alignment) ResourceDocument();
            }

            virtual constexpr u32                  GetResourceSize()             const { return sizeof(ResourceDocument); }
            virtual constexpr u32                  GetResourceAlignment()        const { return alignof(ResourceDocument); }
            virtual constexpr u32                  GetFileAlignment()            const { return cFileAlignment; }
            virtual constexpr res::CompressionType GetDefaultCompressionType()   const { return cDefaultCompressionType; }
            virtual constexpr float                GetDefaultFallbackSizeScale() const { return 2.0f; }
            virtual constexpr u32                  GetDefaultExtraResourceSize() const { return cDefaultExtraSize; }
    };
}
