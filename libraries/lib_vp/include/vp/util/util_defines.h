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

#define ALWAYS_INLINE __attribute__((always_inline)) inline
#define NO_INLINE __attribute__((noinline))
#define NO_CONSTANT_PROPAGATION __attribute__((optimize("-fno-ipa-cp")))
#define NO_RETURN __attribute__((noreturn))

#define VP_UNLIKELY(expression) __builtin_expect((expression), 0)
#define VP_LIKELY(expression)   __builtin_expect((expression), 1)

#define TOSTRING(var) #var

namespace vp::impl {
    
    template<typename... ArgTypes>
    constexpr ALWAYS_INLINE void UnusedImpl(ArgTypes &&... args) {
        (static_cast<void>(args), ...);
    }
}

#define VP_UNUSED(...) ::vp::impl::UnusedImpl(__VA_ARGS__)
