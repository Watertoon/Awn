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

    class TextureBinder {
        public:
            DescriptorSlot m_texture_slot;
        public:
            constexpr ALWAYS_INLINE TextureBinder() : m_texture_slot(cInvalidDescriptorSlot) {/*...*/}

            TextureBinder &operator=(const TextureBinder &rhs) {
                this->BindTexture(rhs.m_texture_slot);
                return *this;
            }

            void BindTexture(TextureInfo *texture_info, TextureViewInfo *texture_view_info) {
                this->ReleaseTexture();
                m_texture_slot = TextureSamplerManager::GetInstance()->RegisterTextureView(texture_info, texture_view_info);
            }
            void BindTexture(void *texture_gpu_memory_address, TextureInfo *texture_info, TextureViewInfo *texture_view_info) {
                this->ReleaseTexture();
                m_texture_slot = TextureSamplerManager::GetInstance()->RegisterTextureView(texture_gpu_memory_address, texture_info, texture_view_info);
            }

            void BindTexture(DescriptorSlot texture_slot) {
                this->ReleaseTexture();
                m_texture_slot = TextureSamplerManager::GetInstance()->ReferenceTextureView(texture_slot);
            }

            void ReleaseTexture() {
                if (m_texture_slot != cInvalidDescriptorSlot) { TextureSamplerManager::GetInstance()->UnregisterTextureView(m_texture_slot); m_texture_slot = cInvalidDescriptorSlot; }
            }

            Texture *GetTexture() {
                return TextureSamplerManager::GetInstance()->TryGetTextureByHandle(m_texture_slot);
            }

            TextureView *GetTextureView() {
                return TextureSamplerManager::GetInstance()->TryGetTextureViewByHandle(m_texture_slot);
            }
    };

    class SamplerBinder {
        public:
            DescriptorSlot m_sampler_slot;
        public:
            constexpr ALWAYS_INLINE SamplerBinder() : m_sampler_slot(cInvalidDescriptorSlot) {/*...*/}
            
            SamplerBinder &operator=(const SamplerBinder &rhs) {
                this->BindSampler(rhs.m_sampler_slot);
                return *this;
            }

            void BindSampler(SamplerInfo *sampler_info) {
                this->ReleaseSampler();
                m_sampler_slot = TextureSamplerManager::GetInstance()->RegisterSampler(sampler_info);
            }

            void BindSampler(DescriptorSlot sampler_slot) {
                this->ReleaseSampler();
                m_sampler_slot = TextureSamplerManager::GetInstance()->ReferenceSampler(sampler_slot);
            }

            void ReleaseSampler() {
                if (m_sampler_slot != cInvalidDescriptorSlot) { TextureSamplerManager::GetInstance()->UnregisterSampler(m_sampler_slot); m_sampler_slot = cInvalidDescriptorSlot; }
            }

            Sampler *GetSampler() {
                return TextureSamplerManager::GetInstance()->TryGetSamplerByHandle(m_sampler_slot);
            }
    };

    struct TextureSamplerInfo {
        TextureInfo           *texture_info;
        TextureViewInfo       *texture_view_info;
        SamplerInfo           *sampler_info;
    };

    class TextureSamplerBinder {
        private:
            TextureBinder m_texture_binder;
            SamplerBinder m_sampler_binder;
        public:
            constexpr ALWAYS_INLINE TextureSamplerBinder() : m_texture_binder(), m_sampler_binder() {/*...*/}

            TextureSamplerBinder &operator=(const TextureSamplerBinder &rhs) {
                m_texture_binder = rhs.m_texture_binder;
                m_sampler_binder = rhs.m_sampler_binder;
                return *this;
            }

            void BindTextureSampler(TextureSamplerInfo *texture_sampler_info) {
                m_texture_binder.BindTexture(texture_sampler_info->texture_info, texture_sampler_info->texture_view_info);
                m_sampler_binder.BindSampler(texture_sampler_info->sampler_info);
            }
            void BindTextureSampler(void *texture_gpu_memory_address, TextureSamplerInfo *texture_sampler_info) {
                m_texture_binder.BindTexture(texture_gpu_memory_address, texture_sampler_info->texture_info, texture_sampler_info->texture_view_info);
                m_sampler_binder.BindSampler(texture_sampler_info->sampler_info);
            }

            void ReleaseTextureSampler() {
                m_texture_binder.ReleaseTexture();
                m_sampler_binder.ReleaseSampler();
            }
    };
}
