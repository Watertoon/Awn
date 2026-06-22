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

    namespace result {        
        static constexpr inline u32 cModuleBits         = 9;
        static constexpr inline u32 cModuleBitMask      = (~((~0) << cModuleBits));
        static constexpr inline u32 cDescriptionBits    = 13;
        static constexpr inline u32 cDescriptionBitMask = (~((~0) << cDescriptionBits));
        static constexpr inline u32 cReserveBits        = 10;
        static constexpr inline u32 cReserveBitMask     = (~((~0) << cReserveBits));
    }

    struct [[nodiscard]] Result {
        u32 value;

        constexpr bool operator==(const Result &rhs) const {
            return this->value == rhs.value;
        }
        constexpr u32 GetValue() const {
            return this->value;
        }
        constexpr u32 GetModule() const {
            return this->value & ::vp::result::cModuleBitMask;
        }

        constexpr u32 GetDescription() const {
            return (this->value >> ::vp::result::cModuleBits) & ::vp::result::cDescriptionBitMask;
        }
    };

    static constexpr inline Result ResultSuccess = { 0, };

    #define RESULT_RETURN_SUCCESS \
        return ::vp::ResultSuccess

    #define RESULT_DISCARD(result_expression) \
    { \
        const ::vp::Result __result = (result_expression); \
        VP_UNUSED(__result); \
    }

    #define RESULT_RETURN_IF(expression, result_expression) \
    { \
        if ((expression) == true) { \
            const ::vp::Result __result = (result_expression); \
            return __result; \
        } \
    }

    #define RESULT_RETURN_UNLESS(expression, result_expression)  \
    { \
        if ((expression) == false) { \
            const ::vp::Result __result = (result_expression); \
            return __result; \
        } \
    }

    #define RESULT_RETURN(result_expression)  \
    { \
        const ::vp::Result __result = (result_expression); \
        return __result; \
    }

    #define RESULT_TRY(result_expression)  \
    { \
        const ::vp::Result __result = (result_expression); \
        if (RESULT_FAILED(__result)) { \
            return __result; \
        } \
    }

    #define RESULT_SUCCEEDED(result)  \
        ((result) == ::vp::ResultSuccess)

    #define RESULT_FAILED(result)  \
        ((result) != ::vp::ResultSuccess)

    #define RESULT_MATCHES(result0, result1)  \
        ((result0) == (result1))

#ifdef VP_TARGET_BINARY_TYPE_release
    #define RESULT_ABORT_UNLESS(result_expression)  \
    { \
        const ::vp::Result __result = (result_expression); \
        if (VP_UNLIKELY(__result != ::vp::ResultSuccess)) { \
            ::vp::trace::AbortResult("", "", 0, __result, ""); \
        } \
    }
    #define RESULT_ABORT_UNLESS_EXPECTED(result_expression, expected_result)  \
    { \
        const ::vp::Result __result = (result_expression); \
        if (VP_UNLIKELY(__result != expected_result)) { \
            ::vp::trace::AbortResultExpected("", "", "", 0, __result, ""); \
        } \
    }

    #define RESULT_RANGE_ABORT_UNLESS_EXPECTED(result_expression, expected_result_range) \
        { \
            const vp::Result __result = (result_expression); \
            if (__result < Result##expected_result_range##Start || Result##expected_result_range##End < __result) { \
                ::vp::trace::AbortResultExpected("", "", "", 0, __result, ""); \
            } \
        }
#else
    #define RESULT_ABORT_UNLESS(result_expression)  \
    { \
        const ::vp::Result __result = (result_expression); \
        if (VP_UNLIKELY(__result != ::vp::ResultSuccess)) { \
            ::vp::trace::AbortResult(__PRETTY_FUNCTION__, __FILE__, __LINE__, __result, TOSTRING(result_expression)); \
        } \
    }
    #define RESULT_ABORT_UNLESS_EXPECTED(result_expression, expected_result)  \
    { \
        const ::vp::Result __result = (result_expression); \
        if (VP_UNLIKELY(__result != expected_result)) { \
            ::vp::trace::AbortResultExpected(TOSTRING(expected_result), __PRETTY_FUNCTION__, __FILE__, __LINE__, __result, TOSTRING(result_expression)); \
        } \
    }

    #define RESULT_RANGE_ABORT_UNLESS_EXPECTED(result_expression, expected_result_range) \
        { \
            const vp::Result __result = (result_expression); \
            if (__result < Result##expected_result_range##Start || Result##expected_result_range##End < __result) { \
                ::vp::trace::AbortResultExpected(TOSTRING(expected_result_range), __PRETTY_FUNCTION__, __FILE__, __LINE__, __result, TOSTRING(result_expression)); \
            } \
        }
#endif

    #define DECLARE_RESULT_MODULE(module_num) \
        static constexpr inline u32 ResultModule = module_num; \
        static_assert(0 < ResultModule && ResultModule < (1ull << ::vp::result::cModuleBits));

    #define DECLARE_RESULT(result_name, description) \
        static constexpr inline Result Result##result_name = { ((description & ::vp::result::cDescriptionBitMask) << ::vp::result::cModuleBits) | (ResultModule & ::vp::result::cModuleBitMask), }; \
        static_assert( description < (1ull << ::vp::result::cDescriptionBits));

    #define DECLARE_RESULT_RANGE(result_name, description_start, description_end) \
        static constexpr inline Result Result##result_name##Start =  ((description_start & ::vp::result::cDescriptionBitMask) << ::vp::result::cModuleBits) | (ResultModule & ::vp::result::cModuleBitMask); \
        static constexpr inline Result Result##result_name##End   =  ((description_end & ::vp::result::cDescriptionBitMask) << ::vp::result::cModuleBits) | (ResultModule & ::vp::result::cModuleBitMask);
}
