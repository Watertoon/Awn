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

namespace awn::gfx {

    Result SamplerBinder::RegisterSampler(const SamplerInfo *sampler_info) {
        RESULT_RETURN_IF(m_sampler_slot.free_count != cInvalidDescriptorSlotFreeCount, ResultRegisterGuardSet);
        return impl::TextureViewSamplerManager::GetInstance()->RegisterSampler(std::addressof(m_sampler_slot), sampler_info);
    }

    Result SamplerBinder::ReferenceSampler(SamplerBinder *reference_sampler_binder) {
        return impl::TextureViewSamplerManager::GetInstance()->ReferenceSampler(std::addressof(m_sampler_slot), reference_sampler_binder->m_sampler_slot);
    }

    void SamplerBinder::Release() {
        impl::TextureViewSamplerManager::GetInstance()->ReleaseSampler(m_sampler_slot);
        m_sampler_slot.free_count = cInvalidDescriptorSlotFreeCount;
        return;
    }
}
