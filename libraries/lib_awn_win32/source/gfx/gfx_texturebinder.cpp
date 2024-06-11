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

    Result TextureBinder::RegisterTextureAsync(const TextureRegisterInfo *register_info) {

        /* Fail if we previously scheduled a register */
        RESULT_RETURN_IF(m_register_guard, ResultRegisterGuardSet);

        /* Clear previous state and set register guard */
        m_state  = (m_state & 0xf0) | 1;
        m_status = Status::Uninitialized;

        /* Defer to manager */
        return AsyncGpuResourceManager::GetInstance()->RegisterTextureAsync(this, register_info);
    }

    Result TextureBinder::RegisterTextureSync(const TextureRegisterInfo *register_info) {

        /* Fail if we previously scheduled a register */
        RESULT_RETURN_IF(m_register_guard, ResultRegisterGuardSet);

        /* Clear previous state and set register guard */
        m_state  = (m_state & 0xf0) | 1;
        m_status = Status::Uninitialized;

        /* Defer to manager */
        return AsyncGpuResourceManager::GetInstance()->RegisterTextureSync(this, register_info);
    }

    Result TextureBinder::ReferenceBinderSync(TextureBinder *reference_texture_binder) {

        /* Fail if we previously scheduled a register */
        RESULT_RETURN_IF(m_register_guard, ResultRegisterGuardSet);

        /* Clear previous state and set register guard */
        m_state  = (m_state & 0xf0) | 1;
        m_status = Status::Uninitialized;

        /* Set texture unit */
        TextureUnit *texture_unit = reference_texture_binder->m_texture_unit;
        RESULT_RETURN_IF(texture_unit == nullptr, ResultInvalidReferenceBinder);
        m_texture_unit = texture_unit;
        texture_unit->IncrementReferenceCount();

        /* Update status */
        m_status = Status::Registered;

        RESULT_RETURN_SUCCESS;
    }

    void TextureBinder::Release() {
        
        /* Nothing to do if a register has not occurred */
        if (m_register_guard == false) { return; }
        
        /* Reset state and set release guard */
        m_state = (m_state & 0xf8) | 0x4;

        /* Cancel watcher */
        if (m_watcher.HasTask() == true) {
            m_watcher.CancelTask();
        }

        /* Nothing else to do if no resource unit */
        if (m_texture_unit == nullptr) { m_status = Status::Cancelled; return; }

        /* Finalize binder impl */
        AsyncGpuResourceManager *manager = AsyncGpuResourceManager::GetInstance();
        manager->ReleaseTextureBinder(this);

        return;
    }

    void TextureBinder::WaitForRegister() {
        m_watcher.WaitForCompletion();
        if (m_texture_unit != nullptr) { m_texture_unit->WaitForTransfer(); }
    }
}
