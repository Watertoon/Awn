#include <awn.hpp>

namespace awn::sys {

    void ThreadBase::InternalThreadMain(void *arg) {

        /* Recover Thread object */
        ThreadBase *thread = reinterpret_cast<ThreadBase*>(arg);

        /* Run thread */
        thread->Run();

        /* Destruct thread tls */
        ThreadManager::GetInstance()->InvokeThreadTlsDestructors(thread);

        return;
    }
    long unsigned int ThreadBase::InternalServiceThreadMain(void *arg) {

        /* Recover Thread object */
        ThreadBase *thread = reinterpret_cast<ThreadBase*>(arg);

        /* Run thread */
        thread->Run();

        /* Destruct thread tls */
        ThreadManager::GetInstance()->InvokeThreadTlsDestructors(thread);

        return 0;
    }
    
}
