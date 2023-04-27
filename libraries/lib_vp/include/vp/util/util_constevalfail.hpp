#pragma once

namespace vp::util {

    /* This function acts as an unreachable in constant expression context */
    void _consteval_fail();
}
