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
#include <awn.hpp>

namespace awn::render::impl {

    AWN_SINGLETON_TRAITS_IMPL(NativeVideoManager);

    void NativeVideoManager::Initialize() {

        /* Register window classes */
        const WNDCLASSEXW wnd_class_display = {
            .cbSize = sizeof(WNDCLASSEXW),
            .lpfnWndProc = NativeDisplay::WindowProcedure,
            .hInstance = ::GetModuleHandle(nullptr),
            .lpszClassName = cNativeDisplayWindowClassName,
        };
        ATOM atom1 = ::RegisterClassExW(std::addressof(wnd_class_display));
        VP_ASSERT(atom1 != 0);

        return;
    }

    void NativeVideoManager::Finalize() {

        std::scoped_lock l(m_list_cs);

        /* Force finalize native objects */
        auto layer_iter = m_layer_list.begin();
        while (layer_iter != m_layer_list.end()) {
            NativeLayer &native_layer = *layer_iter;
            ++layer_iter;
            native_layer.Finalize();
        }
        auto display_iter = m_display_list.begin();
        while (display_iter != m_display_list.end()) {
            NativeDisplay &native_display = *display_iter;
            ++display_iter;
            native_display.Finalize();
        }

        /* Unregister window classes */
        const bool result1 = ::UnregisterClassW(cNativeDisplayWindowClassName, ::GetModuleHandle(nullptr));
        VP_ASSERT(result1);

        return;
    }

    void NativeVideoManager::Calculate() {

        std::scoped_lock l(m_list_cs);

        /* Apply direct composition updates */
        IDCompositionDesktopDevice *dcmp_device = ContextDxgi::GetInstance()->GetDcmpDevice();
        for (NativeDisplay &native_display : m_display_list) {
            native_display.FinishResize();
        }

        /* Update layers */
        for (NativeLayer &native_layer : m_layer_list) {
            native_layer.ApplyUpdates();
        }

        const HRESULT hresult0 = dcmp_device->WaitForCommitCompletion();
        VP_ASSERT(hresult0 == S_OK);
        const HRESULT hresult1 = dcmp_device->Commit();
        VP_ASSERT(hresult1 == S_OK);

        /* Update displays */
        HDWP hdwp = ::BeginDeferWindowPos(m_display_count);
        for (NativeDisplay &native_display : m_display_list) {
            native_display.ApplyTransform(hdwp);
        }
        ::EndDeferWindowPos(hdwp);

        return;
    }

    void NativeVideoManager::AddLayer(NativeLayer *layer) {
        std::scoped_lock l(m_list_cs);
        VP_ASSERT(!layer->m_mgr_node.IsLinked());
        m_layer_list.PushBack(*layer);
    }

    void NativeVideoManager::RemoveLayer(NativeLayer *layer) {
        std::scoped_lock l(m_list_cs);
        if (!layer->m_mgr_node.IsLinked()) { return; }
        m_layer_list.Remove(*layer);
    }

    void NativeVideoManager::AddDisplay(NativeDisplay *display) {
        std::scoped_lock l(m_list_cs);
        VP_ASSERT(!display->m_mgr_node.IsLinked());
        m_display_list.PushBack(*display);
    }

    void NativeVideoManager::RemoveDisplay(NativeDisplay *display) {
        std::scoped_lock l(m_list_cs);
        if (!display->m_mgr_node.IsLinked()) { return; }
        m_display_list.Remove(*display);
    }
}
