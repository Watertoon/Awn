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

namespace awn::gfx {

    class SamplerBinder {
        private:
            gfx::DescriptorSlot m_sampler_slot;
        public:
            constexpr SamplerBinder() : m_sampler_slot() {/*...*/}
            ~SamplerBinder() { this->Release(); }

            Result RegisterSampler(const SamplerInfo *sampler_info);
            Result ReferenceSampler(SamplerBinder *reference_sampler_binder);

            void Release();

            constexpr DescriptorSlot GetDescriptorSlot() const {
                impl::TextureViewSamplerManager *tex_view_samp_mgr = impl::TextureViewSamplerManager::GetInstance();
                if (!tex_view_samp_mgr->IsSamplerSlotValid(m_sampler_slot)) { return DescriptorSlot{}; }
                return m_sampler_slot; 
            }
    };
}
