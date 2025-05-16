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

namespace awn::grr {

    void Display::SwapchainTarget::CreateTarget(IPresentationManager *presentation_mgr, gfx::ImageFormat format, vp::util::Int2 size) {

        /* Check create guard */
        if (this->create_guard) {

            /* Ensure same texture dimensions */
            const vp::util::Int2 prev_size = this->texture.GetSize2d();
            if (prev_size == size && format == this->texture.GetImageFormat()) {

                /* Update presentation manager if different */
                if (presentation_mgr == this->parent_manager) {
                    return;
                } else {
                    this->AddToPresentManager(presentation_mgr);
                    return;
                }
            }
            this->DestroyTarget();
        }

        /* Initialize d3d11 scan-out texture */
        impl::ContextDxgi11 *context_dxgi = impl::ContextDxgi11::GetInstance();
        ID3D11Device5 *d3d11_device = context_dxgi->GetD3d11Device();
        D3D11_TEXTURE2D_DESC desc = {
            .Width = static_cast<u32>(size[0]),
            .Height = static_cast<u32>(size[1]),
            .MipLevels = 1,
            .ArraySize = 1,
            .Format = impl::GfxImageFormatToDxgiFormat(format),
            .SampleDesc = {
                .Count = 1,
                .Quality = 0,
            },
            .Usage = D3D11_USAGE_DEFAULT,
            .BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET,
            .CPUAccessFlags = 0,
            .MiscFlags = D3D11_RESOURCE_MISC_SHARED | D3D11_RESOURCE_MISC_SHARED_NTHANDLE | D3D11_RESOURCE_MISC_SHARED_DISPLAYABLE,
        };
        const HRESULT hresult0 = d3d11_device->CreateTexture2D(std::addressof(desc), nullptr, std::addressof(this->d3d11_scanout_texture));
        VP_ASSERT(hresult0 == S_OK);
        
        /* Initialize d3d11 render target texture */
        desc.MiscFlags = D3D11_RESOURCE_MISC_SHARED | D3D11_RESOURCE_MISC_SHARED_NTHANDLE;
        const HRESULT hresult1 = d3d11_device->CreateTexture2D(std::addressof(desc), nullptr, std::addressof(this->d3d11_render_texture));
        VP_ASSERT(hresult1 == S_OK);

        /* Add to present manager */
        if (presentation_mgr != nullptr) {
            this->AddToPresentManager(presentation_mgr);
        }

        /* Export d3d11 render texture handle for Vulkan gfx */
        IDXGIResource1 *resource_1 = nullptr;
        const HRESULT hresult2 = this->d3d11_render_texture->QueryInterface(IID_PPV_ARGS(std::addressof(resource_1)));
        VP_ASSERT(hresult2 == S_OK);
        HANDLE resource_handle = nullptr;
        const HRESULT hresult3 = resource_1->CreateSharedHandle(nullptr, DXGI_SHARED_RESOURCE_READ | DXGI_SHARED_RESOURCE_WRITE, nullptr, std::addressof(resource_handle));
        VP_ASSERT(hresult3 == S_OK);
        resource_1->Release();

        /* Create import gfx texture for rendering to the d3d11 render texture */
        VkExternalMemoryImageCreateInfo external_memory = {
            .sType = VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_IMAGE_CREATE_INFO,
            .handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_D3D11_TEXTURE_BIT,
        };
        gfx::TextureInfo texture_info;
        texture_info.SetDefaults();
        texture_info.SetImageFormat(format);
        texture_info.SetGpuAccessFlags(gfx::GpuAccessFlags::RenderTargetColor);
        texture_info.SetWidth(size[0]);
        texture_info.SetHeight(size[1]);
        RESULT_ABORT_UNLESS(this->texture.InitializeForDedicatedAllocation(std::addressof(texture_info), VK_IMAGE_USAGE_TRANSFER_DST_BIT, std::addressof(external_memory)));

        /* Bind d3d11 texture memory to gfx texture */
        gfx::impl::Context *context = gfx::impl::Context::GetInstance();
        gfx::GpuMemoryRequirements memory_requirements = gfx::impl::GetVkImageMemoryRequirements(this->texture.GetVkImage());
        const VkImportMemoryWin32HandleInfoKHR  import_win32_info = {
            .sType = VK_STRUCTURE_TYPE_IMPORT_MEMORY_WIN32_HANDLE_INFO_KHR,
            .handleType = VK_EXTERNAL_MEMORY_HANDLE_TYPE_D3D11_TEXTURE_BIT,
            .handle = resource_handle,
        };
        gfx::MemoryPoolInfo pool_info;
        pool_info.SetDefaults();
        pool_info.SetMemoryPropertyFlags(gfx::MemoryPropertyFlags::GpuUncached);
        pool_info.SetSize(memory_requirements.size);
        RESULT_ABORT_UNLESS(this->memory_pool.InitializeImpl(std::addressof(pool_info), std::addressof(import_win32_info)));
        const VkResult vk_result = ::pfn_vkBindImageMemory(context->GetVkDevice(), this->texture.GetVkImage(), this->memory_pool.GetVkDeviceMemory(), 0);
        VP_ASSERT(vk_result == VK_SUCCESS);
        ::CloseHandle(resource_handle);

        /* Initialize gfx render target */
        gfx::RenderTargetInfo render_target_info;
        render_target_info.SetDefaults();
        render_target_info.SetImageFormat(format);
        render_target_info.SetTexture(std::addressof(this->texture));
        this->render_target_color.Initialize(std::addressof(render_target_info));

        this->create_guard = true;

        return;
    }

