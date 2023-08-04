#pragma once

namespace awn::sys {
    
    enum class ThreadRunMode {
        WaitForMessage,
        Looping,
    };

    using TlsDestructor = void (*)(void*);
    using TlsSlot       = u32;

    class ThreadManager;

    class ThreadBase {
        public:
            static constexpr size_t cMaxThreadTlsSlotCount = 32;
        public:
            friend class sys::ThreadManager;
        protected:
            mem::Heap                   *m_thread_heap;
            mem::Heap                   *m_lookup_heap;
            sys::ServiceMessageQueue     m_message_queue;
            size_t                      *m_message_queue_buffer;
            size_t                       m_exit_message;
            u32                          m_stack_size;
            s32                          m_priority;
            u64                          m_core_mask;
            u32                          m_run_mode;
            void                        *m_tls_slot_array[cMaxThreadTlsSlotCount];
            vp::util::IntrusiveListNode  m_thread_manager_list_node;
        protected:
            static void InternalThreadMain(void *arg);
            static long unsigned int InternalServiceThreadMain(void *arg);
        public:
            virtual void Run() {
                size_t current_message = static_cast<size_t>(-1);

                if (m_run_mode == static_cast<u32>(ThreadRunMode::WaitForMessage)) {
                    m_message_queue.ReceiveMessage(std::addressof(current_message));
                }

                while(current_message != m_exit_message) {
                    this->ThreadMain(current_message);
                    if (m_run_mode == static_cast<u32>(ThreadRunMode::WaitForMessage)) {
                        m_message_queue.ReceiveMessage(std::addressof(current_message));
                    } else {
                        if (m_message_queue.TryReceiveMessage(std::addressof(current_message)) == false) {
                            current_message = 0;
                        }
                    }
                }
            }

            virtual void ThreadMain([[maybe_unused]] size_t message) {/*...*/}
        public:
            ALWAYS_INLINE ThreadBase(mem::Heap *thread_heap, ThreadRunMode run_mode, size_t exit_code, u32 max_messages , u32 stack_size, s32 priority) : m_thread_heap(thread_heap), m_lookup_heap(nullptr), m_message_queue(), m_message_queue_buffer(), m_exit_message(exit_code), m_stack_size(stack_size), m_priority(priority), m_core_mask(), m_run_mode(static_cast<u32>(run_mode)), m_tls_slot_array(), m_thread_manager_list_node() {
                m_message_queue.Initialize(thread_heap, max_messages);
            }
            ALWAYS_INLINE ThreadBase(mem::Heap *thread_heap) : m_thread_heap(thread_heap), m_lookup_heap(nullptr) {
            }

            virtual ~ThreadBase() {
                this->WaitForThreadExit();
                m_message_queue.Finalize();
            }

            virtual void StartThread()                    {/*...*/}
            virtual void WaitForThreadExit()              {/*...*/}
            virtual void ResumeThread()                   {/*...*/}
            virtual void SuspendThread()                  {/*...*/}
            virtual void SleepThread(vp::TimeSpan timeout_ns) {/*...*/}

            virtual void SetPriority([[maybe_unused]] s32 priority)  {/*...*/}
            virtual void SetCoreMask([[maybe_unused]] u64 core_mask) {/*...*/}

            constexpr ALWAYS_INLINE s32 GetPriority() const {
                return m_priority;
            }
            constexpr ALWAYS_INLINE u64 GetCoreMask() const {
                return m_core_mask;
            }

            void SendMessage(size_t message) { m_message_queue.SendMessage(message); }

            constexpr ALWAYS_INLINE void SetThreadCurrentHeap(mem::Heap *heap) { m_thread_heap = heap; }
            constexpr ALWAYS_INLINE void SetLookupHeap(mem::Heap *heap)        { m_lookup_heap = heap; }

            constexpr ALWAYS_INLINE       mem::Heap *GetThreadHeap()       { return m_thread_heap; }
            constexpr ALWAYS_INLINE const mem::Heap *GetThreadHeap() const { return m_thread_heap; }

            constexpr ALWAYS_INLINE       mem::Heap *GetLookupHeap()       { return m_lookup_heap; }
            constexpr ALWAYS_INLINE const mem::Heap *GetLookupHeap() const { return m_lookup_heap; }

            constexpr ALWAYS_INLINE void SetTlsData(TlsSlot slot, void *object) {
                VP_ASSERT(slot < cMaxThreadTlsSlotCount);
                m_tls_slot_array[slot] = object;
            }
            constexpr ALWAYS_INLINE void *GetTlsData(TlsSlot slot) {
                VP_ASSERT(slot < cMaxThreadTlsSlotCount);
               return m_tls_slot_array[slot];
            }
    };
}
