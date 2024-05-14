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

    constexpr inline s32 cPriorityLow         = THREAD_PRIORITY_LOWEST;
    constexpr inline s32 cPriorityBelowNormal = THREAD_PRIORITY_BELOW_NORMAL;
    constexpr inline s32 cPriorityNormal      = THREAD_PRIORITY_NORMAL;
    constexpr inline s32 cPriorityAboveNormal = THREAD_PRIORITY_ABOVE_NORMAL;
    constexpr inline s32 cPriorityHigh        = THREAD_PRIORITY_HIGHEST;
    constexpr inline u32 cInvalidTlsSlot      = 0xffff'ffff;

    class ThreadManager {
        public:
            using ThreadList = vp::util::IntrusiveListTraits<ThreadBase, &ThreadBase::m_thread_manager_list_node>::List;
        private:
            ThreadList                         m_thread_list;
            sys::ServiceCriticalSection        m_list_cs;
            sys::ServiceCriticalSection        m_tls_cs;
            vp::util::TypedStorage<MainThread> m_main_thread;
            u32                                m_current_service_thread_tls_slot;
            u32                                m_tls_slot_count;
            TlsDestructor                      m_tls_destructor_array[ThreadBase::cMaxThreadTlsSlotCount];
        public:
            AWN_SINGLETON_TRAITS(ThreadManager);
        private:
            constexpr u32 SearchUnusedTlsSlotUnsafe(bool is_internal) {
                
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
            constexpr ThreadManager() : m_thread_list(), m_list_cs(), m_tls_cs(), m_main_thread(), m_current_service_thread_tls_slot(0), m_tls_slot_count(0), m_tls_destructor_array() {/*...*/}
            ~ThreadManager();

            void Initialize(mem::Heap *heap);

            void InitializeMainThread(mem::Heap *heap);

            void PushBackThreadSafe(ThreadBase *thread);

            void RemoveThreadUnsafe(ThreadBase *thread);
            void RemoveThreadSafe(ThreadBase *thread);

            ThreadBase *GetCurrentThread();
            bool IsMainThread();
            bool IsServiceThreadCurrent();

            bool IsThreadValid(sys::ThreadBase *thread);
            void SetCurrentServiceThread(ThreadBase *thread);
        public:
            bool AllocateTlsSlot(TlsSlot *out_slot, TlsDestructor destructor, bool is_internal);
            void FreeTlsSlot(TlsSlot slot);

            void InvokeThreadTlsDestructors(ThreadBase *thread);

            constexpr ALWAYS_INLINE u32 GetUsedTlsSlotCount() const { return m_tls_slot_count; }
    };
}
