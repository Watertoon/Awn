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
            class CreateInfo : public res::IComposeInfo {
                public:
                    const char *m_file_name;
                    const char *m_class_name;
                    bool m_is_yaml;
                public:
                    VP_RTTI_DERIVED(CreateInfo, res::IComposeInfo);
                public:
                    constexpr CreateInfo(const char *file_name, const char *class_name) : IComposeInfo(), m_file_name(file_name), m_class_name(class_name), m_is_yaml() {/*...*/}
                    virtual ~CreateInfo() override {/*...*/}
            };
        public:
            Document *CreateDocument(mem::Heap *heap, const char *file_path, const char *class_name, void *file, size_t size, bool is_yaml);
        private:
            Document *m_document;
            vp::util::TypedStorage<res::ResourceBinder> m_resource_binder_bgyml;
#ifdef VP_ENABLE_LIBYAML
            vp::res::YamlDocument *m_yaml_doc;
#endif
        public:
            VP_RTTI_DERIVED(ResourceDocument, res::Resource);
        public:
            constexpr ResourceDocument() : Resource(), m_document(), m_resource_binder_bgyml(), m_yaml_doc() {/*...*/}
            virtual ~ResourceDocument() override {/*...*/}

            virtual constexpr bool IsRequireCompose() const override { return true; }

            virtual Result Compose(mem::Heap *heap, res::IComposeInfo *compose_info, void *file, size_t file_size) override;
            virtual void Finalize() override;
            virtual Result PreFinalize() override;

            Result ReferenceBinder(res::ResourceBinder *binder) {
                vp::util::ConstructAt(m_resource_binder_bgyml);
                RESULT_TRY(vp::util::GetReference(m_resource_binder_bgyml).ReferenceBinderSync(binder));
                RESULT_RETURN_SUCCESS;
            }
        public:
            constexpr Document *GetDocument() { return m_document; }
    };

    class ResourceFactoryDocument : public res::ResourceFactoryBase {
        public:
            static constexpr u32 cResourceSize = sizeof(ResourceDocument);
            static constexpr u32 cResourceAlignment = alignof(ResourceDocument);
            static constexpr u32 cFileAlignment = alignof(u32);
            static constexpr res::CompressionType cDefaultCompressionType = res::CompressionType::None;
            static constexpr float cDefaultFallbackSizeScale = 2.0f;
            static constexpr u32 cDefaultExtraSize = 0;
        public:
            constexpr ResourceFactoryDocument() : ResourceFactoryBase("bgyml", cResourceSize, cResourceAlignment, cFileAlignment, cDefaultCompressionType, cDefaultFallbackSizeScale, cDefaultExtraSize, true, false) {/*...*/}
            constexpr ~ResourceFactoryDocument() {/*...*/}

            virtual res::Resource *AllocateResource(mem::Heap *heap, s32 alignment) const override {
                return new (heap, alignment) ResourceDocument();
            }
    };
}