    void Display::SwapchainTarget::AddToPresentManager(IPresentationManager *presentation_mgr) {

        /* Release previous buffer */
        if (this->presentation_buffer != nullptr) {
            this->presentation_buffer->Release();
            this->presentation_buffer = nullptr;
        }

        /* Export d3d11 scanout texture handle for composition swapchain */
        IDXGIResource1 *resource_1 = nullptr;
        const HRESULT hresult0 = this->d3d11_scanout_texture->QueryInterface(IID_PPV_ARGS(std::addressof(resource_1)));
        VP_ASSERT(hresult0 == S_OK);
        HANDLE resource_handle = nullptr;
        const HRESULT hresult1 = resource_1->CreateSharedHandle(nullptr, DXGI_SHARED_RESOURCE_READ | DXGI_SHARED_RESOURCE_WRITE, nullptr, std::addressof(resource_handle));
        VP_ASSERT(hresult1 == S_OK);

        /* Add d3d11 scanout texture to composition swapchain presentation manager  */
        const HRESULT hresult2 = presentation_mgr->AddBufferFromResource(resource_1, std::addressof(this->presentation_buffer));
        VP_ASSERT(hresult2 == S_OK);
        resource_1->Release();
        ::CloseHandle(resource_handle);
        this->parent_manager = presentation_mgr;

        return;
    }

    void Display::SwapchainTarget::DestroyTarget() {
        this->create_guard = false;
        this->parent_manager = nullptr;
        this->render_target_color.Finalize();
        this->texture.Finalize();
        this->memory_pool.Finalize();
        if (this->presentation_buffer != nullptr) {
            this->presentation_buffer->Release();
            this->presentation_buffer = nullptr;
        }
        if (this->d3d11_scanout_texture != nullptr) {
            this->d3d11_scanout_texture->Release();
            this->d3d11_scanout_texture = nullptr;
        }
        if (this->d3d11_render_texture != nullptr) {
            this->d3d11_render_texture->Release();
            this->d3d11_render_texture = nullptr;
        }
        return;
    }

    void Display::SwapchainTarget::BarrierTarget() {

        /* Check target is available */
        if (!this->create_guard) { return; }

        boolean is_available;
        const HRESULT hresult0 = this->presentation_buffer->IsAvailable(std::addressof(is_available));
        VP_ASSERT(hresult0 == S_OK);
        if (is_available) { return; }

        /* Wait for available event */
        HANDLE event = 0;
        const HRESULT hresult1 = this->presentation_buffer->GetAvailableEvent(std::addressof(event));
        VP_ASSERT(hresult1 == S_OK);
        ON_SCOPE_EXIT { ::CloseHandle(event); };
        const u32 result = ::WaitForSingleObjectEx(event, INFINITE, false);
        VP_ASSERT(result == WAIT_OBJECT_0);

        return;
    }

