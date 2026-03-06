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

namespace awn::hid {

    enum class KeyboardAttribute : u32 {};

    /* From: https://switchbrew.org/wiki/HID_services#KeyboardModifierSet */
    enum class KeyboardModifier : u32 {
        Control    = 0,
        Shift      = 1,
        LeftAlt    = 2,
        RightAlt   = 3,
        Gui        = 4,
        CapsLock   = 8,
        ScrollLock = 9,
        NumLock    = 10,
        Katakana   = 11,
        Hiragana   = 12,
        Invalid   = 0x20,
    };

    /* From: https://switchbrew.org/wiki/HID_services#KeyboardKey */
    enum class KeyboardKey : u32 {
        Invalid           = 0,
        A                 = 4,
        B                 = 5,
        C                 = 6,
        D                 = 7,
        E                 = 8,
        F                 = 9,
        G                 = 10,
        H                 = 11,
        I                 = 12,
        J                 = 13,
        K                 = 14,
        L                 = 15,
        M                 = 16,
        N                 = 17,
        O                 = 18,
        P                 = 19,
        Q                 = 20,
        R                 = 21,
        S                 = 22,
        T                 = 23,
        U                 = 24,
        V                 = 25,
        W                 = 26,
        X                 = 27,
        Y                 = 28,
        Z                 = 29,
        D1                = 30,
        D2                = 31,
        D3                = 32,
        D4                = 33,
        D5                = 34,
        D6                = 35,
        D7                = 36,
        D8                = 37,
        D9                = 38,
        D0                = 39,
        Return            = 40,
        Escape            = 41,
        Backspace         = 42,
        Tab               = 43,
        Space             = 44,
        Dash              = 45,
        Equals            = 46,
        OpenBracket       = 47,
        CloseBracket      = 48,
        Pipe              = 49,
        Tilde             = 50,
        Semicolon         = 51,
        Quote             = 52,
        Backquote         = 53,
        Comma             = 54,
        Period            = 55,
        Slash             = 56,
        CapsLock          = 57,
        F1                = 58,
        F2                = 59,
        F3                = 60,
        F4                = 61,
        F5                = 62,
        F6                = 63,
        F7                = 64,
        F8                = 65,
        F9                = 66,
        F10               = 67,
        F11               = 68,
        F12               = 69,
        PrintScreen       = 70,
        ScrollLock        = 71,
        Pause             = 72,
        Insert            = 73,
        Home              = 74,
        PageUp            = 75,
        Delete            = 76,
        End               = 77,
        PageDown          = 78,
        RightArrow        = 79,
        LeftArrow         = 80,
        DownArrow         = 81,
        UpArrow           = 82,
        NumLock           = 83,
        NumPad_Divide     = 84,
        NumPad_Multiply   = 85,
        NumPad_Subtract   = 86,
        NumPad_Add        = 87,
        NumPad_Enter      = 88,
        NumPad_1          = 89,
        NumPad_2          = 90,
        NumPad_3          = 91,
        NumPad_4          = 92,
        NumPad_5          = 93,
        NumPad_6          = 94,
        NumPad_7          = 95,
        NumPad_8          = 96,
        NumPad_9          = 97,
        NumPad_0          = 98,
        NumPad_Dot        = 99,
        Backslash         = 100,
        Application       = 101,
        Power             = 102,
        NumPad_Equals     = 103,
        F13               = 104,
        F14               = 105,
        F15               = 106,
        F16               = 107,
        F17               = 108,
        F18               = 109,
        F19               = 110,
        F20               = 111,
        F21               = 112,
        F22               = 113,
        F23               = 114,
        F24               = 115,
        NumPad_Comma      = 133,
        Ro                = 135,
        KatakanaHiragana  = 136,
        Yen               = 137,
        Henkan            = 138,
        Muhenkan          = 139,
        NumPad_CommaPc98  = 140,
        HangulEnglish     = 144,
        Hanja             = 145,
        Katakana          = 146,
        Hiragana          = 147,
        ZenkakuHankaku    = 148,
        LeftControl       = 224,
        LeftShift         = 225,
        LeftAlt           = 226,
        LeftGui           = 227,
        RightControl      = 228,
        RightShift        = 229,
        RightAlt          = 230,
        RightGui          = 231,
    };

