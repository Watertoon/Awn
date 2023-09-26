#pragma once

namespace awn::async {
    
    DECLARE_RESULT_MODULE(13);
    DECLARE_RESULT(Incomplete,      1);
    DECLARE_RESULT(AlreadyQueued,   2);
    DECLARE_RESULT(InvalidPriority, 3);
}
