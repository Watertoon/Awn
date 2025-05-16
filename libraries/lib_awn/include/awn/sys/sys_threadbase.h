/*
 *  Copyright (C) W. Michael Knudson
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as 
 *  published by the Free Software Foundation.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along with this program; 
 *  if not, see <https://www.gnu.org/licenses/>.
 */
#pragma once

namespace awn::sys {
    
    enum class ThreadRunMode {
        WaitForMessage,
        Looping,
    };

    enum class ExitType {
        Jam,
        Send,
    };

    struct ThreadInfo {
        const char *name;
        size_t exit_code;
        ThreadRunMode run_mode;
        u32 message_count;
        size_t *message_memory;
        u32 stack_size;
#ifdef VP_TARGET_PLATFORM_nx
        void *stack_memory;
#endif
        u32 priority;
    };
    
    using TlsDestructor = void (*)(size_t);
    using TlsSlot       = u32;

    /* A higher value is a lower priority */
    constexpr inline u32 cMinPriority = 0x1f;
    constexpr inline u32 cMaxPriority = 0x0;

    constexpr inline u32 cPriorityHigh = 0x4;
    constexpr inline u32 cPriorityAboveNormal = 0xb;
    constexpr inline u32 cPriorityNormal = 0x10;
    constexpr inline u32 cPriorityBelowNormal = 0x15;
    constexpr inline u32 cPriorityLow = 0x1b;
    constexpr inline u32 cInvalidTlsSlot = 0xffff'ffff;

    class ThreadManager;

    class ThreadBase {
        public:
            static constexpr size_t cMaxThreadTlsSlotCount = 32;
        public:
            friend class sys::ThreadManager;
        protected:
            mem::Heap                   *m_thread_heap;
            mem::Heap                   *m_lookup_heap;
            sys::MessageQueue            m_message_queue;
            size_t                      *m_message_queue_memory;
            size_t                       m_exit_message;
            u32                          m_stack_size;
            u32                          m_priority;
            CoreMask                     m_core_mask;
            u32                          m_run_mode;
#ifdef VP_TARGET_PLATFORM_win32
            uintptr_t                    m_tls_slot_array[cMaxThreadTlsSlotCount];
#endif
            vp::util::IntrusiveListNode  m_thread_manager_list_node;
        public:
            VP_RTTI_BASE(ThreadBase);
        protected:
            static void InternalThreadMain(void *arg);
            static long unsigned int InternalServiceThreadMain(void *arg);
        public:
            virtual void Run() {

                /* Message loop */
                size_t current_message = static_cast<size_t>(-1);
                if (m_run_mode == static_cast<u32>(ThreadRunMode::WaitForMessage)) {
                    m_message_queue.ReceiveMessage(std::addressof(current_message));
                }
                while(current_message != m_exit_message) {
                    this->ThreadMain(current_message);
                    if (m_run_mode == static_cast<u32>(ThreadRunMode::WaitForMessage)) {
                        m_message_queue.ReceiveMessage(std::addressof(current_message));
                    } else {
                        if (!m_message_queue.TryReceiveMessage(std::addressof(current_message))) {
                            current_message = 0;
                        }
                    }
                }

                return;
            }

            virtual void ThreadMain([[maybe_unused]] size_t message) {/*...*/}
        public:
            constexpr ThreadBase(): m_thread_heap(), m_lookup_heap(), m_message_queue(), m_message_queue_memory(), m_exit_message(), m_stack_size(), m_priority(), m_core_mask(), m_run_mode(), 
            #ifdef VP_TARGET_PLATFORM_win32
                m_tls_slot_array(), 
            #endif
                m_thread_manager_list_node()
                {/*...*/}
            virtual ~ThreadBase() {/*...*/}

            void Initialize(mem::Heap *thread_heap, const ThreadInfo *thread_info);

            virtual void StartThread() {/*...*/}
            virtual void ExitThread(ExitType exit_type);
            virtual void WaitForThreadExit() {/*...*/}
            virtual void ResumeThread() {/*...*/}
            virtual void SuspendThread() {/*...*/}
            virtual void SleepThread([[maybe_unused]] vp::TimeSpan timeout_ns) {/*...*/}
            virtual void YieldThread() {/*...*/}

            virtual void SetPriority([[maybe_unused]] u32 priority)  {/*...*/}
            virtual void SetCoreMask([[maybe_unused]] CoreMask core_mask) {/*...*/}

            constexpr ALWAYS_INLINE u32 GetPriority() const {
                return m_priority;
            }
            constexpr ALWAYS_INLINE u64 GetCoreMask() const {
                return m_core_mask;
            }

            ALWAYS_INLINE void SendMessage(size_t message) { m_message_queue.SendMessage(message); }
            ALWAYS_INLINE void JamMessage(size_t message)  { m_message_queue.JamMessage(message); }
            ALWAYS_INLINE void ReceiveMessage(size_t *out_message)  { m_message_queue.ReceiveMessage(out_message); }
            ALWAYS_INLINE bool TrySendMessage(size_t message) { return m_message_queue.TrySendMessage(message); }
            ALWAYS_INLINE bool TryJamMessage(size_t message) { return m_message_queue.TryJamMessage(message); }
            ALWAYS_INLINE bool TryReceiveMessage(size_t *out_message) { return m_message_queue.TryReceiveMessage(out_message); }
            ALWAYS_INLINE bool TryPeekMessage(size_t *out_message) { return m_message_queue.TryPeekMessage(out_message); }

            constexpr ALWAYS_INLINE void SetThreadCurrentHeap(mem::Heap *heap) { m_thread_heap = heap; }
            constexpr ALWAYS_INLINE void SetLookupHeap(mem::Heap *heap)        { m_lookup_heap = heap; }

            constexpr ALWAYS_INLINE       mem::Heap *GetThreadHeap()       { return m_thread_heap; }
            constexpr ALWAYS_INLINE const mem::Heap *GetThreadHeap() const { return m_thread_heap; }

            constexpr ALWAYS_INLINE       mem::Heap *GetLookupHeap()       { return m_lookup_heap; }
            constexpr ALWAYS_INLINE const mem::Heap *GetLookupHeap() const { return m_lookup_heap; }

            ALWAYS_INLINE void SendExitMessage() { m_message_queue.SendMessage(m_exit_message); }
            constexpr ALWAYS_INLINE size_t GetExitMessage() const { return m_exit_message; }

#ifdef VP_TARGET_PLATFORM_win32
            constexpr ALWAYS_INLINE void SetTlsValue(TlsSlot slot, uintptr_t object) {
                VP_ASSERT(slot < cMaxThreadTlsSlotCount);
                m_tls_slot_array[slot] = object;
            }
            constexpr ALWAYS_INLINE uintptr_t GetTlsValue(TlsSlot slot) {
                VP_ASSERT(slot < cMaxThreadTlsSlotCount);
               return m_tls_slot_array[slot];
            }
#elifdef VP_TARGET_PLATFORM_nx
            ALWAYS_INLINE void SetTlsValue(TlsSlot slot, uintptr_t object) {
                nn::os::SetTlsValue(slot, reinterpret_cast<size_t>(object));
            }
            ALWAYS_INLINE uintptr_t GetTlsValue(TlsSlot slot) {
                return nn::os::GetTlsValue(slot);
            }
#endif
    };
}
