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

    template<typename T>
    using UnderlyingTypeOf = typename std::remove_reference<decltype( std::declval<T>()[0] )>::type;

    template <typename T>
    consteval int GetCountOf(const T&) {
        return sizeof(T) / sizeof(UnderlyingTypeOf<T>);
    }

    template<class T>
    using element_type = std::remove_reference_t<decltype(*std::begin(std::declval<T&>()))>;

    //template <typename T>
    //struct Parent { static T a; };

    template <typename T>
    struct ParentOfMemberTraits;

    template <typename M, typename P>
    struct ParentOfMemberTraits<M P::*> {
        using Parent = P;
        using Member = M;
    };

    template <auto M>
    using ParentType = ParentOfMemberTraits<decltype(M)>::Parent;

    template <auto M>
    using MemberType = ParentOfMemberTraits<decltype(M)>::Member;

    namespace {

        #pragma pack(push, 1)
        template<typename Member, size_t Size>
        struct _UnionPadding {
            char _pad[Size];
            Member member;
        };
        #pragma pack(pop)

        template<typename Member>
        struct _UnionPadding<Member, 0> {
            Member member;
        };

        template <typename Parent, typename Member, size_t Size>
        struct _OffsetOfUnion {
            union Impl {
                char c;
                Parent parent;
                _UnionPadding<Member, Size> pad;

                constexpr Impl() : c{} {};
                constexpr ~Impl() {};
            };
            constexpr inline static Impl impl{};
        };

        template <typename Parent, typename Member>
        struct _OffsetOfImpl {

            template<auto MemberPtr, size_t Low, size_t High, auto Union = &_OffsetOfUnion<Parent, Member, (Low + High) / 2>::impl>
            static constexpr size_t GetOffset() {

                constexpr size_t cNext = (Low + High) / 2;

                constexpr auto *parent_member_offset = std::addressof(Union->parent.*MemberPtr);
                constexpr auto *union_member_offset  = std::addressof(Union->pad.member);

                if constexpr (parent_member_offset < union_member_offset) {
                    return GetOffset<MemberPtr, Low, cNext - 1>();
                } else if constexpr (parent_member_offset > union_member_offset) {
                    return GetOffset<MemberPtr, cNext + 1, High>();
                } else {
                    return cNext;
                }
            }
        };
    }

    template <auto M>
    constexpr size_t OffsetOf() {
        using Parent = typename ParentOfMemberTraits<decltype(M)>::Parent;
        using Member = typename ParentOfMemberTraits<decltype(M)>::Member;
        return _OffsetOfImpl<Parent, Member>::template GetOffset<M, 0, sizeof(Parent)>();
    }

    //constexpr size_t OffsetOfElementAfter(auto member) {
    //    using Parent = typename ParentOfMemberTraits<decltype(member)>::Parent;
    //    using Member = typename ParentOfMemberTraits<decltype(member)>::Member;
    //    return _OffsetOfImpl<Parent, Member>::template GetOffset<0>(member) + sizeof(Member);
    //}

    /* Only use on virtual functions */
    //template <class T>
    //    requires std::is_member_function_pointer<T>::value
    //ALWAYS_INLINE int GetVTableOffset(T function) {
    //    union
    //    {
    //        T pointer;
    //        int i;
    //    };
    //    pointer = function;
    //
    //    return i - 1;
    //}
}
