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

    ALWAYS_INLINE bool IsDebuggerPresent() {
        return ::IsDebuggerPresent();
    }

    ALWAYS_INLINE void Break() {
        if (IsDebuggerPresent() == true) {
            ::DebugBreak();
        }
    }

    namespace impl {

        NO_RETURN void AbortImpl(const char *expected_result, const char *function_name, const char *full_file_path, u32 line_number, const Result result, const char *format, ...);

        NO_RETURN void Abort(const Result result);
    }
}
