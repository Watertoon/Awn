#pragma once

namespace vp::util {

    template <typename T, size_t Count>
    struct TVectorTypeHelper {
        static constexpr bool cIsDummy = true;
    };

    template <>
    struct TVectorTypeHelper<float, 2> {
        static constexpr bool cIsDummy = false;
        using Type = v2f;
    };

    template <>
    struct TVectorTypeHelper<float, 4> {
        static constexpr bool cIsDummy = false;
        using Type = v4f;
    };

    template <>
    struct TVectorTypeHelper<float, 8> {
        static constexpr bool cIsDummy = false;
        using Type = v8f;
    };

    template <>
    struct TVectorTypeHelper<float, 16> {
        static constexpr bool cIsDummy = false;
        using Type = v16f;
    };

    template <>
    struct TVectorTypeHelper<float, 32> {
        static constexpr bool cIsDummy = false;
        using Type = v32f;
    };

    template <>
    struct TVectorTypeHelper<double, 2> {
        static constexpr bool cIsDummy = false;
        using Type = v2d;
    };

    template <>
    struct TVectorTypeHelper<double, 4> {
        static constexpr bool cIsDummy = false;
        using Type = v4d;
    };

    template <>
    struct TVectorTypeHelper<double, 8> {
        static constexpr bool cIsDummy = false;
        using Type = v8d;
    };

    template <>
    struct TVectorTypeHelper<double, 16> {
        static constexpr bool cIsDummy = false;
        using Type = v16d;
    };

    template <>
    struct TVectorTypeHelper<long long, 2> {
        static constexpr bool cIsDummy = false;
        using Type = v2sll;
    };

    template <>
    struct TVectorTypeHelper<long long, 4> {
        static constexpr bool cIsDummy = false;
        using Type = v4sll;
    };

    template <>
    struct TVectorTypeHelper<long long, 8> {
        static constexpr bool cIsDummy = false;
        using Type = v8sll;
    };

    template <>
    struct TVectorTypeHelper<long long, 16> {
        static constexpr bool cIsDummy = false;
        using Type = v16sll;
    };

    template <>
    struct TVectorTypeHelper<unsigned long long, 2> {
        static constexpr bool cIsDummy = false;
        using Type = v2ull;
    };

    template <>
    struct TVectorTypeHelper<unsigned long long, 4> {
        static constexpr bool cIsDummy = false;
        using Type = v4ull;
    };

    template <>
    struct TVectorTypeHelper<unsigned long long, 8> {
        static constexpr bool cIsDummy = false;
        using Type = v8ull;
    };

    template <>
    struct TVectorTypeHelper<unsigned long long, 16> {
        static constexpr bool cIsDummy = false;
        using Type = v16ull;
    };

    template <>
    struct TVectorTypeHelper<int, 4> {
        static constexpr bool cIsDummy = false;
        using Type = v4si;
    };

    template <>
    struct TVectorTypeHelper<int, 8> {
        static constexpr bool cIsDummy = false;
        using Type = v8si;
    };

    template <>
    struct TVectorTypeHelper<int, 16> {
        static constexpr bool cIsDummy = false;
        using Type = v16si;
    };

    template <>
    struct TVectorTypeHelper<int, 32> {
        static constexpr bool cIsDummy = false;
        using Type = v32si;
    };

    template <>
    struct TVectorTypeHelper<unsigned int, 4> {
        static constexpr bool cIsDummy = false;
        using Type = v4si;
    };

    template <>
    struct TVectorTypeHelper<unsigned int, 8> {
        static constexpr bool cIsDummy = false;
        using Type = v8si;
    };

    template <>
    struct TVectorTypeHelper<unsigned int, 16> {
        static constexpr bool cIsDummy = false;
        using Type = v16si;
    };

    template <>
    struct TVectorTypeHelper<unsigned int, 32> {
        static constexpr bool cIsDummy = false;
        using Type = v32si;
    };

    template <>
    struct TVectorTypeHelper<short, 8> {
        static constexpr bool cIsDummy = false;
        using Type = v8ss;
    };

    template <>
    struct TVectorTypeHelper<short, 16> {
        static constexpr bool cIsDummy = false;
        using Type = v16ss;
    };

    template <>
    struct TVectorTypeHelper<short, 32> {
        static constexpr bool cIsDummy = false;
        using Type = v32ss;
    };

    template <>
    struct TVectorTypeHelper<short, 64> {
        static constexpr bool cIsDummy = false;
        using Type = v64ss;
    };

    template <>
    struct TVectorTypeHelper<unsigned short, 8> {
        static constexpr bool cIsDummy = false;
        using Type = v8us;
    };

    template <>
    struct TVectorTypeHelper<unsigned short, 16> {
        static constexpr bool cIsDummy = false;
        using Type = v16us;
    };

    template <>
    struct TVectorTypeHelper<unsigned short, 32> {
        static constexpr bool cIsDummy = false;
        using Type = v32us;
    };

    template <>
    struct TVectorTypeHelper<unsigned short, 64> {
        static constexpr bool cIsDummy = false;
        using Type = v64us;
    };

    template <>
    struct TVectorTypeHelper<char, 16> {
        static constexpr bool cIsDummy = false;
        using Type = v16cc;
    };

    template <>
    struct TVectorTypeHelper<char, 32> {
        static constexpr bool cIsDummy = false;
        using Type = v32cc;
    };

    template <>
    struct TVectorTypeHelper<char, 64> {
        static constexpr bool cIsDummy = false;
        using Type = v64cc;
    };

    template <>
    struct TVectorTypeHelper<char, 128> {
        static constexpr bool cIsDummy = false;
        using Type = v64cc;
    };

    template <>
    struct TVectorTypeHelper<signed char, 16> {
        static constexpr bool cIsDummy = false;
        using Type = v16sc;
    };

    template <>
    struct TVectorTypeHelper<signed char, 32> {
        static constexpr bool cIsDummy = false;
        using Type = v32sc;
    };

    template <>
    struct TVectorTypeHelper<signed char, 64> {
        static constexpr bool cIsDummy = false;
        using Type = v64sc;
    };

    template <>
    struct TVectorTypeHelper<signed char, 128> {
        static constexpr bool cIsDummy = false;
        using Type = v128sc;
    };

    template <>
    struct TVectorTypeHelper<unsigned char, 16> {
        static constexpr bool cIsDummy = false;
        using Type = v16uc;
    };

    template <>
    struct TVectorTypeHelper<unsigned char, 32> {
        static constexpr bool cIsDummy = false;
        using Type = v32uc;
    };

    template <>
    struct TVectorTypeHelper<unsigned char, 64> {
        static constexpr bool cIsDummy = false;
        using Type = v64uc;
    };

    template <>
    struct TVectorTypeHelper<unsigned char, 128> {
        static constexpr bool cIsDummy = false;
        using Type = v128uc;
    };

    template <typename T, size_t Count>
        requires (!TVectorTypeHelper<std::remove_cv_t<T>, Count>::cIsDummy)
    using TVectorType = TVectorTypeHelper<std::remove_cv_t<T>, Count>::Type;
}
