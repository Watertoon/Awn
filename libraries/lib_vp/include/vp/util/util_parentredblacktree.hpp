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

    template<typename RP, typename K>
    class ParentRedBlackTreeNode {
        public:
            enum Color : u8 {
                Color_Red   = 0,
                Color_Black = 1,
            };
        public:
            using key_type = K;
            using pointer = RP*;
        public:
            ParentRedBlackTreeNode *m_left;
            ParentRedBlackTreeNode *m_right;
            ParentRedBlackTreeNode *m_parent;
            bool                    m_color;
            key_type                m_key;
            pointer                 m_owner;
        public:
            constexpr ALWAYS_INLINE  ParentRedBlackTreeNode(pointer owner) : m_left(), m_right(), m_parent(), m_color(), m_key(), m_owner(owner) {/*...*/}
            constexpr ALWAYS_INLINE  ParentRedBlackTreeNode(pointer owner, key_type key) : m_left(), m_right(), m_parent(), m_color(), m_key(key), m_owner(owner) {/*...*/}
            constexpr ALWAYS_INLINE ~ParentRedBlackTreeNode() {/*...*/}

            constexpr bool IsEnd() const {
                return (m_right == nullptr) && ((m_parent == nullptr) || (m_parent->m_right == this));
            }

            constexpr bool IsLinked() const {
                return (m_parent != nullptr) || (m_left != nullptr) || (m_right != nullptr);
            }

            constexpr ALWAYS_INLINE void ForceUnlink() {
                m_left   = nullptr;
                m_right  = nullptr;
                m_parent = nullptr;
                m_color  = Color_Red;
            }

            constexpr ParentRedBlackTreeNode *GetNext() {

                ParentRedBlackTreeNode *iter = m_right;
                ParentRedBlackTreeNode *next = nullptr;

                /* Traverse down right node's left */
                if (m_right != nullptr) {
                    do {
                        next = iter;
                        iter = next->m_left;
                    } while (next->m_left != nullptr);
                    return next;
                }
                
                /* Return parent */
                next = m_parent;
                if (next != nullptr && next->m_left != this) {
                    ParentRedBlackTreeNode *r_child = this;
                    do {
                        iter    = next;
                        if (r_child != iter->m_right) { return iter; }
                        next    = iter->m_parent;
                        r_child = iter;
                    } while (next != nullptr);
                }

                return next;
            }

            constexpr const ParentRedBlackTreeNode *GetNext() const {

                const ParentRedBlackTreeNode *iter = m_right;
                const ParentRedBlackTreeNode *next = nullptr;

                /* Traverse down right node's left */
                if (m_right != nullptr) {
                    do {
                        next = iter;
                        iter = next->m_left;
                    } while (next->m_left != nullptr);
                    return next;
                }

                /* Return parent */
                next = m_parent;
                if (next != nullptr && next->m_left != this) {
                    const ParentRedBlackTreeNode *r_child = this;
                    do {
                        iter    = next;
                        if (r_child != iter->m_right) { return iter; }
                        next    = iter->m_parent;
                        r_child = iter;
                    } while (next != nullptr);
                }

                return next;
            }

            constexpr ALWAYS_INLINE void SetKey(key_type hash) {
                m_key = hash;
            }
            constexpr ALWAYS_INLINE key_type GetKey() const {
                return m_key;
            }
    };

	template<typename T, typename K, class Traits, Traits::ComparisonFunction Compare, Traits::CollisionFunction Collision>
	class ParentRedBlackTree {
		public:
            using value_type      = T;
            using reference       = T&;
            using const_reference = const T&;
            using pointer         = T*;
            using const_pointer   = const T*;
            using node_type       = ParentRedBlackTreeNode<T, K>;
            using key_type        = K;
        public:
            class Iterator {
                private:
                    node_type *m_node;
                public:
                    constexpr ALWAYS_INLINE Iterator(node_type *node) : m_node(node) {/*...*/}
                    constexpr ALWAYS_INLINE ~Iterator() {/*...*/}

                    constexpr ALWAYS_INLINE reference operator*() {
                        return *m_node->m_owner;
                    }
                    constexpr ALWAYS_INLINE const_reference operator*() const {
                        return *m_node->m_owner;
                    }

                    constexpr ALWAYS_INLINE bool operator==(const Iterator &rhs) const {
                        return !(m_node != rhs.m_node);
                    }

                    constexpr ALWAYS_INLINE bool operator!=(const Iterator &rhs) const {
                        return m_node != rhs.m_node;
                    }

                    constexpr ALWAYS_INLINE Iterator &operator++() {
                        m_node = GetNext(m_node);
                        return *this;
                    }
                    constexpr ALWAYS_INLINE Iterator &operator++([[maybe_unused]]int) {
                        m_node = GetNext(m_node);
                        return *this;
                    }
            };
            class ConstIterator {
                private:
                    const node_type *m_node;
                public:
                    constexpr ALWAYS_INLINE ConstIterator(const node_type *node) : m_node(node) {/*...*/}
                    constexpr ALWAYS_INLINE ~ConstIterator() {/*...*/}

                    constexpr ALWAYS_INLINE const_reference operator*() const {
                        return *m_node->m_owner;
                    }

                    constexpr ALWAYS_INLINE bool operator==(const ConstIterator &rhs) const {
                        return !(m_node != rhs.m_node);
                    }

                    constexpr ALWAYS_INLINE bool operator!=(const ConstIterator &rhs) const {
                        return m_node != rhs.m_node;
                    }

                    constexpr ALWAYS_INLINE ConstIterator &operator++() {
                        m_node = GetNext(m_node);
                        return *this;
                    }
                    constexpr ALWAYS_INLINE ConstIterator &operator++([[maybe_unused]]int) {
                        m_node = GetNext(m_node);
                        return *this;
                    }
            };
        public:
            using iterator = Iterator;
            using const_iterator = ConstIterator;
        public:
            node_type *m_root;
        public:
            constexpr node_type *RotateRight(node_type *rot_i) {

                /* Save rot_i's left, parent, and left's right */
                node_type *original_left       = rot_i->m_left;
                node_type *original_parent     = rot_i->m_parent;
                node_type *original_left_right = original_left->m_right;

                /* Move the original left's right to rot_i's new left */
                rot_i->m_left = original_left_right;
                if (original_left_right != nullptr) { original_left_right->m_parent = rot_i; }

                /* Move rot_i to the right of the original left */
                original_left->m_right = rot_i;
                rot_i->m_parent = original_left;

                /* Set original left's parent to the root if no original parent */
                if (original_parent == nullptr) {
                    m_root = original_left;
                    original_left->m_parent = nullptr;
                    return original_left;
                }

                /* Set original left's parent to the original parent */
                if (original_parent->m_left != rot_i) {
                    original_parent->m_right = original_left;
                } else {
                    original_parent->m_left = original_left;
                }
                original_left->m_parent = original_parent;

                return original_left;
            }

            constexpr node_type *RotateLeft(node_type *rot_i) {

                /* Save rot_i's right, parent, and right's left */
                node_type *original_right      = rot_i->m_right;
                node_type *original_parent     = rot_i->m_parent;
                node_type *original_right_left = original_right->m_left;

                /* Move the original right's left to rot_i's new right */
                rot_i->m_right = original_right_left;
                if (original_right_left != nullptr) { original_right_left->m_parent = rot_i; }

                /* Move rot_i to the left of the original right */
                original_right->m_left = rot_i;
                rot_i->m_parent = original_right;

                /* Set original right's parent to the root if no original parent */
                if (original_parent == nullptr) {
                    m_root = original_right;
                    original_right->m_parent = nullptr;
                    return original_right;
                }

                /* Set original right's parent to the original parent */
                if (original_parent->m_left != rot_i) {
                    original_parent->m_right = original_right;
                } else {
                    original_parent->m_left = original_right;
                }
                original_right->m_parent = original_parent;

                return original_right;
            }

            constexpr void InsertFixup(node_type *insert, TreeNodeTrace node_trace) {

                /* Nothing to do if no traversal count */
                if (node_trace.traversal_count < 2) { return; }

                /* Setup trace iterators */
                u32 traversal_iter = node_trace.traversal_count;

                size_t side_iter      = node_trace.trace;
                size_t next_side_iter = node_trace.trace >> 1;

                /* Percolate up as long as there is a parent */
                node_type *parent_iter = insert->m_parent;
                while (parent_iter != nullptr && parent_iter->m_color == static_cast<bool>(node_type::Color_Red)) {

                    /* Advance trace */
                    node_trace.trace           = next_side_iter >> 1;
                    node_trace.traversal_count = traversal_iter - 2;

                    /* Get parent_iter parent and the parent_parent_iter's side */
                    node_type *parent_parent_iter = parent_iter->m_parent;
                    node_type *parent_parent_side = ((next_side_iter & 1) == 0) ? parent_parent_iter->m_right: parent_parent_iter->m_left;

                    /* Repaint nodes if parent parent side is red */
                    if ((parent_parent_side != nullptr) && (parent_parent_side->m_color == static_cast<bool>(node_type::Color_Red))) {

                        /* Repaint  */
                        parent_iter->m_color = node_type::Color_Black;
                        parent_parent_side->m_color  = node_type::Color_Black;
                        parent_parent_iter->m_color  = node_type::Color_Red;

                        /* Advance parent iterator */
                        parent_iter = parent_parent_iter->m_parent;

                        /* Advance traversal count iter */
                        traversal_iter = node_trace.traversal_count;
                        if (traversal_iter < 2) { break; }

                        /* Advance depth side mask */
                        side_iter      = node_trace.trace;
                        next_side_iter = side_iter >> 1;

                        continue;
                    }

                    /* Determine rotation */
                    node_type *original_side = nullptr;
                    if (((next_side_iter & 1) == 0) == ((side_iter & 1) == 0)) {
                        /* Dual black left rotation or red right rotation */
                        if ((side_iter & 1) == 0) { original_side = this->RotateRight(parent_parent_iter); }
                        else                { original_side = this->RotateLeft(parent_parent_iter); }
                    } else if ((side_iter & 1) == 0) {
                        /* Red black, right rotate, left rotate */
                        original_side = this->RotateRight(parent_iter);
                        this->RotateLeft(parent_parent_iter);
                    } else {
                        /* Black red left rotate, right rotate */
                        original_side = this->RotateLeft(parent_iter);
                        this->RotateRight(parent_parent_iter);
                    }

                    original_side->m_color = node_type::Color_Black;
                    parent_parent_iter->m_color = node_type::Color_Red;

                    return;
                }

                return;
            }

            constexpr TreeNodeTrace TraceParent(node_type *parent_node, node_type *node) {

                size_t     mask = 0;
                u32        i    = 0;
                do {

                    /* Build mask */
                    const size_t p_side = (parent_node->m_left != node); 
                    mask                = mask | (p_side << (i & 0x3f));

                    /* Iterate */
                    ++i;
                    node        = parent_node;
                    parent_node = parent_node->m_parent;
                } while (parent_node != nullptr);

                return { mask, i };
            }

            constexpr void Transplant(node_type **root, node_type *replace_node, node_type *move_node) {

                node_type *parent_node = replace_node->m_parent;
                if (parent_node == nullptr) {
                    *root = move_node;
                } else if (parent_node->m_left == replace_node) {
                    parent_node->m_left = move_node;
                } else {
                    parent_node->m_right = move_node;
                }
            }

            constexpr TreeNodeTrace RemoveSwap(node_type **out_remove, u32 *out_fixup_color) {

                /* Handle transplant of left, right, left right */
                TreeNodeTrace  trace        = {};
                node_type     *remove_node = *out_remove;
                node_type     *moved_node  = nullptr;
                node_type     *parent_node = remove_node->m_parent;
                u32            fixup_color = remove_node->m_color;
                if (remove_node->m_right == nullptr) {

                    /* Select node to move */
                    moved_node = remove_node->m_left;

                    /* Get parent */
                    if (parent_node != nullptr) {
                        /* Trace parent */
                        trace = this->TraceParent(parent_node, remove_node);
                    }

                    /* Transplant left node to parent */
                    this->Transplant(std::addressof(m_root), remove_node, moved_node);
                    *out_remove = parent_node;

                    /* Move parent */
                    if (moved_node != nullptr) {
                        moved_node->m_parent = parent_node;
                    }

                } else if (remove_node->m_left == nullptr) {

                    /* Select node to move */
                    moved_node = remove_node->m_right; 

                    if (parent_node != nullptr) {
                        /* Trace parent */
                        trace = this->TraceParent(parent_node, remove_node);
                    }

                    /* Transplant right node to parent */
                    this->Transplant(std::addressof(m_root), remove_node, moved_node);
                    *out_remove = parent_node;

                    /* Move parent */
                    if (moved_node != nullptr) {
                        moved_node->m_parent = parent_node;
                    }

                } else {

                    /* Get next node */
                    node_type *next_node = nullptr;
                    node_type *iter      = remove_node->m_right;
                    do {
                        next_node = iter;
                        iter = next_node->m_left;
                    } while (iter != nullptr);

                    /* Trace next parent */
                    node_type *next_parent = next_node->m_parent;
                    fixup_color            = next_node->m_color;
                    *out_remove = next_parent;
                    if (next_parent != nullptr) {
                        trace = this->TraceParent(next_parent, next_node);
                    }

                    /* Move remove node color to next node */
                    next_node->m_color = remove_node->m_color;

                    /* If next node is directly to the right */
                    if (remove_node->m_right == next_node) {

                        /* Transplant next node to remove node's place */
                        this->Transplant(std::addressof(m_root), remove_node, next_node);
                        next_node->m_parent = parent_node;

                        /* Move remove node's left node to the next node */
                        next_node->m_left = remove_node->m_left;
                        remove_node->m_left->m_parent = next_node;

                        *out_remove = next_node;

                    } else {

                        /* Move next node to remove node's parent */
                        node_type *right_next_node = next_node->m_right;
                        if (right_next_node != nullptr) {
                            if (next_parent->m_left != next_node) {
                                next_parent->m_right = right_next_node;
                            } else {
                                next_parent->m_left = right_next_node;
                            }
                            right_next_node->m_parent = next_parent;
                        } else {
                            next_parent->m_left = nullptr;
                        }
                        next_node->m_right  = remove_node->m_right;
                        next_node->m_left   = remove_node->m_left;
                        next_node->m_parent = remove_node->m_parent;
                        next_node->m_color  = remove_node->m_color;

                        /* Transplant next node to parent */
                        this->Transplant(std::addressof(m_root), remove_node, next_node);

                        /* Update next node's parents to the next node */
                        if (next_node->m_left != nullptr) {
                            next_node->m_left->m_parent = next_node;
                        }
                        if (next_node->m_right != nullptr) {
                            next_node->m_right->m_parent = next_node;
                        }
                    }
                }
                *out_fixup_color = fixup_color;

                return trace;
            }

            constexpr void RemoveFixup(node_type *parent_iter, TreeNodeTrace node_trace, bool remove_color) {

                /* No fixup on red */
                if (remove_color != static_cast<bool>(node_type::Color_Black)) { return; }

                /* Setup traversal iter */
                u32 traversal_iter = node_trace.traversal_count;
                if (traversal_iter < 1) { return; }

                /* Setup side iter */
                size_t side_iter = node_trace.trace;

                /* If the parent's child node is red paint it to black and complete */
                node_type *side = (side_iter & 1) ? parent_iter->m_right : parent_iter->m_left;
                if (side != nullptr && side->m_color == static_cast<bool>(node_type::Color_Red)) { 
                    side->m_color = node_type::Color_Black; 
                    return;
                }

                /* Fixup loop */
                size_t next_trace       = 0;
                u32    next_traversal_count = 0;
                for(;;) {

                    /* Set next iterators */
                    next_trace           = side_iter >> 1;
                    next_traversal_count = traversal_iter - 1;

                    /* Select opposite side */
                    if ((side_iter & 1) == 0) {
                        side = parent_iter->m_right;
                    } else {
                        side = parent_iter->m_left;
                    }

                    /* Rotate around the parent on red */
                    if (side->m_color == static_cast<bool>(node_type::Color_Red)) {

                        /* Repaint parent_iter to red, side to black */
                        parent_iter->m_color = node_type::Color_Red;
                        side->m_color = node_type::Color_Black;

                        /* Rotate parent (adjust iterators) */
                        if ((side_iter & 1) == 0) {
                            this->RotateLeft(parent_iter);
                            side = parent_iter->m_right;
                            next_trace = (next_trace << 1);
                        } else {
                            this->RotateRight(parent_iter);
                            side = parent_iter->m_left;
                            next_trace = (next_trace << 1) | 1;
                        }
                        next_traversal_count = next_traversal_count + 1;
                    }

                    /* Double rotate and complete if opposite side's child is red */
                    node_type *side_child = (side_iter & 1) ? side->m_right : side->m_left;
                    if (side_child != nullptr && side_child->m_color == static_cast<bool>(node_type::Color_Red)) {

                        /* Double rotate side and parent */
                        const u32  original_color = parent_iter->m_color;
                        node_type *original_side   = nullptr;
                        if ((side_iter & 1) == 0) {
                            original_side = this->RotateRight(side);
                            this->RotateLeft(parent_iter);
                        } else {
                            original_side = this->RotateLeft(side);
                            this->RotateRight(parent_iter);
                        }

                        /* Move parent_iter color to original side, set parent_iter color to Black, set side's color to Black */
                        original_side->m_color = original_color;
                        parent_iter->m_color = node_type::Color_Black;
                        side->m_color = node_type::Color_Black;

                        return;
                    }

                    /* Single rotate if the side's child is red */
                    node_type *side_opposite_child = ((side_iter & 1) == 0) ? side->m_right : side->m_left;
                    if (side_opposite_child != nullptr && side_opposite_child->m_color == static_cast<bool>(node_type::Color_Red)) {

                        /* Rotate parent iter */
                        const u32  original_color = parent_iter->m_color;
                        node_type *original_side   = nullptr;
                        if ((side_iter & 1) == 0) {
                            original_side = this->RotateLeft(parent_iter);
                        } else {
                            original_side = this->RotateRight(parent_iter);
                        }

                        /* Move parent_iter color to original side, set parent_iter color to Black, set side's color to Black */
                        original_side->m_color = original_color;
                        parent_iter->m_color = node_type::Color_Black;
                        side_opposite_child->m_color = node_type::Color_Black;

                        return;
                    }

                    /* Set side as red */
                    side->m_color = node_type::Color_Red;

                    /* Complete if parent is red */
                    if (parent_iter->m_color == static_cast<bool>(node_type::Color_Red)) { 
                        parent_iter->m_color = node_type::Color_Black;
                        return; 
                    }

                    /* Advance iterators */
                    side_iter      = next_trace;
                    traversal_iter = next_traversal_count;

                    /* Break if traversed */
                    if (traversal_iter < 1) { break; }

                    /* Advance parent iter */
                    parent_iter = parent_iter->m_parent;
                }

                return;
            }
        private:
            constexpr void InsertImpl(node_type *insert) {

                /* Set insert as root if null */
                if (m_root == nullptr) { 
                    m_root = insert;
                    return;
                }

                /* Find and trace insert locations (0 = left, 1 = right) */
                node_type *next_node       = m_root;
                node_type *node_iter       = nullptr;
                u32        side            = 0;
                size_t     trace_mask      = 0;        
                u32        traversal_count = 0;
                do {
                    node_iter       = next_node;

                    const std::strong_ordering comp  = Compare(node_iter->m_key, insert->m_key);
                    if (comp == 0) { Collision(node_iter->m_owner, insert->m_owner); return; }

                    side            = comp < 0;
                    trace_mask      = (trace_mask << 1) | side;
                    traversal_count = traversal_count + 1;
                    if (side) {
                        next_node = node_iter->m_right;
                    } else {
                        next_node = node_iter->m_left;
                    }
                } while(next_node != nullptr);
                VP_ASSERT(traversal_count < 0x40);

                /* Insert node */
                if (side) {
                    node_iter->m_right = insert;
                } else {
                    node_iter->m_left = insert;
                }

                /* Set node state */
                insert->m_left  = nullptr;
                insert->m_right = nullptr;
                insert->m_parent = nullptr;
                insert->m_color = node_type::Color_Red;

                /* Insert fixup */
                this->InsertFixup(insert, { trace_mask, traversal_count });

                return;
            }

            constexpr void RemoveImpl(key_type key) {

                /* Integrity check tree is not empty */
                VP_ASSERT(m_root != nullptr);

                /* Find removal location */
                node_type  *remove_node = m_root;
                node_type **right_node  = nullptr;
                std::strong_ordering comp(std::strong_ordering::equal);
                while (
                    (comp = Compare(remove_node->m_key, key)),
                    (right_node = std::addressof(remove_node->m_left), comp > 0) ||
                    (right_node = std::addressof(remove_node->m_right), comp < 0)) {
                    remove_node = *right_node;
                    VP_ASSERT(remove_node != nullptr);
                }

                /* Swap the removal node with a child and trace if possible */
                u32        fixup_color = 0;
                node_type *remove_iter = remove_node;
                TreeNodeTrace node_trace = this->RemoveSwap(std::addressof(remove_iter), std::addressof(fixup_color));

                /* Fixup tree violations for remove */
                this->RemoveFixup(remove_iter, node_trace, fixup_color);

                /* Clear remove node */
                remove_node->m_left  = nullptr;
                remove_node->m_right = nullptr;

                return;
            }

            constexpr static ALWAYS_INLINE node_type *FindImpl(node_type *iter, key_type key) {

                /* Binary iterate */
                while (iter != nullptr) {
                    const std::strong_ordering comp = Compare(iter->m_key, key);
                    if (comp == 0) { return iter; }
                    const bool side = comp < 0;
                    if (side) {
                        iter = iter->m_right;
                    } else {
                        iter = iter->m_left;
                    }
                }

                return nullptr;
            }
		public:
			constexpr ALWAYS_INLINE  ParentRedBlackTree() : m_root() {/*...*/}
			constexpr ALWAYS_INLINE ~ParentRedBlackTree() {/*...*/}

            constexpr void Insert(pointer node) {
                this->Insert(Traits::GetTreeNode(node));
            }
            constexpr void Insert(node_type *node) {

                /* Insert node into tree */
                this->InsertImpl(node);

                /* Paint root black */
                m_root->m_parent = nullptr;
                m_root->m_color = node_type::Color_Black;

                return;
            }

            constexpr void Remove(pointer node) {
                this->Remove(Traits::GetTreeNode(node));
            }
            constexpr void Remove(node_type *node) {
                this->RemoveImpl(node->GetKey());
                if (m_root != nullptr) { m_root->m_parent = nullptr; m_root->m_color = node_type::Color_Black; }
            }
            constexpr void Remove(key_type hash) {
                this->RemoveImpl(hash);
                if (m_root != nullptr) { m_root->m_parent = nullptr; m_root->m_color = node_type::Color_Black; }
            }

            constexpr pointer Find(key_type hash) {
                node_type *node = this->FindImpl(m_root, hash);
                if (node == nullptr) { return nullptr; }
                return node->m_owner;
            }
            constexpr const_pointer Find(key_type hash) const {
                const node_type *node = this->FindImpl(m_root, hash);
                if (node == nullptr) { return nullptr; }
                return node->m_owner;
            }

            constexpr iterator begin() {
                if (m_root == nullptr) { return nullptr; }
                node_type *ret = m_root;
                while (ret->m_left != nullptr) {
                    ret = ret->m_left;
                }
                return iterator(ret);
            }
            constexpr iterator end() {
                return iterator(nullptr);
            }

            constexpr const_iterator begin() const {
                if (m_root == nullptr) { return nullptr; }
                const node_type *ret = m_root;
                while (ret->m_left != nullptr) {
                    ret = ret->m_left;
                }
                return const_iterator(ret);
            }
            constexpr const_iterator end() const {
                return const_iterator(nullptr);
            }

            constexpr static pointer GetNext(pointer node) {
                node_type *next = Traits::GetTreeNode(node)->GetNext();
                return (next != nullptr) ? next->m_owner : nullptr;
            }
            constexpr static node_type *GetNext(node_type *node) {
                node_type *next = node->GetNext();
                return next;
            }
            constexpr static const_pointer GetNext(const_pointer node) {
                const node_type *next = Traits::GetTreeNode(node)->GetNext();
                return (next != nullptr) ? next->m_owner : nullptr;
            }
            constexpr static const node_type *GetNext(const node_type *node) {
                const node_type *next = node->GetNext();
                return next;
            }

            constexpr void ForceReset() { m_root = nullptr; }

            constexpr void ClearAll(auto lambda = [] ALWAYS_INLINE_LAMBDA ([[maybe_unused]] pointer obj) {/*...*/}) {

                iterator iter = this->begin();
                while (iter != this->end()) {
                    pointer obj = std::addressof(*iter);
                    ++iter;
                    (lambda)(obj);
                    Traits::GetTreeNode(obj)->ForceUnlink();
                }

                return;
            }

            constexpr size_t GetCount() {
                
                size_t count = 0;

                if (m_root == nullptr) { return count; }

                node_type *node = m_root;
                while (node->m_left != nullptr) {
                    node = node->m_left;
                }

                do {
                    ++count;
                    node = this->GetNext(node);
                } while (node != nullptr);

                return count;
            }
	};

    template<class RP, typename K, auto M>
    class ParentRedBlackTreeMemberTraits {
        public:
            using ComparisonFunction = std::strong_ordering(*)(const K&, const K&);
            using CollisionFunction  = void(*)(RP*,RP*);
        public:

            static constexpr ALWAYS_INLINE ParentRedBlackTreeNode<ParentType<M>, K> *GetTreeNode(RP *parent) {
                return std::addressof(parent->*M);
            }
            static constexpr ALWAYS_INLINE const ParentRedBlackTreeNode<ParentType<M>, K> *GetTreeNode(const RP *parent) {
                return std::addressof(parent->*M);
            }
    };

    template<class RP, auto M, auto Compare = RedBlackTreeDefaultCompare<typename vp::util::MemberType<M>::key_type>, auto Collision = RedBlackTreeDefaultCollision<RP>>
    struct ParentRedBlackTreeTraits {
        using Key                = vp::util::MemberType<M>::key_type;
        using Parent             = vp::util::ParentType<M>;
        using Traits             = ParentRedBlackTreeMemberTraits<RP, Key, M>;
        using Node               = ParentRedBlackTreeNode<ParentType<M>, Key>;
        using Tree               = ParentRedBlackTree<ParentType<M>, Key, Traits, Compare, Collision>;
        //using ReverseTree        = ParentRedBlackTree<ParentType<M>, K, Traits>;
    };

    template <class RP, auto M>
        requires (std::is_same<typename vp::util::MemberType<M>::key_type, const char*>::value == true || std::is_same<typename vp::util::MemberType<M>::key_type, char*>::value == true)
    using ParentRedBlackTreeStringTraits = ParentRedBlackTreeTraits<RP, M, RedBlackTreeDefaultStringCompare<typename vp::util::MemberType<M>::key_type>>;
}
