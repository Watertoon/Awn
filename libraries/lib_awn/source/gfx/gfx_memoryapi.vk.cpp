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

namespace awn::gfx {

    void FlushCpuCache(void *gpu_address, size_t size) {
        mem::FindGpuRootHeapContextFromAddress(gpu_address)->FlushCpuCache(gpu_address, size);
    }

    void InvalidateCpuCache(void *gpu_address, size_t size) {
        mem::FindGpuRootHeapContextFromAddress(gpu_address)->InvalidateCpuCache(gpu_address, size);
    }
}
