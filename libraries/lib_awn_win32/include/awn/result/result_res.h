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

namespace awn::res {

    DECLARE_RESULT_MODULE(14);
    DECLARE_RESULT(UnknownOSError,             1);
    DECLARE_RESULT(FileNotFound,               2);
    DECLARE_RESULT(PathNotFound,               3);
    DECLARE_RESULT(PathTooLong,                4);
    DECLARE_RESULT(FileSharingViolation,       5);
    DECLARE_RESULT(FileLockViolation,          6);
    DECLARE_RESULT(OpenFileExhaustion,         7);
    DECLARE_RESULT(InvalidBufferSize,          8);
    DECLARE_RESULT(InvalidReadDivAlignment,    9);
    DECLARE_RESULT(InvalidFileBufferSize,      10);
    DECLARE_RESULT(InvalidFileHandle,          11);
    DECLARE_RESULT(InvalidNullPath,            12);
    DECLARE_RESULT(InvalidOpenMode,            13);
    DECLARE_RESULT(InvalidSize,                14);
    DECLARE_RESULT(NullFileHandle,             15);
    DECLARE_RESULT(NullOutBuffer,              16);
    DECLARE_RESULT(NullPath,                   17);
    DECLARE_RESULT(FileSizeRetrievalFailed,    18);
    DECLARE_RESULT(ResourceAllocationFailed,   19);
    DECLARE_RESULT(DirectoryNotFound,          20);
    DECLARE_RESULT(DirectoryExhausted,         21);
    DECLARE_RESULT(FailedToOpenFile,           22);
    DECLARE_RESULT(ExhaustedDirectoryDepth,    23);
    DECLARE_RESULT(FailedToAllocateFileMemory, 24);
    DECLARE_RESULT(InvalidDecompressor,        25);
    DECLARE_RESULT(InvalidFileOffset,          26);
    DECLARE_RESULT(NullDirectoryHandle,        27);
    DECLARE_RESULT(InvalidFile,                28);
    DECLARE_RESULT(NullOutputResource,         29);
    DECLARE_RESULT(NullResourceFactory,        30);
}
