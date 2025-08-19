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

namespace awn::hid::impl {

    class KeyboardManager;

    class Keyboard {
        public:
            friend class KeyboardManager;
        private:
            static constexpr u32 cLifoCount = cMaxKeyboardStateCount+1;
        private:
            using StateRing = vp::util::FixedAtomicLifo<KeyboardState, &KeyboardState::sampling_count, cLifoCount>;
        private:
            HANDLE m_device;
            KeyboardState m_current_state;
            StateRing m_ring;
            bool m_is_ignore_no_focus;
            bool m_is_dirty;
        public:
            constexpr Keyboard() : m_device(), m_current_state(), m_ring(), m_is_ignore_no_focus(), m_is_dirty() {/*...*/}
            constexpr ~Keyboard() {/*...*/}

            void RegisterDevice(HANDLE device);
            void Disconnect();

            void ProcessInput(RAWKEYBOARD *input, bool is_unfocus);
            void ReportState();
    };

    class KeyboardManager {
        private:
            static constexpr const char *cClassName = "awn::hid::KeyboardRawInput";
            static constexpr const u32 cReportRateHz = 200;
            static constexpr const u32 cMaxMessageCount = 32;
        public:
            enum Message : size_t {
                Message_Disconnect = 0x1'0000'0000,
                Message_ExitCode = 0x7fff'ffff'ffff'ffff,
            };
            static_assert(sizeof(size_t) == sizeof(u64));
        private:
            using KeyboardArray = vp::util::HeapArray<Keyboard>;
            using UsedKeyboardArray = vp::util::PointerArray<Keyboard>;
        public:
            sys::FunctionThread m_thread;
            sys::ThreadMemberFunction<KeyboardManager> m_function;
            size_t m_message_memory[cMaxMessageCount];
            HWND m_hwnd;
            KeyboardArray m_keyboard_array;
            sys::Mutex m_used_cs;
            UsedKeyboardArray m_used_keyboard_array;
            Keyboard m_all_keyboard;
        public:
            AWN_SINGLETON_TRAITS(KeyboardManager);
        private:
            static long long int KeyboardWindowProc(HWND window, UINT message, WPARAM wparam, LPARAM lparam);

            void ThreadMain(size_t message);

            void InitializeRawInput();
            void FinalizeRawInput();

            void ReportStates();
        private:
            Keyboard *AddKeyboard(HANDLE device);
            void RemoveKeyboard(HANDLE device);

            Keyboard *GetKeyboardByDevice(HANDLE device);
        public:
            constexpr KeyboardManager() : m_thread(), m_function(this, &KeyboardManager::ThreadMain), m_message_memory(), m_hwnd(), m_keyboard_array(), m_used_cs(), m_used_keyboard_array(), m_all_keyboard() {/*...*/}
            constexpr ~KeyboardManager() {/*...*/}

            Result Initialize(mem::Heap *heap, u32 max_keyboard_count);
            void Finalize();
        public:
            u32 GetKeyboardStates(KeyboardState *out_state_array, u32 state_count, u32 keyboard_id);
            void SwapKeyboardAssignment(u32 id0, u32 id1);
            void DisconnectKeyboard(u32 keyboard_id);
    };

}
