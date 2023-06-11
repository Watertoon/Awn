#pragma once

namespace awn::sys {

    constexpr inline s32 cLowPriority         = THREAD_PRIORITY_LOWEST;
    constexpr inline s32 cBelowNormalPriority = THREAD_PRIORITY_BELOW_NORMAL;
    constexpr inline s32 cNormalPriority      = THREAD_PRIORITY_NORMAL;
    constexpr inline s32 cAboveNormalPriority = THREAD_PRIORITY_ABOVE_NORMAL;
    constexpr inline s32 cHighPriority        = THREAD_PRIORITY_HIGHEST;

    class ThreadManager {
        public:
            using ThreadList = vp::util::IntrusiveListTraits<ThreadBase, &ThreadBase::m_thread_manager_list_node>::List;
        private:
            ThreadList                        m_thread_list;
            sys::ServiceCriticalSection       m_list_cs;
            sys::ServiceCriticalSection       m_tls_cs;
            vp::util::TypeStorage<MainThread> m_main_thread;
            u32                               m_current_service_thread_tls_slot;
            u32                               m_tls_slot_count;
            TlsDestructor                     m_tls_destructor_array[ThreadBase::cMaxThreadTlsSlotCount];
        public:
            AWN_SINGLETON_TRAITS(ThreadManager);
        private:
            u32 SearchUnusedTlsSlotUnsafe(bool is_internal) {
                
                /* Manually unrolled search */
                if (is_internal == false) {
                    if (m_tls_destructor_array[ 0] == nullptr) { return 0;  }
                    if (m_tls_destructor_array[ 1] == nullptr) { return 1;  }
                    if (m_tls_destructor_array[ 2] == nullptr) { return 2;  }
                    if (m_tls_destructor_array[ 3] == nullptr) { return 3;  }
                    if (m_tls_destructor_array[ 4] == nullptr) { return 4;  }
                    if (m_tls_destructor_array[ 5] == nullptr) { return 5;  }
                    if (m_tls_destructor_array[ 6] == nullptr) { return 6;  }
                    if (m_tls_destructor_array[ 7] == nullptr) { return 7;  }
                    if (m_tls_destructor_array[ 8] == nullptr) { return 8;  }
                    if (m_tls_destructor_array[ 9] == nullptr) { return 9;  }
                    if (m_tls_destructor_array[10] == nullptr) { return 10; }
                    if (m_tls_destructor_array[11] == nullptr) { return 11; }
                    if (m_tls_destructor_array[12] == nullptr) { return 12; }
                    if (m_tls_destructor_array[13] == nullptr) { return 13; }
                    if (m_tls_destructor_array[14] == nullptr) { return 14; }
                    if (m_tls_destructor_array[15] == nullptr) { return 15; }
                } else {
                    if (m_tls_destructor_array[31] == nullptr) { return 31; }
                    if (m_tls_destructor_array[30] == nullptr) { return 30; }
                    if (m_tls_destructor_array[29] == nullptr) { return 29; }
                    if (m_tls_destructor_array[28] == nullptr) { return 28; }
                    if (m_tls_destructor_array[27] == nullptr) { return 27; }
                    if (m_tls_destructor_array[26] == nullptr) { return 26; }
                    if (m_tls_destructor_array[25] == nullptr) { return 25; }
                    if (m_tls_destructor_array[24] == nullptr) { return 24; }
                    if (m_tls_destructor_array[23] == nullptr) { return 23; }
                    if (m_tls_destructor_array[22] == nullptr) { return 22; }
                    if (m_tls_destructor_array[21] == nullptr) { return 21; }
                    if (m_tls_destructor_array[20] == nullptr) { return 20; }
                    if (m_tls_destructor_array[19] == nullptr) { return 19; }
                    if (m_tls_destructor_array[18] == nullptr) { return 18; }
                    if (m_tls_destructor_array[17] == nullptr) { return 17; }
                    if (m_tls_destructor_array[16] == nullptr) { return 16; }
                }
                VP_ASSERT(false);
            }

            static void DefaultTlsDestructor([[maybe_unused]] void *arg) {/*...*/}
        public:
            ThreadManager() : m_thread_list(), m_list_cs() {/*...*/}

