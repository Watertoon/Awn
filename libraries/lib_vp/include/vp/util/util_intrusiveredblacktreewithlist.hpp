#pragma once

namespace vp::util {

	template <typename K>
	class IntrusiveRedBlackTreeNodeWithList {
        public:
            using key_type  = K;
            using node_type = IntrusiveRedBlackTreeNodeWithList<key_type>;
		public:
			vp::util::IntrusiveRedBlackTreeNode<key_type>  m_tree_node;
            node_type                                     *m_next;
            node_type                                     *m_prev;
        public:
            constexpr ALWAYS_INLINE  IntrusiveRedBlackTreeNodeWithList() : m_tree_node(), m_next(this), m_prev(this) {/*...*/}
            constexpr ALWAYS_INLINE  IntrusiveRedBlackTreeNodeWithList(key_type key) : m_tree_node(key), m_next(this), m_prev(this) {/*...*/}
            constexpr ALWAYS_INLINE ~IntrusiveRedBlackTreeNodeWithList() {/*...*/}

            constexpr ALWAYS_INLINE bool IsEnd() const {
                node_type *next_node = m_next;
                return next_node->m_tree_node.IsLinked() && next_node->m_tree_node.IsEnd();
            }

            constexpr ALWAYS_INLINE bool IsLinked() const {
                return m_tree_node.IsLinked() || !(this == m_next);
            }

            node_type *GetNext() {

                /* Check if the next node is the parent */
                node_type *next_node = m_next;
                if (!next_node->m_tree_node.IsLinked()) { return next_node; }

                /* Get next tree node */
                return reinterpret_cast<node_type*>(next_node->m_tree_node.GetNext());
            }
            
            constexpr ALWAYS_INLINE void SetKey(key_type hash) {
                m_tree_node.SetKey(hash);
            }
            constexpr ALWAYS_INLINE key_type GetKey() const {
                return m_tree_node.GetKey();
            }
        public:
            constexpr ALWAYS_INLINE void LinkNext(node_type *new_node) {
                node_type *old_next = new_node->m_next;
                new_node->m_next = m_next;
                old_next->m_prev = this;
                m_next->m_prev   = new_node;
                m_next           = old_next;
            }

            constexpr ALWAYS_INLINE void LinkPrev(node_type *new_node) {
                node_type *new_prev = new_node->m_prev;
                new_node->m_prev = m_prev;
                new_prev->m_next = this;
                m_prev->m_next   = new_node;
                m_prev           = new_prev;
            }

            constexpr ALWAYS_INLINE void Unlink() {
                m_prev->m_next = m_next;
                m_next->m_prev = m_prev;
                m_next = this;
                m_prev = this;
            }

            constexpr ALWAYS_INLINE bool IsListLinked() const {
                return !(this == m_next);
            }
            constexpr ALWAYS_INLINE bool IsTreeLinked() const {
                return m_tree_node.IsLinked();
            }
	};

	template<typename T, typename K, class Traits, Traits::ComparisonFunction Compare, Traits::CollisionFunction Collision>
    class IntrusiveRedBlackTreeWithList {
        public:
            using value_type      = T;
            using reference       = T&;
            using const_reference = const T&;
            using pointer         = T*;
            using const_pointer   = const T*;
            using node_type       = vp::util::IntrusiveRedBlackTreeNodeWithList<K>;
            using key_type        = K;
            using BaseTree        = vp::util::IntrusiveRedBlackTreeTraits<node_type, &node_type::m_tree_node, Compare, Collision>::Tree;
        private:
            BaseTree m_tree;
            u32  m_count;
        public:
            class Iterator {
                private:
                    node_type *m_node;
                public:
                    constexpr Iterator(node_type *node) : m_node(node) {/*...*/}
                    constexpr ~Iterator() {/*...*/}

                    ALWAYS_INLINE reference operator*() {
                        return Traits::GetParentReference(m_node);
                    }
                    ALWAYS_INLINE const_reference operator*() const {
                        return Traits::GetParentReference(m_node);
                    }

                    Iterator &operator++() {
                        m_node = m_node.GetNext();
                        return *this;
                    }
                    constexpr bool operator==(const Iterator& rhs) const {
                        return (m_node == rhs.m_node);
                    }
            };
            class ConstIterator {
                private:
                    node_type *m_node;
                public:
                    constexpr ConstIterator(node_type *node) : m_node(node) {/*...*/}
                    constexpr ~ConstIterator() {/*...*/}

                    ALWAYS_INLINE reference operator*() {
                        return Traits::GetParentReference(m_node);
                    }
                    ALWAYS_INLINE const_reference operator*() const {
                        return Traits::GetParentReference(m_node);
                    }

