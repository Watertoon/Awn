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

namespace awn::gfx::impl {

    class TextureUnit;
    class BufferUnit;
    class ShaderArchiveUnit;

    struct AsyncGpuResourceManagerInfo {
        u32    texture_unit_count;
        u32    buffer_unit_count;
        u32    shader_archive_unit_count;
        u32    memory_thread_priority;
        u32    transfer_thread_priority;
        u32    transfer_task_count;
        size_t max_memory_size_cpu_uncached;
        size_t max_memory_size_cpu_cached;
        size_t max_memory_size_gpu_uncached;
    };

	class AsyncGpuResourceManager {
        public:
            friend class TextureUnit;
            friend class BufferUnit;
            friend class ShaderArchiveUnit;
        public:
            static constexpr u32 cMaxTextureUnitCount       = 0xffff;
            static constexpr u32 cMaxBufferUnitCount        = 0xffff;
            static constexpr u32 cMaxShaderArchiveUnitCount = 0xffff;
            static constexpr u32 cMaxRegisterPriority       = 0x2;
        public:
            using MemberTaskFunction              = async::MemberTaskFunction<AsyncGpuResourceManager>;
            using TextureUnitIndexType            = vp::util::MinUIntType<cMaxTextureUnitCount>;
            using TextureUnitIndexAllocator       = vp::util::AtomicIndexAllocator<TextureUnitIndexType>;
            using TextureUnitArray                = vp::util::HeapArray<TextureUnit>;
            using BufferUnitIndexType             = vp::util::MinUIntType<cMaxBufferUnitCount>;
            using BufferUnitIndexAllocator        = vp::util::AtomicIndexAllocator<BufferUnitIndexType>;
            using BufferUnitArray                 = vp::util::HeapArray<BufferUnit>;
            using ShaderArchiveUnitIndexType      = vp::util::MinUIntType<cMaxShaderArchiveUnitCount>;
            using ShaderArchiveUnitIndexAllocator = vp::util::AtomicIndexAllocator<ShaderArchiveUnitIndexType>;
            using ShaderArchiveUnitArray          = vp::util::HeapArray<ShaderArchiveUnit>;
		private:
            mem::Heap                       *m_meta_data_heap;
            TextureUnitIndexAllocator        m_texture_unit_index_allocator;
			TextureUnitArray                 m_texture_unit_array;
            BufferUnitIndexAllocator         m_buffer_unit_index_allocator;
			BufferUnitArray                  m_buffer_unit_array;
            ShaderArchiveUnitIndexAllocator  m_shader_archive_unit_index_allocator;
			ShaderArchiveUnitArray           m_shader_archive_unit_array;
			async::AsyncQueue                m_memory_queue;
			async::AsyncQueue                m_transfer_queue;
            async::AsyncQueueThread         *m_memory_thread;
            async::AsyncQueueThread         *m_transfer_thread;
			async::AsyncTaskAllocator        m_register_task_allocator;
            MemberTaskFunction               m_task_function_texture_memory;
            async::StaticTaskFunction        m_task_function_texture_transfer;
            MemberTaskFunction               m_task_function_finalize_texture_unit;
            MemberTaskFunction               m_task_function_buffer_memory;
            async::StaticTaskFunction        m_task_function_buffer_transfer;
            MemberTaskFunction               m_task_function_finalize_buffer_unit;
            MemberTaskFunction               m_task_function_shader_archive_memory;
            async::StaticTaskFunction        m_task_function_shader_archive_transfer;
            MemberTaskFunction               m_task_function_finalize_shader_archive_unit;
            size_t                           m_max_memory_size_cpu_uncached;
            size_t                           m_memory_usage_cpu_uncached;
            size_t                           m_max_memory_size_cpu_cached;
            size_t                           m_memory_usage_cpu_cached;
            size_t                           m_max_memory_size_gpu_uncached;
            size_t                           m_memory_usage_gpu_uncached;
        public:
            AWN_SINGLETON_TRAITS(AsyncGpuResourceManager);
        public:
            constexpr AsyncGpuResourceManager() : m_texture_unit_index_allocator(), m_texture_unit_array(), m_buffer_unit_index_allocator(), m_buffer_unit_array(), m_shader_archive_unit_index_allocator(), m_shader_archive_unit_array(),
            m_memory_queue(), m_transfer_queue(), m_memory_thread(), m_transfer_thread(), m_register_task_allocator(),
            m_task_function_texture_memory(this, &AsyncGpuResourceManager::TextureMemoryExe), m_task_function_texture_transfer(TextureTransferExe), m_task_function_finalize_texture_unit(this, &AsyncGpuResourceManager::FinalizeTextureUnitExe),
            m_task_function_buffer_memory(this, &AsyncGpuResourceManager::BufferMemoryExe), m_task_function_buffer_transfer(BufferTransferExe), m_task_function_finalize_buffer_unit(this, &AsyncGpuResourceManager::FinalizeBufferUnitExe),
            m_task_function_shader_archive_memory(this, &AsyncGpuResourceManager::ShaderArchiveMemoryExe), m_task_function_shader_archive_transfer(ShaderArchiveTransferExe), m_task_function_finalize_shader_archive_unit(this, &AsyncGpuResourceManager::FinalizeShaderArchiveUnitExe),
            m_max_memory_size_cpu_uncached(), m_memory_usage_cpu_uncached(), m_max_memory_size_cpu_cached(), m_memory_usage_cpu_cached(), m_max_memory_size_gpu_uncached(), m_memory_usage_gpu_uncached()
            {/*...*/}
            ~AsyncGpuResourceManager() {/*...*/}