    constexpr KeyboardKey ConvertScanCodeToKeyboardKey(u32 scan_code1) {
        switch (scan_code1) {
            case 0x0029:
                return KeyboardKey::Tilde;
            case 0x0002:
                return KeyboardKey::D1;
            case 0x0003:
                return KeyboardKey::D2;
            case 0x0004:
                return KeyboardKey::D3;
            case 0x0005:
                return KeyboardKey::D4;
            case 0x0006:
                return KeyboardKey::D5;
            case 0x0007:
                return KeyboardKey::D6;
            case 0x0008:
                return KeyboardKey::D7;
            case 0x0009:
                return KeyboardKey::D8;
            case 0x000a:
                return KeyboardKey::D9;
            case 0x000b:
                return KeyboardKey::D0;
            case 0x000c:
                return KeyboardKey::Dash;
            case 0x000d:
                return KeyboardKey::Equals;
            case 0x007d:
                return KeyboardKey::Yen;
            case 0x000e:
                return KeyboardKey::Backspace;
            case 0x000f:
                return KeyboardKey::Tab;
            case 0x0010:
                return KeyboardKey::Q;
            case 0x0011:
                return KeyboardKey::W;
            case 0x0012:
                return KeyboardKey::E;
            case 0x0013:
                return KeyboardKey::R;
            case 0x0014:
                return KeyboardKey::T;
            case 0x0015:
                return KeyboardKey::Y;
            case 0x0016:
                return KeyboardKey::U;
            case 0x0017:
                return KeyboardKey::I;
            case 0x0018:
                return KeyboardKey::O;
            case 0x0019:
                return KeyboardKey::P;
            case 0x001a:
                return KeyboardKey::OpenBracket;
            case 0x001b:
                return KeyboardKey::CloseBracket;
            case 0x002b:
                return KeyboardKey::Pipe;
            case 0x003a:
                return KeyboardKey::CapsLock;
            case 0x001e:
                return KeyboardKey::A;
            case 0x001f:
                return KeyboardKey::S;
            case 0x0020:
                return KeyboardKey::D;
            case 0x0021:
                return KeyboardKey::F;
            case 0x0022:
                return KeyboardKey::G;
            case 0x0023:
                return KeyboardKey::H;
            case 0x0024:
                return KeyboardKey::J;
            case 0x0025:
                return KeyboardKey::K;
            case 0x0026:
                return KeyboardKey::L;
            case 0x0027:
                return KeyboardKey::Semicolon;
            case 0x0028:
                return KeyboardKey::Quote;
            case 0x001c:
                return KeyboardKey::Return;
            case 0x002a:
                return KeyboardKey::LeftShift;
            case 0x002c:
                return KeyboardKey::Z;
            case 0x002d:
                return KeyboardKey::X;
            case 0x002e:
                return KeyboardKey::C;
            case 0x002f:
                return KeyboardKey::V;
            case 0x0030:
                return KeyboardKey::B;
            case 0x0031:
                return KeyboardKey::N;
            case 0x0032:
                return KeyboardKey::M;
            case 0x0033:
                return KeyboardKey::Comma;
            case 0x0034:
                return KeyboardKey::Period;
            case 0x0035:
                return KeyboardKey::Slash;
            case 0x0073:
                return KeyboardKey::Backslash;
            case 0x0036:
                return KeyboardKey::RightShift;
            case 0x001d:
                return KeyboardKey::LeftControl;
            case 0x0038:
                return KeyboardKey::LeftAlt;
            case 0x0039:
                return KeyboardKey::Space;
            case 0xe038:
                return KeyboardKey::RightAlt;
            case 0xe01d:
                return KeyboardKey::RightControl;
            case 0xe052:
                return KeyboardKey::Insert;
            case 0xe053:
                return KeyboardKey::Delete;
            case 0xe04b:
                return KeyboardKey::LeftArrow;
            case 0xe047:
                return KeyboardKey::Home;
            case 0xe04F:
                return KeyboardKey::End;
            case 0xe048:
                return KeyboardKey::UpArrow;
            case 0xe050:
                return KeyboardKey::DownArrow;
            case 0xe049:
                return KeyboardKey::PageUp;
            case 0xe051:
                return KeyboardKey::PageDown;
            case 0xe04d:
                return KeyboardKey::RightArrow;
            case 0x0045:
            case 0xe045:
                return KeyboardKey::NumLock;
            case 0x0047:
                return KeyboardKey::NumPad_7;
            case 0x004b:
                return KeyboardKey::NumPad_4;
            case 0x004f:
                return KeyboardKey::NumPad_1;
            case 0xe035:
                return KeyboardKey::NumPad_Divide;
            case 0x0048:
                return KeyboardKey::NumPad_8;
            case 0x004c:
                return KeyboardKey::NumPad_5;
            case 0x0050:
                return KeyboardKey::NumPad_2;
            case 0x0052:
                return KeyboardKey::NumPad_0;
            case 0x0037:
                return KeyboardKey::NumPad_Multiply;
            case 0x0049:
                return KeyboardKey::NumPad_9;
            case 0x004d:
                return KeyboardKey::NumPad_6;
            case 0x0051:
                return KeyboardKey::NumPad_3;
            case 0x0053:
                return KeyboardKey::NumPad_Dot;
            case 0x004a:
                return KeyboardKey::NumPad_Subtract;
            case 0x004e:
                return KeyboardKey::NumPad_Add;
            case 0x007e:
                return KeyboardKey::NumPad_Comma;
            case 0xe01c:
                return KeyboardKey::NumPad_Enter;
            case 0x0001:
                return KeyboardKey::Escape;
            case 0x003b:
                return KeyboardKey::F1;
            case 0x003c:
                return KeyboardKey::F2;
            case 0x003d:
                return KeyboardKey::F3;
            case 0x003e:
                return KeyboardKey::F4;
            case 0x003f:
                return KeyboardKey::F5;
            case 0x0040:
                return KeyboardKey::F6;
            case 0x0041:
                return KeyboardKey::F7;
            case 0x0042:
                return KeyboardKey::F8;
            case 0x0043:
                return KeyboardKey::F9;
            case 0x0044:
                return KeyboardKey::F10;
            case 0x0057:
                return KeyboardKey::F11;
            case 0x0058:
                return KeyboardKey::F12;
            case 0x0054:
            case 0xe037:
                return KeyboardKey::PrintScreen;
            case 0x0046:
                return KeyboardKey::ScrollLock;
            case 0xe046:
                return KeyboardKey::Pause;
            case 0xe05b:
                return KeyboardKey::LeftGui;
            case 0xe05c:
                return KeyboardKey::RightGui;
            case 0x007b:
                return KeyboardKey::Muhenkan;
            case 0x0079:
                return KeyboardKey::Henkan;
            case 0x0070:
                return KeyboardKey::KatakanaHiragana;
            case 0x0056:
            default:
                break;
        };
        return KeyboardKey::Invalid;
    }

