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
            ResourceFactoryBase         m_default_resource_factory;
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
                ResourceFactoryBase *res_factory = m_resource_factory_map.Find(ext_hash);
                return (res_factory != nullptr) ? res_factory : std::addressof(m_default_resource_factory);
            }

            Result LoadResource(Resource **out_resource, const char *path, ResourceLoadContext *resource_load_context);
            Result LoadResourceWithDecompressor(Resource **out_resource, const char *path, ResourceLoadContext *resource_load_context, IDecompressor *decompressor);
    };
}
