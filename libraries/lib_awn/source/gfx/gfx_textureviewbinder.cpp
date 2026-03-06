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

    Result TextureViewBinder::RegisterTextureView(const TextureViewInfo *texture_view_info) {
        RESULT_RETURN_IF(m_texture_slot.free_count != cInvalidDescriptorSlotFreeCount, ResultRegisterGuardSet);
        return impl::TextureViewSamplerManager::GetInstance()->RegisterTextureView(std::addressof(m_texture_slot), texture_view_info);
    }

    Result TextureViewBinder::ReferenceTextureView(TextureViewBinder *reference_texture_view_binder) {
        RESULT_RETURN_IF(m_texture_slot.free_count != cInvalidDescriptorSlotFreeCount, ResultRegisterGuardSet);
        return impl::TextureViewSamplerManager::GetInstance()->ReferenceTextureView(std::addressof(m_texture_slot), reference_texture_view_binder->m_texture_slot);
    }

    void TextureViewBinder::Release() {
        impl::TextureViewSamplerManager::GetInstance()->ReleaseTextureView(m_texture_slot);
        m_texture_slot.free_count = cInvalidDescriptorSlotFreeCount;
        return;
    }
}
