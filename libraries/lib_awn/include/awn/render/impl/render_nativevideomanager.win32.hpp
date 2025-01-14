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

namespace awn::render::impl {

    static constexpr const wchar_t *cNativeDisplayWindowClassName = L"awn::render::NativeDisplay";

    class NativeVideoManager {
        public:
            friend class render::NativeLayer;
            friend class render::NativeDisplay;
        public:
            static constexpr size_t cMessageExitCode = 0x7fff'ffff'ffff'ffff;
            static constexpr u32 cMaxCommandListCount = 32;
        public:
            using NativeLayerList = vp::util::IntrusiveListTraits<NativeLayer, &NativeLayer::m_display_node>::List;
            using NativeDisplayList = vp::util::IntrusiveListTraits<NativeDisplay, &NativeDisplay::m_mgr_node>::List;
            using ManagerNativeLayerList = vp::util::IntrusiveListTraits<NativeLayer, &NativeLayer::m_mgr_node>::List;
            using PresentThreadFunc = sys::ThreadDelegate<NativeVideoManager>;
        private:
            sys::Mutex              m_list_cs;
            NativeDisplayList       m_display_list;
            ManagerNativeLayerList  m_layer_list;
            u32                     m_display_count;
            u32                     m_status;
            sys::DelegateThread    *m_present_thread;
            PresentThreadFunc       m_present_func;
            HANDLE                  m_submit_event;
            ID3D11Fence            *m_submit_fence;
            gfx::Sync               m_submit_sync;
            sys::Event              m_present_event;
            u32                     m_command_list_count;
            gfx::CommandList        m_command_list_array[cMaxCommandListCount];
        public:
            AWN_SINGLETON_TRAITS(NativeVideoManager);
        protected:
            void PresentThreadMain(size_t message) {
                VP_UNUSED(message);
                this->SubmitAndPresent();
                return;
            }
            void SubmitAndPresent();
        public:
            constexpr NativeVideoManager() : m_list_cs(), m_display_list(), m_layer_list(), m_display_count(), 
            m_present_thread(), m_present_func(this, &NativeVideoManager::PresentThreadMain), m_submit_event(), m_submit_fence(), m_submit_sync(), m_present_event(sys::SignalState::Cleared, sys::ResetMode::Manual), m_command_list_count(), m_command_list_array()  {/*...*/}
            constexpr ~NativeVideoManager() {/*...*/}

            void Initialize(mem::Heap *heap);
            void Finalize();

            void SubmitAndPresentAsync(gfx::CommandList *command_list_array, u32 command_list_count);
            void WaitAndCompletePresent();
        protected:
            void AddLayer(NativeLayer *layer);
            void RemoveLayer(NativeLayer *layer);

            void AddDisplay(NativeDisplay *display);
            void RemoveDisplay(NativeDisplay *display);
    };
}
