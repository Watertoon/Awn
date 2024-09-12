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

    class DefaultDocumentAppImpl : public IDocumentApplicationImpl {
        private:
            impl::DummyFileDevice m_dummy_file_device;
            vp::util::AnyFunction<4, bool(Document*)> m_is_release_document_callback;
        private:
            static ResourceDocument *LoadSyncImpl(const char *path, res::AsyncResourceLoadInfo *load_info) {

                /* Load bgyml */
                res::ResourceBinder binder;
                binder.TryLoadSync(path, load_info);

                /* Get resource */
                ResourceDocument *document_res = binder.GetResource<ResourceDocument>();
                if (document_res == nullptr) { return nullptr; }

                /* Reference binder in resource for persistance */
                document_res->ReferenceBinder(std::addressof(binder));

                return document_res;
            }
        public:
            constexpr DefaultDocumentAppImpl() : m_dummy_file_device(), m_is_release_document_callback() {/*...*/}
            virtual ~DefaultDocumentAppImpl() override {/*...*/}

            virtual Document *TryLoadDocument(const char *file_name, const char *class_name, bool is_allow_dummy) override {

                /* Format path */
                res::MaxPathString path;
                path.Format("%s.%s.bgyml", file_name, class_name);

                /* Load document */
                ResourceDocument::CreateInfo create_info(file_name, class_name);
                res::AsyncResourceLoadInfo load_info = {
                    .priority              = 1,
                    .is_allow_loose_files  = true,
                    .resource_factory      = impl::DocumentManager::GetInstance()->GetResourceFactory(),
                    .resource_user_context = std::addressof(create_info),
                };
                ResourceDocument *document_res = this->LoadSyncImpl(path.GetString(), std::addressof(load_info));

                /* Use a fake document if we fail to load a real one */
                if (document_res == nullptr && is_allow_dummy) {
                    load_info.file_device = std::addressof(m_dummy_file_device);
                    document_res = this->LoadSyncImpl(path.GetString(), std::addressof(load_info));
                }

                return document_res->GetDocument();
            }

            virtual void DestructDocument([[maybe_unused]] Document *document) override {/*...*/}

            virtual bool OnReleaseDocument(Document *document) override {

                /* No release if dummy */
                if (document->IsDummy()) { return false; }

                /* Defer to callback if available */
                if (!m_is_release_document_callback.IsValid()) { return true; }
                return m_is_release_document_callback.Invoke(document);
            }
    };
}
