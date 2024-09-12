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

namespace awn::sys {

    u32 GetCurrentThreadId();
    sys::ThreadBase *GetCurrentThread();

    void SleepThread(vp::TimeSpan timeout_ns);
    
    TlsSlot AllocateTlsSlot(TlsDestructor destructor);
    void FreeTlsSlot(TlsSlot slot);

    uintptr_t GetTlsValue(TlsSlot slot);
    void SetTlsValue(TlsSlot slot, uintptr_t data);

    bool IsThreadValid(sys::ThreadBase *thread);
}
