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

    #if defined(VP_DEBUG)

        #define VP_UNEXPECTED_DEFAULT() \
            vp::trace::AbortUnexpectedDefault(__PRETTY_FUNCTION__, __FILE__, __LINE__)

        #define VP_NOT_IMPLEMENTED() \
            vp::trace::AbortNotImplemented(__PRETTY_FUNCTION__, __FILE__, __LINE__)

        #define VP_ASSERT(expression) \
        { \
            const auto _temp_result = (expression); \
            if (__builtin_expect((!_temp_result), 0)) { \
                vp::trace::AbortAssert(__PRETTY_FUNCTION__, __FILE__, __LINE__, TOSTRING(expression)); \
            } \
        }

        #define VP_DETAILED_ASSERT(expression, format, ...) \
        { \
            const auto _temp_result = (expression); \
            if (__builtin_expect((!_temp_result), 0)) { \
                vp::trace::impl::AbortImpl("Assert failed!!!", __PRETTY_FUNCTION__, __FILE__, __LINE__, vp::trace::ResultAssertFailed, format, __VA_ARGS__); \
            } \
        }

    #else

        #define VP_UNEXPECTED_DEFAULT() \
            vp::trace::AbortUnexpectedDefault("", "", 0)

        #define VP_NOT_IMPLEMENTED() \
            vp::trace::AbortNotImplemented("", "", 0); \

        #define VP_ASSERT(expression) \
        { \
            const auto _temp_result = (expression); \
            if (__builtin_expect((!_temp_result), 0)) { \
                vp::trace::AbortAssert("", "", 0, ""); \
            } \
        }

        #define VP_DETAILED_ASSERT(expression, format, ...) \
        { \
            const auto _temp_result = (expression); \
            if (__builtin_expect((!_temp_result), 0)) { \
                vp::trace::impl::AbortImpl("", "", "", 0, vp::trace::ResultAssertFailed, ""); \
            } \
        }

    #endif
}
