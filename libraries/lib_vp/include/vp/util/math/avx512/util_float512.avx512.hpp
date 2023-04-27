#pragma once

namespace vp::util::avx2 {

    typedef float v16s __attribute__((vector_size(64)));
    typedef double v8d __attribute__((vector_size(64)));

    struct v16f {
        union {
            v16s   s;
            v8d    d;
            __m512 mm;
        };

        constexpr ALWAYS_INLINE v16f() : mm() {}
        constexpr ALWAYS_INLINE v16f(const v16f& copy) : mm(copy.mm) {}

        constexpr ALWAYS_INLINE v16f(const v16s& copy) : s(copy) {}
        constexpr ALWAYS_INLINE v16f(const v8d& copy) : d(copy) {}

        constexpr ALWAYS_INLINE v16f(float x, float y, float z, float w) : s{x,y,z,w} {}
        constexpr ALWAYS_INLINE v16f(double x, double y) : d{x,y} {}
    };

}