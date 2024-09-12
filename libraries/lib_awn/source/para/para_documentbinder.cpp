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

namespace awn::para {

	void DocumentBinder::LoadDocumentForResolve(const char *format_path) {

		/* Split path path and class name with no work */
		vp::util::MaxPathString path_no_work;
		FormatNoWorkExtension(std::addressof(path_no_work), format_path);
		const char *file_name  = "";
		const char *class_name = "";
		const bool  is_split   = FormatSplitNameAndClass(std::addressof(file_name), std::addressof(class_name), std::addressof(path_no_work));
        if (is_split == false) { return; }

		/* Unset document */
		m_document = nullptr;

		/* Get document hash */
		const u64 hash = vp::util::DualHashMurmur64(file_name, class_name);

		/* Lock bind cs */
		impl::DocumentManager *manager = impl::DocumentManager::GetInstance();
		std::scoped_lock l(manager->m_bind_cs);

		/* Load document impl */
		Document *document = manager->TryLoadDocumentImpl(file_name, class_name, hash, false);
		if (document == nullptr) { return; }

		/* Resolve parent */
		document->ResolveParent();

		/* Reference and set document */
		vp::util::InterlockedIncrement(std::addressof(document->m_reference_count));
		m_document = document;

		return;
	}

	DocumentBinder &DocumentBinder::operator=(const DocumentBinder &rhs) {
		this->Release();
		m_document = rhs.m_document;
		if (m_document == nullptr) { return *this; }
		vp::util::InterlockedIncrement(std::addressof(m_document->m_reference_count));
		return *this;
	}

	void DocumentBinder::LoadDocument(const char *file_name, const char *class_name, bool is_allow_dummy) {

		/* Unset document */
		m_document = nullptr;

		/* Get document hash */
		const u64 hash = vp::util::DualHashMurmur64(file_name, class_name);

		/* Lock bind cs */
		impl::DocumentManager *manager = impl::DocumentManager::GetInstance();
		std::scoped_lock l(manager->m_bind_cs);

		/* Load document impl */
		Document *document = manager->TryLoadDocumentImpl(file_name, class_name, hash, is_allow_dummy);
		if (document == nullptr) { manager->m_app_impl->OnBindDocument(nullptr); return; }

		/* Callback on bind */
		manager->m_app_impl->OnBindDocument(document);

		/* Reference and set document */
		vp::util::InterlockedIncrement(std::addressof(document->m_reference_count));
		m_document = document;

		/* Resolve parent */
		document->ResolveParent();

		/* On load document callback */
		manager->OnLoadDocumentImpl(document);

		return;
	}

	void DocumentBinder::Release() {

		/* Check valid Document */
		Document *document = m_document;
		if (document == nullptr) { return; }

		/* Atomicly release reference */
		const u32 last_count = vp::util::InterlockedDecrement(std::addressof(document->m_reference_count));
		if (last_count == 1) { impl::DocumentManager::GetInstance()->ReleaseDocumentImpl(document); }

		/* Unset document */
		m_document = nullptr;

		return;
	}
}