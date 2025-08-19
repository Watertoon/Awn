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

    void ThreadBase::Initialize(mem::Heap *thread_heap, const ThreadInfo *info) {

        /* Set info */
        m_thread_heap = thread_heap;
        m_lookup_heap = nullptr;
        m_exit_message = info->exit_code;
        m_stack_size = info->stack_size;
        m_priority = info->priority;
        m_run_mode = static_cast<u32>(info->run_mode);

        /* Allocate or set memory queue memory */
        if (info->message_memory == nullptr) {            
            m_message_queue_memory = nullptr;
            m_message_queue.Initialize(thread_heap, info->message_count);
        } else {
            m_message_queue_memory = info->message_memory;
            m_message_queue.InitializeByReference(m_message_queue_memory, info->message_count);
        }

        /* Allocate or set memory */
        ThreadManager *manager = ThreadManager::GetInstance();
        if (manager != nullptr) {
            manager->PushBackThreadSafe(this);
        }

        return;
    }

    void ThreadBase::InternalThreadMain(void *arg) {

        /* Recover Thread object */
        ThreadBase *thread = reinterpret_cast<ThreadBase*>(arg);

        /* Run thread */
        thread->Run();

#ifdef VP_TARGET_PLATFORM_win32
        /* Destruct thread tls */
        ThreadManager::GetInstance()->InvokeThreadTlsDestructors(thread);
#endif

        return;
    }

    long unsigned int ThreadBase::InternalServiceThreadMain(void *arg) {
#ifdef VP_TARGET_PLATFORM_win32
        VP_ASSERT((::GetCurrentThreadId() & (~0xbfff'ffff)) == 0);
#endif

        /* Recover Thread object */
        ThreadBase *thread = reinterpret_cast<ThreadBase*>(arg);
        ThreadManager::GetInstance()->SetCurrentThread(thread);

        /* Run thread */
        thread->Run();

#ifdef VP_TARGET_PLATFORM_win32
        /* Destruct thread tls */
        ThreadManager::GetInstance()->InvokeThreadTlsDestructors(thread);
#endif

        return 0;
    }

    void ThreadBase::ExitThread(ExitType exit_type) {

        /* Send exit message */
        if (exit_type == ExitType::Jam) {
            m_message_queue.JamMessage(m_exit_message);
        } else {
            m_message_queue.SendMessage(m_exit_message);
        }

        /* Wait until the thread exits */
        this->WaitForThreadExit();

        /* Free the message queue */
        if (m_message_queue_memory == nullptr) {
            m_message_queue.Finalize();
        }

        ThreadManager *manager = ThreadManager::GetInstance();
        if (manager != nullptr) {
            manager->RemoveThreadSafe(this);
        }

        return;
    }
}
