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

/* While the initial idea of some elements were inspired by Atmosphère, for this implementation I only used cppreference and vague mental notes from months ago */
/* If you can make a case for infringement please let me know and I'll correct it */

namespace vp::util {

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

                constexpr ALWAYS_INLINE Impl() : c{} {};
                constexpr ALWAYS_INLINE ~Impl() {};
            };
            constexpr inline static Impl impl{};
        };

        template <typename Parent, typename Member>
        struct _OffsetOfImpl {

            template<size_t Size, auto Union = &_OffsetOfUnion<Parent, Member, Size>::impl>
            static constexpr ALWAYS_INLINE size_t GetOffset(Member Parent::* member) {
                if constexpr(sizeof(Parent) > Size) {
                    const     auto parent_member_offset = std::addressof((static_cast<const Parent*>(std::addressof(Union->parent)))->*member);
                    constexpr auto union_member_offset  = std::addressof(Union->pad.member);

                    if (parent_member_offset > union_member_offset) {
                        constexpr auto min = sizeof(Member) < alignof(Parent) ? sizeof(Member) : alignof(Parent);
                        return GetOffset<Size + min>(member);
                    } else {
                        return Size;
                    }
                } else {
                    return Size;
                }
            }
        };
    }

    constexpr ALWAYS_INLINE size_t OffsetOf(auto member) {
        using Parent = typename ParentOfMemberTraits<decltype(member)>::Parent;
        using Member = typename ParentOfMemberTraits<decltype(member)>::Member;
        return _OffsetOfImpl<Parent, Member>::template GetOffset<0>(member);
    }

    //constexpr size_t OffsetOfElementAfter(auto member) {
    //    using Parent = typename ParentOfMemberTraits<decltype(member)>::Parent;
    //    using Member = typename ParentOfMemberTraits<decltype(member)>::Member;
    //    return _OffsetOfImpl<Parent, Member>::template GetOffset<0>(member) + sizeof(Member);
    //}

    /* Only use on virtual functions */
    template <class T>
        requires std::is_member_function_pointer<T>::value
    ALWAYS_INLINE int GetVTableOffset(T function) {
        union
        {
            T pointer;
            int i;
        };
        pointer = function;

        return i - 1;
    }
}
