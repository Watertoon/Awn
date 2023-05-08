#pragma once

namespace vp::util {

    /* Note; Currently breaks strict aliasing rules */

    class IntrusiveRedBlackTreeNodeBase {
        protected:
            enum Rotate : bool {
                Rotate_Left  = 0,
                Rotate_Right = 1,
            };
            enum Color : bool {
                Color_Black = 0,
                Color_Red   = 1,
            };
        public:
            using node_base = IntrusiveRedBlackTreeNodeBase;
        public:
            IntrusiveRedBlackTreeNodeBase *m_parent;
            IntrusiveRedBlackTreeNodeBase *m_left;
            IntrusiveRedBlackTreeNodeBase *m_right;
            u32                            m_color;
        protected:
            static constexpr void RotateLeft(node_base **root, node_base *pivot) {

                node_base *i_r  = pivot->m_right;
                node_base *i_rl = i_r->m_left;
                pivot->m_right = i_rl;

                if (i_rl != nullptr) {
                    i_rl->m_parent = pivot;
                }
                i_r->m_parent = pivot->m_parent;

                node_base **pivot_parent = root;
                if (pivot->m_parent != nullptr) {
                    pivot_parent = std::addressof(pivot->m_parent->m_left);
                    if (pivot != pivot->m_parent->m_left) {
                        pivot_parent = std::addressof(pivot->m_parent->m_right);
                    }
                }

                *pivot_parent    = i_r;
                i_r->m_left      = pivot;
                pivot->m_parent  = i_r;

                return;
            }
            static constexpr void RotateRight(node_base **root, node_base *pivot) {

                node_base *i_l  = pivot->m_left;
                node_base *i_lr = i_l->m_right;
                pivot->m_left = i_lr;

                if (i_lr != nullptr) {
                    i_lr->m_parent = pivot;
                }
                i_l->m_parent = pivot->m_parent;

                node_base **pivot_parent = root;
                if (pivot->m_parent != nullptr) {
                    pivot_parent = std::addressof(pivot->m_parent->m_left);
                    if (pivot != pivot->m_parent->m_left) {
                        pivot_parent = std::addressof(pivot->m_parent->m_right);
                    }
                }

                *pivot_parent    = i_l;
                i_l->m_right     = pivot;
                pivot->m_parent  = i_l;

                return;
            }

            static constexpr node_base *InsertFixupLeftImpl(node_base **root, node_base *insert_iter) {

                node_base *i_ppr = insert_iter->m_parent->m_parent->m_right;
                if (i_ppr != nullptr && i_ppr->m_color == Color_Red) {
                    i_ppr->m_color                           = Color_Black;
                    insert_iter->m_parent->m_color           = Color_Black;
                    insert_iter->m_parent->m_parent->m_color = Color_Red;
                    return insert_iter->m_parent->m_parent;
                }

                if (insert_iter == insert_iter->m_parent->m_right) {
                    insert_iter = insert_iter->m_parent;
                    RotateLeft(root, insert_iter);
                }

                insert_iter->m_parent->m_color           = Color_Black;
                insert_iter->m_parent->m_parent->m_color = Color_Red;
                RotateRight(root, insert_iter->m_parent->m_parent);

                return insert_iter;
            }
            static constexpr node_base *InsertFixupRightImpl(node_base **root, node_base *insert_iter) {

                node_base *i_ppl = insert_iter->m_parent->m_parent->m_left;
                if (i_ppl != nullptr && i_ppl->m_color == Color_Red) {
                    i_ppl->m_color                           = Color_Black;
                    insert_iter->m_parent->m_color           = Color_Black;
                    insert_iter->m_parent->m_parent->m_color = Color_Red;
                    return insert_iter->m_parent->m_parent;
                }

                if (insert_iter == insert_iter->m_parent->m_left) {
                    insert_iter = insert_iter->m_parent;
                    RotateRight(root, insert_iter);
                }

                insert_iter->m_parent->m_color           = Color_Black;
                insert_iter->m_parent->m_parent->m_color = Color_Red;
                RotateLeft(root, insert_iter->m_parent->m_parent);

                return insert_iter;
            }

            static constexpr void InsertFixup(node_base **root, node_base *insert_iter) {

                while (insert_iter->m_parent != nullptr && insert_iter->m_parent->m_color == Color_Red) {
                    if (insert_iter->m_parent == insert_iter->m_parent->m_parent->m_left) {
                        insert_iter = InsertFixupLeftImpl(root, insert_iter);
                    } else {
                        insert_iter = InsertFixupRightImpl(root, insert_iter);
                    }
                }
                (*root)->m_color = Color_Black;

                return;
            }

            static constexpr ALWAYS_INLINE void Transplant(node_base **root, node_base *pivot, node_base *i) {
                if (pivot->m_parent == nullptr) {
                    *root = i;
                } else if (pivot == pivot->m_parent->m_left) {
                    pivot->m_parent->m_left = i;
                } else {
                    pivot->m_parent->m_right = i;
                }
            }

            static constexpr ALWAYS_INLINE node_base *FindMinimum(node_base *node) {

                /* Traverse down the left of the node */
                while (node->m_left != nullptr) {
                    node = node->m_left;
                }
                return node;
            }