    constexpr KeyboardModifier ConvertKeyboardKeyToKeyboardModifier(KeyboardKey key_location) {
        switch (key_location) {
            case KeyboardKey::LeftControl:
            case KeyboardKey::RightControl:
                return KeyboardModifier::Control;
            case KeyboardKey::LeftShift:
            case KeyboardKey::RightShift:
                return KeyboardModifier::Shift;
            case KeyboardKey::LeftGui:
            case KeyboardKey::RightGui:
                return KeyboardModifier::Gui;
            case KeyboardKey::CapsLock:
                return KeyboardModifier::CapsLock;
            case KeyboardKey::ScrollLock:
                return KeyboardModifier::ScrollLock;
            case KeyboardKey::NumLock:
                return KeyboardModifier::NumLock;
            case KeyboardKey::Katakana:
                return KeyboardModifier::Katakana;
            case KeyboardKey::Hiragana:
                return KeyboardModifier::Hiragana;
            default:
                break;
        };
        return KeyboardModifier::Invalid;
    }

    constexpr bool IsToggleModifier(KeyboardModifier key_modifer_location) {
        switch (key_modifer_location) {
            case KeyboardModifier::NumLock:
            case KeyboardModifier::CapsLock:
            case KeyboardModifier::ScrollLock:
                return true;
            default:
                break;
        };
        return false;
    }

#ifdef VP_TARGET_PLATFORM_linux
    constexpr KeyboardKey ConvertInputEventCodeToKeyboardKey(u16 code) {
        switch (code) {
            case KEY_ESC:
                return KeyboardKey::Escape;
            case KEY_0:
                return KeyboardKey::D0;
            case KEY_1:
                return KeyboardKey::D1;
            case KEY_2:
                return KeyboardKey::D2;
            case KEY_3:
                return KeyboardKey::D3;
            case KEY_4:
                return KeyboardKey::D4;
            case KEY_5:
                return KeyboardKey::D5;
            case KEY_6:
                return KeyboardKey::D6;
            case KEY_7:
                return KeyboardKey::D7;
            case KEY_8:
                return KeyboardKey::D8;
            case KEY_9:
                return KeyboardKey::D9;
            case KEY_MINUS:
                return KeyboardKey::Dash;
            case KEY_EQUAL:
                return KeyboardKey::Equals;
            case KEY_BACKSPACE:
                return KeyboardKey::Backspace;
            case KEY_TAB:
                return KeyboardKey::Tab;
            case KEY_Q:
                return KeyboardKey::Q;
            case KEY_W:
                return KeyboardKey::W;
            case KEY_E:
                return KeyboardKey::E;
            case KEY_R:
                return KeyboardKey::R;
            case KEY_T:
                return KeyboardKey::T;
            case KEY_Y:
                return KeyboardKey::Y;
            case KEY_U:
                return KeyboardKey::U;
            case KEY_I:
                return KeyboardKey::I;
            case KEY_O:
                return KeyboardKey::O;
            case KEY_P:
                return KeyboardKey::P;
            case KEY_LEFTBRACE:
                return KeyboardKey::OpenBracket;
            case KEY_RIGHTBRACE:
                return KeyboardKey::CloseBracket;
            case KEY_ENTER:
                return KeyboardKey::Return;
            case KEY_LEFTCTRL:
                return KeyboardKey::LeftControl;
            case KEY_A:
                return KeyboardKey::A;
            case KEY_S:
                return KeyboardKey::S;
            case KEY_D:
                return KeyboardKey::D;
            case KEY_F:
                return KeyboardKey::F;
            case KEY_G:
                return KeyboardKey::G;
            case KEY_H:
                return KeyboardKey::H;
            case KEY_J:
                return KeyboardKey::J;
            case KEY_K:
                return KeyboardKey::K;
            case KEY_L:
                return KeyboardKey::L;
            case KEY_SEMICOLON:
                return KeyboardKey::Semicolon;
            case KEY_APOSTROPHE:
                return KeyboardKey::Quote;
            case KEY_GRAVE:
                return KeyboardKey::Tilde;
            case KEY_LEFTSHIFT:
                return KeyboardKey::LeftShift;
            case KEY_BACKSLASH:
                return KeyboardKey::Backslash;
            case KEY_Z:
                return KeyboardKey::Z;
            case KEY_X:
                return KeyboardKey::X;
            case KEY_C:
                return KeyboardKey::C;
            case KEY_V:
                return KeyboardKey::V;
            case KEY_B:
                return KeyboardKey::B;
            case KEY_N:
                return KeyboardKey::N;
            case KEY_M:
                return KeyboardKey::M;
            case KEY_COMMA:
                return KeyboardKey::Comma;
            case KEY_DOT:
                return KeyboardKey::Period;
            case KEY_SLASH:
                return KeyboardKey::Slash;
            case KEY_RIGHTSHIFT:
                return KeyboardKey::RightShift;
            case KEY_KPASTERISK:
                return KeyboardKey::NumPad_Multiply;
            case KEY_LEFTALT:
                return KeyboardKey::LeftAlt;
            case KEY_SPACE:
                return KeyboardKey::Space;
            case KEY_CAPSLOCK:
                return KeyboardKey::CapsLock;
            case KEY_F1:
                return KeyboardKey::F1;
            case KEY_F2:
                return KeyboardKey::F2;
            case KEY_F3:
                return KeyboardKey::F3;
            case KEY_F4:
                return KeyboardKey::F4;
            case KEY_F5:
                return KeyboardKey::F5;
            case KEY_F6:
                return KeyboardKey::F6;
            case KEY_F7:
                return KeyboardKey::F7;
            case KEY_F8:
                return KeyboardKey::F8;
            case KEY_F9:
                return KeyboardKey::F9;
            case KEY_F10:
                return KeyboardKey::F10;
            case KEY_NUMLOCK:
                return KeyboardKey::NumLock;
            case KEY_SCROLLLOCK:
                return KeyboardKey::ScrollLock;
            case KEY_KP7:
                return KeyboardKey::NumPad_7;
            case KEY_KP8:
                return KeyboardKey::NumPad_8;
            case KEY_KP9:
                return KeyboardKey::NumPad_9;
            case KEY_KPMINUS:
                return KeyboardKey::NumPad_Subtract;
            case KEY_KP4:
                return KeyboardKey::NumPad_4;
            case KEY_KP5:
                return KeyboardKey::NumPad_5;
            case KEY_KP6:
                return KeyboardKey::NumPad_6;
            case KEY_KPPLUS:
                return KeyboardKey::NumPad_Add;
            case KEY_KP1:
                return KeyboardKey::NumPad_1;
            case KEY_KP2:
                return KeyboardKey::NumPad_2;
            case KEY_KP3:
                return KeyboardKey::NumPad_3;
            case KEY_KP0:
                return KeyboardKey::NumPad_0;
            case KEY_KPDOT:
                return KeyboardKey::NumPad_Dot;
            case KEY_ZENKAKUHANKAKU:
                return KeyboardKey::ZenkakuHankaku;
            case KEY_F11:
                return KeyboardKey::F11;
            case KEY_F12:
                return KeyboardKey::F12;
            case KEY_RO:
                return KeyboardKey::Ro;
            case KEY_KATAKANA:
                return KeyboardKey::Katakana;
            case KEY_HIRAGANA:
                return KeyboardKey::Hiragana;
            case KEY_HENKAN:
                return KeyboardKey::Henkan;
            case KEY_KATAKANAHIRAGANA:
                return KeyboardKey::KatakanaHiragana;
            case KEY_MUHENKAN:
                return KeyboardKey::Muhenkan;
            case KEY_KPJPCOMMA:
                return KeyboardKey::NumPad_CommaPc98;
            case KEY_KPENTER:
                return KeyboardKey::NumPad_Enter;
            case KEY_RIGHTCTRL:
                return KeyboardKey::RightControl;
            case KEY_KPSLASH:
                return KeyboardKey::NumPad_Divide;
            case KEY_SYSRQ:
                return KeyboardKey::PrintScreen;
            case KEY_RIGHTALT:
                return KeyboardKey::RightAlt;
            case KEY_HOME:
                return KeyboardKey::Home;
            case KEY_UP:
                return KeyboardKey::UpArrow;
            case KEY_PAGEUP:
                return KeyboardKey::PageUp;
            case KEY_LEFT:
                return KeyboardKey::LeftArrow;
            case KEY_RIGHT:
                return KeyboardKey::RightArrow;
            case KEY_END:
                return KeyboardKey::End;
            case KEY_DOWN:
                return KeyboardKey::DownArrow;
            case KEY_PAGEDOWN:
                return KeyboardKey::PageDown;
            case KEY_INSERT:
                return KeyboardKey::Insert;
            case KEY_DELETE:
                return KeyboardKey::Delete;
            case KEY_POWER:
                return KeyboardKey::Power;
            case KEY_KPEQUAL:
                return KeyboardKey::NumPad_Equals;
            case KEY_PAUSE:
                return KeyboardKey::Pause;
            case KEY_KPCOMMA:
                return KeyboardKey::NumPad_Comma;
            case KEY_HANGUEL:
                return KeyboardKey::HangulEnglish;
            case KEY_HANJA:
                return KeyboardKey::Hanja;
            case KEY_YEN:
                return KeyboardKey::Yen;
            case KEY_F13:
                return KeyboardKey::F13;
            case KEY_F14:
                return KeyboardKey::F14;
            case KEY_F15:
                return KeyboardKey::F15;
            case KEY_F16:
                return KeyboardKey::F16;
            case KEY_F17:
                return KeyboardKey::F17;
            case KEY_F18:
                return KeyboardKey::F18;
            case KEY_F19:
                return KeyboardKey::F19;
            case KEY_F20:
                return KeyboardKey::F20;
            case KEY_F21:
                return KeyboardKey::F21;
            case KEY_F22:
                return KeyboardKey::F22;
            case KEY_F23:
                return KeyboardKey::F23;
            case KEY_F24:
                return KeyboardKey::F24;
            case KEY_LINEFEED:
            case KEY_MACRO:
            case KEY_MUTE:
            case KEY_VOLUMEUP:
            case KEY_VOLUMEDOWN:
            case KEY_KPPLUSMINUS:
            case KEY_SCALE:
            default:
                break;
        }
        return KeyboardKey::Invalid;
    }
#endif
}
