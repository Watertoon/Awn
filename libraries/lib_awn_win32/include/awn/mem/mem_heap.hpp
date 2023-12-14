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

namespace awn::mem {

    using AllocationMode = vp::imem::AllocationMode;
    using AddressRange   = vp::imem::AddressRange;
    using MemoryRange    = vp::imem::MemoryRange;

    class Heap;

    class ScopedHeapLock {
        private:
            Heap *m_heap;
        public:
            explicit constexpr ScopedHeapLock(Heap *heap);
            constexpr ~ScopedHeapLock();
    };

    class Heap : public vp::imem::IHeap {
        public:
            friend class  IDisposer;
            friend class  ScopedHeapLock;
            friend Heap  *FindHeapByNameImpl(Heap *parent_heap, const char *heap_name);
        public:
            static constexpr size_t cWholeSize = 0;
        private:
            using DisposerList = vp::util::IntrusiveListTraits<IDisposer, &IDisposer::m_disposer_list_node>::List;
        private:
            DisposerList                m_disposer_list;
            sys::ServiceCriticalSection m_heap_cs;
            bool                        m_is_thread_safe;
        public:
            VP_RTTI_DERIVED(Heap, vp::imem::IHeap);
        private:
            void AppendDisposer(IDisposer &disposer) {
                ScopedHeapLock lock(this);
                m_disposer_list.PushBack(disposer);
            }

            void RemoveDisposer(IDisposer &disposer) {
                ScopedHeapLock lock(this);
                m_disposer_list.Remove(disposer);
            }

        protected:
            constexpr ALWAYS_INLINE void LockHeapIfSafe() {
                if (this->IsThreadSafe() == true) { m_heap_cs.Enter(); }
            }
            constexpr ALWAYS_INLINE void UnlockHeapIfSafe() {
                if (this->IsThreadSafe() == true) { m_heap_cs.Leave(); }
            }

            void DisposeAll() {

                auto disposer_iter = m_disposer_list.begin();
                while (disposer_iter != m_disposer_list.end()) {

                    /* Advance disposer iter*/
                    IDisposer *disposer = std::addressof(*disposer_iter);
                    ++disposer_iter;

                    /* Remove disposer from heap */
                    disposer->RemoveContainedHeapUnsafe();

                    /* Destruct disposer */
                    std::destroy_at(disposer);
                }

                return;
            }
        public:
            void PushBackChild(Heap *child) {
                std::scoped_lock l(*GetHeapManagerLock());
                ScopedHeapLock lock(this);
                m_child_list.PushBack(*child);
            }
            void RemoveChild(Heap *child) {
                std::scoped_lock l(*GetHeapManagerLock());
                ScopedHeapLock lock(this);
                m_child_list.Remove(*child);
            }
            void RemoveChildUnsafe(Heap *child) {
                std::scoped_lock l(*GetHeapManagerLock());
                m_child_list.Remove(*child);
            }
            void Destruct() {

                /* Destroy disposers */
                {
                    ScopedHeapLock l(this);
                    this->DisposeAll();
                }

                /* Remove parent heap */
                if (m_parent_heap != nullptr) { 
                    if (awn::mem::Heap::CheckRuntimeTypeInfo(m_parent_heap) == true) {
                        reinterpret_cast<awn::mem::Heap*>(m_parent_heap)->RemoveChild(this);                         
                    } else {         
                        std::scoped_lock l(*GetHeapManagerLock());               
                        m_parent_heap->m_child_list.Remove(*this);
                    }
                    m_parent_heap = nullptr;
                }

                return;
            }
        public:
            explicit Heap(const char *name, IHeap *parent_heap, void *start_address, size_t size, bool is_thread_safe) : IHeap(name, parent_heap, start_address, size), m_disposer_list(), m_heap_cs(), m_is_thread_safe(is_thread_safe) {/*...*/}
            virtual ~Heap() override {

                this->Destruct();

                return;
            }

            constexpr ALWAYS_INLINE bool IsThreadSafe() { return m_is_thread_safe; }

            virtual constexpr bool IsGpuHeap() const { return false; }
    };

    constexpr ScopedHeapLock::ScopedHeapLock(Heap *heap) : m_heap(heap) {
        VP_ASSERT(heap != nullptr);
        m_heap->LockHeapIfSafe();
        
    }
    constexpr ScopedHeapLock::~ScopedHeapLock() {
        m_heap->UnlockHeapIfSafe();
    }
}
