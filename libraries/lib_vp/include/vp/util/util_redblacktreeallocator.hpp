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

    template <class RBTraits>
    class IntrusiveRedBlackTreeAllocator {
        public:
            using TreeType   = RBTraits::Tree;
            using ParentType = RBTraits::Parent;
            using KeyType    = RBTraits::Key;
        private:
            TreeType                        m_tree;
            HeapObjectAllocator<ParentType> m_allocator;
        public:
            constexpr IntrusiveRedBlackTreeAllocator()  : m_tree(), m_allocator() {/*...*/}
            constexpr ~IntrusiveRedBlackTreeAllocator() {/*...*/}

            void Initialize(vp::imem::IHeap *heap, u32 count) {
                m_allocator.Initialize(heap, count);
            }
            void Finalize() {
                m_tree.ForceReset();
                m_allocator.Finalize();
            }

            template <typename ... Args>
            ParentType *Allocate(KeyType key, Args &&... args) {
                ParentType *new_node = m_allocator.Allocate(std::forward<Args>(args) ...);
                RBTraits::Traits::GetTreeNode(new_node)->SetKey(key);
                m_tree.Insert(new_node);
                return new_node;
            }
            void Free(ParentType *node) {
                m_tree.Remove(node);
                m_allocator.Free(node);
            }
            void Free(KeyType key) {
                ParentType *node = this->Find(key);
                this->Free(node);
            }

            ParentType *Find(KeyType key) { return m_tree.Find(key); }

            constexpr TreeType *GetTree() { return std::addressof(m_tree); }
    };
}
