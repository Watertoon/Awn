#pragma once

namespace awn::sys {

    class ThreadManager {
        public:
            using ThreadList = vp::util::IntrusiveListTraits<ThreadBase, &ThreadBase::m_thread_manager_list_node>::List;
        private:
            ThreadList                        m_thread_list;
            sys::ServiceCriticalSection       m_list_cs;
            vp::util::TypeStorage<MainThread> m_main_thread;
            u32                               m_current_service_thread_tls_slot;
        public:
            AWN_SINGLETON_TRAITS(ThreadManager);
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
                ukern::ThreadType *thread = ukern::GetCurrentThread();
                
                /* If we are not a ukern thread we are a service thread */
                if (thread == nullptr) {
                    return reinterpret_cast<ThreadBase*>(::TlsGetValue(m_current_service_thread_tls_slot));
                } else {
                    return reinterpret_cast<ThreadBase*>(thread->user_arg);
                }
            }

            ALWAYS_INLINE void SetCurrentServiceThread(ThreadBase *thread) {
                const bool result = ::TlsSetValue(m_current_service_thread_tls_slot, reinterpret_cast<void*>(thread));
                VP_ASSERT(result == true);
            }

            ALWAYS_INLINE bool IsMainThread() { return this->GetCurrentThread() == reinterpret_cast<ThreadBase*>(vp::util::GetPointer(m_main_thread)); }
    };
}
