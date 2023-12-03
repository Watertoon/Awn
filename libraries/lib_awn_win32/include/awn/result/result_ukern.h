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

    DECLARE_RESULT_MODULE(10);
    DECLARE_RESULT(Timeout,                      1);
    DECLARE_RESULT(InvalidHandle,                2);
    DECLARE_RESULT(Win32Error,                   3);
    DECLARE_RESULT(ThreadStorageExhaustion,      4);
    DECLARE_RESULT(HandleExhaustion,             5);
    DECLARE_RESULT(InvalidThreadFunctionPointer, 6);
    DECLARE_RESULT(InvalidStackSize,             7);
    DECLARE_RESULT(InvalidPriority,              8);
    DECLARE_RESULT(InvalidAddress,               9);
    DECLARE_RESULT(InvalidWaitAddressValue,      10);
    DECLARE_RESULT(InvalidLockAddressValue,      11);
    DECLARE_RESULT(RequiresLock,                 12);
    DECLARE_RESULT(ValueOutOfRange,              13);
    DECLARE_RESULT(SamePriority,                 14);
    DECLARE_RESULT(SameActivityLevel,            15);
    DECLARE_RESULT(SameCoreMask,                 16);
    DECLARE_RESULT(InvalidCoreId,                17);
    DECLARE_RESULT(InvalidArbitrationType,       18);
    DECLARE_RESULT(InvalidSignalType,            19);
    DECLARE_RESULT(NoWaiters,                    20);
    DECLARE_RESULT(InvalidCoreMask,              21);
}
