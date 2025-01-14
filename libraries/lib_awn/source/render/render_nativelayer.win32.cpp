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

namespace awn::render {

    void NativeLayer::SwapchainTarget::CreateTarget(gfx::ImageFormat format, vp::util::Int2 size) {

        /* Skip recreate if same format */
        if (this->is_valid) {
            const vp::util::Int2 prev_size = { static_cast<s32>(this->texture.GetWidth()), static_cast<s32>(this->texture.GetHeight()), };
            if (prev_size == size && format == this->texture.GetImageFormat()) { return; }
        }

        /* Cleanup previous */
        this->DestroyTarget();

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

        /* Initialize IDCompositionTexture */
        IDCompositionDevice4 *dcomp_device4 = context_dxgi->GetDcmpDevice4();
        const HRESULT hresult1 = dcomp_device4->CreateCompositionTexture(this->d3d11_scanout_texture, std::addressof(this->dcomp_texture));
        VP_ASSERT(hresult1 == S_OK);
        const D2D_RECT_U rect = { 0, 0, static_cast<u32>(size[0]), static_cast<u32>(size[1]), };
        this->dcomp_texture->SetSourceRect(rect);
        this->dcomp_texture->SetAlphaMode(DXGI_ALPHA_MODE_IGNORE);
        this->dcomp_texture->SetColorSpace(DXGI_COLOR_SPACE_RGB_FULL_G22_NONE_P709);

        /* Initialize d3d11 render target texture */
        desc.MiscFlags = D3D11_RESOURCE_MISC_SHARED | D3D11_RESOURCE_MISC_SHARED_NTHANDLE;
        const HRESULT hresult2 = d3d11_device->CreateTexture2D(std::addressof(desc), nullptr, std::addressof(this->d3d11_texture));
        VP_ASSERT(hresult2 == S_OK);

        /* Export d3d11 texture handle  */
        IDXGIResource1 *resource_1 = nullptr;
        const HRESULT result3 = this->d3d11_texture->QueryInterface(IID_PPV_ARGS(std::addressof(resource_1)));
        VP_ASSERT(result3 == S_OK);
        HANDLE resource_handle = nullptr;
        const HRESULT result4 = resource_1->CreateSharedHandle(nullptr, DXGI_SHARED_RESOURCE_READ | DXGI_SHARED_RESOURCE_WRITE, nullptr, std::addressof(resource_handle));
        VP_ASSERT(result4 == S_OK);
        resource_1->Release();

        //VkMemoryWin32HandlePropertiesKHR properties = {
        //    .sType = VK_STRUCTURE_TYPE_MEMORY_WIN32_HANDLE_PROPERTIES_KHR,
        // };
        //const VkResult result = ::pfn_vkGetMemoryWin32HandlePropertiesKHR(gfx::impl::Context::GetInstance()->GetVkDevice(), VK_EXTERNAL_MEMORY_HANDLE_TYPE_D3D11_TEXTURE_BIT, resource_handle, std::addressof(properties));
        //VP_ASSERT(result == VK_SUCCESS);
        //::printf("memoryTypeBits: %d\n", properties.memoryTypeBits);

        /* Initialize gfx texture for import */
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
        RESULT_ABORT_UNLESS(mem::AllocateGpuMemoryHandle(std::addressof(this->memory_handle), nullptr, gfx::MemoryPropertyFlags::GpuUncached, memory_requirements.size, std::addressof(import_win32_info)));
        const VkResult result5 = ::pfn_vkBindImageMemory(context->GetVkDevice(), this->texture.GetVkImage(), this->memory_handle.vk_device_memory, 0);
        VP_ASSERT(result5 == VK_SUCCESS);
        ::CloseHandle(resource_handle);

        /* Initialize gfx render target */
        gfx::RenderTargetInfo render_target_info;
        render_target_info.SetDefaults();
        render_target_info.SetImageFormat(format);
        render_target_info.SetTexture(std::addressof(this->texture));
        this->render_target_color.Initialize(std::addressof(render_target_info));

        this->is_valid = true;

        return;
    }

    void NativeLayer::SwapchainTarget::DestroyTarget() {
        this->is_valid = false;
        this->render_target_color.Finalize();
        this->texture.Finalize();
        if (this->memory_handle.IsValid()) {
            mem::FreeGpuMemoryHandle(this->memory_handle);
            this->memory_handle = mem::cInvalidGpuMemoryHandle;
        }
        if (this->dcomp_texture != nullptr) {
            this->dcomp_texture->Release();
            this->dcomp_texture = nullptr;
        }
        if (this->d3d11_scanout_texture != nullptr) {
            this->d3d11_scanout_texture->Release();
            this->d3d11_scanout_texture = nullptr;
        }
        if (this->d3d11_texture != nullptr) {
            this->d3d11_texture->Release();
            this->d3d11_texture = nullptr;
        }
    }

