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

    ThreadBase::ThreadBase(mem::Heap *thread_heap, ThreadRunMode run_mode, size_t exit_code, u32 max_messages , u32 stack_size, s32 priority) : m_thread_heap(thread_heap), m_lookup_heap(nullptr), m_message_queue(), m_message_queue_buffer(), m_exit_message(exit_code), m_stack_size(stack_size), m_priority(priority), m_core_mask(), m_run_mode(static_cast<u32>(run_mode)), m_tls_slot_array(), m_thread_manager_list_node() {
        m_message_queue.Initialize(thread_heap, max_messages);
        ThreadManager *manager = ThreadManager::GetInstance();
        if (manager != nullptr) {
            manager->PushBackThreadSafe(this);
        }
    }
    ThreadBase::ThreadBase(mem::Heap *thread_heap) : m_thread_heap(thread_heap), m_lookup_heap(nullptr) {
        ThreadManager *manager = ThreadManager::GetInstance();
        if (manager != nullptr) {
            manager->PushBackThreadSafe(this);
        }
    }

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
        ThreadManager::GetInstance()->SetCurrentServiceThread(thread);

        /* Run thread */
        thread->Run();

        /* Destruct thread tls */
        ThreadManager::GetInstance()->InvokeThreadTlsDestructors(thread);

        return 0;
    }
    
}
