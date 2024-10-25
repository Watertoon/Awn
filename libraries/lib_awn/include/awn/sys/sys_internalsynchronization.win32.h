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

namespace awn::sys::impl {

    static constexpr u32 cTagLockBitOffset = 0;
    static constexpr u32 cTagLockClearMask = 0xffff'fffe;
    static constexpr u32 cTagLockSetMask = 0x1;

    void ArbitrateLockImpl(u32 *address, u32 tag);
    void ArbitrateUnlockImpl(u32 *address);

    static constexpr u32 cWideKeySingleBitOffset = 0x1f;
    static constexpr u32 cWideKeySingleMask = 0x8000'0000;
    static constexpr u32 cWideKeyBroadcastMask = 0x7fff'ffff;

    void WaitProcessWideKeyImpl(u32 *address_cs, u32 *address_cv, u32 tag);
    void SignalWideKey(u32 *address_cv, bool is_all);

    struct InternalCriticalSectionImplByWin32 {
        u32 tag = 0;

        void Enter();
        void Leave();
        bool TryEnter();

        bool IsLockedByCurrentThread();
    };

    using InternalCriticalSection = InternalCriticalSectionImplByWin32;

    struct InternalConditionVariableImplByWin32 {
        u32 cv = 0;

        void Wait(InternalCriticalSection *cs);
        void Signal();
        void Broadcast();
    };

    using InternalConditionVariable = InternalConditionVariableImplByWin32;
}