    void NativeLayer::SwapchainTarget::BarrierTarget(HANDLE event_handle) {

        /* Get DWM fence */
        UINT64 fence_value = 0;
        ID3D11Fence *fence = nullptr;
        const HRESULT hresult0 = this->dcomp_texture->GetAvailableFence(std::addressof(fence_value), IID_PPV_ARGS(std::addressof(fence)));
        VP_ASSERT(hresult0 == S_OK);
        ON_SCOPE_EXIT { fence->Release(); };

        /* Wait for fence */
        if (fence_value == fence->GetCompletedValue()) { return; }
        const HRESULT hresult1 = fence->SetEventOnCompletion(fence_value, event_handle);
        VP_ASSERT(hresult1 == S_OK);
        const u32 result = ::WaitForSingleObjectEx(event_handle, INFINITE, false);
        VP_ASSERT(result == WAIT_OBJECT_0);

        return;
    }

    void NativeLayer::CopyScanOut() {
        if (!m_is_request_present) { return; }
        impl::ContextDxgi11 *context_dxgi = impl::ContextDxgi11::GetInstance();
        ID3D11DeviceContext4 *d3d11_context4 = context_dxgi->GetD3d11DeviceContext();
        SwapchainTarget *target = std::addressof(m_target_array[m_target_index]);
        d3d11_context4->CopyResource(target->d3d11_scanout_texture, target->d3d11_texture);
    }

    void NativeLayer::CommitVisual() {

        std::scoped_lock l(m_settings_cs);

		/* Update alpha */
		if (m_is_update_alpha) {
			m_is_update_alpha = false;
            const HRESULT hresult0 = m_dcmp_visual->SetOpacity(m_update_settings.alpha);
            VP_ASSERT(hresult0 == S_OK);
            if (m_update_settings.alpha == 0.0f) {
                m_is_force_invisible_for_zero_alpha = true;
                m_settings.parent_display->RemoveLayerVisual(this);
            } else if (m_is_force_invisible_for_zero_alpha) {
                m_is_force_invisible_for_zero_alpha = false;
                m_is_update_is_visible = m_update_settings.is_visible;
            }
			m_settings.alpha = m_update_settings.alpha;
		}

		/* Update visibility */
		if (m_is_update_is_visible && !m_is_force_invisible_for_zero_alpha) {
			m_is_update_is_visible = false;
            if (m_update_settings.is_visible) {
                m_settings.parent_display->AddLayerVisual(this);
            } else {
                m_settings.parent_display->RemoveLayerVisual(this);
            }
			m_settings.is_visible = m_update_settings.is_visible;
		}

		/* Update transform */
		if (m_is_update_transform) {
			m_is_update_transform = false;
            const HRESULT hresult0 = m_dcmp_visual->SetTransform(*reinterpret_cast<D2D_MATRIX_4X4_F*>(std::addressof(m_update_settings.transform)));
            VP_ASSERT(hresult0 == S_OK);
			m_settings.transform = m_update_settings.transform;
		}

        /* Swap image */
        if (m_is_request_present) {
            const HRESULT hresult0 = m_dcmp_visual->SetContent(m_target_array[m_target_index].dcomp_texture);
            VP_ASSERT(hresult0 == S_OK);
        }

        return;
    }

    void NativeLayer::AcquireNextTarget() {

        /* Increment target id */
        const u32 next_index = m_target_index + 1;
        const u32 target_index = (next_index != m_target_array.GetCount()) ? next_index : 0;
        m_target_index = target_index;

        /* Update target format */
        m_target_array[target_index].CreateTarget(m_update_settings.image_format, m_update_settings.size);
        if (m_update_settings.size != m_settings.size) {
            m_settings.size = m_update_settings.size;
            this->InvokeLayerResize();
        }
        m_settings.image_format = m_update_settings.image_format;
        m_target_array[target_index].BarrierTarget(m_target_event);

        return;
    }

    void NativeLayer::CommitTarget() {

        std::scoped_lock l(m_settings_cs);

        /* Update swapchain texture */
        if (m_is_request_present) {
            m_is_request_present = false;
            this->AcquireNextTarget();
        }

        return;
    }

