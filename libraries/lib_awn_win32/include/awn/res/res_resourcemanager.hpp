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

namespace awn::res {

    class ResourceFactoryManager {
        public:
            using ResourceFactoryMap = vp::util::IntrusiveRedBlackTreeTraits<ResourceFactoryBase, &ResourceFactoryBase::m_manager_tree_node>::Tree;
        private:
            sys::ServiceCriticalSection m_factory_map_cs;
            ResourceFactoryMap          m_resource_factory_map;
        public:
            AWN_SINGLETON_TRAITS(ResourceFactoryManager);
        public:
            constexpr ALWAYS_INLINE ResourceFactoryManager() : m_factory_map_cs(), m_resource_factory_map() {/*...*/}
            constexpr ~ResourceFactoryManager() {/*...*/}

            ALWAYS_INLINE void RegisterResourceFactory(ResourceFactoryBase *factory) {
                VP_ASSERT(factory->m_file_extension != nullptr);
                std::scoped_lock lock(m_factory_map_cs);
                m_resource_factory_map.Insert(factory);
            }

            ALWAYS_INLINE void RemoveResourceFactory(ResourceFactoryBase *factory) {
                std::scoped_lock lock(m_factory_map_cs);
                m_resource_factory_map.Remove(factory);
            }

            ALWAYS_INLINE ResourceFactoryBase *FindResourceFactory(const char *file_extension) {
                const u32 ext_hash = vp::util::HashCrc32b(file_extension);
                std::scoped_lock lock(m_factory_map_cs);
                return m_resource_factory_map.Find(ext_hash);
            }

            void CreateResource();

            void TryLoad(ResourceLoadContext *load_arg) {

                

                /* Find factory */
                ResourceFactoryBase *factory = load_arg->resource_factory;
                if (factory == nullptr) {
                    MaxExtensionString extension;
                    vp::util::GetExtension(std::addressof(extension), load_arg->file_load_context.file_path);
                    factory = this->FindResourceFactory(extension.GetString());
                }

                /* Load file with factory */
                RESULT_ABORT_UNLESS(factory->TryLoad(nullptr, load_arg));

                
            }
    };
}
