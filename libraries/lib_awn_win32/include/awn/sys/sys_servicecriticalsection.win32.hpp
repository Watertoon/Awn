#pragma once

namespace awn::sys {

    class ServiceCriticalSection {
        private:
            u32         m_wait_value;
            SRWLOCK     m_win32_lock;
        public:
            constexpr ALWAYS_INLINE ServiceCriticalSection() : m_wait_value(0), m_win32_lock(0) {/*...*/}
            constexpr ALWAYS_INLINE ~ServiceCriticalSection() {/*...*/}

            void Enter() {

                if (::IsThreadAFiber() == false) {
                    ::AcquireSRWLockExclusive(std::addressof(m_win32_lock));
                    return;
                }

                for(;;) {
                    /* Try to lock the Win32 SRWLock */
                    const bool result0 = ::TryAcquireSRWLockExclusive(std::addressof(m_win32_lock));

                    /* Success */
                    if (result0 == true) { return; }

                    /* Fallback wait */
                    ukern::WaitOnAddress(reinterpret_cast<uintptr_t>(std::addressof(m_wait_value)), ukern::ArbitrationType_WaitIfEqual, 0, -1);
                }
            }

            void Leave() {
                ::ReleaseSRWLockExclusive(std::addressof(m_win32_lock));
                ukern::WakeByAddress(reinterpret_cast<uintptr_t>(std::addressof(m_wait_value)), ukern::SignalType_Signal, 0, 1);
            }

            ALWAYS_INLINE void lock()   { this->Enter(); }
            ALWAYS_INLINE void unlock() { this->Leave(); }
    };

    class ServiceMutex {
        private:
            u32              m_wait_value;
            u32              m_lock_count;
            CRITICAL_SECTION m_win32_lock;
        public:
            constexpr ALWAYS_INLINE  ServiceMutex() : m_wait_value(0), m_lock_count(0), m_win32_lock(0) {/*...*/}
            constexpr ALWAYS_INLINE ~ServiceMutex() {/*...*/}

            void Initialize() {
                ::InitializeCriticalSection(std::addressof(m_win32_lock));
            }
            void Finalize() {
                ::DeleteCriticalSection(std::addressof(m_win32_lock));
            }

            void Enter() {

                if (::IsThreadAFiber() == false) {
                    ::EnterCriticalSection(std::addressof(m_win32_lock));
                    ++m_lock_count;
                    return;
                }

                for(;;) {
                    /* Try to lock the Win32 SRWLock */
                    const bool result0 = ::TryEnterCriticalSection(std::addressof(m_win32_lock));

                    /* Success */
                    if (result0 == true) {
                        ++m_lock_count;
                        return; 
                    }

                    /* Fallback wait */
                    ukern::WaitOnAddress(reinterpret_cast<uintptr_t>(std::addressof(m_wait_value)), ukern::ArbitrationType_WaitIfEqual, 0, -1);
                }
            }

            void Leave() {
                --m_lock_count;
                const u32 lock_count = m_lock_count;
                ::LeaveCriticalSection(std::addressof(m_win32_lock));
                if (lock_count == 0) {
                    ukern::WakeByAddress(reinterpret_cast<uintptr_t>(std::addressof(m_wait_value)), ukern::SignalType_Signal, 0, 1);
                }
            }

            ALWAYS_INLINE void lock()   { this->Enter(); }
            ALWAYS_INLINE void unlock() { this->Leave(); }
    };
}
