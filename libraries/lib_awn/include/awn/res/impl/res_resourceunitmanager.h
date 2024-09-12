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

namespace awn::res::impl {

    class ResourceUnitManager {
        public:
            using ResourceUnitMap = vp::util::IntrusiveRedBlackTreeTraits<ResourceUnit, &ResourceUnit::m_resource_unit_manager_tree_node>::Tree;
        private:
            ResourceUnitMap m_resource_unit_map;
            sys::Mutex m_map_cs;
        public:
            constexpr  ResourceUnitManager() : m_resource_unit_map(), m_map_cs() {/*...*/}
            constexpr ~ResourceUnitManager() {/*...*/}

            void Finalize();

            void RegisterResourceUnit(ResourceUnit *resource_unit);
            void UnregisterResourceUnit(ResourceUnit *resource_unit);

            ResourceUnit *FindResourceUnit(const char *path);
    };
}