            ~ThreadManager() {
                /* Ensure all threads exit */
                {
                    std::scoped_lock l(m_list_cs);
                    for (ThreadBase &thread : m_thread_list) {
                        thread.WaitForThreadExit();
                    }
                    for (ThreadBase &thread : m_thread_list) {
                        this->RemoveThreadUnsafe(std::addressof(thread));
                    }
                }

                /* Free current service thread tls slot */
                bool result = ::TlsFree(m_current_service_thread_tls_slot);
                VP_ASSERT(result == true);
                m_current_service_thread_tls_slot = 0;
            }

            void Initialize(mem::Heap *heap) {

                /* Allocate tls slot for the current service thread */
                m_current_service_thread_tls_slot = ::TlsAlloc();
                VP_ASSERT(m_current_service_thread_tls_slot != TLS_OUT_OF_INDEXES);
                
                /* Initialize the main thread */
                this->InitializeMainThread(heap);
            }

            void InitializeMainThread(mem::Heap *heap) {

                /* Construct main thread object */
                vp::util::ConstructAt(m_main_thread, heap);
            }

            void PushBackThreadSafe(ThreadBase *thread) {
                std::scoped_lock l(m_list_cs);
                m_thread_list.PushBack(*thread);
            }

            ALWAYS_INLINE void RemoveThreadUnsafe(ThreadBase *thread) {
                thread->m_thread_manager_list_node.Unlink();
            }

            void RemoveThreadSafe(ThreadBase *thread) {
                std::scoped_lock l(m_list_cs);
                thread->m_thread_manager_list_node.Unlink();
            }

            ALWAYS_INLINE ThreadBase *GetCurrentThread() {
                
                /* If we are not a ukern thread we are a service thread */
                if (::IsThreadAFiber() == false) {
                    return reinterpret_cast<ThreadBase*>(::TlsGetValue(m_current_service_thread_tls_slot));
                } else {
                    return reinterpret_cast<ThreadBase*>(ukern::GetCurrentThread()->user_arg);
                }
            }

            ALWAYS_INLINE bool IsServiceThreadCurrent() {
                return ::IsThreadAFiber();
            }

            ALWAYS_INLINE void SetCurrentServiceThread(ThreadBase *thread) {
                const bool result = ::TlsSetValue(m_current_service_thread_tls_slot, reinterpret_cast<void*>(thread));
                VP_ASSERT(result == true);
            }

            ALWAYS_INLINE bool IsMainThread() { return this->GetCurrentThread() == reinterpret_cast<ThreadBase*>(vp::util::GetPointer(m_main_thread)); }
        public:
            bool AllocateTlsSlot(TlsSlot *out_slot, TlsDestructor destructor, bool is_internal) {

                /* Lock tls manager */
                std::scoped_lock l(m_tls_cs);

                /* Allocate new slot */
                TlsSlot new_slot = SearchUnusedTlsSlotUnsafe(is_internal);

                /* Set tls destructor */
                m_tls_destructor_array[new_slot] = DefaultTlsDestructor;
                if(destructor != nullptr) {
                    m_tls_destructor_array[new_slot] = destructor;
                }

                /* Set state */
                *out_slot = new_slot;
                m_tls_slot_count = m_tls_slot_count + 1;
                
                return true;
            }

            void FreeTlsSlot(TlsSlot slot) {

                /* Lock Tls manager */
                std::scoped_lock l(m_tls_cs);

                /* Integrity check */
                VP_ASSERT(m_tls_destructor_array[slot] != nullptr);

                /* Free from Manager */
                m_tls_destructor_array[slot] = nullptr;
                m_tls_slot_count = m_tls_slot_count - 1;
            }

            void InvokeCurrentThreadTlsDestructors() {

                /* Lock Tls manager */
                std::scoped_lock l(m_tls_cs);

                /* Call the Tls destructors on the current thread */
                ThreadBase *current_thread = this->GetCurrentThread();
                for (u32 i = 0; i < ThreadBase::cMaxThreadTlsSlotCount; ++i) {
                    if (m_tls_destructor_array[i] != nullptr) {
                        (m_tls_destructor_array[i])(current_thread->m_tls_slot_array[i]);
                    }
                    current_thread->m_tls_slot_array[i] = nullptr;
                }
            }
            
            constexpr ALWAYS_INLINE u32 GetUsedTlsSlotCount() const {
                return m_tls_slot_count;
            }
    };
}
