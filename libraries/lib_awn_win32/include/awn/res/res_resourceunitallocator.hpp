#pragma once

namespace awn::res {

    class ResourceUnitAllocator {
        private:
            using ResourceUnitArray    = vp::util::HeapArray<ResourceUnit>;
            using ResourceUnitFreeRing = vp::util::RingBuffer<ResourceUnit*>;
        private:
            ResourceUnitArray           m_resource_unit_array;
            ResourceUnitFreeRing        m_resource_unit_free_ring;
            sys::ServiceCriticalSection m_ring_cs;
        public:
            constexpr  ResourceUnitAllocator() : m_resource_unit_array(), m_resource_unit_free_ring(), m_ring_cs()  {/*...*/}
            constexpr ~ResourceUnitAllocator() {/*...*/}

            void Initialize(mem::Heap *heap, u32 resource_unit_count) {

                /* Initialize arrays */
                m_resource_unit_array.Initialize(heap, resource_unit_count);
                m_resource_unit_free_ring.Initialize(heap, resource_unit_count);

                /* Fill free ring buffer */
                for (u32 i = 0; i < resource_unit_count; ++i) {
                    m_resource_unit_free_ring.Insert(std::addressof(m_resource_unit_array[i]));
                }

                return;
            }

            void Finalize() {
                m_resource_unit_array.Finalize();
                m_resource_unit_free_ring.Finalize();
            }

            ResourceUnit *Allocate() {

                /* Lock ring */
                std::scoped_lock l(m_ring_cs);

                /* Allocate resource unit */
                ResourceUnit *resource_unit = m_resource_unit_free_ring.RemoveFront();

                return resource_unit;
            }

            void Free(ResourceUnit *resource_unit) {

                /* Lock ring */
                std::scoped_lock l(m_ring_cs);

                /* Free reource unit */
                m_resource_unit_free_ring.Insert(resource_unit);

                return;
            }

            constexpr ALWAYS_INLINE ResourceUnit *GetResourceUnitByIndex(u32 index) {
                return std::addressof(m_resource_unit_array[index]);
            }

            constexpr ALWAYS_INLINE u32 GetResourceUnitIndex(ResourceUnit *resource_unit) const {
                return m_resource_unit_array.GetIndexOf(resource_unit);
            }
    };
}
