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

    Result LoadFile(const char *path, FileLoadContext *file_load_context) {
		return FileDeviceManager::GetInstance()->LoadFile(path, file_load_context);
	}
    Result LoadResource(Resource **out_resource, const char *path, ResourceLoadContext *resource_load_context) {
		return ResourceFactoryManager::GetInstance()->LoadResource(out_resource, path, resource_load_context);
	}
    Result LoadResourceWithDecompressor(Resource **out_resource, const char *path, ResourceLoadContext *resource_load_context, IDecompressor *decompressor) {
		return ResourceFactoryManager::GetInstance()->LoadResourceWithDecompressor(out_resource, path, resource_load_context, decompressor);
	}
}
