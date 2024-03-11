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
#include <awn.hpp>

namespace awn::res {

    AWN_SINGLETON_TRAITS_IMPL(ResourceFactoryManager);
    
    Result ResourceFactoryManager::LoadResource(Resource **out_resource, const char *path, ResourceLoadContext *resource_load_context) {

        /* Find factory */
        ResourceFactoryBase *factory = resource_load_context->resource_factory;
        if (factory == nullptr) {
            MaxExtensionString extension;
            vp::util::GetExtension(std::addressof(extension), path);
            factory = this->FindResourceFactory(extension.GetString());
        }

        /* Load file with factory */
        return factory->LoadResource(out_resource, path, resource_load_context);
    }
    Result ResourceFactoryManager::LoadResourceWithDecompressor(Resource **out_resource, const char *path, ResourceLoadContext *resource_load_context, IDecompressor *decompressor) {

        /* Get factory */
        ResourceFactoryBase *factory = resource_load_context->resource_factory;
        if (factory == nullptr) {
            MaxExtensionString extension;
            vp::util::GetExtension(std::addressof(extension), path);
            factory = this->FindResourceFactory(extension.GetString());
        }

        /* Load file with factory */
        return factory->LoadResourceWithDecompressor(out_resource, path, resource_load_context, decompressor);
    }
}
