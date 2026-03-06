#pragma once

namespace awn::hid {

    enum class MouseButton : u32 {
        Left = 0,
        Right = 1,
        Middle = 2,
        Forward = 3,
        Backward = 4,
        Invalid = 0x20,
    };
    using MouseButtonSet = vp::util::FixedConnection<u32, 0x20>;

    enum class MouseAttribute : u32 {
        Transferable = 0,
        IsConnected = 1,
    };
    using MouseAttributeSet = vp::util::FixedConnection<u32, 0x20>;

#ifdef VP_TARGET_PLATFORM_linux
    constexpr MouseButton ConvertInputEventCodeToMouseButton(u16 code) {
        switch (code) {
            case BTN_LEFT:
                return MouseButton::Left;
            case BTN_RIGHT:
                return MouseButton::Right;
            case BTN_MIDDLE:
                return MouseButton::Middle;
            case BTN_FORWARD:
                return MouseButton::Forward;
            case BTN_BACK:
                return MouseButton::Backward;
            default:
                break;
        }
        return MouseButton::Invalid;
    }
#endif
}