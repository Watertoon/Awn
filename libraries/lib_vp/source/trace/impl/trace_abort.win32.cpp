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
#include <vp.hpp>

namespace vp::trace::impl {

    void AbortImpl(const char *expected_result, const char *function_name, const char *full_file_path, u32 line_number, const u32 result, const char *format, ...) {

        /* Create format string */
        va_list var_args;
        ::va_start(var_args, format);
        const int abort_info_length = ::vsnprintf(nullptr, 0, format, var_args);
        char *abort_info = reinterpret_cast<char*>(::malloc(abort_info_length + 1));
        if (abort_info == nullptr) { Abort(result); }
        ::vsnprintf(abort_info, abort_info_length, format, var_args);
        ::va_end(var_args);
        
        /* Create file output */
        const int output_length = ::snprintf(nullptr, 0, "VP ABORT\nFunction: %s\n File: %s:%d\nExpected: %s\n%s", function_name, full_file_path, line_number, expected_result, abort_info);
        char *output = reinterpret_cast<char*>(::malloc(output_length));
        if (output == nullptr) { Abort(result); }
        ::snprintf(output, output_length, "VP ABORT\nFunction: %s\nFile: %s:%d\nExpected: %s\n%s", function_name, full_file_path, line_number, expected_result, abort_info);
        
        /* Output to stdout */
        ::puts(output);

        /* Try to create an output file for crash data */
        char file_path[vp::util::cMaxPath] = {};
        ::snprintf(file_path, vp::util::cMaxPath, "crash_report_module_%d_desc_%d.txt", result::GetModule(result), result::GetDescription(result));
        HANDLE crash_file = ::CreateFile(file_path, GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);

        /* Try write output to file */
        if (crash_file == INVALID_HANDLE_VALUE) {
            ::puts("failed to create file");
            Abort(result);
        }
        const bool write_result = ::WriteFile(crash_file, output, output_length, nullptr, nullptr);
        if (write_result == false) {
            Abort(result);
        }

        /* Abort */
        Abort(result);
    }

    void Abort(const Result result) {

        /* Signal debugger if able */
        Break();

        /* Kill process */
        ::ExitProcess(result);
    }
}
