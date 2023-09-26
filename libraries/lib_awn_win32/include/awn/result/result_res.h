#pragma once

namespace awn::res {

    DECLARE_RESULT_MODULE(14);
    DECLARE_RESULT(UnknownOSError,            1);
    DECLARE_RESULT(FileNotFound,              2);
    DECLARE_RESULT(PathNotFound,              3);
    DECLARE_RESULT(PathTooLong,               4);
    DECLARE_RESULT(FileSharingViolation,      5);
    DECLARE_RESULT(FileLockViolation,         6);
    DECLARE_RESULT(OpenFileExhaustion,        7);
    DECLARE_RESULT(InvalidBufferSize,         8);
    DECLARE_RESULT(InvalidReadDivAlignment,   9);
    DECLARE_RESULT(InvalidFileBufferSize,     10);
    DECLARE_RESULT(InvalidHandle,             11);
    DECLARE_RESULT(InvalidNullPath,           12);
    DECLARE_RESULT(InvalidOpenMode,           13);
    DECLARE_RESULT(InvalidSize,               14);
    DECLARE_RESULT(NullHandle,                15);
    DECLARE_RESULT(NullOutBuffer,             16);
    DECLARE_RESULT(NullPath,                  17);
    DECLARE_RESULT(FileSizeRetrievalFailed,   18);
    DECLARE_RESULT(ResourceAllocationFailed,  19);
    DECLARE_RESULT(DirectoryNotFound,         20);
    DECLARE_RESULT(DirectoryExhausted,        21);
    DECLARE_RESULT(FailedToOpenFile,          22);
    DECLARE_RESULT(ExhaustedDirectoryDepth,   23);
}
