#pragma once

namespace awn::sys {

    class SystemEvent {
        private:
            HANDLE m_handle;
        public:
            constexpr SystemEvent() : m_handle() {/*...*/}
            constexpr ~SystemEvent() {/*...*/}

            void Initialize(ResetMode reset_mode, bool is_interprocess) {
                VP_ASSERT(m_handle == nullptr);
                if (is_interprocess) {
                    VP_NOT_IMPLEMENTED();
                }
                const bool is_manual = (reset_mode == ResetMode::Manual);
                m_handle = ::CreateEventA(nullptr, is_manual, false, nullptr);
                return;
            }

            void Finalize() {
                if (m_handle != nullptr) {
                    ::CloseHandle(m_handle);
                    m_handle = nullptr;
                }
            }

            void Wait() {
                const u32 result = ::WaitForSingleObjectEx(m_handle, INFINITE, false);
                VP_ASSERT(result != WAIT_FAILED && result != WAIT_TIMEOUT);
                return;
            }

            void Signal() {
                const bool result = ::SetEvent(m_handle);
                VP_ASSERT(result);
                //if (m_multi_wait_node.IsLinked()) {
                //    m_holder->Notify();
                //}
            }

            void Clear() {
                const bool result = ::ResetEvent(m_handle);
                VP_ASSERT(result);
            }
        public:
            constexpr void SetNativeHandle(sys::NativeHandle native_handle) { m_handle = native_handle; }
            constexpr sys::NativeHandle GetNativeHandle() const { return m_handle; }
    };
}
