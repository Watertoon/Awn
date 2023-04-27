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
        private:
            using DisposerList = vp::util::IntrusiveListTraits<IDisposer, &IDisposer::m_disposer_list_node>::List;
        private:
            DisposerList                m_disposer_list;
            sys::ServiceCriticalSection m_heap_cs;
            bool                        m_is_thread_safe;
        public:
            VP_RTTI_DERIVED(Heap, IHeap);
        private:
            void AppendDisposer(IDisposer &disposer) {
                ScopedHeapLock lock(this);
                m_disposer_list.PushBack(disposer);
            }

            void RemoveDisposer(IDisposer &disposer) {
                ScopedHeapLock lock(this);
                m_disposer_list.Remove(disposer);
            }
        public:
            void PushBackChild(Heap *child) {
                ScopedHeapLock lock(this);
                m_child_list.PushBack(*child);
            }
        public:
            explicit Heap(const char *name, IHeap *parent_heap, void *start_address, size_t size, bool is_thread_safe) : IHeap(name, parent_heap, start_address, size), m_disposer_list(), m_heap_cs(), m_is_thread_safe(is_thread_safe) {/*...*/}
    };

    constexpr ScopedHeapLock::ScopedHeapLock(Heap *heap) : m_heap(heap) {
        VP_ASSERT(heap != nullptr);
        if (m_heap->m_is_thread_safe == true) { m_heap->m_heap_cs.Enter(); }
    }
    constexpr ScopedHeapLock::~ScopedHeapLock() {
        if (m_heap->m_is_thread_safe == true) { m_heap->m_heap_cs.Leave(); }
    }
}
