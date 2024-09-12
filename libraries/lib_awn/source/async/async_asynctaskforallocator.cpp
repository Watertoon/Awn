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

namespace awn::async {

	void AsyncTaskForAllocator::SetWatcher(AsyncTaskWatcher *watcher, AsyncQueue *queue) {

		/* Skip set if watcher is already set */
		if (m_watcher != nullptr) { return; }

		/* Set watcher */
		m_watcher = watcher;
		if (watcher == nullptr) { return; }

		/* Setup watcher */
		watcher->m_async_task = this;
		watcher->m_queue      = queue;
		{
			std::scoped_lock l(queue->m_queue_mutex);
			watcher->m_reference_count = watcher->m_reference_count + 1;
		}
		watcher->m_state = static_cast<u32>(AsyncTaskWatcher::State::Pending);

		return;
	}

	void AsyncTaskForAllocator::FreeToAllocator() {

		/* Lock queue */
		std::scoped_lock l(m_queue->m_queue_mutex);

		/* Nothing to release if no watcher */
		if (m_watcher == nullptr) { return; }

		/* Release watcher */
		AsyncTaskWatcher *watcher = m_watcher;
		m_watcher                 = nullptr;
		if (watcher->m_state < static_cast<u32>(AsyncTaskWatcher::State::Complete)) { return; }

		/* Free task to allocator */
		if (m_task_allocator != nullptr) {
			m_task_allocator->FreeTask(this);
		}

		return;
	}

	void AsyncTaskForAllocator::ReleaseWatcher(AsyncTaskWatcher::State state) {

		/* Free by watcher */
		if (m_watcher != nullptr) {
			m_watcher->m_state = static_cast<u32>(state);
			m_watcher->ReleaseReference();
			return;
		}

		/* Free manually if no watcher */
		if (m_task_allocator != nullptr) {
			m_task_allocator->FreeTask(this);
		}

		return;
	}
}
