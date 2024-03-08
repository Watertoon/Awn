#pragma once

namespace awn::res {

    class ResourceUnitManager {
        public:
            using ResourceUnitMap = vp::util::IntrusiveRedBlackTreeTraits<ResourceUnit, &ResourceUnit::m_resource_unit_manager_tree_node>::Tree;
        private:
            ResourceUnitMap              m_resource_unit_map;
            sys::ServiceCriticalSection  m_map_cs;
        public:
            constexpr  ResourceUnitManager() : m_resource_unit_map(), m_map_cs() {/*...*/}
            constexpr ~ResourceUnitManager() {/*...*/}

            void Finalize() {
                std::scoped_lock l(m_map_cs);
                m_resource_unit_map.ClearAll([](ResourceUnit *res_unit) { res_unit->m_is_part_of_resource_unit_mgr = false; });
            }

            void RegisterResourceUnit(ResourceUnit *resource_unit) {

                /* Lock map */
                if (resource_unit->m_is_part_of_resource_unit_mgr == true) { return; }
                std::scoped_lock l(m_map_cs);

                /* Insert into map */
                m_resource_unit_map.Insert(resource_unit);
                resource_unit->m_is_part_of_resource_unit_mgr = true;

                return;
            }
            void UnregisterResourceUnit(ResourceUnit *resource_unit) {

                /* Lock map */
                if (resource_unit->m_is_part_of_resource_unit_mgr == false) { return; }
                std::scoped_lock l(m_map_cs);

                /* Remove from map */
                m_resource_unit_map.Remove(resource_unit);
                resource_unit->m_is_part_of_resource_unit_mgr = false;

                return;
            }

            ResourceUnit *FindResourceUnit(const char *path) {
                const u32 hash = vp::util::HashCrc32b(path);
                std::scoped_lock l(m_map_cs);
                return m_resource_unit_map.Find(hash);
            }
    };
}
