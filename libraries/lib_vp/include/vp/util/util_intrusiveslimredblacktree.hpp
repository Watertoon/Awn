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

    struct TreeNodeTrace {
        size_t trace;
        u32    traversal_count;
    };

    template<typename K>
    class IntrusiveRedBlackTreeNode {
        public:
            enum class NodeColor : u8 {
                Red   = 0,
                Black = 1,
            };
        public:
            using key_type = K;
        public:
            IntrusiveRedBlackTreeNode                    *m_left;
            IntrusiveRedBlackTreeNode                    *m_right;
            PointerBitFlag<IntrusiveRedBlackTreeNode, 1>  m_parent_color;
            key_type                                      m_key;
        public:
            constexpr ALWAYS_INLINE  IntrusiveRedBlackTreeNode() : m_left(nullptr), m_right(nullptr), m_parent_color(nullptr), m_key() {/*...*/}
            constexpr ALWAYS_INLINE  IntrusiveRedBlackTreeNode(key_type key) : m_left(nullptr), m_right(nullptr), m_parent_color(nullptr), m_key(key) {/*...*/}
            constexpr ALWAYS_INLINE ~IntrusiveRedBlackTreeNode() {/*...*/}

            constexpr ALWAYS_INLINE bool IsEnd() const {
                return (m_right == nullptr) & ((m_parent_color.GetPointer() == nullptr) || (m_parent_color.GetPointer()->m_right == this));
            }

            constexpr ALWAYS_INLINE bool IsLinked() const {
                return (m_parent_color.GetPointer() == nullptr) & (m_left == nullptr) & (m_right == nullptr);
            }

            IntrusiveRedBlackTreeNode *GetNext() {

                IntrusiveRedBlackTreeNode *iter = m_right;
                IntrusiveRedBlackTreeNode *next = nullptr;

                /* Traverse down right node's left */
                if (m_right != nullptr) {
                    do {
                        next = iter;
                        iter = next->m_left;
                    } while (next->m_left != nullptr);
                    return next;
                }
                
                /* Return parent */
                next = m_parent_color.GetPointer();
                if (next != nullptr && next->m_left != this) {
                    IntrusiveRedBlackTreeNode *r_child = this;
                    do {
                        iter    = next;
                        if (r_child != iter->m_right) { return iter; }
                        next    = iter->m_parent_color.GetPointer();
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

	template<typename T, typename K, class Traits>
	class IntrusiveRedBlackTree {
		public:
            using value_type       = T;
            using reference        = T&;
            using const_reference  = const T&;
            using pointer          = T*;
            using const_pointer    = const T*;
            using node_type        = IntrusiveRedBlackTreeNode<K>;
            using key_type         = K;
        private:
            node_type *m_root;
        private:
            node_type *RotateRight(node_type *rot_i) {

                /* Save rot_i's left, parent, and left's right */
                node_type *original_left       = rot_i->m_left;
                node_type *original_parent     = rot_i->m_parent_color.GetPointer();
                node_type *original_left_right = original_left->m_right;

                /* Move the original left's right to rot_i's new left */
                rot_i->m_left = original_left_right;
                if (original_left_right != nullptr) { original_left_right->m_parent_color.SetPointer(rot_i); }

                /* Move rot_i to the right of the original left */
                original_left->m_right = rot_i;
                rot_i->m_parent_color.SetPointer(original_left);

                /* Set original left's parent to the root if no original parent */
                if (original_parent == nullptr) {
                    m_root = original_left;
                    original_left->m_parent_color.SetPointer(nullptr);
                    return original_left;
                }

                /* Set original left's parent to the original parent */
                std::addressof(original_parent->m_left)[original_parent->m_left != rot_i] = original_left;
                original_left->m_parent_color.SetPointer(original_parent);

                return original_left;
            }
            node_type *RotateLeft(node_type *rot_i) {

                /* Save rot_i's right, parent, and right's left */
                node_type *original_right      = rot_i->m_right;
                node_type *original_parent     = rot_i->m_parent_color.GetPointer();
                node_type *original_right_left = original_right->m_left;

                /* Move the original right's left to rot_i's new right */
                rot_i->m_right = original_right_left;
                if (original_right_left != nullptr) { original_right_left->m_parent_color.SetPointer(rot_i); }

                /* Move rot_i to the left of the original right */
                original_right->m_left = rot_i;
                rot_i->m_parent_color.SetPointer(original_right);

                /* Set original right's parent to the root if no original parent */
                if (original_parent == nullptr) {
                    m_root = original_right;
                    original_right->m_parent_color.SetPointer(nullptr);
                    return original_right;
                }

                /* Set original right's parent to the original parent */
                std::addressof(original_parent->m_left)[original_parent->m_left != rot_i] = original_right;
                original_right->m_parent_color.SetPointer(original_parent);

                return original_right;
            }

            void InsertFixup(node_type *insert, TreeNodeTrace node_trace) {

                /* Nothing to do if no traversal count */
                if (node_trace.traversal_count < 1) { return; }

                /* Setup trace iterators */
                u32 traversal_iter = node_trace.traversal_count;

                size_t side_iter      = node_trace.trace;
                size_t next_side_iter = node_trace.trace >> 1;

                /* Percolate up as long as there is a parent */
                node_type *parent_iter = insert->m_parent_color.GetPointer();
                while (parent_iter != nullptr && parent_iter->m_parent_color.GetBit(0) == static_cast<bool>(node_type::NodeColor::Red)) {

                    /* Advance trace */
                    node_trace.trace           = next_side_iter >> 1;
                    node_trace.traversal_count = traversal_iter - 2;

                    /* Get parent_iter parent and the parent_parent_iter's side */
                    node_type *parent_parent_iter = parent_iter->m_parent_color.GetPointer();
                    node_type *parent_parent_side = std::addressof(parent_parent_iter->m_left)[(next_side_iter & 1) == 0];

                    /* Repaint nodes if parent parent side is red */
                    if ((parent_parent_side != nullptr) && (parent_parent_side->m_parent_color.GetBit(0) == static_cast<bool>(node_type::NodeColor::Red))) {

                        /* Repaint  */
                        parent_iter->m_parent_color.SetBit(0);
                        parent_parent_side->m_parent_color.SetBit(0);
                        parent_parent_iter->m_parent_color.ClearBit(0);

                        /* Advance parent iterator */
                        parent_iter = parent_parent_iter->m_parent_color.GetPointer();

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
                        if (side_iter == 0) { original_side = this->RotateRight(parent_parent_iter); }
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

                    original_side->m_parent_color.SetBit(0);
                    parent_parent_iter->m_parent_color.ClearBit(0);

                    return;
                }

                return;
            }

            void InsertImpl(node_type *insert) {

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
                    VP_ASSERT(node_iter->m_key != insert->m_key);
                    side            = (node_iter->m_key <= insert->m_key);
                    trace_mask      = (trace_mask << 1) | side;
                    traversal_count = traversal_count + 1;
                    next_node       = std::addressof(node_iter->m_left)[side];
                } while(next_node != nullptr);

                /* Insert node */
                std::addressof(node_iter->m_left)[side] = insert;

                /* Set node state */
                insert->m_left  = nullptr;
                insert->m_right = nullptr;
                insert->m_parent_color.SetPointerAndClearFlags(node_iter);

                /* Insert fixup */
                this->InsertFixup(insert, { trace_mask, traversal_count });

                return;
            }

            TreeNodeTrace TraceParent(node_type *parent_node, node_type *node) {

                size_t     mask = 0;
                u32        i    = 0;
                do {

                    /* Build mask */
                    const size_t p_side = (parent_node->m_left != node); 
                    mask                = mask | (p_side << (i & 0x3f));

                    /* Iterate */
                    ++i;
                    node        = parent_node;
                    parent_node = parent_node->m_parent_color.GetPointer();
                } while (parent_node != nullptr);

                return { mask, i };
            }

            static constexpr ALWAYS_INLINE void Transplant(node_type **root, node_type *replace_node, node_type *move_node) {

                node_type *parent_node = replace_node->m_parent_color.GetPointer();
                if (parent_node == nullptr) {
                    *root = move_node;
                } else if (parent_node->m_left == replace_node) {
                    parent_node->m_left = move_node;
                } else {
                    parent_node->m_right = move_node;
                }
            }

            TreeNodeTrace RemoveSwap(node_type **out_remove, u32 *out_fixup_color) {

                /* Handle transplant of left, right, left right */
                TreeNodeTrace  trace        = {};
                node_type     *remove_node = *out_remove;
                node_type     *moved_node  = nullptr;
                node_type     *parent_node = remove_node->m_parent_color.GetPointer();
                u32            fixup_color = remove_node->m_parent_color.GetBit(0);
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
                        moved_node->m_parent_color.SetPointer(parent_node);
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
                        moved_node->m_parent_color.SetPointer(parent_node);
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
                    node_type *next_parent = next_node->m_parent_color.GetPointer();
                    fixup_color            = next_node->m_parent_color.GetBit(0);
                    *out_remove = next_parent;
                    if (next_parent != nullptr) {
                        trace = this->TraceParent(next_parent, next_node);
                    }

                    /* Move remove node color to next node */
                    next_node->m_parent_color.SetBitMask(remove_node->m_parent_color.GetBitMask());

                    /* If next node is directly to the right */
                    if (remove_node->m_right == next_node) {

                        /* Transplant next node to remove node's place */
                        this->Transplant(std::addressof(m_root), remove_node, next_node);
                        next_node->m_parent_color.SetPointer(parent_node);

                        /* Move remove node's left node to the next node */
                        next_node->m_left = remove_node->m_left;
                        remove_node->m_left->m_parent_color.SetPointer(next_node);

                        *out_remove = next_node;

                    } else {

                        /* Move next node to remove node's parent */
                        node_type *right_next_node = next_node->m_right;
                        if (right_next_node != nullptr) {
                            std::addressof(next_parent->m_left)[next_parent->m_left != next_node] = right_next_node;
                            right_next_node->m_parent_color.SetPointer(next_parent);
                        } else {
                            next_parent->m_left = nullptr;
                        }
                        next_node->m_right        = remove_node->m_right;
                        next_node->m_left         = remove_node->m_left;
                        next_node->m_parent_color = remove_node->m_parent_color;

                        /* Transplant next node to parent */
                        this->Transplant(std::addressof(m_root), remove_node, next_node);

                        /* Update next node's parents to the next node */
                        if (next_node->m_left != nullptr) {
                            next_node->m_left->m_parent_color.SetPointer(next_node);
                        }
                        if (next_node->m_right != nullptr) {
                            next_node->m_right->m_parent_color.SetPointer(next_node);
                        }
                    }
                }
                *out_fixup_color = fixup_color;

                return trace;
            }

            void RemoveFixup(node_type *parent_iter, TreeNodeTrace node_trace, bool remove_color) {

                /* No fixup on red */
                if (remove_color != static_cast<bool>(node_type::NodeColor::Black)) { return; }

                /* Setup traversal iter */
                u32 traversal_iter = node_trace.traversal_count;
                if (traversal_iter < 1) { return; }

                /* Setup side iter */
                size_t side_iter = node_trace.trace;

                /* If the parent's child node is red paint it to black and complete */
                node_type *side = std::addressof(parent_iter->m_left)[side_iter & 1];
                if (side != nullptr && side->m_parent_color.GetBit(0) == static_cast<bool>(node_type::NodeColor::Red)) { 
                    side->m_parent_color.SetBit(0); 
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
                    side = std::addressof(parent_iter->m_left)[((side_iter & 1) == 0)];

                    /* Rotate around the parent on red */
                    if (side->m_parent_color.GetBit(0) == static_cast<bool>(node_type::NodeColor::Red)) {

                        /* Repaint parent_iter to red, side to black */
                        parent_iter->m_parent_color.ClearBit(0);
                        side->m_parent_color.SetBit(0);

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
                    if (std::addressof(side->m_left)[(side_iter & 1)] != nullptr && std::addressof(side->m_left)[(side_iter & 1)]->m_parent_color.GetBit(0) == static_cast<bool>(node_type::NodeColor::Red)) {

                        /* Double rotate side and parent */
                        const u32  original_color = parent_iter->m_parent_color.GetBit(0);
                        node_type *original_side   = nullptr;
                        if ((side_iter & 1) == 0) {
                            original_side = this->RotateRight(side);
                            this->RotateLeft(parent_iter);
                        } else {
                            original_side = this->RotateLeft(side);
                            this->RotateRight(parent_iter);
                        }

                        /* Move parent_iter color to original side, set parent_iter color to Black, set side's color to Black */
                        original_side->m_parent_color.SetBitMask(original_color);
                        parent_iter->m_parent_color.SetBit(0);
                        side->m_parent_color.SetBit(0);

                        return;
                    }
                    
                    /* Single rotate if the side's child is red */
                    node_type *side_opposite_child = std::addressof(side->m_left)[((side_iter & 1) == 0)];
                    if (side_opposite_child != nullptr && side_opposite_child->m_parent_color.GetBit(0) == static_cast<bool>(node_type::NodeColor::Red)) {
                   
                        /* Rotate parent iter */
                        const u32  original_color = parent_iter->m_parent_color.GetBit(0);
                        node_type *original_side   = nullptr;
                        if ((side_iter & 1) == 0) {
                            original_side = this->RotateLeft(parent_iter);
                        } else {
                            original_side = this->RotateRight(parent_iter);
                        }
                        
                        /* Move parent_iter color to original side, set parent_iter color to Black, set side's color to Black */
                        original_side->m_parent_color.SetBitMask(original_color);
                        parent_iter->m_parent_color.SetBit(0);
                        side_opposite_child->m_parent_color.SetBit(0);

                        return;
                    }

                    /* Set side as red */
                    side->m_parent_color.ClearBit(0);

                    /* Complete if parent is red */
                    if (parent_iter->m_parent_color.GetBit(0) == static_cast<bool>(node_type::NodeColor::Red)) { 
                        parent_iter->m_parent_color.SetBit(0);
                        return; 
                    }

                    /* Advance iterators */
                    side_iter      = next_trace;
                    traversal_iter = next_traversal_count;

                    /* Break if traversed */
                    if (traversal_iter < 1) { break; }

                    /* Advance parent iter */
                    parent_iter = parent_iter->m_parent_color.GetPointer();
                }

                return;
            }

            void RemoveImpl(key_type key) {

                /* Integrity check tree is not empty */
                VP_ASSERT(m_root != nullptr);

                /* Find removal location */
                node_type  *remove_node = m_root;
                node_type **right_node  = nullptr;
                while (
                    (right_node = std::addressof(remove_node->m_left),  key < remove_node->m_key) ||
                    (right_node = std::addressof(remove_node->m_right), remove_node->m_key < key)) {
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

            static constexpr ALWAYS_INLINE node_type *FindImpl(node_type *iter, key_type key) {

                /* Binary iterate */
                for (; iter != nullptr; iter = iter->m_right) {
                    while (key < iter->m_key) {
                        iter = iter->m_left;
                        if (iter == nullptr) { return nullptr; } 
                    }
                    if (key <= iter->m_key) {
                        return iter;
                    }
                }

                return nullptr;
            }
		public:
			constexpr ALWAYS_INLINE  IntrusiveRedBlackTree() : m_root(nullptr) {/*...*/}
			constexpr ALWAYS_INLINE ~IntrusiveRedBlackTree() {/*...*/}

            ALWAYS_INLINE void Insert(pointer node) {

                /* Insert node into tree */
                this->InsertImpl(Traits::GetTreeNode(node));

                /* Paint root black */
                m_root->m_parent_color.SetBit(0);

                return;
            }
            ALWAYS_INLINE void Insert(node_type *node) {

                /* Insert node into tree */
                this->InsertImpl(node);

                /* Paint root black */
                m_root->m_parent_color.SetBit(0);

                return;
            }

            ALWAYS_INLINE void Remove(pointer node) {
                this->RemoveImpl(Traits::GetTreeNode(node)->GetKey());
                if (m_root != nullptr) { m_root->m_parent_color.SetBit(0); }
            }
            ALWAYS_INLINE void Remove(node_type *node) {
                this->RemoveImpl(node->GetKey());
                if (m_root != nullptr) { m_root->m_parent_color.SetBit(0); }
            }
            ALWAYS_INLINE void Remove(key_type hash) {
                this->RemoveImpl(hash);
                if (m_root != nullptr) { m_root->m_parent_color.SetBit(0); }
            }

            ALWAYS_INLINE pointer Find(key_type hash) {
                node_type *node = this->FindImpl(m_root, hash);
                if (node == nullptr) { return nullptr; }
                return Traits::GetParent(node);
            }
            
                        pointer Start() {

                if (m_root == nullptr) { return nullptr; }

                node_type *ret = m_root;
                while (ret->m_left != nullptr) {
                    ret = reinterpret_cast<node_type*>(ret->m_left);
                }
                return Traits::GetParent(ret);
            }

            pointer GetNext(pointer node) {
                node_type *next = reinterpret_cast<node_type*>(Traits::GetTreeNode(node)->GetNext());
                return (next != nullptr) ? Traits::GetParent(next) : nullptr;
            }
            node_type *GetNext(node_type *node) {
                node_type *next = reinterpret_cast<node_type*>(node->GetNext());
                return (next != nullptr) ? next : nullptr;
            }

            pointer End() {

                if (m_root == nullptr) { return nullptr; }

                node_type *ret = m_root;
                while (ret->m_right != nullptr) {
                    ret = reinterpret_cast<node_type*>(ret->m_right);
                }
                return Traits::GetParent(ret);
            }

            size_t GetCount() {
                
                size_t count = 0;

                if (m_root == nullptr) { return count; }

                node_type *node = m_root;
                while (node->m_left != nullptr) {
                    node = reinterpret_cast<node_type*>(node->m_left);
                }

                do {
                    ++count;
                    node = this->GetNext(node);
                } while (node != nullptr);

                return count;
            }
	};

    template<class RP, typename K, auto M>
    class IntrusiveRedBlackTreeMemberTraits {
        public:
            static ALWAYS_INLINE RP *GetParent(IntrusiveRedBlackTreeNode<K> *node) {
                return reinterpret_cast<RP*>(reinterpret_cast<uintptr_t>(node) - OffsetOf(M));
            }
            static ALWAYS_INLINE RP &GetParentReference(IntrusiveRedBlackTreeNode<K> *node) {
                return *reinterpret_cast<RP*>(reinterpret_cast<uintptr_t>(node) - OffsetOf(M));
            }

            static ALWAYS_INLINE const RP *GetParent(const IntrusiveRedBlackTreeNode<K> *node) {
                return reinterpret_cast<RP*>(reinterpret_cast<uintptr_t>(node) - OffsetOf(M));
            }
            static ALWAYS_INLINE const RP &GetParentReference(const IntrusiveRedBlackTreeNode<K> *node) {
                return *reinterpret_cast<const RP*>(reinterpret_cast<uintptr_t>(node) - OffsetOf(M));
            }

            static constexpr ALWAYS_INLINE IntrusiveRedBlackTreeNode<K> *GetTreeNode(RP *parent) {
                return std::addressof(parent->*M);
            }
            static constexpr ALWAYS_INLINE const IntrusiveRedBlackTreeNode<K> *GetTreeNode(const RP *parent) {
                return std::addressof(parent->*M);
            }
    };

    template<class RP, auto M>
    struct IntrusiveRedBlackTreeTraits {
        using K           = vp::util::MemberType<M>::key_type;
        using Traits      = IntrusiveRedBlackTreeMemberTraits<RP, K, M>;
        using Node        = IntrusiveRedBlackTreeNode<K>;
        using Tree        = IntrusiveRedBlackTree<ParentType<M>, K, Traits>;
        using ReverseTree = IntrusiveRedBlackTree<ParentType<M>, K, Traits>;
    };
}
