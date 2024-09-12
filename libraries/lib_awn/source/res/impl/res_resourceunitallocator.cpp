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

    void ResourceUnitAllocator::Initialize(mem::Heap *heap, u32 resource_unit_count) {

        /* Initialize arrays */
        m_resource_unit_array.Initialize(heap, resource_unit_count);
        m_resource_unit_free_ring.Initialize(heap, resource_unit_count);

        /* Fill free ring buffer */
        for (u32 i = 0; i < resource_unit_count; ++i) {
            m_resource_unit_free_ring.PushBack(std::addressof(m_resource_unit_array[i]));
        }

        return;
    }

    void ResourceUnitAllocator::Finalize() {
        m_resource_unit_array.Finalize();
        m_resource_unit_free_ring.Finalize();
    }

    ResourceUnit *ResourceUnitAllocator::Allocate() {

        /* Lock ring */
        std::scoped_lock l(m_ring_cs);

        /* Allocate resource unit */
        ResourceUnit *resource_unit = m_resource_unit_free_ring.RemoveFront();

        return resource_unit;
    }

    void ResourceUnitAllocator::Free(ResourceUnit *resource_unit) {

        /* Lock ring */
        std::scoped_lock l(m_ring_cs);

        /* Free reource unit */
        m_resource_unit_free_ring.PushBack(resource_unit);

        return;
    }
}
