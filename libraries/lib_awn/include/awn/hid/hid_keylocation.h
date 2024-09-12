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

    /* From: https://switchbrew.org/wiki/HID_services#KeyboardModifierSet */
    enum class KeyboardModifier : u32 {
        Invalid    = 0,
        Control    = (1 << 0),
        Shift      = (1 << 1),
        LeftAlt    = (1 << 2),
        RightAlt   = (1 << 3),
        Gui        = (1 << 4),
        CapsLock   = (1 << 8),
        ScrollLock = (1 << 9),
        NumLock    = (1 << 10),
        Katakana   = (1 << 11),
        Hiragana   = (1 << 12),
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

    constexpr const char *ConvertKeyboardKeyToString(KeyboardKey key_location) {
        switch (key_location) {
            case KeyboardKey::A:
                return "A";
            case KeyboardKey::B:
                return "B";
            case KeyboardKey::C:
                return "C";
            case KeyboardKey::D:
                return "D";
            case KeyboardKey::E:
                return "E";
            case KeyboardKey::F:
                return "F";
            case KeyboardKey::G:
                return "G";
            case KeyboardKey::H:
                return "H";
            case KeyboardKey::I:
                return "I";
            case KeyboardKey::J:
                return "J";
            case KeyboardKey::K:
                return "K";
            case KeyboardKey::L:
                return "L";
            case KeyboardKey::M:
                return "M";
            case KeyboardKey::N:
                return "N";
            case KeyboardKey::O:
                return "O";
            case KeyboardKey::P:
                return "P";
            case KeyboardKey::Q:
                return "Q";
            case KeyboardKey::R:
                return "R";
            case KeyboardKey::S:
                return "S";
            case KeyboardKey::T:
                return "T";
            case KeyboardKey::U:
                return "U";
            case KeyboardKey::V:
                return "V";
            case KeyboardKey::W:
                return "W";
            case KeyboardKey::X:
                return "X";
            case KeyboardKey::Y:
                return "Y";
            case KeyboardKey::Z:
                return "Z";
            case KeyboardKey::D1:
                return "D1";
            case KeyboardKey::D2:
                return "D2";
            case KeyboardKey::D3:
                return "D3";
            case KeyboardKey::D4:
                return "D4";
            case KeyboardKey::D5:
                return "D5";
            case KeyboardKey::D6:
                return "D6";
            case KeyboardKey::D7:
                return "D7";
            case KeyboardKey::D8:
                return "D8";
            case KeyboardKey::D9:
                return "D9";
            case KeyboardKey::D0:
                return "D0";
            case KeyboardKey::Return:
                return "Return";
            case KeyboardKey::Escape:
                return "Escape";
            case KeyboardKey::Backspace:
                return "Backspace";
            case KeyboardKey::Tab:
                return "Tab";
            case KeyboardKey::Space:
                return "Space";
            case KeyboardKey::Dash:
                return "Dash";
            case KeyboardKey::Equals:
                return "Equals";
            case KeyboardKey::OpenBracket:
                return "OpenBracket";
            case KeyboardKey::CloseBracket:
                return "CloseBracket";
            case KeyboardKey::Pipe:
                return "Pipe";
            case KeyboardKey::Tilde:
                return "Tilde";
            case KeyboardKey::Semicolon:
                return "Semicolon";
            case KeyboardKey::Quote:
                return "Quote";
            case KeyboardKey::Backquote:
                return "Backquote";
            case KeyboardKey::Comma:
                return "Comma";
            case KeyboardKey::Period:
                return "Period";
            case KeyboardKey::Slash:
                return "Slash";
            case KeyboardKey::CapsLock:
                return "CapsLock";
            case KeyboardKey::F1:
                return "F1";
            case KeyboardKey::F2:
                return "F2";
            case KeyboardKey::F3:
                return "F3";
            case KeyboardKey::F4:
                return "F4";
            case KeyboardKey::F5:
                return "F5";
            case KeyboardKey::F6:
                return "F6";
            case KeyboardKey::F7:
                return "F7";
            case KeyboardKey::F8:
                return "F8";
            case KeyboardKey::F9:
                return "F9";
            case KeyboardKey::F10:
                return "F10";
            case KeyboardKey::F11:
                return "F11";
            case KeyboardKey::F12:
                return "F12";
            case KeyboardKey::PrintScreen:
                return "PrintScreen";
            case KeyboardKey::ScrollLock:
                return "ScrollLock";
            case KeyboardKey::Pause:
                return "Pause";
            case KeyboardKey::Insert:
                return "Insert";
            case KeyboardKey::Home:
                return "Home";
            case KeyboardKey::PageUp:
                return "PageUp";
            case KeyboardKey::Delete:
                return "Delete";
            case KeyboardKey::End:
                return "End";
            case KeyboardKey::PageDown:
                return "PageDown";
            case KeyboardKey::RightArrow:
                return "RightArrow";
            case KeyboardKey::LeftArrow:
                return "LeftArrow";
            case KeyboardKey::DownArrow:
                return "DownArrow";
            case KeyboardKey::UpArrow:
                return "UpArrow";
            case KeyboardKey::NumLock:
                return "NumLock";
            case KeyboardKey::NumPad_Divide:
                return "NumPad_Divide";
            case KeyboardKey::NumPad_Multiply:
                return "NumPad_Multiply";
            case KeyboardKey::NumPad_Subtract:
                return "NumPad_Subtract";
            case KeyboardKey::NumPad_Add:
                return "NumPad_Add";
            case KeyboardKey::NumPad_Enter:
                return "NumPad_Enter";
            case KeyboardKey::NumPad_1:
                return "NumPad_1";
            case KeyboardKey::NumPad_2:
                return "NumPad_2";
            case KeyboardKey::NumPad_3:
                return "NumPad_3";
            case KeyboardKey::NumPad_4:
                return "NumPad_4";
            case KeyboardKey::NumPad_5:
                return "NumPad_5";
            case KeyboardKey::NumPad_6:
                return "NumPad_6";
            case KeyboardKey::NumPad_7:
                return "NumPad_7";
            case KeyboardKey::NumPad_8:
                return "NumPad_8";
            case KeyboardKey::NumPad_9:
                return "NumPad_9";
            case KeyboardKey::NumPad_0:
                return "NumPad_0";
            case KeyboardKey::NumPad_Dot:
                return "NumPad_Dot";
            case KeyboardKey::Backslash:
                return "Backslash";
            case KeyboardKey::Application:
                return "Application";
            case KeyboardKey::Power:
                return "Power";
            case KeyboardKey::NumPad_Equals:
                return "NumPad_Equals";
            case KeyboardKey::F13:
                return "F13";
            case KeyboardKey::F14:
                return "F14";
            case KeyboardKey::F15:
                return "F15";
            case KeyboardKey::F16:
                return "F16";
            case KeyboardKey::F17:
                return "F17";
            case KeyboardKey::F18:
                return "F18";
            case KeyboardKey::F19:
                return "F19";
            case KeyboardKey::F20:
                return "F20";
            case KeyboardKey::F21:
                return "F21";
            case KeyboardKey::F22:
                return "F22";
            case KeyboardKey::F23:
                return "F23";
            case KeyboardKey::F24:
                return "F24";
            case KeyboardKey::NumPad_Comma:
                return "NumPad_Comma";
            case KeyboardKey::Ro:
                return "Ro";
            case KeyboardKey::KatakanaHiragana:
                return "KatakanaHiragana";
            case KeyboardKey::Yen:
                return "Yen";
            case KeyboardKey::Henkan:
                return "Henkan";
            case KeyboardKey::Muhenkan:
                return "Muhenkan";
            case KeyboardKey::NumPad_CommaPc98:
                return "NumPad_CommaPc98";
            case KeyboardKey::HangulEnglish:
                return "HangulEnglish";
            case KeyboardKey::Hanja:
                return "Hanja";
            case KeyboardKey::Katakana:
                return "Katakana";
            case KeyboardKey::Hiragana:
                return "Hiragana";
            case KeyboardKey::ZenkakuHankaku:
                return "ZenkakuHankaku";
            case KeyboardKey::LeftControl:
                return "LeftControl";
            case KeyboardKey::LeftShift:
                return "LeftShift";
            case KeyboardKey::LeftAlt:
                return "LeftAlt";
            case KeyboardKey::LeftGui:
                return "LeftGui";
            case KeyboardKey::RightControl:
                return "RightControl";
            case KeyboardKey::RightShift:
                return "RightShift";
            case KeyboardKey::RightAlt:
                return "RightAlt";
            case KeyboardKey::RightGui:
                return "RightGui";
            default:
                break;
        };
        return "Invalid";
    }
}
