#pragma once

namespace awn::res {

    class ResourceFactoryManager {
        public:
            using ResourceFactoryList = vp::util::IntrusiveListTraits<ResourceFactoryBase, &ResourceFactoryBase::m_manager_list_node>::List;
        private:
            sys::ServiceCriticalSection m_list_cs;
            ResourceFactoryList         m_resource_factory_list;
        public:
            AWN_SINGLETON_TRAITS(ResourceFactoryManager);
        public:
            constexpr ALWAYS_INLINE ResourceFactoryManager() : m_list_cs(), m_resource_factory_list() {/*...*/}
            constexpr ~ResourceFactoryManager() {/*...*/}

            void RegisterResourceFactory(ResourceFactoryBase *factory) {
                VP_ASSERT(factory->m_file_extension != nullptr);
                std::scoped_lock lock(m_list_cs);
                m_resource_factory_list.PushBack(*factory);
            }

            void RemoveResourceFactory(ResourceFactoryBase *factory) {
                std::scoped_lock lock(m_list_cs);
                factory->m_manager_list_node.Unlink();
            }

            ResourceFactoryBase *FindResourceFactory(const char *file_extension) {
                std::scoped_lock lock(m_list_cs);
                for (ResourceFactoryBase &rf : m_resource_factory_list) {
                    if (::strcmp(rf.m_file_extension, file_extension) == 0) {
                        return std::addressof(rf);
                    }
                }
                return nullptr;
            }

            void TryLoad(ResourceLoadContext *load_arg) {

                /* Find factory */
                ResourceFactoryBase *factory = load_arg->resource_factory;
                if (factory == nullptr) {
                    vp::util::FixedString<vp::util::cMaxPath> extension;
                    vp::util::GetExtensionFromPath(std::addressof(extension), load_arg->file_load_context.file_path);
                    factory = this->FindResourceFactory(extension.GetString());
                }

                /* Load file with factory */
                RESULT_ABORT_UNLESS(factory->TryLoad(nullptr, load_arg));
                
                
            }
    };
}
