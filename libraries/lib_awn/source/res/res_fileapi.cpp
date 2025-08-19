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
		return impl::FileDeviceManager::GetInstance()->LoadFile(path, file_load_context);
	}
    Result LoadFileWithDecompressor(const char *path, FileLoadContext *file_load_context, IDecompressor *decompressor) {
		return impl::FileDeviceManager::GetInstance()->LoadFileWithDecompressor(path, file_load_context, decompressor);
	}

    Result SaveFile(const char *path, FileSaveInfo *save_info) {
		return impl::FileDeviceManager::GetInstance()->SaveFile(path, save_info);
    }
    Result SaveFileWithCompressor(const char *path, FileSaveInfo *save_info, ICompressor *compressor) {
		return impl::FileDeviceManager::GetInstance()->SaveFileWithCompressor(path, save_info, compressor);
    }

    void RegisterFileDevice(FileDeviceBase *file_device) {
        impl::FileDeviceManager::GetInstance()->RegisterFileDevice(file_device);
    }
    FileDeviceBase *FindFileDevice(const char *path) {
        return impl::FileDeviceManager::GetInstance()->GetFileDeviceByPath(path);
    }
}