	Result NativeLayer::Initialize(mem::Heap *heap, const NativeLayerInfo *layer_info) {

		/* Set params */
		m_name = layer_info->name;

        /* Create direct composition visual */
        impl::ContextDxgi11 *context_dxgi = impl::ContextDxgi11::GetInstance();
        IDCompositionDevice4 *dcmp_device4 = context_dxgi->GetDcmpDevice4();           
        IDCompositionVisual2 *visual_2 = nullptr;
        const HRESULT hresult0 = dcmp_device4->CreateVisual(std::addressof(visual_2));
        VP_ASSERT(hresult0 == S_OK);
        IDCompositionVisual3 *visual_3 = nullptr;
        const HRESULT hresult1 = visual_2->QueryInterface(IID_PPV_ARGS(std::addressof(visual_3)));
        VP_ASSERT(hresult1 == S_OK);
        visual_2->Release();
        m_dcmp_visual = visual_3;

        /* Add to parent display */
        if (layer_info->parent_display != nullptr) {
            layer_info->parent_display->AddLayer(this);
            m_update_settings.parent_display = layer_info->parent_display;
            m_settings.parent_display = layer_info->parent_display;
        }

        /* Set visual settings */
        const HRESULT hresult3 = m_dcmp_visual->SetOpacityMode(DCOMPOSITION_OPACITY_MODE_LAYER);
        VP_ASSERT(hresult3 == S_OK);
		m_update_settings.transform = layer_info->transform;
        m_is_update_transform = true;
        m_update_settings.alpha = layer_info->alpha;
        m_is_update_alpha = true;
        m_update_settings.is_visible = layer_info->is_visible;
        m_is_update_is_visible = layer_info->is_visible;

        /* Create target array */
        m_target_array.Initialize(heap, layer_info->max_image_count);
        m_image_count = layer_info->max_image_count;

        /* Set target settings */
        m_update_settings.size = layer_info->size;
        m_update_settings.image_format = layer_info->image_format;
        m_settings.size = layer_info->size;
        m_settings.image_format = layer_info->image_format;
        m_target_index = 0;
        m_target_array[0].CreateTarget(layer_info->image_format, layer_info->size);

        /* Add layer to video manager */
        impl::NativeVideoManager *video_mgr = impl::NativeVideoManager::GetInstance();
        VP_ASSERT(video_mgr != nullptr);
        video_mgr->AddLayer(this);

        /* Create target event */
        m_target_event = ::CreateEventA(nullptr, false, false, nullptr);
        VP_ASSERT(m_target_event != nullptr);

        /* Set resize functions */
        if (layer_info->display_resize_function != nullptr) {
            m_display_resize_function.SetFunction(*layer_info->display_resize_function);
        }
        if (layer_info->layer_resize_function != nullptr) {
            m_layer_resize_function.SetFunction(*layer_info->layer_resize_function);
        }

		RESULT_RETURN_SUCCESS;
	}

	void NativeLayer::Finalize() {

        /* Remove layer from video manager */
        impl::NativeVideoManager *video_mgr = impl::NativeVideoManager::GetInstance();
        video_mgr->RemoveLayer(this);

        /* Remove layer from parent */
        if (m_settings.parent_display != nullptr) {
            m_settings.parent_display->RemoveLayer(this);
        }

        /* Release visual and destroy swapchain */
        if (m_dcmp_visual != nullptr) {
            m_dcmp_visual->Release();
            m_dcmp_visual = nullptr;
        }
        if (m_target_event != nullptr) {
            ::CloseHandle(m_target_event);
            m_target_event = nullptr;
        }

        /* Destroy swapchain targets */
        for (SwapchainTarget &target : m_target_array) {
            target.DestroyTarget();
        }
        m_target_array.Finalize();

		return;
	}

	void NativeLayer::RequestSetTransform(const vp::util::MatrixRowMajor4x3f transform_matrix) {
		std::scoped_lock l(m_settings_cs);
		if (m_update_settings.transform == transform_matrix) { return; }
		m_update_settings.transform = transform_matrix;
		m_is_update_transform = true;
	}

	void NativeLayer::RequestSetZ(u32 z) {
		std::scoped_lock l(m_settings_cs);
		if (m_update_settings.z_order == z) { return; }
		m_is_update_parent = true;
		m_update_settings.z_order = z;
	}

	void NativeLayer::RequestSetSize(vp::util::Int2 size) {
        VP_UNUSED(size);
		std::scoped_lock l(m_settings_cs);
		m_update_settings.size = size;
	}

	void NativeLayer::RequestSetIsVisible(bool is_visible) {
		std::scoped_lock l(m_settings_cs);
		if (m_update_settings.is_visible == is_visible) { return; }
		m_is_update_is_visible = true;
		m_update_settings.is_visible = is_visible;
	}

	void NativeLayer::RequestSetAlpha(float alpha) {
		std::scoped_lock l(m_settings_cs);
        VP_ASSERT(0.0f <= alpha || alpha <= 1.0f);
		if (m_update_settings.alpha == alpha) { return; }
		m_is_update_alpha = true;
		m_update_settings.alpha = alpha;
	}

	void NativeLayer::RequestSetDisplay(NativeDisplay *native_display) {
		std::scoped_lock l(m_settings_cs);
		if (m_update_settings.parent_display == native_display) { return; }
		m_is_update_parent = true;
		m_update_settings.parent_display = native_display;
	}
}
