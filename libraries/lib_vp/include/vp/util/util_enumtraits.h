#pragma once

#define VP_ENUM_FLAG_TRAITS(enum_name) \
    constexpr enum_name operator|(const enum_name &lhs, const enum_name &rhs) { \
        return static_cast<enum_name>(static_cast<std::underlying_type_t<enum_name>>(lhs) | static_cast<std::underlying_type_t<enum_name>>(rhs)); \
    }
