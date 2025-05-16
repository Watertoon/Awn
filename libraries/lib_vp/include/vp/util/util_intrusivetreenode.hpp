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

    class IntrusiveTreeNode {
        private:
            IntrusiveTreeNode *m_parent;
            IntrusiveTreeNode *m_child;
            IntrusiveTreeNode *m_sibling_next;
            IntrusiveTreeNode *m_sibling_prev;
        public:
            template <bool IsConst>
            class Iterator {
                public:
                    using value_type      = IntrusiveTreeNode;
                    using reference       = IntrusiveTreeNode&;
                    using const_reference = const IntrusiveTreeNode&;
                    using pointer         = IntrusiveTreeNode*;
                    using const_pointer   = const IntrusiveTreeNode*;
                private:
                    IntrusiveTreeNode *m_node;
                public:
                    constexpr ALWAYS_INLINE Iterator() : m_node() {/*...*/}
                    constexpr ALWAYS_INLINE Iterator(IntrusiveTreeNode *node) : m_node(node) {/*...*/}
                    constexpr ALWAYS_INLINE Iterator(const IntrusiveTreeNode *node) : m_node(const_cast<IntrusiveTreeNode*>(node)) {/*...*/}
                    constexpr ALWAYS_INLINE ~Iterator() {/*...*/}

                    constexpr ALWAYS_INLINE reference operator*() {
                        return *m_node;
                    }
                    constexpr ALWAYS_INLINE const_reference operator*() const {
                        return *m_node;
                    }

                    constexpr ALWAYS_INLINE bool operator==(const Iterator<IsConst> &rhs) const {
                        return m_node == rhs.m_node;
                    }

                    constexpr ALWAYS_INLINE bool operator!=(const Iterator<IsConst> &rhs) const {
                        return m_node != rhs.m_node;
                    }

                    constexpr ALWAYS_INLINE Iterator<IsConst> &operator++() {
                        IntrusiveTreeNode *child = m_node->GetChild();
                        if (child != nullptr) {
                            m_node = child;
                        } else {                            
                            m_node = m_node->GetNextSibling();
                        }
                        return *this;
                    }
                    //constexpr ALWAYS_INLINE Iterator<IsConst> &operator--() {
                    //    IntrusiveTreeNode *parent = m_node->GetParent();
                    //    if (parent != nullptr) {
                    //        m_node = parent;
                    //    } else {                            
                    //        m_node = m_node->GetPrevSibling();
                    //    }
                    //    return *this;
                    //}
            };
        public:
            using iterator = Iterator<false>;
            using const_iterator = Iterator<true>;
        private:
            constexpr void ClearChildrenRecursive() {
                IntrusiveTreeNode *iter = m_child;
                while (iter != nullptr) {
                    iter = iter->m_sibling_next;
                    m_child->ClearChildrenRecursive();
                    m_child->Clear();
                }
            }
        public:
            constexpr IntrusiveTreeNode() : m_parent(), m_child(), m_sibling_next(), m_sibling_prev() {/*...*/}
            constexpr ~IntrusiveTreeNode() {/*...*/}

            constexpr iterator begin() {
                return iterator(this);
            }
            constexpr iterator end() {
                return iterator();
            }
            constexpr const_iterator begin() const {
                return const_iterator(this);
            }
            constexpr const_iterator end() const {
                return const_iterator();
            }

            constexpr void Clear() {
                m_parent = nullptr;
                m_child = nullptr;
                m_sibling_next = nullptr;
                m_sibling_prev = nullptr;
            }

            constexpr void DetachAll() {
                this->DetachSubTree();
                this->ClearChildrenRecursive();
                this->Clear();
            }

            constexpr void DetachSubTree() {

                IntrusiveTreeNode *iter = m_parent;

                /* Detach from parent */
                if (iter == nullptr || iter->m_child != this) {
                    iter = m_sibling_prev;
                    if (iter != nullptr) {
                        iter->m_sibling_next = m_sibling_next;
                    }
                    iter = m_sibling_next;
                    if (iter == nullptr) {
                        if (m_parent != nullptr) {
                            m_parent->m_child->m_sibling_prev = m_sibling_prev;
                        }
                    } else {
                        iter->m_sibling_prev = m_sibling_prev;
                        m_sibling_next = nullptr;
                    }
                } else {
                    iter->m_child = m_sibling_next;
                    iter = m_sibling_next;
                    if(iter != nullptr) {
                        iter->m_sibling_prev = m_sibling_prev;
                        m_sibling_next = nullptr;
                    }
                }

                m_sibling_prev = nullptr;
                m_parent = nullptr;
            }

            constexpr void PushBackChild(IntrusiveTreeNode *new_child) {
                new_child->DetachSubTree();

                if (m_child == nullptr) {
                    m_child                   = new_child;
                    new_child->m_parent       = this;
                    new_child->m_sibling_prev = new_child;
                    return;
                }

                IntrusiveTreeNode *sibling_end = m_child->m_sibling_prev;
                sibling_end->m_sibling_next    = new_child;
                new_child->m_sibling_prev      = sibling_end;
                new_child->m_parent            = sibling_end->m_parent;
                m_child->m_sibling_prev        = new_child;
            }

            constexpr void PushFrontChild(IntrusiveTreeNode *new_child) {
                new_child->DetachSubTree();

                if (m_child == nullptr) {
                    m_child                   = new_child;
                    new_child->m_parent       = this;
                    new_child->m_sibling_prev = new_child;
                    return;
                }

                new_child->m_sibling_next = m_child;
                new_child->m_sibling_prev = m_child->m_sibling_prev;
                m_child->m_sibling_prev = new_child;
                m_child = new_child;
                new_child->m_parent = this;
            }

            constexpr void PushBackSibling(IntrusiveTreeNode *new_sibling) {
                new_sibling->DetachSubTree();

                /* Find end of sibling list */
                IntrusiveTreeNode *sibling_list_end = nullptr;
                if (m_parent != nullptr && m_parent->m_child != nullptr) {
                    sibling_list_end                  = m_parent->m_child->m_sibling_prev;
                    m_parent->m_child->m_sibling_prev = new_sibling;
                } else {
                    sibling_list_end = this;
                    while (sibling_list_end->m_sibling_next != nullptr) {
                        sibling_list_end = sibling_list_end->m_sibling_next;
                    }
                }

                sibling_list_end->m_sibling_next = new_sibling;
                new_sibling->m_sibling_prev      = sibling_list_end;
                new_sibling->m_parent            = sibling_list_end->m_parent;
            }

            constexpr IntrusiveTreeNode *FindRoot() {
                IntrusiveTreeNode *root = this;
                while (root->m_parent != nullptr) {
                    root = root->m_parent;
                }
                return root;
            }

            constexpr const IntrusiveTreeNode *FindRoot() const {
                const IntrusiveTreeNode *root = this;
                while (root->m_parent != nullptr) {
                    root = root->m_parent;
                }
                return root;
            }

            constexpr u32 GetCount() const {
                u32 count = 0;
                for ([[maybe_unused]] const IntrusiveTreeNode &node : *this) {
                    ++count;
                }
                return count;
            }

            constexpr IntrusiveTreeNode *GetChild()       { return m_child; }
            constexpr IntrusiveTreeNode *GetParent()      { return m_parent; }
            constexpr IntrusiveTreeNode *GetNextSibling() { return m_sibling_next; }
            constexpr IntrusiveTreeNode *GetPrevSibling() { return m_sibling_prev; }

            constexpr IntrusiveTreeNode *GetChild()       const { return m_child; }
            constexpr IntrusiveTreeNode *GetParent()      const { return m_parent; }
            constexpr IntrusiveTreeNode *GetNextSibling() const { return m_sibling_next; }
            constexpr IntrusiveTreeNode *GetPrevSibling() const { return m_sibling_prev; } 
    };

    template<class RP, auto M>
    struct IntrusiveTreeNodeMemberTraits {
        static constexpr uintptr_t cOffset = OffsetOf<M>();

        static ALWAYS_INLINE RP *GetParent(IntrusiveTreeNode *node) {
            return reinterpret_cast<RP*>(reinterpret_cast<uintptr_t>(node) - cOffset);
        }
        static ALWAYS_INLINE RP &GetParentReference(IntrusiveTreeNode *node) {
            return *reinterpret_cast<RP*>(reinterpret_cast<uintptr_t>(node) - cOffset);
        }

        static ALWAYS_INLINE const RP *GetParent(const IntrusiveTreeNode *node) {
            return reinterpret_cast<RP*>(reinterpret_cast<uintptr_t>(node) - cOffset);
        }
        static ALWAYS_INLINE const RP &GetParentReference(const IntrusiveTreeNode *node) {
            return *reinterpret_cast<const RP*>(reinterpret_cast<uintptr_t>(node) - cOffset);
        }

        static constexpr ALWAYS_INLINE IntrusiveTreeNode *GetTreeNode(RP *parent) {
            return std::addressof(parent->*M);
        }
        static constexpr ALWAYS_INLINE IntrusiveTreeNode *GetTreeNode(const RP *parent) {
            return std::addressof(parent->*M);
        }
    };
}
