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

    DecompressorHandle AllocateDecompressorHandle() {
        return impl::AsyncResourceManager::GetInstance()->AllocateDecompressor();
    }

    void FreeDecompressorHandle(DecompressorHandle handle) {
        impl::AsyncResourceManager::GetInstance()->FreeDecompressor(handle);
    }

    IDecompressor *GetDecompressor(DecompressorHandle handle, CompressionType compression_type, s32 priority, sys::CoreMask core_mask) {
        return impl::AsyncResourceManager::GetInstance()->GetDecompressor(handle, compression_type, priority, core_mask);
    }
}
