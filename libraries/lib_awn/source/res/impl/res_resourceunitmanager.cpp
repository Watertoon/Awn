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

namespace awn::res::impl {

    void ResourceUnitManager::Finalize() {
        std::scoped_lock l(m_map_cs);
        m_resource_unit_map.ClearAll([](ResourceUnit *res_unit) { res_unit->m_is_part_of_resource_unit_mgr = false; });
    }

    void ResourceUnitManager::RegisterResourceUnit(ResourceUnit *resource_unit) {

        /* Lock map */
        if (resource_unit->m_is_part_of_resource_unit_mgr) { return; }
        std::scoped_lock l(m_map_cs);

        /* Insert into map */
        m_resource_unit_map.Insert(resource_unit);
        resource_unit->m_is_part_of_resource_unit_mgr = true;

        return;
    }

    void ResourceUnitManager::UnregisterResourceUnit(ResourceUnit *resource_unit) {

        /* Lock map */
        if (!resource_unit->m_is_part_of_resource_unit_mgr) { return; }
        std::scoped_lock l(m_map_cs);

        /* Remove from map */
        m_resource_unit_map.Remove(resource_unit);
        resource_unit->m_is_part_of_resource_unit_mgr = false;

        return;
    }

    bool ResourceUnitManager::LoopResourceUnits(const ResourceUnitLoopFunction *loop_function) {
        std::scoped_lock l(m_map_cs);
        auto iter = m_resource_unit_map.begin();
        auto end = m_resource_unit_map.end();
        while (iter != end) {
            ResourceUnit *resource_unit = std::addressof(*iter);
            ++iter;
            const bool is_continue = loop_function->Invoke(this, resource_unit);
            if (!is_continue) { return false; }
        }
        return true;
    }

    ResourceUnit *ResourceUnitManager::FindResourceUnit(const char *path) {
        const vp::util::HashString<u32> hash_string = {
            .hash = vp::util::HashCrc32b(path),
            .string = path,
        };
        std::scoped_lock l(m_map_cs);
        return m_resource_unit_map.Find(hash_string);
    }
}
