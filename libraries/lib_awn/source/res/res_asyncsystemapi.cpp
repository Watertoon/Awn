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

    void SetControlThreadPriority(s32 priority) {
        impl::AsyncResourceManager::GetInstance()->SetControlThreadPriority(priority);
    }

	void SetMemoryThreadPriority(s32 priority) {
        impl::AsyncResourceManager::GetInstance()->SetMemoryThreadPriority(priority);
    }

	void SetLoadThreadsPriority(s32 priority) {
        impl::AsyncResourceManager::GetInstance()->SetLoadThreadsPriority(priority);
    }

    Result LoadAsyncResourceSync(Resource **out_resource, const char *path, AsyncResourceLoadInfo *load_info) {
        VP_UNUSED(out_resource, path, load_info); 
        VP_NOT_IMPLEMENTED();
    }
}
