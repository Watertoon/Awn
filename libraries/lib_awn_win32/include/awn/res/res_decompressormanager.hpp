#pragma once

namespace awn::res {

	class DecompressorManager {
        public:
            using HandleAllocator       = vp::util::AtomicIndexAllocator<u8>;
            using ZstdDecompressorArray = vp::util::HeapArray<ZstdDecompressor>;
		private:
            HandleAllocator       m_index_allocator;
            ZstdDecompressorArray m_zstd_decompressor_array;
            sys::ServiceEvent     m_free_event;
        public:
            constexpr DecompressorManager() : m_index_allocator(), m_zstd_decompressor_array(), m_free_event() {/*...*/}
            constexpr ~DecompressorManager() {/*...*/}

            void Initialize(mem::Heap *heap, u32 core_count) {

                /* Initialize arrays */
                m_index_allocator.Initialize(heap, core_count);
                m_zstd_decompressor_array.Initialize(heap, core_count);

                /* Initialize decompressors */
                for (u32 i = 0; i < core_count; ++i) {
                    m_zstd_decompressor_array[i].Initialize(heap);
                }

                /* Initialize auto reset free event */
                m_free_event.Initialize(sys::SignalState::Cleared, sys::ResetMode::Auto);

                return;
            }

            void Finalize() {

                /* Finalize decompressors */
                for (u32 i = 0; i < m_zstd_decompressor_array.GetCount(); ++i) {
                    m_zstd_decompressor_array[i].Finalize();
                }

                /* Finalize arrays */
                m_index_allocator.Finalize();
                m_zstd_decompressor_array.Finalize();

                return;
            }

            u32 AllocateDecompressorHandle() {

                /* Allocate handle */
                u32 index = 0;
                for (;;) {
                    index = m_index_allocator.Allocate();
                    if (index == HandleAllocator::cInvalidHandle) { m_free_event.Wait(); continue; }
                    break;
                }

                return index;
            }

            void FreeDecompressorHandle(u32 handle) {

                /* Free handle */
                m_index_allocator.Free(handle);
                m_free_event.Signal();

                return;
            }

            IDecompressor *GetDecompressor(u32 handle, CompressionType decompressor_type, u32 priority, sys::CoreMask core_mask) {

                /* Get decompressor */
                IDecompressor *decompressor = nullptr;
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
        };
}
