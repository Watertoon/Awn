#pragma once

namespace awn::mem {
    
    constexpr ALWAYS_INLINE IDisposer::IDisposer() : m_contained_heap(nullptr), m_disposer_list_node() {

        if (std::is_constant_evaluated() == true) { return; }
    
        /* Find heap containing IDisposer */
        m_contained_heap = mem::FindHeapFromAddress(reinterpret_cast<void*>(this));

        if (m_contained_heap != nullptr) {
            /* Add to contained heap */
            m_contained_heap->AppendDisposer(*this);
        }
    }

    constexpr ALWAYS_INLINE IDisposer::IDisposer(Heap *heap) : m_contained_heap(nullptr), m_disposer_list_node() {

        if (std::is_constant_evaluated() == true) { return; }

        /* Set contained heap */
        m_contained_heap = heap;
        
        /* Fallback to heap manager */
        if (m_contained_heap == nullptr) {
            m_contained_heap = mem::FindHeapFromAddress(reinterpret_cast<void*>(this));
        }

        /* Add to contained heap */
        if (m_contained_heap != nullptr) {
            m_contained_heap->AppendDisposer(*this);
        }
    }

    constexpr ALWAYS_INLINE IDisposer::~IDisposer() {
        
        /* Remove from contained heap disposer list */
        if (m_contained_heap != nullptr) {
            VP_ASSERT(m_disposer_list_node.IsLinked() == true);
            m_contained_heap->RemoveDisposer(*this);
            m_contained_heap = nullptr;
        }
    }
}
