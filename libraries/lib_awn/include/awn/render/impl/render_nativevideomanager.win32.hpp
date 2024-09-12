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

    static constexpr const char *cNativeDisplayWindowClassName = "awn::render::NativeDisplay";

    class NativeVideoManager {
        public:
            friend class render::NativeLayer;
            friend class render::NativeDisplay;
        public:
            using NativeLayerList = vp::util::IntrusiveListTraits<NativeLayer, &NativeLayer::m_display_node>::List;
            using NativeDisplayList = vp::util::IntrusiveListTraits<NativeDisplay, &NativeDisplay::m_mgr_node>::List;
            using ManagerNativeLayerList = vp::util::IntrusiveListTraits<NativeLayer, &NativeLayer::m_mgr_node>::List;
        private:
            sys::Mutex              m_list_cs;
            NativeDisplayList       m_display_list;
            ManagerNativeLayerList  m_layer_list;
            u32                     m_display_count;
        public:
            AWN_SINGLETON_TRAITS(NativeVideoManager);
        public:
            constexpr NativeVideoManager() : m_list_cs(), m_display_list(), m_layer_list(), m_display_count() {/*...*/}
            constexpr ~NativeVideoManager() {/*...*/}

            void Initialize();
            void Finalize();

            void Calculate();
        protected:
            void AddLayer(NativeLayer *layer);
            void RemoveLayer(NativeLayer *layer);

            void AddDisplay(NativeDisplay *display);
            void RemoveDisplay(NativeDisplay *display);
    };
}