                    ConstIterator &operator++() {
                        m_node = m_node.GetNext();
                        return *this;
                    }
                    constexpr bool operator==(const ConstIterator& rhs) const {
                        return (m_node == rhs.m_node);
                    }
            };
        public:
            using iterator = Iterator;
            using const_iterator = ConstIterator;
        public:
			constexpr ALWAYS_INLINE  IntrusiveRedBlackTreeWithList() : m_tree(), m_count() {/*...*/}
			constexpr ALWAYS_INLINE ~IntrusiveRedBlackTreeWithList() {/*...*/}

            iterator begin() {
                node_type *node = std::addressof(*m_tree.begin());
                return iterator(node);
            }
            constexpr iterator end() {
                return iterator(nullptr);
            }

            const_iterator begin() const {
                node_type *node = std::addressof(*m_tree.begin());
                return const_iterator(node);
            }
            constexpr const_iterator end() const {
                return const_iterator(nullptr);
            }

            ALWAYS_INLINE void Insert(pointer node) {
                this->Insert(Traits::GetTreeNode(node));
            }
            void Insert(node_type *node) {

                /* Increase count on exit */
                ON_SCOPE_EXIT { ++m_count; };

                /* Insert node into tree if no collisions in tree */
                node_type *found_node = m_tree.Find(node->GetKey());
                if (found_node != nullptr) {
                    m_tree.Insert(found_node);
                    return;
                }

                /* Link for collision */
                found_node->LinkNext(node);

                return;
            }

            ALWAYS_INLINE void Remove(pointer node) {
                this->Remove(Traits::GetTreeNode(node));
            }
            void Remove(node_type *node) {
                if (node->IsListLinked()) { node->Unlink(); }
                if (node->IsTreeLinked()) { m_tree.Remove(node); }
                --m_count;
            }

            ALWAYS_INLINE pointer Find(key_type hash) {
                node_type *node = m_tree.Find(hash);
                return Traits::GetParent(node);
            }

            pointer GetNext(pointer node) {
                node_type *next = reinterpret_cast<node_type*>(Traits::GetTreeNode(node)->GetNext());
                return (next != nullptr) ? Traits::GetParent(next) : nullptr;
            }
            node_type *GetNext(node_type *node) {
                node_type *next = reinterpret_cast<node_type*>(node->GetNext());
                return next;
            }

            constexpr ALWAYS_INLINE u32 GetCount() const { return m_count; }
    };

    template<class RP, typename K, auto M>
    class IntrusiveRedBlackTreeWithListMemberTraits {
        public:
            static constexpr uintptr_t cOffset = OffsetOf<M>();
        public:
            using ComparisonFunction = std::strong_ordering(*)(const K&, const K&);
            using CollisionFunction  = void(*)(vp::util::IntrusiveRedBlackTreeNodeWithList<K>*,vp::util::IntrusiveRedBlackTreeNodeWithList<K>*);
        public:
            static ALWAYS_INLINE RP *GetParent(IntrusiveRedBlackTreeNodeWithList<K> *node) {
                return reinterpret_cast<RP*>(reinterpret_cast<uintptr_t>(node) - cOffset);
            }
            static ALWAYS_INLINE RP &GetParentReference(IntrusiveRedBlackTreeNodeWithList<K> *node) {
                return *reinterpret_cast<RP*>(reinterpret_cast<uintptr_t>(node) - cOffset);
            }

            static ALWAYS_INLINE const RP *GetParent(const IntrusiveRedBlackTreeNodeWithList<K> *node) {
                return reinterpret_cast<RP*>(reinterpret_cast<uintptr_t>(node) - cOffset);
            }
            static ALWAYS_INLINE const RP &GetParentReference(const IntrusiveRedBlackTreeNodeWithList<K> *node) {
                return *reinterpret_cast<const RP*>(reinterpret_cast<uintptr_t>(node) - cOffset);
            }

            static constexpr ALWAYS_INLINE IntrusiveRedBlackTreeNodeWithList<K> *GetTreeNode(RP *parent) {
                return std::addressof(parent->*M);
            }
            static constexpr ALWAYS_INLINE const IntrusiveRedBlackTreeNodeWithList<K> *GetTreeNode(const RP *parent) {
                return std::addressof(parent->*M);
            }
    };

    template<class RP, auto M, auto Compare = RedBlackTreeDefaultCompare<typename vp::util::MemberType<M>::key_type>, auto Collision = RedBlackTreeDefaultCollision<vp::util::IntrusiveRedBlackTreeNodeWithList<typename vp::util::MemberType<M>::key_type>>>
    struct IntrusiveRedBlackTreeWithListTraits {
        using Key    = vp::util::MemberType<M>::key_type;
        using Parent = vp::util::ParentType<M>;
        using Traits = IntrusiveRedBlackTreeWithListMemberTraits<RP, Key, M>;
        using Node   = IntrusiveRedBlackTreeNodeWithList<Key>;
        using Tree   = IntrusiveRedBlackTreeWithList<ParentType<M>, Key, Traits, Compare, Collision>;
        //using ReverseTree        = IntrusiveRedBlackTree<ParentType<M>, K, Traits>;
    };
}