    void Display::Initialize(mem::Heap *heap, const DisplayInfo *info) {

        /* Set window settings */
        m_native_window = info->native_window;
        m_settings.image_format = info->image_format;
        m_settings.active_target_count = info->active_target_count;
        m_settings.present_interval = info->present_interval;
        m_settings.is_framebuffer_alpha = info->is_framebuffer_alpha;
        m_update_settings = m_settings;

        /* Create initial target */
        m_target_array.Initialize(heap, info->target_count);
        SwapchainTarget *target = std::addressof(m_target_array[0]);
        if (m_native_window->GetNativeHandle() != nullptr) {
            this->CreatePresentationManager();
        }
        const vp::util::Int2 size = m_native_window->GetSize();
        target->CreateTarget(m_presentation_manager, m_settings.image_format, size);

        /* Add to manager */
        impl::DisplayManager *display_manager = impl::DisplayManager::GetInstance();
        if (display_manager != nullptr) {
            display_manager->AddDisplay(this);
        }

        return;
    }

    void Display::Finalize() {
        impl::DisplayManager *display_manager = impl::DisplayManager::GetInstance();
        if (display_manager != nullptr) {
            display_manager->RemoveDisplay(this);
        }
        this->DestroyPresentationManager();
        for (SwapchainTarget &target : m_target_array) {
            target.DestroyTarget();
        }
        m_target_array.Finalize();
        return;
    }

    void Display::CreatePresentationManager() {

        /* Create direct composition target */
        impl::ContextDxgi11 *context_dxgi = impl::ContextDxgi11::GetInstance();
        IDCompositionDesktopDevice *device_desktop = context_dxgi->GetDcmpDevice();
        IDCompositionTarget *dcmp_target = nullptr;
        NativeWindowHandle window_handle = m_native_window->GetNativeHandle();
        m_handle = window_handle;
        const HRESULT hresult0 = device_desktop->CreateTargetForHwnd(window_handle, false, std::addressof(dcmp_target));
        VP_ASSERT(hresult0 == S_OK);

        /* Create direct composition root visual */
        IDCompositionVisual2 *visual_2 = nullptr;
        const HRESULT hresult1 = device_desktop->CreateVisual(std::addressof(visual_2));
        VP_ASSERT(hresult1 == S_OK);
        IDCompositionVisual3 *visual_main = nullptr;
        const HRESULT hresult2 = visual_2->QueryInterface(std::addressof(visual_main));
        VP_ASSERT(hresult2 == S_OK);
        visual_2->Release();

        /* Set root visual to direct composition target */
        const HRESULT hresult3 = dcmp_target->SetRoot(visual_main);
        VP_ASSERT(hresult3 == S_OK);

        /* Create composition swapchain presentation manager */
        IPresentationFactory *present_factory = context_dxgi->GetPresentationFactory();
        IPresentationManager *present_mgr = nullptr;
        const HRESULT hresult4 = present_factory->CreatePresentationManager(std::addressof(present_mgr));
        VP_ASSERT(hresult4 == S_OK);
        
        /* Create direct composition surface handle */
        HANDLE surface_handle = nullptr;
        const HRESULT hresult5 = ::DCompositionCreateSurfaceHandle(COMPOSITIONOBJECT_ALL_ACCESS, nullptr, std::addressof(surface_handle));
        VP_ASSERT(hresult5 == S_OK);

        /* Create composition swapchain presentation surface  */
        IPresentationSurface *present_surface = nullptr;
        const HRESULT hresult6 = present_mgr->CreatePresentationSurface(surface_handle, std::addressof(present_surface));
        VP_ASSERT(hresult6 == S_OK);
        const vp::util::Int2 size = m_native_window->GetSize();
        const RECT rect = { 0, 0, static_cast<LONG>(size[0]), static_cast<LONG>(size[1]), };
        const HRESULT hresult7 = present_surface->SetSourceRect(std::addressof(rect));
        VP_ASSERT(hresult7 == S_OK);
        const DXGI_ALPHA_MODE alpha_mode = (m_settings.is_framebuffer_alpha) ? DXGI_ALPHA_MODE_PREMULTIPLIED : DXGI_ALPHA_MODE_IGNORE; 
        const HRESULT hresult8 = present_surface->SetAlphaMode(alpha_mode);
        VP_ASSERT(hresult8 == S_OK);
        const DXGI_COLOR_SPACE_TYPE color_space = (impl::IsHdrFormat(m_settings.image_format)) ? DXGI_COLOR_SPACE_RGB_FULL_G22_NONE_P2020 : DXGI_COLOR_SPACE_RGB_FULL_G22_NONE_P709; 
        const HRESULT hresult9 = present_surface->SetColorSpace(color_space);
        VP_ASSERT(hresult9 == S_OK);

        /* Retrieve composition swapchain presentation surface's direct composition surface and set it as the root visual's content  */
        IDCompositionDesktopDevice *dcmp_desktop_device = context_dxgi->GetDcmpDevice();           
        IUnknown *dcmp_surface = nullptr;
        const HRESULT hresult10 = dcmp_desktop_device->CreateSurfaceFromHandle(surface_handle, std::addressof(dcmp_surface));
        VP_ASSERT(hresult10 == S_OK);
        const HRESULT hresult11 = visual_main->SetContent(dcmp_surface);
        VP_ASSERT(hresult11 == S_OK);

        /* Commit composition changes */
        IDCompositionDevice4 *dcmp_device4 = impl::ContextDxgi11::GetInstance()->GetDcmpDevice4();
        const HRESULT hresult12 = dcmp_device4->Commit();
        VP_ASSERT(hresult12 == S_OK);
        const HRESULT hresult13 = dcmp_device4->WaitForCommitCompletion();
        VP_ASSERT(hresult13 == S_OK);

        /* Set display interval */
        const u64 pref_time = (m_settings.present_interval == 0) ? cPresentTimeArray[0] : cPresentTimeArray[m_settings.present_interval - 1];
        const HRESULT hresult14 = present_mgr->SetPreferredPresentDuration(SystemInterruptTime{pref_time}, SystemInterruptTime{100});
        VP_ASSERT(hresult14 == S_OK);

        /* Get present id */
        m_target_array[m_image_id].present_id = present_mgr->GetNextPresentId();

        /* Save interfaces */
        m_dcmp_target = dcmp_target;
        m_dcmp_visual = visual_main;
        m_presentation_manager = present_mgr;
        m_surface_handle = surface_handle;
        m_presentation_surface = present_surface;
        m_dcmp_surface = dcmp_surface;

        /* Setup present time */
        ::QueryInterruptTimePrecise(std::addressof(m_timer));
        m_target_array[m_image_id].present_time = m_timer;
        const TimeSpan span = TimeSpan::FromNanoSeconds(m_timer * 100);
        m_native_window->SetPresentTime(span);

        return;
    }

