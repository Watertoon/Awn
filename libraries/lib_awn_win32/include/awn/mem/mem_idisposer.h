#pragma once

namespace awn::mem {

    class Heap;

    class IDisposer {
        public:
            friend class Heap;
        private:
            Heap                        *m_contained_heap;
            vp::util::IntrusiveListNode  m_disposer_list_node;
        public:
            constexpr ALWAYS_INLINE IDisposer();
            constexpr ALWAYS_INLINE IDisposer(Heap *heap);
            constexpr ALWAYS_INLINE virtual ~IDisposer();
            
            constexpr ALWAYS_INLINE Heap *GetDisposerHeap() { return m_contained_heap; }
    };
}
