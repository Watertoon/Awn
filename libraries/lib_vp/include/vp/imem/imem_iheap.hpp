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

namespace vp::imem {

    enum class AllocationMode : u8 {
        FirstFit = 0,
        BestFit = 1,
        WorstFit = 2,
    };

    struct MemoryRange {
        void   *address;
        size_t  size;
    };

    struct AddressRange {
        void *start;
        void *end;
        
        ALWAYS_INLINE ptrdiff_t GetSize() const { return reinterpret_cast<intptr_t>(start) - reinterpret_cast<intptr_t>(end); }
    };

    class IHeap {
        public:
            using SynchronizationDelegate = util::IFunction<void*>;
        protected:
            void                    *m_start_address;
            void                    *m_end_address;
            IHeap                   *m_parent_heap;
            util::IntrusiveListNode  m_child_list_node;
        protected:
            using ChildList = util::IntrusiveListTraits<IHeap, &IHeap::m_child_list_node>::List;
        public:
            ChildList                m_child_list;
            const char              *m_name;
        public:
            VP_RTTI_BASE(IHeap);
        public:
            explicit IHeap(const char *name, IHeap *parent_heap, void *start_address, size_t size) : m_start_address(start_address), m_end_address(reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(start_address) +  size)), m_parent_heap(parent_heap), m_child_list_node(), m_child_list(), m_name(name) {/*...*/}
            virtual constexpr ~IHeap() {/*...*/}

            virtual void Finalize() {/*...*/}

            virtual MemoryRange AdjustHeap() { return { m_end_address, 0 }; }
            virtual size_t AdjustAllocation(void *address, size_t new_size) { VP_UNUSED(address, new_size); return 0; }

            virtual void *TryAllocate(size_t size, s32 alignment) { VP_UNUSED(size, alignment); return nullptr; }
            virtual void *TryReallocate(void *address, size_t size, s32 alignment) { VP_UNUSED(address, size, alignment); return nullptr; }

            virtual void Free(void *address) { VP_UNUSED(address); }
            virtual void FreeAll() {/*...*/}

            virtual IHeap *FindHeapFromAddress(void *address) {

                /* Check if address is in this IHeap */
                if (!this->IsAddressInHeap(address)) { return nullptr; }

                /* Search for address in child IHeaps */
                for (IHeap &child : m_child_list) {
                    if (!child.IsAddressInHeap(address)) { continue; }

                    /* Continue the search in child IHeap */
                    return child.FindHeapFromAddress(address);
                }

                /* Not in children, we own it */
                return this;
            }

            virtual bool IsAddressInHeap(const void *address) { return (m_start_address <= address && address < m_end_address); }
            virtual bool IsAddressAllocation(const void *address) { VP_UNUSED(address); return false; }

            virtual bool IsEmpty() const { return false; }

            virtual size_t GetTotalSize() const {
                return reinterpret_cast<uintptr_t>(m_end_address) - reinterpret_cast<uintptr_t>(m_start_address);
            }
            virtual size_t GetTotalFreeSize() { return 0; }
            virtual size_t GetMaximumAllocatableSize(s32 alignment) { VP_UNUSED(alignment); return 0; }
        public:
            constexpr ALWAYS_INLINE const char *GetName()     const { return m_name; }
            constexpr ALWAYS_INLINE bool        HasChildren() const { return m_child_list.IsEmpty(); }

            constexpr ALWAYS_INLINE void *GetStartAddress() const { return m_start_address; }
            constexpr ALWAYS_INLINE void *GetEndAddress()   const { return m_end_address; }

            constexpr ALWAYS_INLINE IHeap *GetParentHeap() const { return m_parent_heap; }
    };
}
