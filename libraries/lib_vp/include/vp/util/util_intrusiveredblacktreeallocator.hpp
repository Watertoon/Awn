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
            TreeType m_tree;
            ObjectAllocator<ParentType> m_allocator;
            u32 m_count;
            u32 m_max;
        public:
            constexpr IntrusiveRedBlackTreeAllocator()  : m_tree(), m_allocator() {/*...*/}
            constexpr ~IntrusiveRedBlackTreeAllocator() {/*...*/}

            Result Initialize(vp::imem::IHeap *heap, u32 count) {
                RESULT_TRY(m_allocator.Initialize(heap, count));
                m_count = 0;
                m_max = count;
                RESULT_RETURN_SUCCESS;
            }
            void Finalize() {
                m_tree.ForceReset();
                m_allocator.Finalize();
            }
        public:
            ALWAYS_INLINE TreeType::iterator begin() { return m_tree.begin(); }
            constexpr TreeType::iterator end()   { return m_tree.end(); }
            ALWAYS_INLINE TreeType::const_iterator begin() const { return m_tree.begin(); }
            constexpr TreeType::const_iterator end() const { return m_tree.end(); }

            template <typename ... Args>
            ParentType *Allocate(KeyType key, Args &&... args) {
                ParentType *new_node = m_allocator.Allocate(std::forward<Args>(args) ...);
                VP_ASSERT(new_node != nullptr);
                RBTraits::Traits::GetTreeNode(new_node)->SetKey(key);
                m_tree.Insert(new_node);
                ++m_count;
                return new_node;
            }
            void Free(ParentType *node) {
                VP_ASSERT(m_allocator.IsContain(node, m_max));
                m_tree.Remove(node);
                m_allocator.Free(node);
                --m_count;
                return;
            }
            void Free(KeyType key) {
                ParentType *node = this->Find(key);
                VP_ASSERT(node != nullptr);
                this->Free(node);
            }

            ParentType *Find(KeyType key) { return m_tree.Find(key); }

            void Clear() { m_allocator.Clear(m_max); }

            constexpr TreeType *GetTree() { return std::addressof(m_tree); }
            constexpr u32 GetCount() const { return m_count; }
            constexpr u32 GetMaxCount() const { return m_max; }
            constexpr bool IsValid() const { return m_allocator.IsValid(); }
    };
}
