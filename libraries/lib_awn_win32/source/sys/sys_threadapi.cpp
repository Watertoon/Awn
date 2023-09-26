#include <awn.hpp>

namespace awn::sys {

    sys::ThreadBase *GetCurrentThread() {
        return sys::ThreadManager::GetInstance()->GetCurrentThread();
    }

    void SleepThread(vp::TimeSpan timeout_ns) {
        sys::ThreadManager::GetInstance()->GetCurrentThread()->SleepThread(timeout_ns);
    }
}
