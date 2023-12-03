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

/* Thanks to the Reswitched Discord and Atmosphere-NX (https://github.com/Atmosphere-NX/Atmosphere) for leading me to Andrei Alexandrescu's talk "Systemic Error Handling" (https://youtu.be/kaI4R0Ng4E8?feature=shared)  */

namespace vp::util {

    template <typename Function>
    struct ScopeGuard {
        Function function;
        bool     is_active;

        constexpr ALWAYS_INLINE ScopeGuard() = delete;
        constexpr ALWAYS_INLINE ScopeGuard(const ScopeGuard&) = delete;
        constexpr ALWAYS_INLINE ScopeGuard(const ScopeGuard&& rhs) : function(rhs.function), is_active(rhs.is_active) {
            rhs.Cancel();
        }

        constexpr ALWAYS_INLINE ScopeGuard(Function function) : function(function), is_active(true) {/*...*/}
        constexpr ALWAYS_INLINE ~ScopeGuard() { if (is_active != false) { (function)(); } }
        constexpr ALWAYS_INLINE void Cancel() { is_active = false; }
    };

    enum class _EnumClassForScopeGuard {/*...*/};

    template <typename Function>
    constexpr ALWAYS_INLINE ScopeGuard<Function> operator+(_EnumClassForScopeGuard, Function &&function) {
        return ScopeGuard<Function>(std::forward<Function>(function));
    }

	#define SCOPE_GUARD vp::util::_EnumClassForScopeGuard() + [&]() ALWAYS_INLINE_LAMBDA 
	#define ON_SCOPE_EXIT auto ANONYMOUS_NAME(_scope_guard_) = SCOPE_GUARD
}
