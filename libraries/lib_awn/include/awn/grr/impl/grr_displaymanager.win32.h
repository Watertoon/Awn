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

namespace awn::grr::impl {

    constexpr LPCWSTR cNativeWindowClassName = L"awn::grr::NativeWindow";

    class DisplayManager {
        public:
            friend class grr::NativeWindow;
            friend class grr::Display;
        public:
            static constexpr size_t cMessageExitCode = 0x7fff'ffff'ffff'ffff;
            static constexpr u32 cMaxCommandListCount = 32;
        public:
            using NativeWindowList = vp::util::IntrusiveListTraits<NativeWindow, &NativeWindow::m_list_node>::List;
            using DisplayList = vp::util::IntrusiveListTraits<Display, &Display::m_list_node>::List;
        private:
            sys::Event m_present_event;
            sys::ThreadMemberFunction<DisplayManager> m_present_thread_function;
            sys::FunctionThread m_present_thread;
            size_t m_message_memory[8];
            sys::Mutex m_window_cs;
            sys::Mutex m_display_cs;
            NativeWindowList m_native_window_list;
            DisplayList m_display_list;
            u32 m_window_count;
            ID3D11Fence *m_submit_fence;
            gfx::Sync m_submit_sync;
            HANDLE m_submit_event;
            u32 m_command_list_count;
            gfx::CommandList m_command_list_array[cMaxCommandListCount];
            bool m_is_present;
        public:
            AWN_SINGLETON_TRAITS(DisplayManager);
        private:
            void Present();
            void PresentThreadMain(size_t message);
        private:
            void AddNativeWindow(NativeWindow *native_window);
            void AddDisplay(Display *display);
            void RemoveNativeWindow(NativeWindow *native_window);
            void RemoveDisplay(Display *display);
        public:
            constexpr DisplayManager() : m_present_event(sys::SignalState::Signaled, sys::ResetMode::Manual), m_present_thread_function(this, &DisplayManager::PresentThreadMain), m_present_thread(), m_message_memory{}, m_window_cs(), m_display_cs(), m_native_window_list(), m_display_list(), m_submit_fence(), m_submit_sync(), m_submit_event(), m_command_list_count(), m_command_list_array(), m_is_present() {/*...*/}
            constexpr ~DisplayManager() {/*...*/}

            void Initialize(mem::Heap *heap);
            void Finalize();

            void SubmitAndPresentAsync(gfx::CommandList *command_list_array, u32 command_list_count);
            void WaitAndCompletePresent();
    };
}
