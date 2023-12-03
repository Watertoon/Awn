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
    
    
    constexpr ALWAYS_INLINE void IDisposer::RemoveContainedHeapUnsafe() {
        if (m_contained_heap != nullptr) {
            VP_ASSERT(m_disposer_list_node.IsLinked() == true);
            m_contained_heap->m_disposer_list.Remove(*this);
            m_contained_heap = nullptr;
        }
    }
}
