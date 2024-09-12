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

    class ResourceUnitAllocator {
        private:
            using ResourceUnitArray    = vp::util::HeapArray<ResourceUnit>;
            using ResourceUnitFreeRing = vp::util::RingBuffer<ResourceUnit*>;
        private:
            ResourceUnitArray m_resource_unit_array;
            ResourceUnitFreeRing m_resource_unit_free_ring;
            sys::Mutex m_ring_cs;
        public:
            constexpr  ResourceUnitAllocator() : m_resource_unit_array(), m_resource_unit_free_ring(), m_ring_cs()  {/*...*/}
            constexpr ~ResourceUnitAllocator() {/*...*/}

            void Initialize(mem::Heap *heap, u32 resource_unit_count);
            void Finalize();

            ResourceUnit *Allocate();
            void Free(ResourceUnit *resource_unit);

            constexpr ALWAYS_INLINE ResourceUnit *GetResourceUnitByIndex(u32 index) {
                return std::addressof(m_resource_unit_array[index]);
            }

            constexpr ALWAYS_INLINE u32 GetResourceUnitIndex(ResourceUnit *resource_unit) const {
                return m_resource_unit_array.IndexOf(resource_unit);
            }
    };
}
