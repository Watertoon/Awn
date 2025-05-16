#pragma once

namespace awn::gfx {

	class BufferAllocator {
        public:
            using BufferArray = vp::util::HeapArray<Buffer>;
		public:
			BufferArray m_buffer_array;
			u32 m_current_index;
			u32 m_last_index;
            u32 m_memory_iter;
            u32 m_size;
		public:
			constexpr  BufferAllocator() : m_buffer_array(), m_current_index(), m_last_index(), m_memory_iter(), m_size() {/*...*/}
			constexpr ~BufferAllocator() {/*...*/}

            Result Initialize(mem::Heap *heap, gfx::MemoryPool *memory_pool, size_t memory_offset, GpuAccessFlags access_flags, u32 per_buffer_size, u32 buffer_count) {

                /* Get buffer aligned size */
                BufferInfo buffer_info;
                buffer_info.SetDefaults();
                buffer_info.SetSize(per_buffer_size);
                buffer_info.SetGpuAccessFlags(access_flags);
                const gfx::GpuMemoryRequirements reqs = Buffer::GetGpuMemoryRequirements(std::addressof(buffer_info));

                /* Initialize buffers */
                m_size = reqs.size;
                m_buffer_array.Initialize(heap, buffer_count);
                auto fail_guard = SCOPE_GUARD {
                    this->Finalize();
                };
                for (Buffer &buffer : m_buffer_array) {
                    RESULT_TRY(buffer.Initialize(memory_pool, memory_offset, std::addressof(buffer_info)));
                    memory_offset += reqs.size;
                }

                fail_guard.Cancel();
                RESULT_RETURN_SUCCESS;
            }

            void Finalize() {
                for (Buffer &buffer : m_buffer_array) {
                    buffer.Finalize();
                }
                m_buffer_array.Finalize();
                return;
            }

            constexpr u32 Allocate(u32 memory_size) {
                return vp::util::InterlockedFetchAddAcquire(std::addressof(m_memory_iter), memory_size);
            }

            constexpr Buffer *GetBuffer() {
                return std::addressof(m_buffer_array[m_current_index]);
            }

            template <typename T>
            ALWAYS_INLINE T *GetOffset(u32 offset) const {
                void *base_address = m_buffer_array[m_current_index].GetAddress();
                return reinterpret_cast<T*>(reinterpret_cast<uintptr_t>(base_address) + offset);
            }

            constexpr void SetBufferIndex(u32 index) {
                VP_ASSERT(index < m_buffer_array.GetCount());
                m_last_index    = m_current_index;
                m_current_index = index;
                m_memory_iter   = 0;
            }
	};
}
