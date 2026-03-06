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

    class AsyncResourceManager;

    class ResourceUnitAllocator {
        public:
            friend class AsyncResourceManager;
        private:
            using ResourceUnitArray = vp::util::HeapArray<ResourceUnit>;
            using ResourceUnitIdAllocator = vp::util::IndexAllocator<u32>;
            using ResourceUnitFreeRing = vp::util::AtomicRingBuffer<u64, false>;
        private:
            ResourceUnitArray m_resource_unit_array;
            ResourceUnitIdAllocator m_resource_unit_id_allocator;
            ResourceUnitFreeRing m_resource_unit_free_ring;
        private:
            ResourceUnit *AllocateImpl();
            void FreeCache(bool is_block);
        public:
            constexpr  ResourceUnitAllocator() : m_resource_unit_array(), m_resource_unit_id_allocator(), m_resource_unit_free_ring()  {/*...*/}
            constexpr ~ResourceUnitAllocator() {/*...*/}

            Result Initialize(mem::Heap *heap, u32 resource_unit_count);
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
