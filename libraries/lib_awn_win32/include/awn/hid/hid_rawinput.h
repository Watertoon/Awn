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

    enum VirtualKey : u8 {
        VirtualKey_LButton          = 0x01,
        VirtualKey_RButton          = 0x02,
        VirtualKey_Cancel           = 0x03,
        VirtualKey_MButton          = 0x04,
        VirtualKey_XButton1         = 0x05,
        VirtualKey_XButton2         = 0x06,
        VirtualKey_Backspace        = 0x08,
        VirtualKey_Tab              = 0x09,
        VirtualKey_Clear            = 0x0C,
        VirtualKey_Enter            = 0x0D,
        VirtualKey_Shift            = 0x10,
        VirtualKey_Ctrl             = 0x11,
        VirtualKey_Alt              = 0x12,
        VirtualKey_Pause            = 0x13,
        VirtualKey_CapsLock         = 0x14,
        VirtualKey_KanaMode         = 0x15,
        VirtualKey_HanguelMode      = 0x15,
        VirtualKey_HangulMode       = 0x15,
        VirtualKey_IMEOn            = 0x16,
        VirtualKey_JunjaMode        = 0x17,
        VirtualKey_FinalMode        = 0x18,
        VirtualKey_KanjiMode        = 0x19,
        VirtualKey_IMEOff           = 0x1A,
        VirtualKey_Escape           = 0x1B,
        VirtualKey_Convert          = 0x1C,
        VirtualKey_NonConvert       = 0x1D,
        VirtualKey_Accept           = 0x1E,
        VirtualKey_ModeChange       = 0x1F,
        VirtualKey_Space            = 0x20,
        VirtualKey_PageUp           = 0x21,
        VirtualKey_PageDown         = 0x22,
        VirtualKey_End              = 0x23,
        VirtualKey_Home             = 0x24,
        VirtualKey_Left             = 0x25,
        VirtualKey_Up               = 0x26,
        VirtualKey_Right            = 0x27,
        VirtualKey_Down             = 0x28,
        VirtualKey_Select           = 0x29,
        VirtualKey_Print            = 0x2A,
        VirtualKey_Execute          = 0x2B,
        VirtualKey_PrintScreen      = 0x2C,
        VirtualKey_Insert           = 0x2D,
        VirtualKey_Delete           = 0x2E,
        VirtualKey_Help             = 0x2F,
        VirtualKey_0                = 0x30,
        VirtualKey_1                = 0x31,
        VirtualKey_2                = 0x32,
        VirtualKey_3                = 0x33,
        VirtualKey_4                = 0x34,
        VirtualKey_5                = 0x35,
        VirtualKey_6                = 0x36,
        VirtualKey_7                = 0x37,
        VirtualKey_8                = 0x38,
        VirtualKey_9                = 0x39,
        VirtualKey_A                = 0x41,
        VirtualKey_B                = 0x42,
        VirtualKey_C                = 0x43,
        VirtualKey_D                = 0x44,
        VirtualKey_E                = 0x45,
        VirtualKey_F                = 0x46,
        VirtualKey_G                = 0x47,
        VirtualKey_H                = 0x48,
        VirtualKey_I                = 0x49,
        VirtualKey_J                = 0x4A,
        VirtualKey_K                = 0x4B,
        VirtualKey_L                = 0x4C,
        VirtualKey_M                = 0x4D,
        VirtualKey_N                = 0x4E,
        VirtualKey_O                = 0x4F,
        VirtualKey_P                = 0x50,
        VirtualKey_Q                = 0x51,
        VirtualKey_R                = 0x52,
        VirtualKey_S                = 0x53,
        VirtualKey_T                = 0x54,
        VirtualKey_U                = 0x55,
        VirtualKey_V                = 0x56,
        VirtualKey_W                = 0x57,
        VirtualKey_X                = 0x58,
        VirtualKey_Y                = 0x59,
        VirtualKey_Z                = 0x5A,
        VirtualKey_LeftWin          = 0x5B,
        VirtualKey_RightWin         = 0x5C,
        VirtualKey_Apps             = 0x5D,
        VirtualKey_Sleep            = 0x5F,
        VirtualKey_Numpad0          = 0x60,
        VirtualKey_Numpad1          = 0x61,
        VirtualKey_Numpad2          = 0x62,
        VirtualKey_Numpad3          = 0x63,
        VirtualKey_Numpad4          = 0x64,
        VirtualKey_Numpad5          = 0x65,
        VirtualKey_Numpad6          = 0x66,
        VirtualKey_Numpad7          = 0x67,
        VirtualKey_Numpad8          = 0x68,
        VirtualKey_Numpad9          = 0x69,
        VirtualKey_Multiply         = 0x6A,
        VirtualKey_Add              = 0x6B,
        VirtualKey_Seperator        = 0x6C,
        VirtualKey_Subtract         = 0x6D,
        VirtualKey_Decimal          = 0x6E,
        VirtualKey_Divide           = 0x6F,
        VirtualKey_F1               = 0x70,
        VirtualKey_F2               = 0x71,
        VirtualKey_F3               = 0x72,
        VirtualKey_F4               = 0x73,
        VirtualKey_F5               = 0x74,
        VirtualKey_F6               = 0x75,
        VirtualKey_F7               = 0x76,
        VirtualKey_F8               = 0x77,
        VirtualKey_F9               = 0x78,
        VirtualKey_F10              = 0x79,
        VirtualKey_F11              = 0x7A,
        VirtualKey_F12              = 0x7B,
        VirtualKey_F13              = 0x7C,
        VirtualKey_F14              = 0x7D,
        VirtualKey_F15              = 0x7E,
        VirtualKey_F16              = 0x7F,
        VirtualKey_F17              = 0x80,
        VirtualKey_F18              = 0x81,
        VirtualKey_F19              = 0x82,
        VirtualKey_F20              = 0x83,
        VirtualKey_F21              = 0x84,
        VirtualKey_F22              = 0x85,
        VirtualKey_F23              = 0x86,
        VirtualKey_F24              = 0x87,
        VirtualKey_NumLock          = 0x90,
        VirtualKey_ScrollLock       = 0x91,
        VirtualKey_LeftShift        = 0xA0,
        VirtualKey_RightShift       = 0xA1,
        VirtualKey_LeftControl      = 0xA2,
        VirtualKey_RightControl     = 0xA3,
        VirtualKey_LeftMenu         = 0xA4,
        VirtualKey_RightMenu        = 0xA5,
        VirtualKey_BrowserBack      = 0xA6,
        VirtualKey_BrowserForward   = 0xA7,
        VirtualKey_BrowserRefresh   = 0xA8,
        VirtualKey_BrowserStop      = 0xA9,
        VirtualKey_BrowserSearch    = 0xAA,
        VirtualKey_BrowserFavorites = 0xAB,
        VirtualKey_BrowserHome      = 0xAC,
        VirtualKey_VolumeMute       = 0xAD,
        VirtualKey_VolumeDown       = 0xAE,
        VirtualKey_VolumeUp         = 0xAF,
        VirtualKey_MediaNext        = 0xB0,
        VirtualKey_MediaPrev        = 0xB1,
        VirtualKey_MediaStop        = 0xB2,
        VirtualKey_MediaPlayPause   = 0xB3,
        VirtualKey_LaunchMail       = 0xB4,
        VirtualKey_LaunchMedia      = 0xB5,
        VirtualKey_LaunchApp1       = 0xB6,
        VirtualKey_LaunchApp2       = 0xB7,
        VirtualKey_Oem1             = 0xBA,
        VirtualKey_OemPlus          = 0xBB,
        VirtualKey_OemComma         = 0xBC,
        VirtualKey_OemMinus         = 0xBD,
        VirtualKey_OemPeriod        = 0xBE,
        VirtualKey_Oem2             = 0xBF,
        VirtualKey_Oem3             = 0xC0,
        VirtualKey_Oem4             = 0xDB,
        VirtualKey_Oem5             = 0xDC,
        VirtualKey_Oem6             = 0xDD,
        VirtualKey_Oem7             = 0xDE,
        VirtualKey_Oem8             = 0xDF,
        VirtualKey_Oem102           = 0xE2,
        VirtualKey_ProcessKey       = 0xE5,
        VirtualKey_Packet           = 0xE7,
        VirtualKey_Attn             = 0xF6,
        VirtualKey_CrSel            = 0xF7,
        VirtualKey_ExSel            = 0xF8,
        VirtualKey_EraseEof         = 0xF9,
        VirtualKey_Play             = 0xFA,
        VirtualKey_Zoom             = 0xFB,
        VirtualKey_NoName           = 0xFC,
        VirtualKey_Pa1              = 0xFD,
        VirtualKey_OemClear         = 0xFE
    };

    struct KeyState {
        union {
            u64 state_array[4];
            struct {
                u64 reserve17            : 1; /* 0x00 */
                u64 vk_lbutton           : 1; /* 0x01 */
                u64 vk_rbutton           : 1; /* 0x02 */
                u64 vk_cancel            : 1; /* 0x03 */
                u64 vk_mbutton           : 1; /* 0x04 */
                u64 vk_xbutton1          : 1; /* 0x05 */
                u64 vk_xbutton2          : 1; /* 0x06 */
                u64 reserve0             : 1; /* 0x07 */
                u64 vk_backspace         : 1; /* 0x08 */
                u64 vk_tab               : 1; /* 0x09 */
                u64 reserve1             : 2; /* 0x0A-B */
                u64 vk_clear             : 1; /* 0x0C */
                u64 vk_enter             : 1; /* 0x0D */
                u64 reserve2             : 2; /* 0x0E-F */
                u64 vk_shift             : 1; /* 0x10 */
                u64 vk_ctrl              : 1; /* 0x11 */
                u64 vk_alt               : 1; /* 0x12 */
                u64 vk_pause             : 1; /* 0x13 */
                u64 vk_caps_lock         : 1; /* 0x14 */
                u64 vk_kana_mode         : 1; /* 0x15 */
                u64 vk_ime_enable        : 1; /* 0x16 */
                u64 vk_junja_mode        : 1; /* 0x17 */
                u64 vk_final_mode        : 1; /* 0x18 */
                u64 vk_kanji_mode        : 1; /* 0x19 */
                u64 vk_ime_disable       : 1; /* 0x1A */
                u64 vk_esc               : 1; /* 0x1B */
                u64 vk_convert           : 1; /* 0x1C */
                u64 vk_nonconvert        : 1; /* 0x1D */
                u64 vk_accept            : 1; /* 0x1E */
                u64 vk_mode_change       : 1; /* 0x1F */
                u64 vk_space             : 1; /* 0x20 */
                u64 vk_page_up           : 1; /* 0x21 */
                u64 vk_page_down         : 1; /* 0x22 */
                u64 vk_end               : 1; /* 0x23 */
                u64 vk_home              : 1; /* 0x24 */
                u64 vk_left              : 1; /* 0x25 */
                u64 vk_up                : 1; /* 0x26 */
                u64 vk_right             : 1; /* 0x27 */
                u64 vk_down              : 1; /* 0x28 */
                u64 vk_select            : 1; /* 0x29 */
                u64 vk_print             : 1; /* 0x2A */
                u64 vk_execute           : 1; /* 0x2B */
                u64 vk_print_screen      : 1; /* 0x2C */
                u64 vk_insert            : 1; /* 0x2D */
                u64 vk_delete            : 1; /* 0x2E */
                u64 vk_help              : 1; /* 0x2F */
                u64 vk_0                 : 1; /* 0x30 */
                u64 vk_1                 : 1; /* 0x31 */
                u64 vk_2                 : 1; /* 0x32 */
                u64 vk_3                 : 1; /* 0x33 */
                u64 vk_4                 : 1; /* 0x34 */
                u64 vk_5                 : 1; /* 0x35 */
                u64 vk_6                 : 1; /* 0x36 */
                u64 vk_7                 : 1; /* 0x37 */
                u64 vk_8                 : 1; /* 0x38 */
                u64 vk_9                 : 1; /* 0x39 */
                u64 reserve3             : 6; /* 0x3A-40 */
                u64 vk_a                 : 1; /* 0x41 */
                u64 vk_b                 : 1; /* 0x42 */
                u64 vk_c                 : 1; /* 0x43 */
                u64 vk_d                 : 1; /* 0x44 */
                u64 vk_e                 : 1; /* 0x45 */
                u64 vk_f                 : 1; /* 0x46 */
                u64 vk_g                 : 1; /* 0x47 */
                u64 vk_h                 : 1; /* 0x48 */
                u64 vk_i                 : 1; /* 0x49 */
                u64 vk_j                 : 1; /* 0x4A */
                u64 vk_k                 : 1; /* 0x4B */
                u64 vk_l                 : 1; /* 0x4C */
                u64 vk_m                 : 1; /* 0x4D */
                u64 vk_n                 : 1; /* 0x4E */
                u64 vk_o                 : 1; /* 0x4F */
                u64 vk_p                 : 1; /* 0x50 */
                u64 vk_q                 : 1; /* 0x51 */
                u64 vk_r                 : 1; /* 0x52 */
                u64 vk_s                 : 1; /* 0x53 */
                u64 vk_t                 : 1; /* 0x54 */
                u64 vk_u                 : 1; /* 0x55 */
                u64 vk_v                 : 1; /* 0x56 */
                u64 vk_w                 : 1; /* 0x57 */
                u64 vk_x                 : 1; /* 0x58 */
                u64 vk_y                 : 1; /* 0x59 */
                u64 vk_z                 : 1; /* 0x5A */
                u64 vk_lwin              : 1; /* 0x5B */
                u64 vk_rwin              : 1; /* 0x5C */
                u64 vk_apps              : 1; /* 0x5D */
                u64 reserve4             : 1; /* 0x5E */
                u64 vk_sleep             : 1; /* 0x5F */
                u64 vk_numpad_0          : 1; /* 0x60 */
                u64 vk_numpad_1          : 1; /* 0x61 */
                u64 vk_numpad_2          : 1; /* 0x62 */
                u64 vk_numpad_3          : 1; /* 0x63 */
                u64 vk_numpad_4          : 1; /* 0x64 */
                u64 vk_numpad_5          : 1; /* 0x65 */
                u64 vk_numpad_6          : 1; /* 0x66 */
                u64 vk_numpad_7          : 1; /* 0x67 */
                u64 vk_numpad_8          : 1; /* 0x68 */
                u64 vk_numpad_9          : 1; /* 0x69 */
                u64 vk_multiply          : 1; /* 0x6A */
                u64 vk_add               : 1; /* 0x6B */
                u64 vk_seperator         : 1; /* 0x6C */
                u64 vk_subtract          : 1; /* 0x6D */
                u64 vk_decimal           : 1; /* 0x6E */
                u64 vk_divide            : 1; /* 0x6F */
                u64 vk_f1                : 1; /* 0x70 */
                u64 vk_f2                : 1; /* 0x71 */
                u64 vk_f3                : 1; /* 0x72 */
                u64 vk_f4                : 1; /* 0x73 */
                u64 vk_f5                : 1; /* 0x74 */
                u64 vk_f6                : 1; /* 0x75 */
                u64 vk_f7                : 1; /* 0x76 */
                u64 vk_f8                : 1; /* 0x77 */
                u64 vk_f9                : 1; /* 0x78 */
                u64 vk_f10               : 1; /* 0x79 */
                u64 vk_f11               : 1; /* 0x7A */
                u64 vk_f12               : 1; /* 0x7B */
                u64 vk_f13               : 1; /* 0x7C */
                u64 vk_f14               : 1; /* 0x7D */
                u64 vk_f15               : 1; /* 0x7E */
                u64 vk_f16               : 1; /* 0x7F */
                u64 vk_f17               : 1; /* 0x80 */
                u64 vk_f18               : 1; /* 0x81 */
                u64 vk_f19               : 1; /* 0x82 */
                u64 vk_f20               : 1; /* 0x83 */
                u64 vk_f21               : 1; /* 0x84 */
                u64 vk_f22               : 1; /* 0x85 */
                u64 vk_f23               : 1; /* 0x86 */
                u64 vk_f24               : 1; /* 0x87 */
                u64 reserve5             : 7; /* 0x88-F */
                u64 vk_num_lock          : 1; /* 0x90 */
                u64 vk_scroll_lock       : 1; /* 0x91 */
                u64 vk_oem_specific      : 4; /* 0x92-6 */
                u64 reserve6             : 8; /* 0x97-F */
                u64 vk_lshift            : 1; /* 0xA0 */
                u64 vk_rshift            : 1; /* 0xA1 */
                u64 vk_lcontrol          : 1; /* 0xA2 */
                u64 vk_rcontrol          : 1; /* 0xA3 */
                u64 vk_lmenu             : 1; /* 0xA4 */
                u64 vk_rmenu             : 1; /* 0xA5 */
                u64 vk_browser_back      : 1; /* 0xA6 */
                u64 vk_browser_forward   : 1; /* 0xA7 */
                u64 vk_browser_refresh   : 1; /* 0xA8 */
                u64 vk_browser_stop      : 1; /* 0xA9 */
                u64 vk_browser_search    : 1; /* 0xAA */
                u64 vk_browser_favorites : 1; /* 0xAB */
                u64 vk_browser_home      : 1; /* 0xAC */
                u64 vk_volume_mute       : 1; /* 0xAD */
                u64 vk_volume_down       : 1; /* 0xAE */
                u64 vk_volume_up         : 1; /* 0xAF */
                u64 vk_media_next        : 1; /* 0xB0 */
                u64 vk_media_prev        : 1; /* 0xB1 */
                u64 vk_media_stop        : 1; /* 0xB2 */
                u64 vk_media_play_pause  : 1; /* 0xB3 */
                u64 vk_launch_mail       : 1; /* 0xB4 */
                u64 vk_launch_media      : 1; /* 0xB5 */
                u64 vk_launch_app1       : 1; /* 0xB6 */
                u64 vk_launch_app2       : 1; /* 0xB7 */
                u64 reserve7             : 2; /* 0xB8-9 */
                u64 vk_oem_1             : 1; /* 0xBA */
                u64 vk_oem_plus          : 1; /* 0xBB */
                u64 vk_oem_comma         : 1; /* 0xBC */
                u64 vk_oem_minus         : 1; /* 0xBD */
                u64 vk_oem_period        : 1; /* 0xBE */
                u64 vk_oem_2             : 1; /* 0xBF */
                u64 vk_oem_3             : 1; /* 0xC0 */
                u64 reserve8             : 16; /* 0xC1-D7 */
                u64 reserve9             : 2; /* 0xD8-DA */
                u64 vk_oem_4             : 1; /* 0xDB */
                u64 vk_oem_5             : 1; /* 0xDC */
                u64 vk_oem_6             : 1; /* 0xDD */
                u64 vk_oem_7             : 1; /* 0xDE */
                u64 vk_oem_8             : 1; /* 0xDF */
                u64 reserve10            : 1; /* 0xE0 */
                u64 reserve11            : 1; /* 0xE1 */
                u64 vk_oem_102           : 1; /* 0xE2 */
                u64 reserve12            : 2; /* 0xE3-4 */
                u64 vk_process_key       : 1; /* 0xE5 */
                u64 reserve13            : 1; /* 0xE6 */
                u64 vk_packet            : 1; /* 0xE7 */
                u64 reserve14            : 1; /* 0xE8 */
                u64 reserve15            : 12; /* 0xE9-F5 */
                u64 vk_attn              : 1; /* 0xF6 */
                u64 vk_crsel             : 1; /* 0xF7 */
                u64 vk_exsel             : 1; /* 0xF8 */
                u64 vk_erase_eof         : 1; /* 0xF9 */
                u64 vk_play              : 1; /* 0xFA */
                u64 vk_zoom              : 1; /* 0xFB */
                u64 vk_noname            : 1; /* 0xFC */
                u64 vk_pa1               : 1; /* 0xFD */
                u64 vk_oem_clear         : 1; /* 0xFE */
                u64 reserve16            : 1; /* 0xFF */
            };
        };

        constexpr KeyState operator|(const KeyState& rhs) const {
            KeyState new_state = {*this};
            new_state.state_array[0] |= rhs.state_array[0];
            new_state.state_array[1] |= rhs.state_array[1];
            new_state.state_array[2] |= rhs.state_array[2];
            new_state.state_array[3] |= rhs.state_array[3];
            return new_state;
        }

        constexpr KeyState operator&(const KeyState& rhs) const {
            KeyState new_state = {*this};
            new_state.state_array[0] &= rhs.state_array[0];
            new_state.state_array[1] &= rhs.state_array[1];
            new_state.state_array[2] &= rhs.state_array[2];
            new_state.state_array[3] &= rhs.state_array[3];
            return new_state;
        }

        constexpr KeyState operator~() const {
            KeyState new_state = {};
            new_state.state_array[0] = ~state_array[0];
            new_state.state_array[1] = ~state_array[1];
            new_state.state_array[2] = ~state_array[2];
            new_state.state_array[3] = ~state_array[3];
            return new_state;
        }

        constexpr bool GetKeyState(u8 vk_code) const {
            const u8 index = vk_code >> 6;
            const u64 key_pos = 1 << (vk_code % 64);

            return (state_array[index] & key_pos) == key_pos;
        }

        constexpr void SetKeyState(u8 vk_code) {
            const u8 index = vk_code >> 6;
            const u64 key_pos = 1 << (vk_code % 64);

            state_array[index] |= key_pos;
        }

        constexpr void ClearKeyState(u8 vk_code) {
            const u8 index = vk_code >> 6;
            const u64 key_pos = 1 << (vk_code % 64);

            state_array[index] &= (~key_pos);
        }
    };
    static_assert(sizeof(KeyState) == sizeof(u64) * 4);
    static_assert((~KeyState{(1 << (VirtualKey_XButton1 % 64))}).GetKeyState(VirtualKey_XButton1) == false);
    static_assert((KeyState{(1 << (VirtualKey_XButton1 % 64))} & KeyState{(1 << (VirtualKey_XButton1 % 64))}).GetKeyState(VirtualKey_XButton1) == true);
    static_assert((KeyState{(1 << (VirtualKey_XButton2 % 64))} & KeyState{(1 << (VirtualKey_XButton1 % 64))}).GetKeyState(VirtualKey_XButton1) == false);
    static_assert((KeyState{(1 << (VirtualKey_XButton2 % 64))} | KeyState{(1 << (VirtualKey_XButton1 % 64))}).GetKeyState(VirtualKey_XButton1) == true);
    static_assert((~KeyState{0, (1 << (VirtualKey_W % 64))}).GetKeyState(VirtualKey_W) == false);
    static_assert((KeyState{0, (1 << (VirtualKey_W % 64))} & KeyState{0, (1 << (VirtualKey_W % 64))}).GetKeyState(VirtualKey_W) == true);
    static_assert((KeyState{0, (1 << (VirtualKey_S % 64))} & KeyState{0, (1 << (VirtualKey_W % 64))}).GetKeyState(VirtualKey_W) == false);
    static_assert((KeyState{0, (1 << (VirtualKey_S % 64))} | KeyState{0, (1 << (VirtualKey_W % 64))}).GetKeyState(VirtualKey_W) == true);

    struct KeyboardState {
        KeyState pressed_keys;
        KeyState held_keys;
        KeyState released_keys;

        bool IsKeyPressed(u8 virtual_key_code) const {
            return pressed_keys.GetKeyState(virtual_key_code);
        }

        bool IsKeyHeld(u8 virtual_key_code) const {
            return held_keys.GetKeyState(virtual_key_code);
        }

        bool IsKeyUp(u8 virtual_key_code) const {
            return released_keys.GetKeyState(virtual_key_code);
        }
    };

    enum MouseButton : u16 {
        MouseButton_Left   = 0x01,
        MouseButton_Right  = 0x04,
        MouseButton_Middle = 0x10,
        MouseButton_X1     = 0x40,
        MouseButton_X2     = 0x100
    };
    struct MouseState {
        s32 absolute_x;
        s32 absolute_y;
        s32 delta_x;
        s32 delta_y;
        s32 delta_vwheel;
        s32 delta_hwheel;
        u16 button_state;
        u16 button_held;

        static constexpr u16 ButtonDownMask =  MouseButton_Left | MouseButton_Right | MouseButton_Middle | MouseButton_X1 | MouseButton_X2;
        static constexpr u16 ButtonUpMask   = (MouseButton_Left | MouseButton_Right | MouseButton_Middle | MouseButton_X1 | MouseButton_X2) << 1;

        bool GetButtonDown(u16 mouse_button) const {
            return (button_state & mouse_button) != 0;
        }

        bool GetButtonHeld(u16 mouse_button) const {
            return (button_held & mouse_button) != 0;
        }

        bool GetButtonUp(u16 mouse_button) const {
            return (button_state & (mouse_button << 1)) != 0;
        }
    };

    void SetLastRawInput(HRAWINPUT input_handle);

    void BeginFrame();

    const KeyboardState GetKeyboardState();

    const MouseState    GetMouseState();
}