    void Display::DestroyPresentationManager() {
        if (m_dcmp_surface != nullptr) {
            m_dcmp_surface->Release();
            m_dcmp_surface = nullptr;
        }
        if (m_presentation_surface != nullptr) {
            m_presentation_surface->Release();
            m_presentation_surface = nullptr;
        }
        if (m_surface_handle != nullptr) {
            ::CloseHandle(m_surface_handle);
            m_surface_handle = nullptr;
        }
        if (m_presentation_manager != nullptr) {
            m_presentation_manager->Release();
            m_presentation_manager = nullptr;
        }
        if (m_dcmp_visual != nullptr) {
            m_dcmp_visual->Release();
            m_dcmp_visual = nullptr;
        }
        if (m_dcmp_target != nullptr) {
            m_dcmp_target->Release();
            m_dcmp_target = nullptr;
        }
        return;
    }

    void Display::CopyScanOut() {

        /* Check present requested */
        if (!m_is_request_present || m_handle == nullptr) { return; }

        /* Copy render texture to scan-out texture */
        impl::ContextDxgi11 *context_dxgi = impl::ContextDxgi11::GetInstance();
        ID3D11DeviceContext4 *d3d11_context4 = context_dxgi->GetD3d11DeviceContext();
        SwapchainTarget *target = std::addressof(m_target_array[m_image_id]);
        d3d11_context4->CopyResource(target->d3d11_scanout_texture, target->d3d11_render_texture);

        return;
    }

