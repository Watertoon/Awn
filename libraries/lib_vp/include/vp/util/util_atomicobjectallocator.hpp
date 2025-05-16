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

namespace vp::util {

    template <typename T>
	class AtomicObjectAllocator {
        public:
            static constexpr u32 cInvalidOffset = 0xffff'ffff;
        private:
            struct FreeList {
                u32  next;
                char _padding[sizeof(T) - sizeof(u32)];
            };
            static_assert(sizeof(T) == sizeof(FreeList) && alignof(T) == alignof(FreeList));
        private:
            FreeList *m_array;
            union {
                u64 m_state;
                struct {
                    u64 m_last  : 32;
                    u64 m_count : 32;
                };
            };
        public:
            constexpr  AtomicObjectAllocator() : m_array(), m_state() {/*...*/}
            constexpr ~AtomicObjectAllocator() {/*...*/}

            bool Initialize(vp::imem::IHeap *heap, u32 max_object_count) {

                /* Ensure count */
                if (max_object_count == 0 || max_object_count == cInvalidOffset) { return false; }

                /* Allocate uninitialized array of objects */
                void *allocation = ::operator new(sizeof(T) * max_object_count, heap, alignof(T));
                if (allocation == nullptr) { return false; }

                /* Set free list and start address */
                FreeList *list_array = reinterpret_cast<FreeList*>(allocation);
                m_array              = list_array;
                m_state              = 0;

                /* Construct object free list */
                for (u32 i = 0; i < (max_object_count - 1); ++i) {
                    list_array[i].next = i + 1;
                }
                list_array[max_object_count - 1] = cInvalidOffset;

                return true;
            }

            void Finalize() {
                if (m_array != nullptr) {
                    ::operator delete(m_array);
                    m_array = nullptr;
                }
                return;
            }

            template <typename ... Args>
            T *Allocate(Args &&... args) {

                /* Try to allocate an object while list is not empty */
                u64 last_state = m_state;
                u32 offset     = static_cast<u32>(last_state);
                while (offset != cInvalidOffset) {

                    /* Try acquire object and increment offset */
                    FreeList *list = std::addressof(m_array[offset]);
                    const u64 next_state = ((last_state + 0x1'0000'0000) & 0xffff'ffff'0000'0000) | list->next;
                    if (!vp::util::InterlockedCompareExchangeAcquire(std::addressof(last_state), std::addressof(m_state), next_state, last_state)) {
                        offset = static_cast<u32>(last_state);
                        continue;
                    }

                    /* Construct new object */
                    T *allocation = reinterpret_cast<T>(list);
                    std::construct_at(allocation, std::forward<Args>(args) ...);

                    return allocation;
                }

                return nullptr;
            }

            void Free(T *object) {

                /* Destruct the object */
                std::destroy_at(object);

                /* Get the objects index */
                const u32 absolute_index = (reinterpret_cast<uintptr_t>(object) - reinterpret_cast<uintptr_t>(m_array)) / sizeof(T);

                /* Insert element to free list */
                FreeList *freed_object = reinterpret_cast<FreeList*>(object);
                u64 last_state = m_state;
                bool result;
                do {
                    freed_object->next   = static_cast<u32>(last_state);
                    const u64 next_state = ((last_state + 0x1'0000'0000) & 0xffff'ffff'0000'0000) | absolute_index;
                    result = vp::util::InterlockedCompareExchange(std::addressof(last_state), std::addressof(m_state), next_state, last_state);
                } while (!result);

                return;
            }
    };
}
