#pragma once

namespace vp::image {

    DECLARE_RESULT_MODULE(3);
    DECLARE_RESULT(UnknownPngError,       1);
    DECLARE_RESULT(OutOfMemory,           2);
    DECLARE_RESULT(InvalidReadPosition,   3);
    DECLARE_RESULT(InvalidWritePosition,  4);
    DECLARE_RESULT(FailedToAnalyzeImage,  5);
}
