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

namespace awn::res {

    void SetControlThreadPriority(u32 priority) {
        AsyncResourceManager::GetInstance()->SetControlThreadPriority(priority);
    }
	void SetMemoryThreadPriority(u32 priority) {
        AsyncResourceManager::GetInstance()->SetMemoryThreadPriority(priority);
    }
	void SetLoadThreadsPriority(u32 priority) {
        AsyncResourceManager::GetInstance()->SetLoadThreadsPriority(priority);
    }

	void SetDefaultArchive(ResourceBinder *archive_binder) {
        AsyncResourceManager::GetInstance()->SetDefaultArchive(archive_binder);
    }

    u32  AllocateDecompressor() {
        return AsyncResourceManager::GetInstance()->AllocateDecompressor();
    }
    void FreeDecompressor(u32 handle) {
        AsyncResourceManager::GetInstance()->FreeDecompressor(handle);
    }
    IDecompressor *GetDecompressor(u32 handle, CompressionType compression_type, u32 priority, sys::CoreMask core_mask) {
        return AsyncResourceManager::GetInstance()->GetDecompressor(handle, compression_type, priority, core_mask);
    }

    Result LoadAsyncResourceSync(Resource **out_resource, const char *path, AsyncResourceLoadInfo *load_info) { VP_UNUSED(out_resource, path, load_info); VP_ASSERT(false); }
}
