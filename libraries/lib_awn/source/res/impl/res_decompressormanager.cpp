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

namespace awn::res::impl {

    void FormatPathWithCompressionExtension(MaxPathString *out_string, MaxPathString *file_path, CompressionType compression_type) {
        *out_string = *file_path;
        if (compression_type == CompressionType::Zstandard) {            
            out_string->Append(".zs");
        }
    }

	Result ParallelDecompressorManager::Initialize(mem::Heap *heap, u32 core_count) {

		/* Initialize arrays */
		RESULT_TRY(m_index_allocator.Initialize(heap, core_count));
		RESULT_TRY(m_zstd_decompressor_array.Initialize(heap, core_count));

		/* Initialize decompressors */
		for (u32 i = 0; i < core_count; ++i) {
			m_zstd_decompressor_array[i].Initialize(heap);
		}

		/* Initialize auto reset free event */
		m_free_event.Initialize(sys::SignalState::Cleared, sys::ResetMode::Auto);

		RESULT_RETURN_SUCCESS;
	}

	void ParallelDecompressorManager::Finalize() {

		/* Finalize decompressors */
		for (u32 i = 0; i < m_zstd_decompressor_array.GetCount(); ++i) {
			m_zstd_decompressor_array[i].Finalize();
		}

		/* Finalize arrays */
		m_index_allocator.Finalize();
		m_zstd_decompressor_array.Finalize();

		return;
	}

	u32 ParallelDecompressorManager::AllocateDecompressorHandle() {

		/* Allocate handle */
		u32 index = 0;
		for (;;) {
			index = m_index_allocator.Allocate();
			if (index == HandleAllocator::cInvalidEntryIndex) { m_free_event.Wait(); continue; }
			break;
		}

		return index;
	}

	void ParallelDecompressorManager::FreeDecompressorHandle(u32 handle) {

		/* Free handle */
		m_index_allocator.Free(handle);
		m_free_event.Signal();

		return;
	}

	IParallelDecompressor *ParallelDecompressorManager::GetParallelDecompressor(u32 handle, CompressionType decompressor_type, u32 priority, sys::CoreMask core_mask) {

		/* Get decompressor */
		IParallelDecompressor *decompressor = nullptr;
		if (decompressor_type == CompressionType::Zstandard) {                    
			decompressor = std::addressof(m_zstd_decompressor_array[handle]);
		}
		VP_ASSERT(decompressor != nullptr);

		/* Update priority and core mask */
		if (decompressor->GetPriority() != priority) {                    
			decompressor->SetPriority(priority);
		}
		if (decompressor->GetCoreMask() != core_mask) {
			decompressor->SetCoreMask(core_mask);
		}

		return decompressor;
	}
}
