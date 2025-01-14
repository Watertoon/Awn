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

namespace awn::res::impl {

    void FormatPathWithCompressionExtension(MaxPathString *out_string, MaxPathString *file_path, CompressionType compression_type);

	class ParallelDecompressorManager {
        public:
            using HandleAllocator = vp::util::AtomicIndexAllocator<u8>;
            using ZstdDecompressorArray = vp::util::HeapArray<ParallelZstdDecompressor>;
		private:
            HandleAllocator       m_index_allocator;
            ZstdDecompressorArray m_zstd_decompressor_array;
            sys::Event            m_free_event;
        public:
            constexpr ParallelDecompressorManager() : m_index_allocator(), m_zstd_decompressor_array(), m_free_event() {/*...*/}
            constexpr ~ParallelDecompressorManager() {/*...*/}

            void Initialize(mem::Heap *heap, u32 thread_count);
            void Finalize();

            u32 AllocateDecompressorHandle();
            void FreeDecompressorHandle(u32 handle);

            IParallelDecompressor *GetParallelDecompressor(u32 handle, CompressionType decompressor_type, u32 priority, sys::CoreMask core_mask);
        };
}
