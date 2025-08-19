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

    class MouseManager;

    class Mouse {
        public:
            friend class MouseManager;
        private:
            static constexpr u32 cLifoCount = cMaxMouseStateCount+1;
        private:
            using StateRing = vp::util::FixedAtomicLifo<MouseState, &MouseState::sampling_count, cLifoCount>;
        private:
            HANDLE m_device;
            MouseState m_current_state;
            StateRing m_ring;
            bool m_is_ignore_no_focus;
            bool m_is_dirty;
        public:
            constexpr Mouse() : m_device(), m_current_state(), m_ring(), m_is_ignore_no_focus(), m_is_dirty() {/*...*/}
            constexpr ~Mouse() {/*...*/}

            void RegisterDevice(HANDLE device);
            void Disconnect();

            void ProcessInput(RAWMOUSE *input, bool is_unfocus, bool is_all_mouse);

            void ReportState();
    };

    class MouseManager {
        private:
            static constexpr const char *cClassName = "awn::hid::MouseRawInput";
            static constexpr const u32 cReportRateHz = 200;
            static constexpr const u32 cMaxMessageCount = 32;
        public:
            enum Message : size_t {
                Message_Disconnect = 0x1'0000'0000,
                Message_ExitCode = 0x7fff'ffff'ffff'ffff,
            };
            static_assert(sizeof(size_t) == sizeof(u64));
        private:
            using MouseArray = vp::util::HeapArray<Mouse>;
            using UsedMouseArray = vp::util::PointerArray<Mouse>;
        public:
            sys::FunctionThread m_thread;
            sys::ThreadMemberFunction<MouseManager> m_function;
            size_t m_message_memory[cMaxMessageCount];
            HWND m_hwnd;
            MouseArray m_mouse_array;
            sys::Mutex m_used_cs;
            UsedMouseArray m_used_mouse_array;
            Mouse m_all_mouse;
        public:
            AWN_SINGLETON_TRAITS(MouseManager);
        private:
            static long long int MouseWindowProc(HWND window, UINT message, WPARAM wparam, LPARAM lparam);

            void ThreadMain(size_t message);

            void InitializeRawInput();
            void FinalizeRawInput();

            void ReportStates();
        private:
            Mouse *AddMouse(HANDLE device);
            void RemoveMouse(HANDLE device);

            Mouse *GetMouseByDevice(HANDLE device);
        public:
            constexpr MouseManager() : m_thread(), m_function(this, &MouseManager::ThreadMain), m_message_memory(), m_hwnd(), m_mouse_array(), m_used_cs(), m_used_mouse_array(), m_all_mouse() {/*...*/}
            constexpr ~MouseManager() {/*...*/}

            Result Initialize(mem::Heap *heap, u32 max_mouse_count);
            void Finalize();
        public:
            u32 GetMouseStates(MouseState *out_state_array, u32 state_count, u32 mouse_id);
            void SwapMouseAssignment(u32 id0, u32 id1);
            void DisconnectMouse(u32 mouse_id);
    };
}
