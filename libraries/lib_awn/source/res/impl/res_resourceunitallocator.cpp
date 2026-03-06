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

    Result ResourceUnitAllocator::Initialize(mem::Heap *heap, u32 resource_unit_count) {

        /* Initialize arrays */
        RESULT_TRY(m_resource_unit_array.Initialize(heap, resource_unit_count));
        auto fail_guard0 = SCOPE_GUARD { this->Finalize(); };
        RESULT_TRY(m_resource_unit_id_allocator.Initialize(heap, resource_unit_count));
        RESULT_TRY(m_resource_unit_free_ring.Initialize(heap, resource_unit_count));

        fail_guard0.Cancel();
        RESULT_RETURN_SUCCESS;
    }

    void ResourceUnitAllocator::Finalize() {
        m_resource_unit_array.Finalize();
        m_resource_unit_id_allocator.Finalize();
        m_resource_unit_free_ring.Finalize();
    }

    ResourceUnit *ResourceUnitAllocator::AllocateImpl() {
        const u16 id = m_resource_unit_id_allocator.Allocate();
        if (id == ResourceUnitIdAllocator::cInvalidEntryIndex) { return nullptr; }
        return std::addressof(m_resource_unit_array[id]);
    }

    void ResourceUnitAllocator::FreeCache(bool is_single) {

        /* Process free ring */
        for (;;) {            
            const u32 id = static_cast<u32>(m_resource_unit_free_ring.RemoveFrontWeak());
            if (id == 0) { return; }
            const u32 index = id - 1;

            /* Fail if pending unload (wait for single) */
            ResourceUnit *unit = this->GetResourceUnitByIndex(index);
            if (!unit->CheckOrWaitUnloaded(is_single)) { return; }

            /* Free id */
            m_resource_unit_id_allocator.Free(index);
            if (is_single) { return; }
        }

        return;
    }

    ResourceUnit *ResourceUnitAllocator::Allocate() {

        /* Try alloc */
        ResourceUnit *unit = this->AllocateImpl();
        if (unit != nullptr) { return unit; }
        this->FreeCache(true);
        unit = this->AllocateImpl();
        return unit;
    }

    void ResourceUnitAllocator::Free(ResourceUnit *resource_unit) {

        /* Push unload id */
        VP_ASSERT(m_resource_unit_array.IsContainAddress(resource_unit));
        const u32 index = m_resource_unit_array.IndexOf(resource_unit);
        m_resource_unit_free_ring.PushBack(index + 1);
        
        return;
    }
}
