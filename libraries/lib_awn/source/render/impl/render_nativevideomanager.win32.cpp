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

    void NativeVideoManager::Initialize(mem::Heap *heap) {

        m_status = 0;

        /* Register window classes */
        const WNDCLASSEXW wnd_class_display = {
            .cbSize = sizeof(WNDCLASSEXW),
            .lpfnWndProc = NativeDisplay::WindowProcedure,
            .hInstance = ::GetModuleHandle(nullptr),
            .lpszClassName = cNativeDisplayWindowClassName,
        };
        ATOM atom1 = ::RegisterClassExW(std::addressof(wnd_class_display));
        VP_ASSERT(atom1 != 0);

        /* Create present thread */
        sys::DelegateThread *thread = new sys::DelegateThread(std::addressof(m_present_func), "awn::render::impl::PresentThread", heap, sys::ThreadRunMode::WaitForMessage, cMessageExitCode, 8, vp::util::c128KB, 0);
        thread->StartThread();
        m_present_thread = thread;

        /* Create submission sync */
        m_submit_sync.Initialize();
        ContextDxgi11 *context_dxgi = ContextDxgi11::GetInstance();
        ID3D11Device5 *device_5 = context_dxgi->GetD3d11Device();
        const HRESULT result0 = device_5->CreateFence(0, D3D11_FENCE_FLAG_SHARED, IID_PPV_ARGS(std::addressof(m_submit_fence)));
        VP_ASSERT(result0 == S_OK);
        HANDLE semaphore_handle = nullptr;
        const HRESULT result1 = m_submit_fence->CreateSharedHandle(nullptr, GENERIC_ALL, nullptr, std::addressof(semaphore_handle));
        VP_ASSERT(result1 == S_OK);
        const VkImportSemaphoreWin32HandleInfoKHR import_info = {
            .sType = VK_STRUCTURE_TYPE_IMPORT_SEMAPHORE_WIN32_HANDLE_INFO_KHR,
            .semaphore = m_submit_sync.GetVkSemaphore(),
            .handleType = VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_D3D11_FENCE_BIT,
            .handle = semaphore_handle,
        };
        const VkResult vk_result = ::pfn_vkImportSemaphoreWin32HandleKHR(gfx::impl::Context::GetInstance()->GetVkDevice(), std::addressof(import_info));
        RESULT_ABORT_UNLESS(gfx::impl::ConvertVkResultToResult(vk_result));
        ::CloseHandle(semaphore_handle);

        m_submit_event = ::CreateEventA(nullptr, false, false, nullptr);
        VP_ASSERT(m_submit_event != nullptr);

        /* Boost compositor clock */
        const HRESULT hresult2 = ::DCompositionBoostCompositorClock(true);
        VP_ASSERT(hresult2 == S_OK);

        return;
    }

    void NativeVideoManager::Finalize() {

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

        /* Delete present thread */
        if (m_present_thread != nullptr) {
            delete m_present_thread;
            m_present_thread = nullptr;
        }
        if (m_submit_event != nullptr) {
            ::CloseHandle(m_submit_event);
            m_submit_event = nullptr;
        }
        if (m_submit_fence != nullptr) {
            m_submit_fence->Release();
            m_submit_fence = nullptr;
        }
        m_submit_sync.Finalize();

        /* Reduce compositor clock */
        const HRESULT hresult2 = ::DCompositionBoostCompositorClock(false);
        VP_ASSERT(hresult2 == S_OK);

        return;
    }

    void NativeVideoManager::SubmitAndPresentAsync(gfx::CommandList *command_list_array, u32 command_list_count) {

        /* Set command lists */
        VP_ASSERT(command_list_count <= cMaxCommandListCount);
        for (u32 i = 0; i < command_list_count; ++i) {
            m_command_list_array[i] = command_list_array[i];
        }
        m_command_list_count = command_list_count;

        /* Signal thread to present */
        m_status = 1;
        m_present_event.Clear();
        m_present_thread->SendMessage(0);

        return;
    }

    void NativeVideoManager::SubmitAndPresent() {

        /* Submit command lists and copy scanout */
        VP_ASSERT(m_status == 1);
        awn::gfx::Sync *submit_sync = std::addressof(m_submit_sync);
        awn::gfx::SubmitCommandLists(m_command_list_array, m_command_list_count, nullptr, 0, std::addressof(submit_sync), 1);
        ID3D11DeviceContext4 *device_context = ContextDxgi11::GetInstance()->GetD3d11DeviceContext();
        const HRESULT hresult0 = device_context->Wait(m_submit_fence, m_submit_sync.GetExpectedValue());
        VP_ASSERT(hresult0 == S_OK);
        for (NativeLayer &layer : m_layer_list) {
            layer.CopyScanOut();
        }
        //device_context->Flush1(D3D11_CONTEXT_TYPE_ALL, m_submit_event);

        /* Wait */
        //const u32 result = ::WaitForSingleObjectEx(m_submit_event, INFINITE, false);
        //VP_ASSERT(result == WAIT_OBJECT_0);

        /* Swap images with dcomp */
        for (NativeLayer &layer : m_layer_list) {
            layer.CommitVisual();
        }
        IDCompositionDevice4 *dcmp_device4 = ContextDxgi11::GetInstance()->GetDcmpDevice4();
        const HRESULT hresult2 = dcmp_device4->Commit();
        VP_ASSERT(hresult2 == S_OK);

        /* Signal calc to continue */
        m_status = 2;
        m_present_event.Signal();

        return;
    }

    void NativeVideoManager::WaitAndCompletePresent() {

        /* Wait for submit and present or commit sync */
        m_present_event.Wait();
        VP_ASSERT(m_status == 2 || m_status == 0);
        if (m_status == 2) {
            m_status = 0;

            /* Wait for dcomp commit */
            IDCompositionDevice4 *dcmp_device4 = ContextDxgi11::GetInstance()->GetDcmpDevice4();
            const HRESULT hresult0 = dcmp_device4->WaitForCommitCompletion();
            VP_ASSERT(hresult0 == S_OK);

            /* Wait for compositor to begin frame */
            //::DCompositionWaitForCompositorClock(0, nullptr, INFINITE);

            m_submit_sync.Wait();
            m_submit_sync.IncrementExpectedValue();
        } else {
            IDCompositionDevice4 *dcmp_device4 = ContextDxgi11::GetInstance()->GetDcmpDevice4();
            const HRESULT hresult0 = dcmp_device4->Commit();
            VP_ASSERT(hresult0 == S_OK);
            const HRESULT hresult1 = dcmp_device4->WaitForCommitCompletion();
            VP_ASSERT(hresult1 == S_OK);
        }

        /* Update display transform */
        for (NativeDisplay &native_display : m_display_list) {
            native_display.FinishResize();
        }

        /* Update layer settings */
        for (NativeLayer &native_layer : m_layer_list) {
            native_layer.CommitTarget();
        }

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
