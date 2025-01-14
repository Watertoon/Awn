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

    class IntrusiveRedBlackTreeNodeBase {
        public:
            enum class NodeColor : u8 {
                Red   = 0,
                Black = 1,
            };
        public:
            IntrusiveRedBlackTreeNodeBase                    *m_left;
            IntrusiveRedBlackTreeNodeBase                    *m_right;
            PointerBitFlag<IntrusiveRedBlackTreeNodeBase, 1>  m_parent_color;
        public:
            constexpr ALWAYS_INLINE  IntrusiveRedBlackTreeNodeBase() : m_left(nullptr), m_right(nullptr), m_parent_color() {/*...*/}
            constexpr ALWAYS_INLINE ~IntrusiveRedBlackTreeNodeBase() {/*...*/}

            ALWAYS_INLINE bool IsEnd() const {
                return (m_right == nullptr) & ((m_parent_color.GetPointer() == nullptr) || (m_parent_color.GetPointer()->m_right == this));
            }

            ALWAYS_INLINE bool IsLinked() const {
                return (m_parent_color.GetPointer() != nullptr) || (m_left != nullptr) || (m_right != nullptr);
            }

            constexpr ALWAYS_INLINE void ForceUnlink() {
                m_left  = nullptr;
                m_right = nullptr;
                m_parent_color.SetFlagsAndClearPointer(0);
            }

            IntrusiveRedBlackTreeNodeBase *GetNext();
            const IntrusiveRedBlackTreeNodeBase *GetNext() const;
    };

    template<typename K>
    class IntrusiveRedBlackTreeNode : public IntrusiveRedBlackTreeNodeBase {
        public:
            using key_type = K;
        public:
            key_type m_key;
        public:
            constexpr ALWAYS_INLINE  IntrusiveRedBlackTreeNode() : IntrusiveRedBlackTreeNodeBase(), m_key() {/*...*/}
            constexpr ALWAYS_INLINE  IntrusiveRedBlackTreeNode(key_type key) : IntrusiveRedBlackTreeNodeBase(), m_key(key) {/*...*/}
            constexpr ALWAYS_INLINE ~IntrusiveRedBlackTreeNode() {/*...*/}

            IntrusiveRedBlackTreeNode *GetNext() {
                return reinterpret_cast<IntrusiveRedBlackTreeNode*>(this->IntrusiveRedBlackTreeNodeBase::GetNext());
            }

            const IntrusiveRedBlackTreeNode *GetNext() const {
                return reinterpret_cast<const IntrusiveRedBlackTreeNode*>(this->IntrusiveRedBlackTreeNodeBase::GetNext());
            }

            constexpr ALWAYS_INLINE void SetKey(key_type hash) {
                m_key = hash;
            }
            constexpr ALWAYS_INLINE key_type GetKey() const {
                return m_key;
            }
    };

    class IntrusiveRedBlackTreeBase {
        public:
            using node_base = IntrusiveRedBlackTreeNodeBase;
        protected:
            node_base *m_root;
        public:
            constexpr ALWAYS_INLINE  IntrusiveRedBlackTreeBase() : m_root() {/*...*/}
            constexpr ALWAYS_INLINE ~IntrusiveRedBlackTreeBase() {/*...*/}

            node_base *RotateRight(node_base *rot_i);
            node_base *RotateLeft(node_base *rot_i);

            void InsertFixup(node_base *insert, TreeNodeTrace node_trace);

            static TreeNodeTrace TraceParent(node_base *parent_node, node_base *node);

            static void Transplant(node_base **root, node_base *replace_node, node_base *move_node);

            TreeNodeTrace RemoveSwap(node_base **out_remove, u32 *out_fixup_color);
            void RemoveFixup(node_base *parent_iter, TreeNodeTrace node_trace, bool remove_color);
    };

	template<typename T, typename K, class Traits, Traits::ComparisonFunction Compare, Traits::CollisionFunction Collision>
	class IntrusiveRedBlackTree : public IntrusiveRedBlackTreeBase {
		public:
            using value_type      = T;
            using reference       = T&;
            using const_reference = const T&;
            using pointer         = T*;
            using const_pointer   = const T*;
            using node_type       = IntrusiveRedBlackTreeNode<K>;
            using key_type        = K;
        public:
            class Iterator {
                private:
                    node_type *m_node;
                public:
                    constexpr ALWAYS_INLINE Iterator(node_type *node) : m_node(node) {/*...*/}
                    constexpr ALWAYS_INLINE ~Iterator() {/*...*/}

                    ALWAYS_INLINE reference operator*() {
                        return Traits::GetParentReference(m_node);
                    }
                    ALWAYS_INLINE const_reference operator*() const {
                        return Traits::GetParentReference(m_node);
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

                    ALWAYS_INLINE const_reference operator*() const {
                        return Traits::GetParentReference(m_node);
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
        private:
            void InsertImpl(node_type *insert) {

                /* Set insert as root if null */
                if (m_root == nullptr) { 
                    m_root = reinterpret_cast<node_base*>(insert);
                    return;
                }

                /* Find and trace insert locations (0 = left, 1 = right) */
                node_type *next_node       = reinterpret_cast<node_type*>(m_root);
                node_type *node_iter       = nullptr;
                u32        side            = 0;
                size_t     trace_mask      = 0;        
                u32        traversal_count = 0;
                do {
                    node_iter       = next_node;

                    const std::strong_ordering comp  = Compare(node_iter->m_key, insert->m_key);
                    if (comp == 0) { Collision(Traits::GetParent(node_iter), Traits::GetParent(insert)); return; }

                    side            = comp < 0;
                    trace_mask      = (trace_mask << 1) | side;
                    traversal_count = traversal_count + 1;
                    next_node       = reinterpret_cast<node_type*>(std::addressof(node_iter->m_left)[side]);
                } while(next_node != nullptr);
                VP_ASSERT(traversal_count < 0x40);

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

            void RemoveImpl(key_type key) {

                /* Integrity check tree is not empty */
                VP_ASSERT(m_root != nullptr);

                /* Find removal location */
                node_type  *remove_node = reinterpret_cast<node_type*>(m_root);
                node_type **right_node  = nullptr;
                std::strong_ordering comp(std::strong_ordering::equal);
                while (
                    (comp = Compare(remove_node->m_key, key)),
                    (right_node = reinterpret_cast<node_type**>(std::addressof(remove_node->m_left)), comp > 0) ||
                    (right_node = reinterpret_cast<node_type**>(std::addressof(remove_node->m_right)), comp < 0)) {
                    remove_node = *right_node;
                    VP_ASSERT(remove_node != nullptr);
                }

                /* Swap the removal node with a child and trace if possible */
                u32        fixup_color = 0;
                node_type *remove_iter = remove_node;
                TreeNodeTrace node_trace = this->RemoveSwap(reinterpret_cast<node_base**>(std::addressof(remove_iter)), std::addressof(fixup_color));

                /* Fixup tree violations for remove */
                this->RemoveFixup(remove_iter, node_trace, fixup_color);

                /* Clear remove node */
                remove_node->m_left  = nullptr;
                remove_node->m_right = nullptr;

                return;
            }

            static ALWAYS_INLINE node_type *FindImpl(node_type *iter, key_type key) {

                /* Binary iterate */
                while (iter != nullptr) {
                    const std::strong_ordering comp = Compare(iter->m_key, key);
                    if (comp == 0) { return iter; }
                    const bool side = comp < 0;
                    iter = reinterpret_cast<node_type*>(std::addressof(iter->m_left)[side]);
                }

                return nullptr;
            }
		public:
			constexpr ALWAYS_INLINE  IntrusiveRedBlackTree() : IntrusiveRedBlackTreeBase() {/*...*/}
			constexpr ALWAYS_INLINE ~IntrusiveRedBlackTree() {/*...*/}

            ALWAYS_INLINE void Insert(pointer node) {
                this->Insert(Traits::GetTreeNode(node));
            }
            ALWAYS_INLINE void Insert(node_type *node) {

                /* Insert node into tree */
                this->InsertImpl(node);

                /* Paint root black */
                m_root->m_parent_color.SetFlagsAndClearPointer(1);

                return;
            }

            ALWAYS_INLINE void Remove(pointer node) {
                this->Remove(Traits::GetTreeNode(node));
            }
            ALWAYS_INLINE void Remove(node_type *node) {
                this->RemoveImpl(node->GetKey());
                if (m_root != nullptr) { m_root->m_parent_color.SetFlagsAndClearPointer(1); }
            }
            ALWAYS_INLINE void Remove(key_type hash) {
                this->RemoveImpl(hash);
                if (m_root != nullptr) { m_root->m_parent_color.SetFlagsAndClearPointer(1); }
            }

            ALWAYS_INLINE pointer Find(key_type hash) {
                node_type *node = this->FindImpl(reinterpret_cast<node_type*>(m_root), hash);
                if (node == nullptr) { return nullptr; }
                return Traits::GetParent(node);
            }
            ALWAYS_INLINE const_pointer Find(key_type hash) const {
                const node_type *node = this->FindImpl(reinterpret_cast<node_type*>(m_root), hash);
                if (node == nullptr) { return nullptr; }
                return Traits::GetParent(node);
            }

            iterator begin() {
                if (m_root == nullptr) { return nullptr; }
                node_type *ret = reinterpret_cast<node_type*>(m_root);
                while (ret->m_left != nullptr) {
                    ret = reinterpret_cast<node_type*>(ret->m_left);
                }
                return iterator(ret);
            }
            constexpr iterator end() {
                return iterator(nullptr);
            }

            const_iterator begin() const {
                if (m_root == nullptr) { return nullptr; }
                const node_type *ret = reinterpret_cast<const node_type*>(m_root);
                while (ret->m_left != nullptr) {
                    ret = reinterpret_cast<const node_type*>(ret->m_left);
                }
                return const_iterator(ret);
            }
            constexpr const_iterator end() const {
                return const_iterator(nullptr);
            }

            static pointer GetNext(pointer node) {
                node_type *next = reinterpret_cast<node_type*>(Traits::GetTreeNode(node)->GetNext());
                return (next != nullptr) ? Traits::GetParent(next) : nullptr;
            }
            static node_type *GetNext(node_type *node) {
                node_type *next = reinterpret_cast<node_type*>(node->GetNext());
                return next;
            }
            static const_pointer GetNext(const_pointer node) {
                const node_type *next = reinterpret_cast<const node_type*>(Traits::GetTreeNode(node)->GetNext());
                return (next != nullptr) ? Traits::GetParent(next) : nullptr;
            }
            static const node_type *GetNext(const node_type *node) {
                const node_type *next = reinterpret_cast<const node_type*>(node->GetNext());
                return next;
            }

            constexpr void ForceReset() { m_root = nullptr; }

            void ClearAll(auto lambda = [] ALWAYS_INLINE_LAMBDA ([[maybe_unused]] pointer obj) {/*...*/}) {

                iterator iter = this->begin();
                while (iter != this->end()) {
                    pointer obj = std::addressof(*iter);
                    ++iter;
                    (lambda)(obj);
                    Traits::GetTreeNode(obj)->ForceUnlink();
                }

                return;
            }

            size_t GetCount() {
                
                size_t count = 0;

                if (m_root == nullptr) { return count; }

                node_type *node = reinterpret_cast<node_type*>(m_root);
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
            using ComparisonFunction = std::strong_ordering(*)(const K&, const K&);
            using CollisionFunction  = void(*)(RP*,RP*);
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

    template <typename T>
    constexpr ALWAYS_INLINE std::strong_ordering RedBlackTreeDefaultCompare(const T &rhs, const T &lhs) {
        return (rhs <=> lhs);
    }
    template <typename T>
    constexpr ALWAYS_INLINE void RedBlackTreeDefaultCollision([[maybe_unused]] T *r0, [[maybe_unused]] T *r1) {
        if (std::is_constant_evaluated()) { return; }
        VP_UNEXPECTED_DEFAULT();
    }

    template<class RP, auto M, auto Compare = RedBlackTreeDefaultCompare<typename vp::util::MemberType<M>::key_type>, auto Collision = RedBlackTreeDefaultCollision<RP>>
    struct IntrusiveRedBlackTreeTraits {
        using Key                = vp::util::MemberType<M>::key_type;
        using Parent             = vp::util::ParentType<M>;
        using Traits             = IntrusiveRedBlackTreeMemberTraits<RP, Key, M>;
        using Node               = IntrusiveRedBlackTreeNode<Key>;
        using Tree               = IntrusiveRedBlackTree<ParentType<M>, Key, Traits, Compare, Collision>;
        //using ReverseTree        = IntrusiveRedBlackTree<ParentType<M>, K, Traits>;
    };

    constexpr ALWAYS_INLINE std::strong_ordering ConvertComparisionToStrongOrdering(const s32 comp) {
        const std::strong_ordering first  = (0 < comp) ? std::strong_ordering::greater : std::strong_ordering::equal;
        const std::strong_ordering second = (comp < 0) ? std::strong_ordering::less : first;
        return second;
    }

    template <typename T>
    constexpr ALWAYS_INLINE std::strong_ordering RedBlackTreeDefaultStringCompare(const T &rhs, const T &lhs) {
        return ConvertComparisionToStrongOrdering(::strcmp(lhs, rhs));
    }

    template <class RP, auto M>
        requires (std::is_same_v<typename vp::util::MemberType<M>::key_type, const char*> || std::is_same_v<typename vp::util::MemberType<M>::key_type, char*>)
    using IntrusiveRedBlackTreeStringTraits = IntrusiveRedBlackTreeTraits<RP, M, RedBlackTreeDefaultStringCompare<typename vp::util::MemberType<M>::key_type>>;

    template <typename T>
    struct HashString {
        using HashType = T;

        HashType hash;
        const char *string;
    };

    template <typename T>
    constexpr ALWAYS_INLINE std::strong_ordering RedBlackTreeDefaultHashStringCompare(const T &rhs, const T &lhs) {
        if (lhs.hash > rhs.hash) { return std::strong_ordering::greater; }
        if (lhs.hash < rhs.hash) { return std::strong_ordering::less; }
        return ConvertComparisionToStrongOrdering(::strcmp(lhs.string, rhs.string));
    }

    template <class RP, auto M>
        requires (std::is_same_v<typename vp::util::MemberType<M>::key_type, HashString<typename vp::util::MemberType<M>::key_type::HashType>>)
    using IntrusiveRedBlackTreeHashStringTraits = IntrusiveRedBlackTreeTraits<RP, M, RedBlackTreeDefaultHashStringCompare<typename vp::util::MemberType<M>::key_type>>;
}
