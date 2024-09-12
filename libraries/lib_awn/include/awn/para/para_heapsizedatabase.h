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
#pragma once

namespace awn::para {

    bool GetCpuHeapSize(u64 *out_heap_size, const char *key);
    bool GetCpuUncachedHeapSize(u64 *out_heap_size, const char *key);
    bool GetCpuCachedHeapSize(u64 *out_heap_size, const char *key);
    bool GetGpuUncachedHeapSize(u64 *out_heap_size, const char *key);

    class HeapSizeDatabase {
        private:
            void                   *m_file;
            vp::res::ByamlIterator  m_iter_cpu;
            vp::res::ByamlIterator  m_iter_cpu_uncached;
            vp::res::ByamlIterator  m_iter_cpu_cached;
            vp::res::ByamlIterator  m_iter_gpu_uncached;
        public:
            constexpr  HeapSizeDatabase() : m_file(), m_iter_cpu(), m_iter_cpu_uncached(), m_iter_cpu_cached(), m_iter_gpu_uncached() {/*...*/}
            constexpr ~HeapSizeDatabase() {/*...*/}

            bool Initialize(mem::Heap *heap, const char *file_path);
            void Finalize();

            bool TryGetCpuHeapSize(u64 *out_size, const char *heap_name);
            bool TryGetCpuUncachedHeapSize(u64 *out_size, const char *heap_name);
            bool TryGetCpuCachedSize(u64 *out_size, const char *heap_name);
            bool TryGetGpuUncachedHeapSize(u64 *out_size, const char *heap_name);
    };

    HeapSizeDatabase *GetHeapSizeDatabase();
    void SetHeapSizeDatabase(HeapSizeDatabase *heap_size_db);
}
