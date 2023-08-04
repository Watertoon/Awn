#include <awn.hpp>

namespace awn::sys {

    void SleepThread(vp::TimeSpan timeout_ns) {
        sys::ThreadManager::GetInstance()->GetCurrentThread()->SleepThread(timeout_ns);
    }
}