            static constexpr node_base *RemoveFixupLeftImpl(node_base **root, node_base *fixup_parent, node_base *fixup) {

                node_base *f_r = fixup_parent->m_right;
                
                if (f_r->m_color == Color_Red) {
                    f_r->m_color = Color_Black;
                    fixup_parent->m_color = Color_Red;
                    RotateLeft(root, fixup_parent);
                    f_r = fixup_parent->m_right;
                }

                if (f_r->m_left->m_color == Color_Black && f_r->m_right->m_color == Color_Black) {
                    f_r->m_color = Color_Red;
                    return fixup_parent;
                }

                if (f_r->m_right->m_color == Color_Black) {
                    f_r->m_left->m_color = Color_Black;
                    f_r->m_color = Color_Red;
                    RotateRight(root, f_r);
                    f_r = fixup_parent->m_right;
                }

                f_r->m_color          = fixup_parent->m_color;
                fixup_parent->m_color = Color_Black;
                f_r->m_right->m_color = Color_Black;
                RotateLeft(root, fixup_parent);

                return *root;
            }

            static constexpr node_base *RemoveFixupRightImpl(node_base **root, node_base *fixup_parent, node_base *fixup) {

                node_base *f_l = fixup_parent->m_left;
                
                if (f_l->m_color == Color_Red) {
                    f_l->m_color = Color_Black;
                    fixup_parent->m_color = Color_Red;
                    RotateRight(root, fixup_parent);
                    f_l = fixup_parent->m_left;
                }

                if (f_l->m_right->m_color == Color_Black && f_l->m_left->m_color == Color_Black) {
                    f_l->m_color = Color_Red;
                    return fixup_parent;
                }

                if (f_l->m_left->m_color == Color_Black) {
                    f_l->m_right->m_color = Color_Black;
                    f_l->m_color = Color_Red;
                    RotateLeft(root, f_l);
                    f_l = fixup_parent->m_left;
                }

                f_l->m_color          = fixup_parent->m_color;
                fixup_parent->m_color = Color_Black;
                f_l->m_left->m_color  = Color_Black;
                RotateRight(root, fixup_parent);

                return *root;
            }

            static constexpr ALWAYS_INLINE void RemoveFixup(node_base **root, node_base *fixup_parent, node_base *fixup) {

                while (fixup != nullptr && fixup->m_color == Color_Black && fixup != *root) {
                    if (fixup_parent->m_left == fixup) {
                        fixup = RemoveFixupLeftImpl(root, fixup_parent, fixup);
                    } else {
                        fixup = RemoveFixupRightImpl(root, fixup_parent, fixup);
                    }
                    fixup_parent = fixup->m_parent;
                }
                if (fixup != nullptr) {
                    fixup->m_color = Color_Black;
                }
            }
        public:
            constexpr ALWAYS_INLINE IntrusiveRedBlackTreeNodeBase() : m_parent(nullptr), m_left(nullptr), m_right(nullptr), m_color(false)  {/*...*/}

            constexpr ALWAYS_INLINE bool IsEnd() const {
                return (m_right == nullptr) & ((m_parent == nullptr) || (m_parent->m_right == this));
            }

            static constexpr void RemoveImpl(node_base **root, node_base *remove) {

                node_base *fixup_pivot  = remove->m_right;
                node_base *fixup_parent = remove->m_parent;
                u32 remove_color = remove->m_color;
                if (remove->m_left == nullptr) {
                    if (fixup_pivot != nullptr) { fixup_pivot->m_parent = fixup_parent; }
                    Transplant(root, remove, fixup_pivot);
                } else if (remove->m_right == nullptr) {
                    fixup_pivot = remove->m_left;
                    if (fixup_pivot != nullptr) { fixup_pivot->m_parent = fixup_parent; }
                    Transplant(root, remove, fixup_pivot);
                } else {

                    /* Find next value in tree */
                    node_base *min = FindMinimum(remove->m_right);
                    node_base *min_r = min->m_right;
                    remove_color = min->m_color;

                    if (min_r != nullptr) {
                        min_r->m_parent = min->m_parent;
                    }

                    /* Transplant 1 */
                    Transplant(root, min, min_r);

                    /* Swap fixup pivot to minimum */
                    fixup_parent = min;
                    if (min->m_parent != nullptr) {
                        fixup_parent = min->m_parent;
                    }

                    /* Swap min node with the node to be removed */
                    min->m_left   = remove->m_left;
                    min->m_right  = remove->m_right;
                    min->m_parent = remove->m_parent;
                    min->m_color  = remove->m_color;
                    
                    /* Transplant 2 */
                    Transplant(root, remove, min);
                    
                    remove->m_left->m_parent = min;
                    if (remove->m_right != nullptr) {
                        remove->m_right->m_parent = min;
                    }
                }

                /* Fixup deletion */
                if (remove_color == Color_Black) {
                    RemoveFixup(root, fixup_parent, fixup_pivot);
                }

                return;
            }