            void Initialize(mem::Heap *heap, const AsyncGpuResourceManagerInfo *mgr_info);
            void Finalize();
        public:
            void TextureMemoryExe(void *register_user_data);
            static void TextureTransferExe(void *texture_unit);
            void FinalizeTextureUnitExe(void *texture_unit);

            void BufferMemoryExe(void *register_user_data);
            static void BufferTransferExe(void *buffer_unit);
            void FinalizeBufferUnitExe(void *buffer_unit);

            void ShaderArchiveMemoryExe(void *register_user_data);
            static void ShaderArchiveTransferExe(void *shader_archive_unit);
            void FinalizeShaderArchiveUnitExe(void *shader_archive_unit);

            constexpr mem::Heap *GetMetaDataHeap() { return m_meta_data_heap; }
        public:
            Result ScheduleRegisterTexture(TextureBinder *texture_binder, const TextureRegisterInfo *register_info, bool is_sync);
            Result RegisterTextureAsync(TextureBinder *texture_binder, const TextureRegisterInfo *register_info);
            Result RegisterTextureSync(TextureBinder *texture_binder, const TextureRegisterInfo *register_info);
            void ReleaseTextureBinder(TextureBinder *texture_binder);

            Result ScheduleRegisterBuffer(BufferBinder *buffer_binder, const BufferRegisterInfo *register_info, bool is_sync);
            Result RegisterBufferAsync(BufferBinder *buffer_binder, const BufferRegisterInfo *register_info);
            Result RegisterBufferSync(BufferBinder *buffer_binder, const BufferRegisterInfo *register_info);
            void ReleaseBufferBinder(BufferBinder *buffer_binder);

            Result ScheduleRegisterShaderArchive(ShaderArchiveBinder *shader_archive_binder, const ShaderArchiveRegisterInfo *register_info, bool is_sync);
            Result RegisterShaderArchiveAsync(ShaderArchiveBinder *shader_archive_binder, const ShaderArchiveRegisterInfo *register_info);
            Result RegisterShaderArchiveSync(ShaderArchiveBinder *shader_archive_binder, const ShaderArchiveRegisterInfo *register_info);
            void ReleaseShaderArchiveBinder(ShaderArchiveBinder *shader_archive_binder);
        public:
            TextureUnit *AllocateTextureUnit();
            void FreeTextureUnit(TextureUnit *texture_unit);

            BufferUnit *AllocateBufferUnit();
            void FreeBufferUnit(BufferUnit *buffer_unit);

            ShaderArchiveUnit *AllocateShaderArchiveUnit();
            void FreeShaderArchiveUnit(ShaderArchiveUnit *shader_archive_unit);
        public:
            constexpr bool AddMemoryUsage(GpuMemoryRequirements memory_requirements, MemoryPropertyFlags memory_property_flags) {

                /* Add memory usage if within limits */
                const size_t aligned_size = vp::util::AlignUp(memory_requirements.size, memory_requirements.alignment);
                if (memory_property_flags == (MemoryPropertyFlags::CpuUncached | MemoryPropertyFlags::GpuUncached)) {
                    const size_t next_size = m_memory_usage_gpu_uncached + aligned_size;
                    if (m_memory_usage_gpu_uncached < next_size) { return false; }
                    m_memory_usage_gpu_uncached = next_size;
                } else if (memory_property_flags == (MemoryPropertyFlags::CpuUncached)) {
                    const size_t next_size = m_memory_usage_cpu_uncached + aligned_size;
                    if (m_max_memory_size_cpu_uncached < next_size) { return false; }
                    m_memory_usage_cpu_uncached = next_size;
                } else if (memory_property_flags == (MemoryPropertyFlags::CpuCached)) {
                    const size_t next_size = m_memory_usage_cpu_cached + aligned_size;
                    if (m_max_memory_size_cpu_cached < next_size) { return false; }
                    m_memory_usage_cpu_cached = next_size;
                } else {
                    return false;
                }

                return true;
            }

            constexpr void ReleaseMemoryUsage(GpuMemoryRequirements memory_requirements, MemoryPropertyFlags memory_property_flags) {

                /* Remove memory usage */
                const size_t aligned_size = vp::util::AlignUp(memory_requirements.size, memory_requirements.alignment);
                if (memory_property_flags == (MemoryPropertyFlags::CpuUncached | MemoryPropertyFlags::GpuUncached)) {
                    const size_t next_size = m_memory_usage_gpu_uncached - aligned_size;
                    m_memory_usage_gpu_uncached = next_size;
                } else if (memory_property_flags == (MemoryPropertyFlags::CpuUncached)) {
                    const size_t next_size = m_memory_usage_cpu_uncached - aligned_size;
                    m_memory_usage_cpu_uncached = next_size;
                } else if (memory_property_flags == (MemoryPropertyFlags::CpuCached)) {
                    const size_t next_size = m_memory_usage_cpu_cached - aligned_size;
                    m_memory_usage_cpu_cached = next_size;
                }

                return;
            }
	};
}
