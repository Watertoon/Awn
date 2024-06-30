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
    enum class KeyModifierLocation : u32 {
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
    enum class KeyLocation : u32 {
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

    constexpr KeyLocation ConvertScanCodeToKeyLocation(u32 scan_code1) {
        switch (scan_code1) {
            case 0x0029:
                return KeyLocation::Tilde;
            case 0x0002:
                return KeyLocation::D1;
            case 0x0003:
                return KeyLocation::D2;
            case 0x0004:
                return KeyLocation::D3;
            case 0x0005:
                return KeyLocation::D4;
            case 0x0006:
                return KeyLocation::D5;
            case 0x0007:
                return KeyLocation::D6;
            case 0x0008:
                return KeyLocation::D7;
            case 0x0009:
                return KeyLocation::D8;
            case 0x000a:
                return KeyLocation::D9;
            case 0x000b:
                return KeyLocation::D0;
            case 0x000c:
                return KeyLocation::Dash;
            case 0x000d:
                return KeyLocation::Equals;
            case 0x007d:
                return KeyLocation::Yen;
            case 0x000e:
                return KeyLocation::Backspace;
            case 0x000f:
                return KeyLocation::Tab;
            case 0x0010:
                return KeyLocation::Q;
            case 0x0011:
                return KeyLocation::W;
            case 0x0012:
                return KeyLocation::E;
            case 0x0013:
                return KeyLocation::R;
            case 0x0014:
                return KeyLocation::T;
            case 0x0015:
                return KeyLocation::Y;
            case 0x0016:
                return KeyLocation::U;
            case 0x0017:
                return KeyLocation::I;
            case 0x0018:
                return KeyLocation::O;
            case 0x0019:
                return KeyLocation::P;
            case 0x001a:
                return KeyLocation::OpenBracket;
            case 0x001b:
                return KeyLocation::CloseBracket;
            case 0x002b:
                return KeyLocation::Pipe;
            case 0x003a:
                return KeyLocation::CapsLock;
            case 0x001e:
                return KeyLocation::A;
            case 0x001f:
                return KeyLocation::S;
            case 0x0020:
                return KeyLocation::D;
            case 0x0021:
                return KeyLocation::F;
            case 0x0022:
                return KeyLocation::G;
            case 0x0023:
                return KeyLocation::H;
            case 0x0024:
                return KeyLocation::J;
            case 0x0025:
                return KeyLocation::K;
            case 0x0026:
                return KeyLocation::L;
            case 0x0027:
                return KeyLocation::Semicolon;
            case 0x0028:
                return KeyLocation::Quote;
            case 0x001c:
                return KeyLocation::Return;
            case 0x002a:
                return KeyLocation::LeftShift;
            case 0x002c:
                return KeyLocation::Z;
            case 0x002d:
                return KeyLocation::X;
            case 0x002e:
                return KeyLocation::C;
            case 0x002f:
                return KeyLocation::V;
            case 0x0030:
                return KeyLocation::B;
            case 0x0031:
                return KeyLocation::N;
            case 0x0032:
                return KeyLocation::M;
            case 0x0033:
                return KeyLocation::Comma;
            case 0x0034:
                return KeyLocation::Period;
            case 0x0035:
                return KeyLocation::Slash;
            case 0x0073:
                return KeyLocation::Backslash;
            case 0x0036:
                return KeyLocation::RightShift;
            case 0x001d:
                return KeyLocation::LeftControl;
            case 0x0038:
                return KeyLocation::LeftAlt;
            case 0x0039:
                return KeyLocation::Space;
            case 0xe038:
                return KeyLocation::RightAlt;
            case 0xe01d:
                return KeyLocation::RightControl;
            case 0xe052:
                return KeyLocation::Insert;
            case 0xe053:
                return KeyLocation::Delete;
            case 0xe04b:
                return KeyLocation::LeftArrow;
            case 0xe047:
                return KeyLocation::Home;
            case 0xe04F:
                return KeyLocation::End;
            case 0xe048:
                return KeyLocation::UpArrow;
            case 0xe050:
                return KeyLocation::DownArrow;
            case 0xe049:
                return KeyLocation::PageUp;
            case 0xe051:
                return KeyLocation::PageDown;
            case 0xe04d:
                return KeyLocation::RightArrow;
            case 0x0045:
            case 0xe045:
                return KeyLocation::NumLock;
            case 0x0047:
                return KeyLocation::NumPad_7;
            case 0x004b:
                return KeyLocation::NumPad_4;
            case 0x004f:
                return KeyLocation::NumPad_1;
            case 0xe035:
                return KeyLocation::NumPad_Divide;
            case 0x0048:
                return KeyLocation::NumPad_8;
            case 0x004c:
                return KeyLocation::NumPad_5;
            case 0x0050:
                return KeyLocation::NumPad_2;
            case 0x0052:
                return KeyLocation::NumPad_0;
            case 0x0037:
                return KeyLocation::NumPad_Multiply;
            case 0x0049:
                return KeyLocation::NumPad_9;
            case 0x004d:
                return KeyLocation::NumPad_6;
            case 0x0051:
                return KeyLocation::NumPad_3;
            case 0x0053:
                return KeyLocation::NumPad_Dot;
            case 0x004a:
                return KeyLocation::NumPad_Subtract;
            case 0x004e:
                return KeyLocation::NumPad_Add;
            case 0x007e:
                return KeyLocation::NumPad_Comma;
            case 0xe01c:
                return KeyLocation::NumPad_Enter;
            case 0x0001:
                return KeyLocation::Escape;
            case 0x003b:
                return KeyLocation::F1;
            case 0x003c:
                return KeyLocation::F2;
            case 0x003d:
                return KeyLocation::F3;
            case 0x003e:
                return KeyLocation::F4;
            case 0x003f:
                return KeyLocation::F5;
            case 0x0040:
                return KeyLocation::F6;
            case 0x0041:
                return KeyLocation::F7;
            case 0x0042:
                return KeyLocation::F8;
            case 0x0043:
                return KeyLocation::F9;
            case 0x0044:
                return KeyLocation::F10;
            case 0x0057:
                return KeyLocation::F11;
            case 0x0058:
                return KeyLocation::F12;
            case 0x0054:
            case 0xe037:
                return KeyLocation::PrintScreen;
            case 0x0046:
                return KeyLocation::ScrollLock;
            case 0xe046:
                return KeyLocation::Pause;
            case 0xe05b:
                return KeyLocation::LeftGui;
            case 0xe05c:
                return KeyLocation::RightGui;
            case 0x007b:
                return KeyLocation::Muhenkan;
            case 0x0079:
                return KeyLocation::Henkan;
            case 0x0070:
                return KeyLocation::KatakanaHiragana;
            case 0x0056:
            default:
                break;
        };
        return KeyLocation::Invalid;
    }

    constexpr KeyModifierLocation ConvertKeyLocationToKeyModifierLocation(KeyLocation key_location) {
        switch (key_location) {
            case KeyLocation::LeftControl:
            case KeyLocation::RightControl:
                return KeyModifierLocation::Control;
            case KeyLocation::LeftShift:
            case KeyLocation::RightShift:
                return KeyModifierLocation::Shift;
            case KeyLocation::LeftGui:
            case KeyLocation::RightGui:
                return KeyModifierLocation::Gui;
            case KeyLocation::CapsLock:
                return KeyModifierLocation::CapsLock;
            case KeyLocation::ScrollLock:
                return KeyModifierLocation::ScrollLock;
            case KeyLocation::NumLock:
                return KeyModifierLocation::NumLock;
            case KeyLocation::Katakana:
                return KeyModifierLocation::Katakana;
            case KeyLocation::Hiragana:
                return KeyModifierLocation::Hiragana;
            default:
                break;
        };
        return KeyModifierLocation::Invalid;
    }

    constexpr bool IsToggleModifier(KeyModifierLocation key_modifer_location) {
        switch (key_modifer_location) {
            case KeyModifierLocation::NumLock:
            case KeyModifierLocation::CapsLock:
            case KeyModifierLocation::ScrollLock:
                return true;
            default:
                break;
        };
        return false;
    }

    constexpr const char *ConvertKeyLocationToString(KeyLocation key_location) {
        switch (key_location) {
            case KeyLocation::A:
                return "A";
            case KeyLocation::B:
                return "B";
            case KeyLocation::C:
                return "C";
            case KeyLocation::D:
                return "D";
            case KeyLocation::E:
                return "E";
            case KeyLocation::F:
                return "F";
            case KeyLocation::G:
                return "G";
            case KeyLocation::H:
                return "H";
            case KeyLocation::I:
                return "I";
            case KeyLocation::J:
                return "J";
            case KeyLocation::K:
                return "K";
            case KeyLocation::L:
                return "L";
            case KeyLocation::M:
                return "M";
            case KeyLocation::N:
                return "N";
            case KeyLocation::O:
                return "O";
            case KeyLocation::P:
                return "P";
            case KeyLocation::Q:
                return "Q";
            case KeyLocation::R:
                return "R";
            case KeyLocation::S:
                return "S";
            case KeyLocation::T:
                return "T";
            case KeyLocation::U:
                return "U";
            case KeyLocation::V:
                return "V";
            case KeyLocation::W:
                return "W";
            case KeyLocation::X:
                return "X";
            case KeyLocation::Y:
                return "Y";
            case KeyLocation::Z:
                return "Z";
            case KeyLocation::D1:
                return "D1";
            case KeyLocation::D2:
                return "D2";
            case KeyLocation::D3:
                return "D3";
            case KeyLocation::D4:
                return "D4";
            case KeyLocation::D5:
                return "D5";
            case KeyLocation::D6:
                return "D6";
            case KeyLocation::D7:
                return "D7";
            case KeyLocation::D8:
                return "D8";
            case KeyLocation::D9:
                return "D9";
            case KeyLocation::D0:
                return "D0";
            case KeyLocation::Return:
                return "Return";
            case KeyLocation::Escape:
                return "Escape";
            case KeyLocation::Backspace:
                return "Backspace";
            case KeyLocation::Tab:
                return "Tab";
            case KeyLocation::Space:
                return "Space";
            case KeyLocation::Dash:
                return "Dash";
            case KeyLocation::Equals:
                return "Equals";
            case KeyLocation::OpenBracket:
                return "OpenBracket";
            case KeyLocation::CloseBracket:
                return "CloseBracket";
            case KeyLocation::Pipe:
                return "Pipe";
            case KeyLocation::Tilde:
                return "Tilde";
            case KeyLocation::Semicolon:
                return "Semicolon";
            case KeyLocation::Quote:
                return "Quote";
            case KeyLocation::Backquote:
                return "Backquote";
            case KeyLocation::Comma:
                return "Comma";
            case KeyLocation::Period:
                return "Period";
            case KeyLocation::Slash:
                return "Slash";
            case KeyLocation::CapsLock:
                return "CapsLock";
            case KeyLocation::F1:
                return "F1";
            case KeyLocation::F2:
                return "F2";
            case KeyLocation::F3:
                return "F3";
            case KeyLocation::F4:
                return "F4";
            case KeyLocation::F5:
                return "F5";
            case KeyLocation::F6:
                return "F6";
            case KeyLocation::F7:
                return "F7";
            case KeyLocation::F8:
                return "F8";
            case KeyLocation::F9:
                return "F9";
            case KeyLocation::F10:
                return "F10";
            case KeyLocation::F11:
                return "F11";
            case KeyLocation::F12:
                return "F12";
            case KeyLocation::PrintScreen:
                return "PrintScreen";
            case KeyLocation::ScrollLock:
                return "ScrollLock";
            case KeyLocation::Pause:
                return "Pause";
            case KeyLocation::Insert:
                return "Insert";
            case KeyLocation::Home:
                return "Home";
            case KeyLocation::PageUp:
                return "PageUp";
            case KeyLocation::Delete:
                return "Delete";
            case KeyLocation::End:
                return "End";
            case KeyLocation::PageDown:
                return "PageDown";
            case KeyLocation::RightArrow:
                return "RightArrow";
            case KeyLocation::LeftArrow:
                return "LeftArrow";
            case KeyLocation::DownArrow:
                return "DownArrow";
            case KeyLocation::UpArrow:
                return "UpArrow";
            case KeyLocation::NumLock:
                return "NumLock";
            case KeyLocation::NumPad_Divide:
                return "NumPad_Divide";
            case KeyLocation::NumPad_Multiply:
                return "NumPad_Multiply";
            case KeyLocation::NumPad_Subtract:
                return "NumPad_Subtract";
            case KeyLocation::NumPad_Add:
                return "NumPad_Add";
            case KeyLocation::NumPad_Enter:
                return "NumPad_Enter";
            case KeyLocation::NumPad_1:
                return "NumPad_1";
            case KeyLocation::NumPad_2:
                return "NumPad_2";
            case KeyLocation::NumPad_3:
                return "NumPad_3";
            case KeyLocation::NumPad_4:
                return "NumPad_4";
            case KeyLocation::NumPad_5:
                return "NumPad_5";
            case KeyLocation::NumPad_6:
                return "NumPad_6";
            case KeyLocation::NumPad_7:
                return "NumPad_7";
            case KeyLocation::NumPad_8:
                return "NumPad_8";
            case KeyLocation::NumPad_9:
                return "NumPad_9";
            case KeyLocation::NumPad_0:
                return "NumPad_0";
            case KeyLocation::NumPad_Dot:
                return "NumPad_Dot";
            case KeyLocation::Backslash:
                return "Backslash";
            case KeyLocation::Application:
                return "Application";
            case KeyLocation::Power:
                return "Power";
            case KeyLocation::NumPad_Equals:
                return "NumPad_Equals";
            case KeyLocation::F13:
                return "F13";
            case KeyLocation::F14:
                return "F14";
            case KeyLocation::F15:
                return "F15";
            case KeyLocation::F16:
                return "F16";
            case KeyLocation::F17:
                return "F17";
            case KeyLocation::F18:
                return "F18";
            case KeyLocation::F19:
                return "F19";
            case KeyLocation::F20:
                return "F20";
            case KeyLocation::F21:
                return "F21";
            case KeyLocation::F22:
                return "F22";
            case KeyLocation::F23:
                return "F23";
            case KeyLocation::F24:
                return "F24";
            case KeyLocation::NumPad_Comma:
                return "NumPad_Comma";
            case KeyLocation::Ro:
                return "Ro";
            case KeyLocation::KatakanaHiragana:
                return "KatakanaHiragana";
            case KeyLocation::Yen:
                return "Yen";
            case KeyLocation::Henkan:
                return "Henkan";
            case KeyLocation::Muhenkan:
                return "Muhenkan";
            case KeyLocation::NumPad_CommaPc98:
                return "NumPad_CommaPc98";
            case KeyLocation::HangulEnglish:
                return "HangulEnglish";
            case KeyLocation::Hanja:
                return "Hanja";
            case KeyLocation::Katakana:
                return "Katakana";
            case KeyLocation::Hiragana:
                return "Hiragana";
            case KeyLocation::ZenkakuHankaku:
                return "ZenkakuHankaku";
            case KeyLocation::LeftControl:
                return "LeftControl";
            case KeyLocation::LeftShift:
                return "LeftShift";
            case KeyLocation::LeftAlt:
                return "LeftAlt";
            case KeyLocation::LeftGui:
                return "LeftGui";
            case KeyLocation::RightControl:
                return "RightControl";
            case KeyLocation::RightShift:
                return "RightShift";
            case KeyLocation::RightAlt:
                return "RightAlt";
            case KeyLocation::RightGui:
                return "RightGui";
            default:
                break;
        };
        return "Invalid";
    }
}
