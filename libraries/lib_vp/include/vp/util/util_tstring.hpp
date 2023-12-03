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

    /* These functions are only meant for use in constant evaluated contexts */
    constexpr ALWAYS_INLINE size_t TStringLength(const char *string, size_t max_size) {
        size_t len = 0;
        while ((string[len] != '\0') & (len < max_size)) {
            ++len;
        }
        return len;
    }

    constexpr ALWAYS_INLINE size_t TStringCopy(char *dst, const char *src, size_t max_size) {
        size_t len = 0;
        while ((src[len] != '\0') & (len < max_size)) {
            dst[len] = src[len];
            ++len;
        }
        dst[len] = '\0';
        return len;
    }

    constexpr ALWAYS_INLINE bool TStringCompare(const char *lhs, const char *rhs, size_t max_size) {
        size_t len = 0;
        bool is_same = true;
        bool terminated = false;
        while ((is_same == true) & (terminated == false) & (len < max_size)) {
            is_same = (lhs[len] == rhs[len]);
            terminated = (lhs[len] == '\0') & ('\0' == rhs[len]);
            ++len;
        }
        return is_same;
    }
}
