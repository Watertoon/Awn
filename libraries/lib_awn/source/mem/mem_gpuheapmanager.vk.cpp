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

namespace awn::mem {

    AWN_SINGLETON_TRAITS_IMPL(GpuHeapManager);

    void GpuRootHeapContext::FlushCpuCache(void *address, size_t size) {

        /* Flush cpu cache to memory for gpu visibility */
        const size_t offset = this->GetMemoryOffset(address);
        this->memory_pool->FlushMappedRange(offset, size);
        return;
    }

    void GpuRootHeapContext::InvalidateCpuCache(void *address, size_t size) {

        /* Invalidate cpu cache for cpu visibility of memory */
        const size_t offset = this->GetMemoryOffset(address);
        this->memory_pool->InvalidateMappedRange(offset, size);
        return;
    }

	bool GpuHeapManager::Initialize(mem::Heap *heap, const GpuHeapManagerInfo *gpu_heap_mgr_info) {

		/* Integrity checks */
		VP_ASSERT(gpu_heap_mgr_info != nullptr);

		/* Set counts */
		m_cpu_uncached_heap_count = gpu_heap_mgr_info->cpu_uncached_root_heap_count;
		m_cpu_cached_heap_count = gpu_heap_mgr_info->cpu_cached_root_heap_count;
		m_gpu_uncached_heap_count = gpu_heap_mgr_info->gpu_uncached_root_heap_count;

		/* Setup root heaps */
        constexpr const char *cCpuUncachedNameArray[cMaxGpuRootHeapCount] = {
            "CpuUncachedRootHeap0",
            "CpuUncachedRootHeap1",
        };
        constexpr const char *cCpuCachedNameArray[cMaxGpuRootHeapCount] = {
            "CpuCachedRootHeap0",
            "CpuCachedRootHeap1",
        };
        constexpr const char *cGpuUncachedNameArray[cMaxGpuRootHeapCount] = {
            "GpuUncachedRootHeap0",
            "GpuUncachedRootHeap1",
        };
		for (u32 i = 0; i < gpu_heap_mgr_info->cpu_uncached_root_heap_count; ++i) {

			m_cpu_uncached_heap_context_array[i].memory_pool = gpu_heap_mgr_info->cpu_uncached_info_array[i].memory_pool;
			m_cpu_uncached_heap_context_array[i].memory_size = gpu_heap_mgr_info->cpu_uncached_info_array[i].size;
			m_cpu_uncached_heap_context_array[i].base_address = gpu_heap_mgr_info->cpu_uncached_info_array[i].memory_pool->GetAddress();
			m_cpu_uncached_heap_context_array[i].root_heap = mem::GpuExpHeap::TryCreate(cCpuUncachedNameArray[i], heap, m_cpu_uncached_heap_context_array[i].base_address, m_cpu_uncached_heap_context_array[i].memory_size);
		}
		for (u32 i = 0; i < gpu_heap_mgr_info->cpu_cached_root_heap_count; ++i) {

			m_cpu_cached_heap_context_array[i].memory_pool = gpu_heap_mgr_info->cpu_cached_info_array[i].memory_pool;
			m_cpu_cached_heap_context_array[i].memory_size = gpu_heap_mgr_info->cpu_cached_info_array[i].size;
			m_cpu_cached_heap_context_array[i].base_address = gpu_heap_mgr_info->cpu_cached_info_array[i].memory_pool->GetAddress();
			m_cpu_cached_heap_context_array[i].root_heap = mem::GpuExpHeap::TryCreate(cCpuCachedNameArray[i], heap, m_cpu_cached_heap_context_array[i].base_address, m_cpu_cached_heap_context_array[i].memory_size);
		}
		for (u32 i = 0; i < gpu_heap_mgr_info->gpu_uncached_root_heap_count; ++i) {

			m_gpu_uncached_heap_context_array[i].memory_pool = gpu_heap_mgr_info->gpu_uncached_info_array[i].memory_pool;
			m_gpu_uncached_heap_context_array[i].memory_size = gpu_heap_mgr_info->gpu_uncached_info_array[i].size;
			m_gpu_uncached_heap_context_array[i].base_address = gpu_heap_mgr_info->gpu_uncached_info_array[i].memory_pool->GetAddress();
			m_gpu_uncached_heap_context_array[i].root_heap = mem::GpuExpHeap::TryCreate(cGpuUncachedNameArray[i], heap, m_gpu_uncached_heap_context_array[i].base_address, m_gpu_uncached_heap_context_array[i].memory_size);
		}

		return true;
	}

	void GpuHeapManager::Finalize() {
        
		return;
	}
}
