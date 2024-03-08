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

    class IntrusiveListNode {
        public:
            IntrusiveListNode *m_prev;
            IntrusiveListNode *m_next;
        public:
            constexpr ALWAYS_INLINE  IntrusiveListNode() : m_prev(this), m_next(this) {/*...*/}
            constexpr ALWAYS_INLINE ~IntrusiveListNode() {/*...*/}

            constexpr ALWAYS_INLINE IntrusiveListNode *next() const {
                return m_next;
            }

            constexpr ALWAYS_INLINE IntrusiveListNode *prev() const {
                return m_prev;
            }

            constexpr ALWAYS_INLINE bool IsLinked() const {
                return this != m_next;
            }

            constexpr ALWAYS_INLINE void LinkNext(IntrusiveListNode *new_node) {
                IntrusiveListNode *old_next = new_node->m_next;
                new_node->m_next = m_next;
                old_next->m_prev = this;
                m_next->m_prev   = new_node;
                m_next           = old_next;
            }

            constexpr ALWAYS_INLINE void LinkPrev(IntrusiveListNode *new_node) {
                IntrusiveListNode *new_prev = new_node->m_prev;
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
    };

    template<typename T, class Traits>
    class IntrusiveList {
        public:
            template <bool IsConst>
            class Iterator {
                public:
                    using value_type      = T;
                    using reference       = T&;
                    using const_reference = const T&;
                    using pointer         = T*;
                    using const_pointer   = const T*;
                private:
                    IntrusiveListNode *m_next;
                public:
                    constexpr ALWAYS_INLINE Iterator(IntrusiveListNode *node) : m_next(node) {/*...*/}
                    constexpr ALWAYS_INLINE Iterator(const IntrusiveListNode *node) : m_next(const_cast<IntrusiveListNode*>(node)) {/*...*/}
                    constexpr ALWAYS_INLINE ~Iterator() {/*...*/}

                    ALWAYS_INLINE reference operator*() {
                        return Traits::GetParentReference(m_next);
                    }
                    ALWAYS_INLINE const_reference operator*() const {
                        return Traits::GetParentReference(m_next);
                    }

                    constexpr ALWAYS_INLINE bool operator==(const Iterator<IsConst> &rhs) const {
                        return !(m_next != rhs.m_next);
                    }

                    constexpr ALWAYS_INLINE bool operator!=(const Iterator<IsConst> &rhs) const {
                        return m_next != rhs.m_next;
                    }

                    constexpr ALWAYS_INLINE Iterator<IsConst> &operator++() {
                        m_next = m_next->next();
                        return *this;
                    }
                    constexpr ALWAYS_INLINE Iterator<IsConst> &operator++([[maybe_unused]]int) {
                        m_next = m_next->next();
                        return *this;
                    }
                    constexpr ALWAYS_INLINE Iterator<IsConst> &operator--() {
                        m_next = m_next->prev();
                        return *this;
                    }
            };
        public:
            using value_type      = T;
            using reference       = T&;
            using const_reference = const T&;
            using pointer         = T*;
            using const_pointer   = const T*;
            using iterator        = Iterator<false>;
            using const_iterator  = Iterator<true>;
        private:
            IntrusiveListNode m_list;
        public:
            constexpr ALWAYS_INLINE  IntrusiveList() : m_list() {/*...*/}
            constexpr ALWAYS_INLINE ~IntrusiveList() {/*...*/}

            constexpr ALWAYS_INLINE iterator begin() {
                return iterator(m_list.m_next);
            }
            constexpr ALWAYS_INLINE const_iterator begin() const {
                return const_iterator(m_list.m_next);
            }
            constexpr ALWAYS_INLINE const_iterator cbegin() const {
                return const_iterator(m_list.m_next);
            }

            constexpr ALWAYS_INLINE iterator end() {
                return iterator(std::addressof(m_list));
            }
            constexpr ALWAYS_INLINE const_iterator end() const {
                return const_iterator(std::addressof(m_list));
            }
            constexpr ALWAYS_INLINE const_iterator cend() const {
                return const_iterator(std::addressof(m_list));
            }

            ALWAYS_INLINE reference Front() {
                return Traits::GetParentReference(m_list.m_next);
            }
            ALWAYS_INLINE const_reference Front() const {
                return Traits::GetParentReference(m_list.m_next);
            }

            ALWAYS_INLINE reference Back() {
                return Traits::GetParentReference(m_list.m_prev);
            }
            ALWAYS_INLINE const_reference Back() const {
                return Traits::GetParentReference(m_list.m_prev);
            }

            constexpr ALWAYS_INLINE bool IsEmpty() const {
                return !m_list.IsLinked();
            }

            ALWAYS_INLINE reference GetNext(reference obj) {
                return Traits::GetParentReference(Traits::GetListNode(std::addressof(obj))->next());
            }

            constexpr ALWAYS_INLINE void PushBack(reference obj) {
                m_list.LinkPrev(Traits::GetListNode(std::addressof(obj)));
            }

            constexpr ALWAYS_INLINE void PushFront(reference obj) {
                m_list.LinkNext(Traits::GetListNode(std::addressof(obj)));
            }
            
            ALWAYS_INLINE reference PopFront() {
                IntrusiveListNode *front = m_list.m_next;
                front->Unlink();
                return Traits::GetParentReference(front);
            }

            ALWAYS_INLINE const_reference PopFront() const {
                IntrusiveListNode *front = m_list.m_next;
                front->Unlink();
                return Traits::GetParentReference(front);
            }

            ALWAYS_INLINE reference PopBack() {
                IntrusiveListNode *back = m_list.m_prev;
                back->Unlink();
                return Traits::GetParentReference(back);
            }

            ALWAYS_INLINE const_reference PopBack() const {
                IntrusiveListNode *back = m_list.m_prev;
                back->Unlink();
                return Traits::GetParentReference(back);
            }

            constexpr ALWAYS_INLINE void Clear() {

                /* Remove all nodes */
                iterator iter             = this->begin();
                IntrusiveListNode *i_node = nullptr;
                while (iter != this->end()) {
                    i_node = Traits::GetListNode(std::addressof(*iter));
                    ++iter;
                    i_node->Unlink();
                }
    
                return;
            }

            constexpr ALWAYS_INLINE size_t GetCount() const {
                size_t count = 0;
                for ([[maybe_unused]] const_reference node : *this) {
                    ++count;
                }
                return count;
            }

            static constexpr ALWAYS_INLINE void Remove(reference obj) {
                Traits::GetListNode(std::addressof(obj))->Unlink();
            }

            static constexpr ALWAYS_INLINE iterator IteratorTo(reference obj) {
                return iterator(Traits::GetListNode(std::addressof(obj)));
            }

            static constexpr ALWAYS_INLINE const_iterator IteratorTo(const_reference obj) {
                return const_iterator(Traits::GetListNode(std::addressof(obj)));
            }

            static constexpr pointer GetNext(IntrusiveListNode *node) {
                return Traits::GetParent(node->next());
            }
    };

    template<class RP, auto M>
    struct IntrusiveListMemberTraits {

        static ALWAYS_INLINE RP *GetParent(IntrusiveListNode *node) {
            return reinterpret_cast<RP*>(reinterpret_cast<uintptr_t>(node) - OffsetOf(M));
        }
        static ALWAYS_INLINE RP &GetParentReference(IntrusiveListNode *node) {
            return *reinterpret_cast<RP*>(reinterpret_cast<uintptr_t>(node) - OffsetOf(M));
        }

        static ALWAYS_INLINE const RP *GetParent(const IntrusiveListNode *node) {
            return reinterpret_cast<RP*>(reinterpret_cast<uintptr_t>(node) - OffsetOf(M));
        }
        static ALWAYS_INLINE const RP &GetParentReference(const IntrusiveListNode *node) {
            return *reinterpret_cast<const RP*>(reinterpret_cast<uintptr_t>(node) - OffsetOf(M));
        }

        static constexpr ALWAYS_INLINE IntrusiveListNode *GetListNode(RP *parent) {
            return std::addressof(parent->*M);
        }
        static constexpr ALWAYS_INLINE IntrusiveListNode *GetListNode(const RP *parent) {
            return std::addressof(parent->*M);
        }
    };

    template<class RP, auto M>
    struct IntrusiveListTraits {
        using Traits = IntrusiveListMemberTraits<RP, M>;
        using List   = IntrusiveList<RP, Traits>;
    };
}