    void Display::Present() {

        /* Check present requested */
        if (!m_is_request_present || m_handle == nullptr) { return; }

        /* Set buffer to surface */
        SwapchainTarget *target = std::addressof(m_target_array[m_image_id]);
        const HRESULT hresult0 = m_presentation_surface->SetBuffer(target->presentation_buffer);
        VP_ASSERT(hresult0 == S_OK);
        const RECT src_rect = { 0, 0, static_cast<LONG>(target->texture.GetWidth()), static_cast<LONG>(target->texture.GetHeight()), };
        const HRESULT hresult1 = m_presentation_surface->SetSourceRect(std::addressof(src_rect));
        VP_ASSERT(hresult1 == S_OK);

        /* Adjust present timing for present interval */
        const HRESULT hresult3 = m_presentation_manager->SetTargetTime(SystemInterruptTime{target->present_time});
        VP_ASSERT(hresult3 == S_OK);

        /* Present presentation buffer to manager */
        const HRESULT hresult4 = m_presentation_manager->Present();
        VP_ASSERT(hresult4 == S_OK);

        /* Resize window */
        m_native_window->RequestWindowPos(true);

        return;
    }

    void Display::AcquireNext() {

        /* Check present requested */
        if (!m_is_request_present || m_handle == nullptr) { return; }

        /* Update present settings */
        {
            std::scoped_lock l(m_settings_cs);
            if (m_settings.present_interval != m_update_settings.present_interval) {
                const u64 pref_time = (m_update_settings.present_interval == 0) ? cPresentTimeArray[0] : cPresentTimeArray[m_update_settings.present_interval - 1];
                const HRESULT hresult0 = m_presentation_manager->SetPreferredPresentDuration(SystemInterruptTime{pref_time}, SystemInterruptTime{100});
                VP_ASSERT(hresult0 == S_OK);
                m_settings.present_interval = m_update_settings.present_interval;
            }
            if (m_settings.image_format != m_update_settings.image_format) {
                const DXGI_COLOR_SPACE_TYPE color_space = (impl::IsHdrFormat(m_update_settings.image_format)) ? DXGI_COLOR_SPACE_RGB_FULL_G22_NONE_P2020 : DXGI_COLOR_SPACE_RGB_FULL_G22_NONE_P709; 
                const HRESULT hresult0 = m_presentation_surface->SetColorSpace(color_space);
                VP_ASSERT(hresult0 == S_OK);
                m_settings.image_format = m_update_settings.image_format;
            }
            if (m_settings.is_framebuffer_alpha != m_update_settings.is_framebuffer_alpha) {
                const DXGI_ALPHA_MODE alpha_mode = (m_update_settings.is_framebuffer_alpha) ? DXGI_ALPHA_MODE_PREMULTIPLIED : DXGI_ALPHA_MODE_IGNORE; 
                const HRESULT hresult0 = m_presentation_surface->SetAlphaMode(alpha_mode);
                VP_ASSERT(hresult0 == S_OK);
                m_settings.is_framebuffer_alpha = m_update_settings.is_framebuffer_alpha;
            }
            m_settings.active_target_count = m_update_settings.active_target_count;
        }

        /* Increment target id */
        const u32 next_index = m_image_id + 1;
        const u32 target_index = (next_index < m_settings.active_target_count) ? next_index : 0;
        m_image_id = target_index;

        /* Barrier target */
        SwapchainTarget *target = std::addressof(m_target_array[target_index]);
        target->BarrierTarget();

        /* Update present id */
        target->present_id = m_presentation_manager->GetNextPresentId();

        /* Calc target time */
        const u32 present_interval = m_settings.present_interval;
        target->present_time = m_timer;
        const TimeSpan span = TimeSpan::FromNanoSeconds(target->present_time * 100);
        m_native_window->SetPresentTime(span);
        m_timer = (0 < present_interval) ? m_timer + cPresentTimeArray[present_interval - 1] : m_timer + cPresentTimeArray[0];

        return;
    }

    void Display::CreateTarget() {

        /* (Re)create presentation manager if necessary */
        NativeWindowHandle handle = m_native_window->GetNativeHandle();
        if (handle != nullptr && handle != m_handle) {
            this->DestroyPresentationManager();
            this->CreatePresentationManager();
        }

        /* Check present requested */
        if (!m_is_request_present) { return; }
        m_is_request_present = false;

        /* Update target format */
        const vp::util::Int2 size = m_native_window->GetSize();
        SwapchainTarget *target = std::addressof(m_target_array[m_image_id]);
        target->CreateTarget(m_presentation_manager, m_settings.image_format, size);

        return;
    }
}
