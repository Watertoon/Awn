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

    Result BufferBinder::RegisterBufferAsync(const BufferRegisterInfo *register_info) {

        /* Fail if we previously scheduled a register */
        RESULT_RETURN_IF(m_register_guard, ResultRegisterGuardSet);

        /* Clear previous state and set register guard */
        m_state  = (m_state & 0xf0) | 1;
        m_status = Status::Uninitialized;

        /* Defer to manager */
        return impl::AsyncGpuResourceManager::GetInstance()->RegisterBufferAsync(this, register_info);
    }

    Result BufferBinder::RegisterBufferSync(const BufferRegisterInfo *register_info) {

        /* Fail if we previously scheduled a register */
        RESULT_RETURN_IF(m_register_guard, ResultRegisterGuardSet);

        /* Clear previous state and set register guard */
        m_state  = (m_state & 0xf0) | 1;
        m_status = Status::Uninitialized;

        /* Defer to manager */
        return impl::AsyncGpuResourceManager::GetInstance()->RegisterBufferSync(this, register_info);
    }

    Result BufferBinder::ReferenceBinderSync(BufferBinder *reference_buffer_binder) {

        /* Fail if we previously scheduled a register */
        RESULT_RETURN_IF(m_register_guard, ResultRegisterGuardSet);

        /* Clear previous state and set register guard */
        m_state  = (m_state & 0xf0) | 1;
        m_status = Status::Uninitialized;

        /* Set buffer unit */
        impl::BufferUnit *buffer_unit = reference_buffer_binder->m_buffer_unit;
        RESULT_RETURN_IF(buffer_unit == nullptr, ResultInvalidReferenceBinder);
        m_buffer_unit = buffer_unit;
        buffer_unit->IncrementReferenceCount();

        /* Update status */
        m_status = Status::Registered;

        RESULT_RETURN_SUCCESS;
    }

    void BufferBinder::Release() {
        
        /* Nothing to do if a register has not occurred */
        if (m_register_guard == false) { return; }
        
        /* Reset state and set release guard */
        m_state = (m_state & 0xf8) | 0x4;

        /* Cancel watcher */
        if (m_watcher.HasTask() == true) {
            m_watcher.CancelTask();
        }

        /* Nothing else to do if no resource unit */
        if (m_buffer_unit == nullptr) { m_status = Status::Cancelled; return; }

        /* Finalize binder impl */
        impl::AsyncGpuResourceManager *manager = impl::AsyncGpuResourceManager::GetInstance();
        manager->ReleaseBufferBinder(this);

        return;
    }

    void BufferBinder::WaitForRegister() {
        m_watcher.WaitForCompletion();
        if (m_buffer_unit != nullptr) { m_buffer_unit->WaitForTransfer(); }
    }
}
