#pragma once

namespace awn::sys {

    sys::ThreadBase *GetCurrentThread();

    void SleepThread(vp::TimeSpan timeout_ns);
}
