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

    class TextureBinder;

    class TextureViewBinder {
        public:
            static constexpr inline DescriptorSlot cInvalidDescriptorSlot = { 0, cInvalidDescriptorSlotFreeCount, };
        private:
            gfx::DescriptorSlot m_texture_slot;
        public:
            constexpr TextureViewBinder() : m_texture_slot() {/*...*/}
            ~TextureViewBinder() { this->Release(); }

            Result RegisterTextureView(const TextureViewInfo *texture_view_info);
            Result ReferenceTextureView(TextureViewBinder *reference_texture_view_binder);

            void Release();

            constexpr DescriptorSlot GetDescriptorSlot() const {
                impl::TextureViewSamplerManager *mgr = impl::TextureViewSamplerManager::GetInstance();
                if (!mgr->IsTextureViewSlotValid(m_texture_slot)) { return cInvalidDescriptorSlot; }
                return m_texture_slot; 
            }

            bool IsSameInfo(const TextureViewInfo *texture_view_info);
        public:
            constexpr const impl::TextureViewKey *GetTextureViewKey() const {
                impl::TextureViewSamplerManager *mgr = impl::TextureViewSamplerManager::GetInstance();
                if (!mgr->IsTextureViewSlotValid(m_texture_slot)) { return nullptr; }
                const impl::TextureViewKey *key = std::addressof(mgr->GetTextureViewUnit(m_texture_slot.index)->key);
                return key;
            }
            constexpr VkImageView GetVkImageView() const {
                impl::TextureViewSamplerManager *mgr = impl::TextureViewSamplerManager::GetInstance();
                if (!mgr->IsTextureViewSlotValid(m_texture_slot)) { return nullptr; }
                const VkImageView view = mgr->GetTextureViewUnit(m_texture_slot.index)->vk_image_view;
                return view;
            }
    };
}
