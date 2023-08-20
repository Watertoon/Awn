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

namespace vp {

    typedef u32 Result;

    namespace result {

        constexpr inline u32 ModuleBits      = 9;
        constexpr inline u32 DescriptionBits = 13;
        constexpr inline u32 ReserveBits     = 9;

        constexpr ALWAYS_INLINE GetModule(Result result) {
            return result & ModuleBits;
        }

        constexpr ALWAYS_INLINE GetDescription(Result result) {
            return (result >> ModuleBits) & DescriptionBits;
        }

        /*NO_RETURN void OnUnhandledResult(Result result) {
            if (result.IsSuccess() == true) {
                ::ExitProcess(1);
            }
            vp::trace::Abort("", "", "", result, "");
        }

        NO_RETURN u32 ConvertInvalidImpl(Result result) {
            OnUnhandledResult(result);
            return result;
        }*/
    }

    constexpr inline Result ResultSuccess = 0;

    #define RESULT_RETURN_SUCCESS \
        return vp::ResultSuccess

    #define RESULT_RETURN_IF(expression, return_result) \
    { \
        if ((expression) == true) { \
            return return_result; \
        } \
    }
    #define RESULT_RETURN_UNLESS(expression, return_result)  \
    { \
        if ((expression) == false) { \
            return return_result; \
        } \
    }

    #define RESULT_ABORT_UNLESS(expression)  \
    { \
        const vp::Result __result = (expression); \
        if (VP_UNLIKELY(__result != vp::ResultSuccess)) { \
            vp::trace::impl::AbortImpl(TOSTRING(expected_result), __PRETTY_FUNCTION__, __FILE__, __LINE__, __result, "Failed: %s\n  Module: %d\n  Description %d\n", TOSTRING(expression), vp::result::GetModule(__result), vp::result::GetDescription(__result)); \
        } \
    }
    #define RESULT_ABORT_UNLESS_EXPECTED(expression, expected_result)  \
    { \
        const vp::Result __result = (expression); \
        if (VP_UNLIKELY(__result != expected_result)) { \
            vp::trace::impl::AbortImpl(TOSTRING(expected_result), __PRETTY_FUNCTION__, __FILE__, __LINE__, __result, "Failed: %s\n  Module: %d\n  Description %d\n", TOSTRING(expression), vp::result::GetModule(__result), vp::result::GetDescription(__result)); \
        } \
    }

    #define RESULT_RANGE_ABORT_UNLESS_EXPECTED(expression, expected_result_range) \
        { \
            const vp::Result __result = (expression); \
            if (__result < Result##expected_result_range##Start || Result##expected_result_range##End < __result) { \
                vp::trace::impl::AbortImpl(TOSTRING(expected_result), __PRETTY_FUNCTION__, __FILE__, __LINE__, __result, "Failed: %s\n  Module: %d\n  Description %d\n", TOSTRING(expression), vp::result::GetModule(__result), vp::result::GetDescription(__result)); \
            } \
        }

    #define DECLARE_RESULT_MODULE(module_num) \
        constexpr inline u32 ResultModule = module_num;

    #define DECLARE_RESULT(result_name, description) \
        constexpr inline u32 Result##result_name =  ((description & vp::result::DescriptionBits) << vp::result::ModuleBits) | (ResultModule & vp::result::ModuleBits);

    #define DECLARE_RESULT_RANGE(result_name, description_start, description_end) \
        constexpr inline u32 Result##result_name##Start =  ((description_start & vp::result::DescriptionBits) << vp::result::ModuleBits) | (ResultModule & vp::result::ModuleBits); \
        constexpr inline u32 Result##result_name##End   =  ((description_end & vp::result::DescriptionBits) << vp::result::ModuleBits) | (ResultModule & vp::result::ModuleBits);
}
