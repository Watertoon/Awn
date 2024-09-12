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

    HeapSizeDatabase *sGlobalHeapSizeDatabase = nullptr;

    HeapSizeDatabase *GetHeapSizeDatabase() {
        return sGlobalHeapSizeDatabase;
    }

    void SetHeapSizeDatabase(HeapSizeDatabase *heap_size_db) {
        sGlobalHeapSizeDatabase = heap_size_db;
    }

    bool GetCpuHeapSize(u64 *out_heap_size, const char *key) {
        if (sGlobalHeapSizeDatabase == nullptr) { return false; }
        return sGlobalHeapSizeDatabase->TryGetCpuHeapSize(out_heap_size, key);
    }

    bool GetCpuUncachedHeapSize(u64 *out_heap_size, const char *key) {
        if (sGlobalHeapSizeDatabase == nullptr) { return false; }
        return sGlobalHeapSizeDatabase->TryGetCpuUncachedHeapSize(out_heap_size, key);
    }

    bool GetCpuCachedHeapSize(u64 *out_heap_size, const char *key) {
        if (sGlobalHeapSizeDatabase == nullptr) { return false; }
        return sGlobalHeapSizeDatabase->TryGetCpuCachedSize(out_heap_size, key);
    }

    bool GetGpuUncachedHeapSize(u64 *out_heap_size, const char *key) {
        if (sGlobalHeapSizeDatabase == nullptr) { return false; }
        return sGlobalHeapSizeDatabase->TryGetGpuUncachedHeapSize(out_heap_size, key);
    }

	bool HeapSizeDatabase::Initialize(mem::Heap *heap, const char *file_path) {

		/* Load heap size database */
        m_file = awn::res::LoadFileFull(heap, file_path);
        if (m_file == nullptr) { return false; }

		/* Get heap iterator */
		vp::res::ByamlIterator iter(m_file);
		bool result0 = iter.TryGetIteratorByKey(std::addressof(m_iter_cpu), "CpuHeaps");
        if (!result0) {
            result0 = iter.TryGetIteratorByKey(std::addressof(m_iter_cpu), "Heaps");            
        }
		const bool result1 = iter.TryGetIteratorByKey(std::addressof(m_iter_cpu_uncached), "CpuUncachedHeaps");
		const bool result2 = iter.TryGetIteratorByKey(std::addressof(m_iter_cpu_cached), "CpuCachedHeaps");
		const bool result3 = iter.TryGetIteratorByKey(std::addressof(m_iter_gpu_uncached), "GpuUncachedHeap");
		if (result0 || result1 || result2 || result3) { return true; }

		/* Empty file */
		this->Finalize();

		return false;
	}

	void HeapSizeDatabase::Finalize() {
		if (m_file != nullptr) {
			::operator delete (m_file);
			m_file = nullptr;
		}
	}

	bool HeapSizeDatabase::TryGetCpuHeapSize(u64 *out_size, const char *heap_name) {
		if (!m_iter_cpu.IsValid()) { return false; }
        vp::res::ByamlIterator iter;
		const bool result = m_iter_cpu.TryGetIteratorByKey(std::addressof(iter), heap_name);
		if (!result) { return false; }
		return iter.TryGetU64ByKey(out_size, "Size");
	}

	bool HeapSizeDatabase::TryGetCpuUncachedHeapSize(u64 *out_size, const char *heap_name) {
		if (!m_iter_cpu_uncached.IsValid()) { return false; }
        vp::res::ByamlIterator iter;
		const bool result = m_iter_cpu_uncached.TryGetIteratorByKey(std::addressof(iter), heap_name);
		if (!result) { return false; }
		return iter.TryGetU64ByKey(out_size, "Size");
	}

	bool HeapSizeDatabase::TryGetCpuCachedSize(u64 *out_size, const char *heap_name) {
		if (!m_iter_cpu_cached.IsValid()) { return false; }
        vp::res::ByamlIterator iter;
		const bool result = m_iter_cpu_cached.TryGetIteratorByKey(std::addressof(iter), heap_name);
		if (!result) { return false; }
		return iter.TryGetU64ByKey(out_size, "Size");
	}

	bool HeapSizeDatabase::TryGetGpuUncachedHeapSize(u64 *out_size, const char *heap_name) {
		if (!m_iter_gpu_uncached.IsValid()) { return false; }
        vp::res::ByamlIterator iter;
		const bool result = m_iter_gpu_uncached.TryGetIteratorByKey(std::addressof(iter), heap_name);
		if (!result) { return false; }
		return iter.TryGetU64ByKey(out_size, "Size");
	}
}
