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

	void NativeLayer::ApplyUpdates() {

		std::scoped_lock l(m_settings_cs);

		/* Update parent layer */
		if (m_is_update_parent) {
			m_settings.parent_display->RemoveLayer(this);
            m_settings.parent_display->AddLayer(this);
			m_is_update_parent = false;
			m_settings.parent_display = m_update_settings.parent_display;
		}

		/* Update swapchain */
		if (m_is_update_swapchain) {
            const HRESULT hresult0 = m_dcmp_visual->SetContent(nullptr);
            VP_ASSERT(hresult0 == S_OK);
            m_swapchain.ApplyUpdates();
            const HRESULT hresult1 = m_dcmp_visual->SetContent(m_swapchain.GetDxgiSwapchain());
            VP_ASSERT(hresult1 == S_OK);
            m_is_update_swapchain = false;
		}

		/* Update alpha */
		if (m_is_update_alpha) {
            if (m_update_settings.alpha == 0.0f) {
                m_is_force_invisible_for_zero_alpha = true;
                const HRESULT hresult = m_dcmp_visual->SetVisible(false);
                VP_ASSERT(hresult == S_OK);
            } else {
                if (m_is_force_invisible_for_zero_alpha) {
                    m_is_force_invisible_for_zero_alpha = false;
                    m_is_update_is_visible = true;
                }
                if (m_update_settings.alpha < 1.0f) {
                    const HRESULT hresult0 = m_dcmp_effect_group->SetOpacity(m_update_settings.alpha);
                    VP_ASSERT(hresult0 == S_OK);
                    if (!m_is_opacity_effect) {
                        m_is_opacity_effect = true;
                        const HRESULT hresult1 = m_dcmp_visual->SetEffect(m_dcmp_effect_group);
                        VP_ASSERT(hresult1 == S_OK);
                    }
                } else if (m_is_opacity_effect) {
                    m_is_opacity_effect = false;
                    const HRESULT hresult = m_dcmp_visual->SetEffect(nullptr);
                    VP_ASSERT(hresult == S_OK);
                }
            }
			m_is_update_alpha = false;
			m_settings.alpha = m_update_settings.alpha;
		}
        
		/* Update visibility */
		if (m_is_update_is_visible && !m_is_force_invisible_for_zero_alpha) {
			const HRESULT hresult = m_dcmp_visual->SetVisible(m_update_settings.is_visible);
            VP_ASSERT(hresult == S_OK);
			m_is_update_is_visible = false;
			m_settings.is_visible = m_update_settings.is_visible;
		}
        
        
		/* Update position */
		if (m_is_update_position) {
            const HRESULT hresult0 = m_dcmp_visual->SetOffsetX(m_update_settings.position.x);
            VP_ASSERT(hresult0 == S_OK);
            const HRESULT hresult1 = m_dcmp_visual->SetOffsetY(m_update_settings.position.y);
            VP_ASSERT(hresult1 == S_OK);
			m_is_update_position = false;
			m_settings.position = m_update_settings.position;
		}

		return;
	}

	Result NativeLayer::Initialize(const NativeLayerInfo *layer_info) {

		/* Set params */
		m_name = layer_info->name;

        /* Create direct composition visual */
        impl::ContextDxgi *context = impl::ContextDxgi::GetInstance();
        IDCompositionDesktopDevice *device_desktop = context->GetDcmpDevice();           
        IDCompositionVisual2 *visual_2 = nullptr;
        const HRESULT hresult0 = device_desktop->CreateVisual(std::addressof(visual_2));
        VP_ASSERT(hresult0 == S_OK);
        IDCompositionVisual3 *visual_3 = nullptr;
        const HRESULT hresult1 = visual_2->QueryInterface(IID_PPV_ARGS(std::addressof(visual_3)));
        VP_ASSERT(hresult1 == S_OK);
        visual_2->Release();
        m_dcmp_visual = visual_3;

		/* Create swapchain */
		const SwapchainInfo swapchain_info = layer_info->swapchain_info;
		m_swapchain.Initialize(std::addressof(swapchain_info), layer_info->parent_display->GetNativeHandle());
        const HRESULT hresult2 = m_dcmp_visual->SetContent(m_swapchain.GetDxgiSwapchain());
        VP_ASSERT(hresult2 == S_OK);
        const HRESULT hresult3 = m_dcmp_visual->SetOpacityMode(DCOMPOSITION_OPACITY_MODE_LAYER);
        VP_ASSERT(hresult3 == S_OK);

        /* Set initial position */
        const HRESULT hresult4 = m_dcmp_visual->SetOffsetX(layer_info->position.x);
        VP_ASSERT(hresult4 == S_OK);
        const HRESULT hresult5 = m_dcmp_visual->SetOffsetY(layer_info->position.y);
        VP_ASSERT(hresult5 == S_OK);
		m_update_settings.position = layer_info->position;

        /* Set alpha effect */
        const HRESULT hresult6 = device_desktop->CreateEffectGroup(std::addressof(m_dcmp_effect_group));
        VP_ASSERT(hresult6 == S_OK);
        VP_ASSERT(0.0f <= layer_info->alpha && layer_info->alpha <= 1.0f);
        if (layer_info->alpha == 0.0f) {
            m_is_force_invisible_for_zero_alpha = true;
            m_dcmp_visual->SetVisible(false);
        } else if (layer_info->alpha < 1.0f) {
            m_is_opacity_effect = true;
            const HRESULT hresult7 = m_dcmp_effect_group->SetOpacity(layer_info->alpha);
            VP_ASSERT(hresult7 == S_OK);
            const HRESULT hresult8 = m_dcmp_visual->SetEffect(m_dcmp_effect_group);
            VP_ASSERT(hresult8 == S_OK);
        }
        m_update_settings.alpha = layer_info->alpha;

        /* Set visibility */
        if (!m_is_force_invisible_for_zero_alpha) {
            const HRESULT hresult9 = m_dcmp_effect_group->SetOpacity(layer_info->is_visible);
            VP_ASSERT(hresult9 == S_OK);
        }
        m_update_settings.is_visible = layer_info->is_visible;

        /* Add layer to video manager */
        impl::NativeVideoManager *video_mgr = impl::NativeVideoManager::GetInstance();
        VP_ASSERT(video_mgr != nullptr);
        video_mgr->AddLayer(this);

        if (layer_info->parent_display != nullptr) {
            layer_info->parent_display->AddLayer(this);
            m_update_settings.parent_display = layer_info->parent_display;
        }

        m_settings = m_update_settings;

        if (layer_info->display_resize_function != nullptr) {
            m_resize_function.SetFunction(*layer_info->display_resize_function);
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
        if (m_dcmp_effect_group != nullptr) {
            m_dcmp_effect_group->Release();
            m_dcmp_effect_group = nullptr;
        }
		m_swapchain.Finalize();

		return;
	}

	void NativeLayer::RequestSetPosition(vp::util::Float2 pos) {
		std::scoped_lock l(m_settings_cs);
		if (m_update_settings.position.x == pos.x && m_update_settings.position.y == pos.y) { return; }
		m_is_update_position = true;
		m_update_settings.position = pos;
	}

	void NativeLayer::RequestSetZ(u32 z) {
		std::scoped_lock l(m_settings_cs);
		if (m_update_settings.z == z) { return; }
		m_is_update_parent = true;
		m_update_settings.z = z;
	}

	void NativeLayer::RequestSetSize(vp::util::Int2 size) {
        VP_UNUSED(size);
		std::scoped_lock l(m_settings_cs);
		m_swapchain.RequestSetSize(size);
		m_is_update_swapchain = true;
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
