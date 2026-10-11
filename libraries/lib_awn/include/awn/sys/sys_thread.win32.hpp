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

    constexpr s32 ConvertAwnPriorityToWin32(u32 priority) {
        if (priority <= 0x12) {
            if (0xe <= priority) { return THREAD_PRIORITY_NORMAL; }
            if (0x8 <= priority) { return THREAD_PRIORITY_ABOVE_NORMAL; }
            if (0x1 <= priority) { return THREAD_PRIORITY_HIGHEST; }
            if (0x0 == priority) { return THREAD_PRIORITY_TIME_CRITICAL; }
        } else if (priority <= 0x18) {
            return THREAD_PRIORITY_BELOW_NORMAL;
        } else if (priority <= 0x1e) {
            return THREAD_PRIORITY_LOWEST;
        } else if (priority <= 0x1f) {
            return THREAD_PRIORITY_IDLE;
        }
        VP_UNEXPECTED_DEFAULT();
    }

    static constexpr u32 cMaxThreadNameLength = 0x20;

	class Thread : public sys::ThreadBase {
		protected:
			HANDLE m_handle;
            HANDLE m_waitable_timer_for_sleep;
            long unsigned int m_win32_thread_id;
            vp::util::FixedString8<cMaxThreadNameLength> m_name;
            vp::util::FixedString16<cMaxThreadNameLength> m_name_utf16;
        public:
            VP_RTTI_DERIVED(Thread);
        public:
			constexpr Thread() : ThreadBase(), m_handle(), m_waitable_timer_for_sleep(), m_win32_thread_id(), m_name(), m_name_utf16() {/*...*/}
            virtual ~Thread() override {/*...*/}

            Result Initialize(mem::Heap *thread_heap, const ThreadInfo *info);
            void Finalize();

            virtual void StartThread() override;
            virtual void ExitThread(ExitType exit_type) override;
            virtual void WaitForThreadExit() override;
            
            virtual void ResumeThread() override;
            virtual void SuspendThread() override;
            
            virtual void SleepThread(vp::TimeSpan timeout_ns) override;
            virtual void YieldThread() override;

            virtual void SetPriority(u32 priority) override;
            virtual void SetCoreMask(CoreMask core_mask) override;
        public:
            void SleepThreadPrecise(TimeSpan timeout_ns);
            constexpr bool IsValid() const { return m_handle != nullptr; }
            constexpr HANDLE GetWin32Handle() const { return m_handle; }
            constexpr HANDLE GetWin32Timer() const { return m_waitable_timer_for_sleep; }
            constexpr long unsigned int GetWin32ThreadId() const { return m_win32_thread_id; }
	};
}
