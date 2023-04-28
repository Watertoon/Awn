#pragma once

namespace vp::util {

    namespace {
        constexpr ALWAYS_INLINE float ConvertUnormToFloat(u8 value) {
            return static_cast<float>(value) / 255.0f;
        }
        constexpr ALWAYS_INLINE u8 ConvertFloatToUnorm(float value) {
            const float clamp0 = (1.0f < value) ? 1.0f : value;
            const float clamp1 = (clamp0 < 0.0f) ? 0.0f : clamp0;
            return static_cast<u8>((clamp1 * 255.0f) + 0.5f);
        }
    }

    class Color4f;

    class Color4u8 {
        public:
            union {
                struct {
                    u8 m_r;
                    u8 m_g;
                    u8 m_b;
                    u8 m_a;
                };
                u32 m_packed_color;
            };
        public:
            constexpr ALWAYS_INLINE Color4u8() : m_r(), m_g(), m_b(), m_a(255) {/*...*/}
            constexpr ALWAYS_INLINE Color4u8(u8 r, u8 g, u8 b) : m_r(r), m_g(g), m_b(b), m_a(255) {/*...*/}
            constexpr ALWAYS_INLINE Color4u8(u8 r, u8 g, u8 b, u8 a) : m_r(r), m_g(g), m_b(b), m_a(a) {/*...*/}
            constexpr ALWAYS_INLINE Color4u8(float r, float g, float b) : m_r(ConvertFloatToUnorm(r)), m_g(ConvertFloatToUnorm(g)), m_b(ConvertFloatToUnorm(b)), m_a(255) {/*...*/}
            constexpr ALWAYS_INLINE Color4u8(float r, float g, float b, float a) : m_r(ConvertFloatToUnorm(r)), m_g(ConvertFloatToUnorm(g)), m_b(ConvertFloatToUnorm(b)), m_a(ConvertFloatToUnorm(a)) {/*...*/}
            constexpr ALWAYS_INLINE Color4u8(const Color4u8 &copy) : m_r(copy.m_r), m_g(copy.m_g), m_b(copy.m_b), m_a(copy.m_a) {/*...*/}
            constexpr ALWAYS_INLINE Color4u8(const Color4f &copy);

            constexpr ALWAYS_INLINE Color4u8 &operator=(const Color4u8 &rhs) {
                m_r = rhs.m_r;
                m_g = rhs.m_g;
                m_b = rhs.m_b;
                m_a = rhs.m_a;
                return *this;
            }
            constexpr ALWAYS_INLINE Color4u8 &operator=(const Color4f &rhs);
    };

        class Color4f {
        public:
            float m_r;
            float m_g;
            float m_b;
            float m_a;
        public:
            constexpr ALWAYS_INLINE Color4f() : m_r(), m_g(), m_b(), m_a(1.0f) {/*...*/}
            constexpr ALWAYS_INLINE Color4f(float r, float g, float b) : m_r(r), m_g(g), m_b(b), m_a(1.0f) {/*...*/}
            constexpr ALWAYS_INLINE Color4f(float r, float g, float b, float a) : m_r(r), m_g(g), m_b(b), m_a(a) {/*...*/}
            constexpr ALWAYS_INLINE Color4f(u8 r, u8 g, u8 b) : m_r(ConvertUnormToFloat(r)), m_g(ConvertUnormToFloat(g)), m_b(ConvertUnormToFloat(b)), m_a(1.0f) {/*...*/}
            constexpr ALWAYS_INLINE Color4f(u8 r, u8 g, u8 b, u8 a) : m_r(ConvertUnormToFloat(r)), m_g(ConvertUnormToFloat(g)), m_b(ConvertUnormToFloat(b)), m_a(ConvertUnormToFloat(a)) {/*...*/}
            constexpr ALWAYS_INLINE Color4f(const Color4f &copy) : m_r(copy.m_r), m_g(copy.m_g), m_b(copy.m_b), m_a(copy.m_a) {/*...*/}
            constexpr ALWAYS_INLINE Color4f(const Color4u8 &copy) : m_r(ConvertUnormToFloat(copy.m_r)), m_g(ConvertUnormToFloat(copy.m_g)), m_b(ConvertUnormToFloat(copy.m_b)), m_a(ConvertUnormToFloat(copy.m_a)) {/*...*/}

            constexpr ALWAYS_INLINE Color4f &operator=(const Color4f &rhs) {
                m_r = rhs.m_r;
                m_g = rhs.m_g;
                m_b = rhs.m_b;
                m_a = rhs.m_a;
                return *this;
            }
            constexpr ALWAYS_INLINE Color4f &operator=(const Color4u8 &rhs) {
                m_r = ConvertUnormToFloat(rhs.m_r);
                m_g = ConvertUnormToFloat(rhs.m_g);
                m_b = ConvertUnormToFloat(rhs.m_b);
                m_a = ConvertUnormToFloat(rhs.m_a);
                return *this;
            }
    };

    constexpr ALWAYS_INLINE Color4u8::Color4u8(const Color4f &copy) : m_r(ConvertFloatToUnorm(copy.m_r)), m_g(ConvertFloatToUnorm(copy.m_g)), m_b(ConvertFloatToUnorm(copy.m_b)), m_a(ConvertFloatToUnorm(copy.m_a)) {/*...*/}
    constexpr ALWAYS_INLINE Color4u8 &Color4u8::operator=(const Color4f &rhs) {
        m_r = ConvertFloatToUnorm(rhs.m_r);
        m_g = ConvertFloatToUnorm(rhs.m_g);
        m_b = ConvertFloatToUnorm(rhs.m_b);
        m_a = ConvertFloatToUnorm(rhs.m_a);
        return *this;
    }

    constexpr ALWAYS_INLINE Color4f cBlack = {0.0f, 0.0f, 0.0f, 1.0f};
    constexpr ALWAYS_INLINE Color4f cWhite = {1.0f, 1.0f, 1.0f, 1.0f};
}