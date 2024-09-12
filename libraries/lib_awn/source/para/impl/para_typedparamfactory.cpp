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

namespace awn::para::impl {

    constinit TypedParamFactoryManager sTypedParamFactoryManager;

    TypedParamFactory::TypedParamFactory(u64 class_hash, TypedParamCreateFunction create_function) : m_node(class_hash), m_create_function(create_function) {
        VP_ASSERT(class_hash != 0);
        VP_ASSERT(create_function != nullptr);
        sTypedParamFactoryManager.Insert(this);
    }
}

namespace awn::para {

    TypedParam *CreateTypedParam(mem::Heap *heap, u64 hash) {

        impl::TypedParamFactory *factory = impl::sTypedParamFactoryManager.Find(hash);
        if (factory == nullptr) { return nullptr; }

        return (factory->m_create_function)(heap);
    }
}