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

namespace awn::gfx::impl {

    enum class TextureConversionMode : u32 {
        None             = 0,
        TilingConversion = 1,
        AstcDecode       = 2,
    };

    struct TextureUnitInfo {
        const TextureInfo      *texture_info;
        const void            **import_mip_array;
        GpuMemoryRequirements   memory_requirements;
        MemoryPropertyFlags     memory_property_flags;
        u32                     priority;
    };

    class TextureUnit {
        public:
            friend class AsyncGpuResourceManager;
            friend class gfx::TextureBinder;
        public:
            static constexpr u32 cSmallMemorySize = vp::util::c4KB;
        public:
            enum class Status : u32 {
                Uninitialized    = 0,
                Freed            = 1,
                InRegister       = 2,
                Registered       = 3,
                Error            = 4,
            };
        private:
            Status                 m_status;
            union {
                u32 m_state;
                struct {
                    u32 m_is_fail_create_api_resource  : 1;
                    u32 m_is_fail_out_of_memory        : 1;
                    u32 m_is_fail_memory_allocation    : 1;
                    u32 m_is_fail_memory_copy          : 1;
                    u32 m_texture_conversion_mode      : 2;
                    u32 m_reserve                      : 26;
                };
            };
            u32                    m_reference_count;
            u32                    m_priority;
            MemoryPropertyFlags    m_memory_property_flags;
            mem::GpuMemoryHandle   m_memory_handle;
            GpuMemoryRequirements  m_memory_requirements;
            async::AsyncTask       m_transfer_task;
            gfx::Texture           m_texture;
            const void            *m_import_mip_array[Context::cTargetMaxMipLevelCount];
        private:
            bool InitializeForPlatform(const TextureUnitInfo *texture_unit_info);
            bool TransferForPlatform();
        public:
            TextureUnit() : m_status(), m_state(), m_reference_count(), m_priority(), m_memory_property_flags(), m_memory_handle(), m_memory_requirements{}, m_transfer_task(), m_texture(), m_import_mip_array{} {/*...*/}
            ~TextureUnit() {/*...*/}

            bool Initialize(const TextureUnitInfo *texture_unit_info);
            void Finalize();

            void RequestFinalize();
            bool RequestTransfer();

            constexpr void SetBinderStatusFromError(TextureBinder *texture_binder) {
                if (m_is_fail_create_api_resource) { texture_binder->m_status = TextureBinder::Status::CreateApiResourceError; }
                if (m_is_fail_out_of_memory)       { texture_binder->m_status = TextureBinder::Status::OutOfMemoryError; }
                if (m_is_fail_memory_allocation)   { texture_binder->m_status = TextureBinder::Status::MemoryAllocationError; }
                if (m_is_fail_memory_copy)         { texture_binder->m_status = TextureBinder::Status::CopyMemoryError; }
                return;
            }
        public:
            void Transfer();

            void WaitForTransfer();
        public:
            constexpr gfx::Texture *GetTexture() { return (this->IsRegistered()) ? std::addressof(m_texture) : nullptr; }

            constexpr u32 IncrementReferenceCount() { return ++m_reference_count; }
            constexpr u32 DecrementReferenceCount() { return --m_reference_count; }

            constexpr bool IsInRegister() const {
                if (m_transfer_task.IsCancelled()) { return true; }
                if (m_transfer_task.IsComplete()) { return (m_status == Status::InRegister); }
                return false;
            }
            constexpr bool IsRegistered() const { return m_status == Status::Registered; }
            constexpr bool IsErrorStatus() const { return m_status == Status::Error; }
    };
}

namespace awn::gfx {

    constexpr bool TextureBinder::Complete() {
        /* Succeed if we are already complete */
        if (m_complete_guard) { return true; }
        /* Fail if we did not schedule a register, or have been released/completed since */
        if ((m_state & 0x7) != 1) { return false; }
        /* Fail if the memory task is still pending */
        if (m_watcher.IsPending()) { return false; }
        /* Fail if the texture unit is still in register */
        if (m_texture_unit != nullptr && m_texture_unit->IsInRegister()) { return false; }
        /* Succeed if the memory task got cancelled */
        if (m_texture_unit == nullptr && m_watcher.IsCancelled()) { 
            m_status = Status::Cancelled; 
            return true; 
        }
        /* Succeed if the binder has an error result */
        if (this->IsErrorStatus()) { return true; }
        /* Fail if the memory task did not set a texture unit */
        if (m_texture_unit == nullptr || m_status == Status::Uninitialized) { return false; }

        /* Update completion from TextureUnit status */
        switch(m_texture_unit->m_status) {
            case impl::TextureUnit::Status::Error:
            {
                /* Set error result */
                m_texture_unit->SetBinderStatusFromError(this);
                break;
            }
            case impl::TextureUnit::Status::Registered:
                break;
            default:
                return false;
        };

        /* Complete */
        m_complete_guard = true;

        return true;
    }

    constexpr Texture *TextureBinder::GetTexture() {
        /* Fail if we aren't complete */
        if (!m_complete_guard) { return nullptr; }
        /* Fail if we completed without a texture unit */
        if (m_texture_unit == nullptr) { return nullptr; }
        /* Fail if an error occurred */
        if (this->IsErrorStatus()) { return nullptr; }
        /* Defer to texture unit */
        return m_texture_unit->GetTexture();
    }

    constexpr bool TextureBinder::IsInRegister() const {
        /* Succeed if our memory task is pending */
        if (m_watcher.IsPending()) { return true; }
        /* Fail we did not schedule a register */
        if (!m_register_guard) { return false; }
        /* Fail if we do not have a texture unit after the memory task */
        if (m_texture_unit == nullptr) { return false; }
        /* Defer to the texture unit */
        return m_texture_unit->IsInRegister();
    }

    constexpr bool TextureBinder::IsRegistered() const {
        /* Fail if we did not schedule a register */
        if (!m_register_guard) { return false; }
        /* Fail if the memory task is still pending */
        if (m_watcher.IsPending()) { return false; }
        /* Fail if the memory task did not set a texture unit */
        if (m_texture_unit == nullptr) { return false; }
        /* Fail if we are not complete */
        if (!m_complete_guard) { return false; }
        /* Defer to the texture unit */
        return m_texture_unit->IsRegistered();
    }

    constexpr bool TextureBinder::IsFailed() const {
        /* Check if we are in register*/
        if (this->IsInRegister()) { return false; }
        /* If we have a texture unit, see if it's failed */
        bool is_unit_failed = false;
        if (m_register_guard && m_texture_unit != nullptr) {
            is_unit_failed = m_texture_unit->IsErrorStatus();
        }
        return this->IsErrorStatus() | is_unit_failed;
    }

    constexpr bool TextureBinder::IsErrorStatus() const {
        return (static_cast<u32>(Status::CreateApiResourceError) <= static_cast<u32>(m_status));
    }
}
