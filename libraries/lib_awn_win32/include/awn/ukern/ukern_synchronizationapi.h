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

namespace awn::ukern {

    enum  ArbitrationType {
        ArbitrationType_WaitIfLessThan,
        ArbitrationType_DecrementWaitIfLessThan,
        ArbitrationType_WaitIfEqual,
    };
    enum  SignalType {
        SignalType_Signal,
        SignalType_SignalAndIncrementIfEqual,
        SignalType_SignalAndModifyByWaiterCountIfEqual
    };

    Result ArbitrateLock(UKernHandle handle, uintptr_t address, u32 tag);
    Result ArbitrateUnlock(uintptr_t address);

    Result WaitKey(uintptr_t address, uintptr_t cv_key, u32 tag, s64 timeout_ns);
    Result SignalKey(uintptr_t cv_key, u32 count);

    Result WaitOnAddress(uintptr_t address, u32 arbitration_type, u32 value, s64 timeout_ns);
    Result WakeByAddress(uintptr_t address, u32 signal_type, u32 value, u32 count);
}
