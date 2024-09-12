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
    class TypedParam;
}

namespace awn::para::impl {

    using TypedParamCreateFunction = TypedParam*(*)(mem::Heap*);

    class TypedParamFactory {
        public:
            vp::util::IntrusiveRedBlackTreeNode<u64> m_node;
            TypedParamCreateFunction                 m_create_function;
        public:
            TypedParamFactory(u64 class_hash, TypedParamCreateFunction create_function);
            constexpr ~TypedParamFactory() {/*...*/}
    };

    using TypedParamFactoryManager = vp::util::IntrusiveRedBlackTreeTraits<TypedParamFactory, &TypedParamFactory::m_node>::Tree;
}
