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
#include <ares.h>

namespace awn::res {

	void ResourceSizeTableManager::Initialize(const char *path) {

		/* Load rsizetable */
        if (path == nullptr || *path == '\0') { return; }
        
        Resource *resource = nullptr;
		AsyncResourceLoadInfo async_load_info = {
			
		};
		RESULT_ABORT_UNLESS(res::LoadAsyncResourceSync(std::addressof(resource), path, std::addressof(async_load_info)));

		/* Initialize extractor */
		const bool result = m_extractor.Initialize(resource->GetFile(), resource->GetFileSize());
		VP_ASSERT(result == true);

		return;
	}

	void ResourceSizeTableManager::Finalize() {
		if (m_resource != nullptr) {
			delete m_resource;
			m_resource = nullptr;
		}
		auto table_iter = m_fallback_list.begin();
		while (table_iter != m_fallback_list.end()) {
			RsizetableNode &node = *table_iter;
			++table_iter;
			m_fallback_list.Remove(node);
			delete std::addressof(node);
		}
	}

    u32 ResourceSizeTableManager::GetResourceSize(const char *path) {

        u32 resource_size = m_extractor.TryGetResourceSize(path);
        if (resource_size != cInvalidSize) { return resource_size; }

        for (RsizetableNode &node : m_fallback_list) {
            resource_size = node.extractor.TryGetResourceSize(path);
            if (resource_size != cInvalidSize) { return resource_size; }
        }

        return cInvalidSize;
    }

	bool ResourceSizeTableManager::RegisterResourceSizeTable(mem::Heap *heap, void *rsizetable, u32 rsizetable_size) {

		/* Allocate and add new resource size node */
		RsizetableNode *node = new (heap, alignof(RsizetableNode)) RsizetableNode;
		const bool result = node->extractor.Initialize(rsizetable, rsizetable_size);
		if (result == false) {
			delete node;
			return false;
		}

		m_fallback_list.PushBack(*node);

		return true;
	}

	void ResourceSizeTableManager::UnregisterResourceSizeTable(void *rsizetable) {

		for (RsizetableNode &node : m_fallback_list) {
			if (node.extractor.GetResourceSizeTable() == rsizetable) { continue; }
			m_fallback_list.Remove(node);
			delete std::addressof(node);
			break;
		}

		return;
    }

	void ResourceSizeTableManager::Clear() {

        auto iter = m_fallback_list.begin();
		while (iter != m_fallback_list.end()) {
			RsizetableNode *node = std::addressof(*iter);
            ++iter;
			m_fallback_list.Remove(*node);
			delete node;
		}

		return;
	}
}
