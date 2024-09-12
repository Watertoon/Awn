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

	class ContextDxgi {
        public:
            static constexpr D3D_FEATURE_LEVEL cDirectXFeatureLevel = D3D_FEATURE_LEVEL_12_1;
		private:
            IDXGIFactory7              *m_dxgi_factory;
            IDXGIAdapter4              *m_dxgi_adapter;
            IDCompositionDesktopDevice *m_dcmp_device;
            bool                        m_is_tearing;
            ID3D12Device8              *m_d3d12_device;
            ID3D12CommandQueue         *m_d3d12_command_queue;
#ifdef AWN_DXGI_DEBUG
            IDXGIDebug1                *m_dxgi_debug;
            ID3D12Debug                *m_d3d12_debug;
            ID3D12DebugDevice1         *m_d3d12_debug_device;
#endif
        public:
            AWN_SINGLETON_TRAITS(ContextDxgi);
        public:
            constexpr ContextDxgi() : m_dxgi_factory(), m_dxgi_adapter(),
            m_dcmp_device(), m_is_tearing(), 
            m_d3d12_device(), m_d3d12_command_queue()
#ifdef AWN_DXGI_DEBUG
            , m_dxgi_debug(), m_d3d12_debug(), m_d3d12_debug_device()
#endif
            {/*...*/}
            constexpr ~ContextDxgi() {/*...*/}

            void Initialize();
            void Finalize();

            constexpr bool IsTearing() const { return m_is_tearing; }
            constexpr IDXGIFactory7 *GetDxgiFactory() const { return m_dxgi_factory; }
            constexpr IDXGIAdapter4 *GetDxgiAdapter() const { return m_dxgi_adapter; }
            constexpr ID3D12Device8 *GetD3d12Device() const { return m_d3d12_device; }
            constexpr ID3D12CommandQueue *GetD3d12CommandQueue() const { return m_d3d12_command_queue; }
            constexpr IDCompositionDesktopDevice *GetDcmpDevice() const { return m_dcmp_device; }

            void ReportDebug() {
#ifdef AWN_DXGI_DEBUG
                if (m_dxgi_debug != nullptr) {
                    m_dxgi_debug->ReportLiveObjects(DXGI_DEBUG_DXGI, DXGI_DEBUG_RLO_DETAIL);
                }
                if (m_d3d12_debug_device != nullptr) {
                    m_d3d12_debug_device->ReportLiveDeviceObjects(D3D12_RLDO_DETAIL);
                }
#endif
            }
	};
}
