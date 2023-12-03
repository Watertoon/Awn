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

    class Heap;

    class IDisposer {
        public:
            friend class Heap;
        private:
            Heap                        *m_contained_heap;
            vp::util::IntrusiveListNode  m_disposer_list_node;
        private:
            constexpr ALWAYS_INLINE void RemoveContainedHeapUnsafe();
        public:
            constexpr ALWAYS_INLINE IDisposer();
            constexpr ALWAYS_INLINE IDisposer(Heap *heap);
            constexpr ALWAYS_INLINE virtual ~IDisposer();

            constexpr ALWAYS_INLINE Heap *GetDisposerHeap() { return m_contained_heap; }
    };
}
