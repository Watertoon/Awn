#pragma once

namespace awn::gfx {

    /* Only valid for service threads */
    class Sync {
        private:
            VkSemaphore m_vk_timeline_semaphore;
            size_t      m_expected_value;
        public:
            constexpr Sync() : m_vk_timeline_semaphore(VK_NULL_HANDLE), m_expected_value(0) {/*...*/}

            void Initialize();
            void Finalize();

            void Wait();
            void TimedWait(TimeSpan timeout);

            void Signal();

            constexpr ALWAYS_INLINE void IncrementExpectedValue() { ++m_expected_value; }
            constexpr ALWAYS_INLINE u64  GetExpectedValue() const { return m_expected_value; }

            constexpr ALWAYS_INLINE VkSemaphore GetVkSemaphore() const { return m_vk_timeline_semaphore; }
    };
}