            constexpr node_base *GetNext() {

                node_base *iter = m_right;
                node_base *next = nullptr;

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
                    node_base *r_child = this;
                    do {
                        iter    = next;
                        if (r_child != iter->m_right) { return iter; }
                        next    = iter->m_parent;
                        r_child = iter;
                    } while (next != nullptr);
                }

                return next;
            }
    };

    template<typename K>
    class IntrusiveRedBlackTreeNode : public IntrusiveRedBlackTreeNodeBase {
        public:
            using node_type = IntrusiveRedBlackTreeNode<K>;
            using key_type  = K;
        public:
            key_type                   m_hash_value;
        public:
            static void InsertImpl(node_type **root, node_type *insert) {

                /* Find parent */
                node_type *iter   = *root;
                node_type *parent = nullptr;

                s32 side = 0;
                node_type **select = nullptr;
                while (iter != nullptr) {
                    parent = iter;
                    select = reinterpret_cast<node_type**>(&iter->m_left);
                    side = -1;
                    if (iter->m_hash_value < insert->m_hash_value) {
                        select = reinterpret_cast<node_type**>(&iter->m_right);
                        side = 1;
                    }
                    iter = *select;
                }

                /* Setup node */
                insert->m_left   = nullptr;
                insert->m_right  = nullptr;
                insert->m_parent = parent;
                insert->m_color  = Color_Red;

                /* Place node at respective parent node */
                node_type **child = root;
                if (parent != nullptr) {
                    child = reinterpret_cast<node_type**>(std::addressof(parent->m_left));
                    if (-1 < side) {
                        child = reinterpret_cast<node_type**>(std::addressof(parent->m_right)); 
                    }
                }
                *child = insert;

                /* Fixup tree */
                InsertFixup(reinterpret_cast<IntrusiveRedBlackTreeNodeBase**>(root), insert);

                return;
            }

            static node_type *FindImpl(node_type *iter, key_type hash) {

                if (iter == nullptr) { return nullptr; }

                /* Traverse tree to find hash */
                node_type *ret     = nullptr;
                node_type *temp    = nullptr;
                node_type **select = nullptr;
                do {
                    ret = iter;
                    select = reinterpret_cast<node_type**>(&iter->m_left);
                    if (iter->m_hash_value < hash) {
                        select = reinterpret_cast<node_type**>(&ret->m_right);
                        ret  = temp;
                    }
                    iter = *select;
                    temp = ret;
                } while (iter != nullptr);

                return ret;
            }
        public:
            constexpr ALWAYS_INLINE IntrusiveRedBlackTreeNode() : IntrusiveRedBlackTreeNodeBase(), m_hash_value() {/*...*/}

            constexpr ALWAYS_INLINE void SetKey(key_type hash) {
                m_hash_value = hash;
            }

            node_type *GetLeft() {
                return reinterpret_cast<node_type*>(m_left);
            }

            node_type *GetRight() {
                return reinterpret_cast<node_type*>(m_right);
            }
    };

	template<typename T, typename K, class Traits, bool Reverse>
	class IntrusiveRedBlackTree {
		public:
            using value_type       = T;
            using reference        = T&;
            using const_reference  = const T&;
            using pointer          = T*;
            using const_pointer    = const T*;
            using node_type        = IntrusiveRedBlackTreeNode<K>;
            using node_base        = IntrusiveRedBlackTreeNodeBase;
            using key_type         = K;
        public:
            node_type *m_root;
		public:
			constexpr ALWAYS_INLINE IntrusiveRedBlackTree() : m_root(nullptr) {/*...*/}

            void Insert(node_type *node) {
                node_type::InsertImpl(std::addressof(m_root), node);
            }
            void Insert(pointer node) {
                node_type::InsertImpl(std::addressof(m_root), Traits::GetTreeNode(node));
            }

            void Remove(node_base *node) {
                node_base::RemoveImpl(reinterpret_cast<node_base**>(std::addressof(m_root)), node);
            }

            void Remove(key_type hash) {
                node_type *node = node_type::FindImpl(m_root, hash);
                VP_ASSERT(node->hash == hash);
                if (node == nullptr) { return; }
                node_base::RemoveImpl(reinterpret_cast<node_base**>(std::addressof(m_root)), node);
            }

            pointer Find(key_type hash) {
                node_type *node = node_type::FindImpl(m_root, hash);
                if (node == nullptr || node->m_hash_value != hash) { return nullptr; }
                return Traits::GetParent(node);
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

            static ALWAYS_INLINE IntrusiveRedBlackTreeNode<K> *GetTreeNode(const RP *parent) {
                return reinterpret_cast<IntrusiveRedBlackTreeNode<K>*>(reinterpret_cast<uintptr_t>(parent) + OffsetOf(M));
            }
    };

    template<class RP, typename K, auto M>
    struct IntrusiveRedBlackTreeTraits {
        using Traits      = IntrusiveRedBlackTreeMemberTraits<RP, K, M>;
        using Node        = IntrusiveRedBlackTreeNode<K>;
        using Tree        = IntrusiveRedBlackTree<ParentType<M>, K, Traits, false>;
        using ReverseTree = IntrusiveRedBlackTree<ParentType<M>, K, Traits, true>;
    };
}
