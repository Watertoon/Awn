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
#include <awn.hpp>

namespace awn::sys {

    AWN_SINGLETON_TRAITS_IMPL(ThreadManager);

    ThreadManager::~ThreadManager() {

        /* Ensure all threads exit */
        std::scoped_lock l(m_list_cs);
        for (ThreadBase &thread : m_thread_list) {
            if (std::addressof(thread) == vp::util::GetPointer(m_main_thread)) { continue; }
            thread.WaitForThreadExit();
        }

        m_thread_list.Clear();
    }

    void ThreadManager::Initialize(mem::Heap *heap) {

        /* Allocate tls slot for the current service thread */
        m_current_thread_tls_slot = ::TlsAlloc();
        VP_ASSERT(m_current_thread_tls_slot != TLS_OUT_OF_INDEXES);

        /* Initialize the main thread */
        this->InitializeMainThread(heap);
        this->SetCurrentThread(vp::util::GetPointer(m_main_thread));

        return;
    }

    void ThreadManager::InitializeMainThread(mem::Heap *heap) {
        vp::util::ConstructAt(m_main_thread, heap);
    }

    void ThreadManager::PushBackThreadSafe(ThreadBase *thread) {
        std::scoped_lock l(m_list_cs);
        m_thread_list.PushBack(*thread);
    }
    
    void ThreadManager::RemoveThreadUnsafe(ThreadBase *thread) {
        thread->m_thread_manager_list_node.Unlink();
    }

    void ThreadManager::RemoveThreadSafe(ThreadBase *thread) {
        std::scoped_lock l(m_list_cs);
        thread->m_thread_manager_list_node.Unlink();
    }

    ThreadBase *ThreadManager::GetCurrentThread() {
        return reinterpret_cast<ThreadBase*>(::TlsGetValue(m_current_thread_tls_slot));
    }

    bool ThreadManager::IsThreadValid(sys::ThreadBase *thread) {
        std::scoped_lock l(m_list_cs);
        for (sys::ThreadBase &thread_iter : m_thread_list) {
            if (std::addressof(thread_iter) == thread) { return true; }
        }
        return false;
    }

    void ThreadManager::SetCurrentThread(ThreadBase *thread) {
        const bool result = ::TlsSetValue(m_current_thread_tls_slot, reinterpret_cast<void*>(thread));
        VP_ASSERT(result);
    }

    bool ThreadManager::IsMainThread() { return this->GetCurrentThread() == reinterpret_cast<ThreadBase*>(vp::util::GetPointer(m_main_thread)); }

    bool ThreadManager::AllocateTlsSlot(TlsSlot *out_slot, TlsDestructor destructor, bool is_internal) {

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

    void ThreadManager::FreeTlsSlot(TlsSlot slot) {

        /* Lock Tls manager */
        std::scoped_lock l(m_tls_cs);

        /* Integrity check */
        VP_ASSERT(m_tls_destructor_array[slot] != nullptr);

        /* Free from Manager */
        m_tls_destructor_array[slot] = nullptr;
        m_tls_slot_count = m_tls_slot_count - 1;

        return;
    }

    void ThreadManager::InvokeThreadTlsDestructors(ThreadBase *thread) {

        /* Lock Tls manager */
        std::scoped_lock l(m_tls_cs);

        /* Call the Tls destructors on the current thread */
        for (u32 i = 0; i < ThreadBase::cMaxThreadTlsSlotCount; ++i) {
            if (m_tls_destructor_array[i] != nullptr) {
                (m_tls_destructor_array[i])(reinterpret_cast<size_t>(thread->m_tls_slot_array[i]));
            }
            thread->m_tls_slot_array[i] = 0;
        }

        return;
    }
}
