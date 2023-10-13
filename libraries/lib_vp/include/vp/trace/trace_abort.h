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

namespace vp::trace {

#ifdef VP_TARGET_PLATFORM_win32
    ALWAYS_INLINE bool IsDebuggerPresent() {
        return ::IsDebuggerPresent();
    }

    ALWAYS_INLINE void Break() {
        if (IsDebuggerPresent() == true) {
            ::DebugBreak();
        }
    }
#endif
#ifdef VP_TARGET_PLATFORM_nx
    ALWAYS_INLINE bool IsDebuggerPresent() {
        size_t value = 0;
        //const Result result = nn::svc::aarch64::GetInfo(std::addressof(value), nn::svc::InfoType::IsDebuggerAttached, 0, 0);
        return value;
    }

    ALWAYS_INLINE void Break() {
        if (IsDebuggerPresent() == true) {
            //const Result result = nn::svc::aarch64::Break(0, 0, 0);
        }
    }
#endif

    namespace impl {

        NO_RETURN void AbortImpl(const char *expected_result, const char *function_name, const char *full_file_path, u32 line_number, const Result result, const char *format, ...);

        NO_RETURN void Abort(const Result result);
    }
}
