#pragma once

namespace awn::sys {

    class ConditionVariable {
        private:
            ukern::InternalConditionVariable m_cond_var;
        public:
            constexpr  ConditionVariable() : m_cond_var() {/*...*/}
            constexpr ~ConditionVariable() {/*...*/}

            void Wait(Mutex *mutex) {
                mutex->m_lock_count = mutex->m_lock_count - 1;
                m_cond_var.Wait(std::addressof(mutex->m_internal_cs));
                mutex->m_lock_count = mutex->m_lock_count + 1;
            }
            void TimedWait(Mutex *mutex, TimeSpan timeout) {
                mutex->m_lock_count = mutex->m_lock_count - 1;
                m_cond_var.TimedWait(std::addressof(mutex->m_internal_cs), timeout);
                mutex->m_lock_count = mutex->m_lock_count + 1;
            }
            void Signal() {
                m_cond_var.Signal();
            }
            void Broadcast() {
                m_cond_var.Broadcast();
            }
    };
}
