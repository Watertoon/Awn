#pragma once

namespace awn::gfx {

	class BufferAllocator {
		public:
			vp::util::HeapArray<Buffer>  m_buffer_array;
			u32							 m_current_index;
			u32							 m_last_index;
            void                        *m_gpu_memory;
            u32                          m_memory_iter;
            u32                          m_size;
		public:
			 BufferAllocator() : m_buffer_array(), m_current_index(), m_last_index(), m_gpu_memory(), m_memory_iter(), m_size() {/*...*/}
			~BufferAllocator() {/*...*/}

            void Initialize(mem::Heap *heap, mem::Heap *gpu_heap, GpuAccessFlags access_flags, u32 per_buffer_size, u32 buffer_count) {

                /* Get buffer alignment */
                const BufferInfo buffer_info = {
                    .size             = per_buffer_size,
                    .gpu_access_flags = static_cast<u32>(access_flags),
                };
                u32 buffer_alignment = Buffer::GetMemoryRequirements(std::addressof(buffer_info)).alignment;

                /* Allocate gpu memory */
                const u32 aligned_size = vp::util::AlignUp(per_buffer_size, buffer_alignment);
                m_size       = aligned_size;
                m_gpu_memory = ::operator new(aligned_size * buffer_count, gpu_heap, buffer_alignment);

                /* Initialize buffers */
                m_buffer_array.Initialize(heap, buffer_count);
                for (u32 i = 0; i < buffer_count; ++i) {
                    m_buffer_array[i].Initialize(reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(m_gpu_memory) + (i * aligned_size)), std::addressof(buffer_info));
                }

                return;
            }

            u32 Allocate(u32 memory_size) {
                return vp::util::InterlockedFetchAddAcquire(std::addressof(m_memory_iter), memory_size);
            }

            template <typename T>
            ALWAYS_INLINE T *GetOffset(u32 offset) const {
                return reinterpret_cast<T*>(reinterpret_cast<uintptr_t>(m_gpu_memory) + m_size * m_current_index + offset);
            }

            void SetBufferIndex(u32 index) {
                VP_ASSERT(index < m_buffer_array.GetCount());
                m_last_index    = m_current_index;
                m_current_index = index;
                m_memory_iter   = 0;
            }
	};
}
