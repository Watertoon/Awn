#pragma once

namespace awn::frm {

    DECLARE_RESULT_MODULE(17);
    DECLARE_RESULT(FailedToAllocateRootHeap,         1);
    DECLARE_RESULT(FailedToInitializeMemHeapManager, 2);
    DECLARE_RESULT(FailedToInitializeWindow,         3);
    DECLARE_RESULT(FailedToInitializeGfxContext,     4);
}
