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

#define VP_ENUM_FLAG_TRAITS(enum_name) \
    constexpr enum_name operator|(const enum_name &lhs, const enum_name &rhs) { \
        return static_cast<enum_name>(static_cast<std::underlying_type_t<enum_name>>(lhs) | static_cast<std::underlying_type_t<enum_name>>(rhs)); \
    } \
    constexpr enum_name operator&(const enum_name &lhs, const enum_name &rhs) { \
        return static_cast<enum_name>(static_cast<std::underlying_type_t<enum_name>>(lhs) & static_cast<std::underlying_type_t<enum_name>>(rhs)); \
    }
