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

namespace awn::gfx::impl {

    AWN_SINGLETON_TRAITS_IMPL(AsyncGpuResourceManager);

    void AsyncGpuResourceManager::Initialize(mem::Heap *heap, const AsyncGpuResourceManagerInfo *mgr_info) {

        /* Initialize meta data heap */
        if constexpr (0 < mem::cGpuMemoryHandleMetaDataSize) {
            const size_t block_size = mem::cGpuMemoryHandleMetaDataSize;
            const size_t memory_size = block_size * (mgr_info->texture_unit_count + mgr_info->buffer_unit_count);
            m_meta_data_heap = mem::UnitHeap::TryCreate("awn::gfx::GpuMemoryHandleMetaDataHeap", heap, memory_size, block_size, mem::cGpuMemoryHandleMetaDataAlignment);
        }

        /* Initialize unit arrays */
        m_texture_unit_index_allocator.Initialize(heap, mgr_info->texture_unit_count);
        m_texture_unit_array.Initialize(heap, mgr_info->texture_unit_count);
        m_buffer_unit_index_allocator.Initialize(heap, mgr_info->buffer_unit_count);
        m_buffer_unit_array.Initialize(heap, mgr_info->buffer_unit_count);
        m_shader_archive_unit_index_allocator.Initialize(heap, mgr_info->shader_archive_unit_count);
        m_shader_archive_unit_array.Initialize(heap, mgr_info->shader_archive_unit_count);

        /* Initialize task queues, threads, and allocator */
        const async::AsyncQueuePerSetCount per_set_count =  {
            .thread_count_primary = 1,
        };
        const async::AsyncQueueInfo queue_info_memory = {
            .priority_level_count = 3,
            .thread_set_count = 1,
            .per_set_count_array = std::addressof(per_set_count),
        };
        m_memory_queue.Initialize(heap, std::addressof(queue_info_memory));
        const async::AsyncQueueThreadInfo memory_thread_info = {
            .queue = std::addressof(m_memory_queue),
        };
        m_memory_thread = new (heap, alignof(async::AsyncQueueThread)) async::AsyncQueueThread(std::addressof(memory_thread_info), "AwnGfxMemoryThread", heap, vp::util::c16KB, mgr_info->memory_thread_priority);

        const async::AsyncQueueInfo queue_info_transfer = {
            .priority_level_count = 3,
            .thread_set_count = 1,
            .per_set_count_array = std::addressof(per_set_count),
        };
        m_transfer_queue.Initialize(heap, std::addressof(queue_info_transfer));
        const async::AsyncQueueThreadInfo transfer_thread_info = {
            .queue = std::addressof(m_transfer_queue),
        };
        m_transfer_thread = new (heap, alignof(async::AsyncQueueThread)) async::AsyncQueueThread(std::addressof(transfer_thread_info), "AwnGfxTransferThread", heap, vp::util::c32KB, mgr_info->transfer_thread_priority);

        const auto task_create_lambda = [](async::AsyncTaskAllocator *allocator, mem::Heap *heap) {
            VP_UNUSED(allocator);
            return new (heap, alignof(RegisterTask)) RegisterTask();
        };
        auto task_create_function = vp::util::MakeLambdaFunction<async::AsyncTaskForAllocator*(async::AsyncTaskAllocator*, mem::Heap*)>(task_create_lambda);
        m_register_task_allocator.Initialize(heap, std::addressof(task_create_function), mgr_info->transfer_task_count);

        return;
    }

   void AsyncGpuResourceManager::TextureMemoryExe(void *register_user_data) {

        /* Cast user data */
        RegisterTask::UserData *user_data      = reinterpret_cast<RegisterTask::UserData*>(register_user_data);
        TextureBinder          *texture_binder = reinterpret_cast<TextureBinder*>(user_data->binder);

        /* Get memory requirements */
        const TextureInfo     *texture_info        = reinterpret_cast<const TextureInfo*>(user_data->gfx_info);
        GpuMemoryRequirements  memory_requirements = Texture::GetMemoryRequirements(texture_info);

        /* Allocate TextureUnit */
        TextureUnit *texture_unit = this->AllocateTextureUnit();
        VP_ASSERT(texture_unit != nullptr);
        texture_binder->m_texture_unit = texture_unit;

        /* Initialize texture unit */
        const TextureUnitInfo texture_unit_info = {
            .texture_info          = texture_info,
            .import_mip_array      = user_data->copy_memory_array,
            .memory_requirements   = memory_requirements,
            .memory_property_flags = user_data->memory_property_flags,
            .priority              = user_data->priority,
        };
        texture_unit->Initialize(std::addressof(texture_unit_info));

        /* Request copy */
        texture_unit->RequestTransfer();

        return;
    }

    void AsyncGpuResourceManager::TextureTransferExe(void *texture_unit) {
        reinterpret_cast<TextureUnit*>(texture_unit)->Transfer();
    }

    void AsyncGpuResourceManager::FinalizeTextureUnitExe(void *texture_unit_data) {
        TextureUnit *texture_unit = reinterpret_cast<TextureUnit*>(texture_unit_data);
        this->FreeTextureUnit(texture_unit);
    }

    void AsyncGpuResourceManager::BufferMemoryExe(void *register_user_data) {

        /* Cast user data */
        RegisterTask::UserData *user_data = reinterpret_cast<RegisterTask::UserData*>(register_user_data);

        /* Get memory requirements */
        const BufferInfo      *buffer_info         = reinterpret_cast<const BufferInfo*>(user_data->gfx_info);
        GpuMemoryRequirements  memory_requirements = Buffer::GetMemoryRequirements(buffer_info);

        /* Allocate BufferUnit and set to binder */
        BufferUnit   *buffer_unit = this->AllocateBufferUnit();
        VP_ASSERT(buffer_unit != nullptr);
        BufferBinder *binder      = reinterpret_cast<BufferBinder*>(user_data->binder);
        binder->m_buffer_unit     = buffer_unit;

        /* Initialize buffer unit */
        const BufferUnitInfo buffer_unit_info = {
            .buffer_info           = buffer_info,
            .copy_memory           = user_data->copy_memory_array[0],
            .memory_requirements   = memory_requirements,
            .memory_property_flags = user_data->memory_property_flags,
            .priority              = user_data->priority,
        };
        buffer_unit->Initialize(std::addressof(buffer_unit_info));

        /* Request copy */
        buffer_unit->RequestTransfer();

        return;
    }

    void AsyncGpuResourceManager::BufferTransferExe(void *buffer_unit) {
        reinterpret_cast<BufferUnit*>(buffer_unit)->Transfer();
    }

    void AsyncGpuResourceManager::FinalizeBufferUnitExe(void *buffer_unit_data) {
        BufferUnit *buffer_unit = reinterpret_cast<BufferUnit*>(buffer_unit_data);
        this->FreeBufferUnit(buffer_unit);
    }

    void AsyncGpuResourceManager::ShaderArchiveMemoryExe(void *register_user_data) {

        /* Cast user data */
        RegisterTask::UserData *user_data = reinterpret_cast<RegisterTask::UserData*>(register_user_data);

        /* Allocate ShaderArchiveUnit */
        ShaderArchiveUnit *shader_archive_unit = this->AllocateShaderArchiveUnit();
        VP_ASSERT(shader_archive_unit != nullptr);

        /* Initialize */
        const ShaderArchiveUnitInfo unit_info = {
            .bnsh     = reinterpret_cast<vp::res::ResBnsh*>(user_data->res),
            .priority = user_data->priority,
        };
        shader_archive_unit->Initialize(std::addressof(unit_info));

        /* Request transfer */
        shader_archive_unit->RequestTransfer();

        return;
    }

    void AsyncGpuResourceManager::ShaderArchiveTransferExe(void *shader_archive_unit) {
        reinterpret_cast<ShaderArchiveUnit*>(shader_archive_unit)->Transfer();
    }

    void AsyncGpuResourceManager::FinalizeShaderArchiveUnitExe(void *shader_archive_unit_data) {
        ShaderArchiveUnit *shader_archive_unit = reinterpret_cast<ShaderArchiveUnit*>(shader_archive_unit_data);
        this->FreeShaderArchiveUnit(shader_archive_unit);
    }
    
    Result AsyncGpuResourceManager::ScheduleRegisterTexture(TextureBinder *texture_binder, const TextureRegisterInfo *register_info, bool is_sync) {

        /* Verify arguments */
        VP_ASSERT(texture_binder != nullptr);
        VP_ASSERT(register_info != nullptr);
        RESULT_RETURN_UNLESS(register_info->priority < cMaxRegisterPriority, ResultInvalidPriority);
        RESULT_RETURN_UNLESS(register_info->texture_info != nullptr, ResultNoTextureInfo);
        RESULT_RETURN_UNLESS(register_info->copy_mip_level_count == 0 || register_info->texture_info->mip_level_count == register_info->copy_mip_level_count, ResultMipLevelCountMismatch);
        RESULT_RETURN_UNLESS(register_info->texture_info->mip_level_count < Context::cTargetMaxMipLevelCount, ResultMaxMipLevelCountExceeded);
        RESULT_RETURN_UNLESS(register_info->memory_property_flags == MemoryPropertyFlags::CpuUncached ||
                             register_info->memory_property_flags == MemoryPropertyFlags::CpuCached ||
                             register_info->memory_property_flags == (MemoryPropertyFlags::GpuCached | MemoryPropertyFlags::CpuUncached), ResultInvalidMemoryPropertyFlags);

        /* Schedule register task */
        RegisterTaskPushInfo push_info = {
            async::AsyncTaskPushInfo {
                .queue_thread  = m_memory_thread,
                .task_function = std::addressof(m_task_function_texture_memory),
                .priority      = register_info->priority + 1,
                .is_sync       = is_sync,
            },
            /*.binder                =*/ texture_binder,
            /*.res                   =*/ nullptr,
            /*.gfx_info              =*/ register_info->texture_info,
            /*.memory_property_flags =*/ register_info->memory_property_flags,
            /*.copy_count            =*/ register_info->copy_mip_level_count,
            /*.copy_memory           =*/ register_info->copy_mip_level_array,
        };
        async::AsyncTaskScheduleInfo schedule_info = {
            .push_info = std::addressof(push_info),
            .watcher   = std::addressof(texture_binder->m_watcher),
        };
        m_register_task_allocator.ScheduleTask(std::addressof(schedule_info));

        RESULT_RETURN_SUCCESS;
    }

    Result AsyncGpuResourceManager::RegisterTextureAsync(TextureBinder *texture_binder, const TextureRegisterInfo *register_info) {
                return this->ScheduleRegisterTexture(texture_binder, register_info, false);
    }

    Result AsyncGpuResourceManager::RegisterTextureSync(TextureBinder *texture_binder, const TextureRegisterInfo *register_info) {
        RESULT_TRY(this->ScheduleRegisterTexture(texture_binder, register_info, true));
        texture_binder->WaitForRegister();
        texture_binder->Complete();
        RESULT_RETURN_SUCCESS;
    }

    void AsyncGpuResourceManager::ReleaseTextureBinder(TextureBinder *texture_binder) {

        /* Release texture unit from binder */
        TextureUnit *texture_unit = texture_binder->m_texture_unit;
        if (texture_unit == nullptr) { return; }
        texture_binder->m_texture_unit = nullptr;

        /* Decrement ref count */
        const u32 ref_count = texture_unit->DecrementReferenceCount();
        if (ref_count != 0) { return; }

        /* Defer free */
        texture_unit->RequestFinalize();

        return;
    }

    Result AsyncGpuResourceManager::ScheduleRegisterBuffer(BufferBinder *buffer_binder, const BufferRegisterInfo *register_info, bool is_sync) {

        /* Verify arguments */
        VP_ASSERT(buffer_binder != nullptr);
        VP_ASSERT(register_info != nullptr);
        RESULT_RETURN_UNLESS(register_info->priority < cMaxRegisterPriority, ResultInvalidPriority);
        RESULT_RETURN_UNLESS(register_info->buffer_info != nullptr, ResultNoBufferInfo);
        RESULT_RETURN_UNLESS(register_info->memory_property_flags == MemoryPropertyFlags::CpuUncached ||
                             register_info->memory_property_flags == MemoryPropertyFlags::CpuCached ||
                             register_info->memory_property_flags == (MemoryPropertyFlags::GpuCached | MemoryPropertyFlags::CpuUncached), ResultInvalidMemoryPropertyFlags);

        /* Schedule register task */
        RegisterTaskPushInfo push_info = {
            async::AsyncTaskPushInfo {
                .queue_thread  = m_memory_thread,
                .task_function = std::addressof(m_task_function_buffer_memory),
                .priority      = register_info->priority + 1,
                .is_sync       = is_sync,
            },
            /*.binder                =*/ buffer_binder,
            /*.res                   =*/ nullptr,
            /*.gfx_info              =*/ register_info->buffer_info,
            /*.memory_property_flags =*/ register_info->memory_property_flags,
            /*.copy_count            =*/ (register_info->copy_memory != nullptr) ? 1u : 0u,
            /*.copy_memory           =*/ std::addressof(register_info->copy_memory),
        };
        async::AsyncTaskScheduleInfo schedule_info = {
            .push_info = std::addressof(push_info),
            .watcher   = std::addressof(buffer_binder->m_watcher),
        };
        m_register_task_allocator.ScheduleTask(std::addressof(schedule_info));

        RESULT_RETURN_SUCCESS;
    }

    Result AsyncGpuResourceManager::RegisterBufferAsync(BufferBinder *buffer_binder, const BufferRegisterInfo *register_info) {
        return this->ScheduleRegisterBuffer(buffer_binder, register_info, false);
    }

    Result AsyncGpuResourceManager::RegisterBufferSync(BufferBinder *buffer_binder, const BufferRegisterInfo *register_info) {
        RESULT_TRY(this->ScheduleRegisterBuffer(buffer_binder, register_info, true));
        buffer_binder->WaitForRegister();
        buffer_binder->Complete();
        RESULT_RETURN_SUCCESS;
    }

    void AsyncGpuResourceManager::ReleaseBufferBinder(BufferBinder *buffer_binder) {
        
        /* Release buffer unit from binder */
        BufferUnit *buffer_unit = buffer_binder->m_buffer_unit;
        if (buffer_unit == nullptr) { return; }
        buffer_binder->m_buffer_unit = nullptr;

        /* Decrement ref count */
        const u32 ref_count = buffer_unit->DecrementReferenceCount();
        if (ref_count != 0) { return; }

        /* Defer free */
        buffer_unit->RequestFinalize();

        return;
    }

    Result AsyncGpuResourceManager::ScheduleRegisterShaderArchive(ShaderArchiveBinder *shader_archive_binder, const ShaderArchiveRegisterInfo *register_info, bool is_sync) {
        
        /* Verify arguments */
        VP_ASSERT(shader_archive_binder != nullptr);
        VP_ASSERT(register_info != nullptr);
        RESULT_RETURN_UNLESS(register_info->priority < cMaxRegisterPriority, ResultInvalidPriority);
        RESULT_RETURN_UNLESS(register_info->bnsh != nullptr, ResultNoBnsh);

        /* Schedule register task */
        RegisterTaskPushInfo push_info = {
            async::AsyncTaskPushInfo {
                .queue_thread  = m_memory_thread,
                .task_function = std::addressof(m_task_function_buffer_memory),
                .priority      = register_info->priority + 1,
                .is_sync       = is_sync,
            },
            /*.binder =*/ shader_archive_binder,
            /*.res    =*/ register_info->bnsh,
        };
        async::AsyncTaskScheduleInfo schedule_info = {
            .push_info = std::addressof(push_info),
            .watcher   = std::addressof(shader_archive_binder->m_watcher),
        };
        m_register_task_allocator.ScheduleTask(std::addressof(schedule_info));

        RESULT_RETURN_SUCCESS;
    }

    Result AsyncGpuResourceManager::RegisterShaderArchiveAsync(ShaderArchiveBinder *shader_archive_binder, const ShaderArchiveRegisterInfo *register_info) {
        return this->ScheduleRegisterShaderArchive(shader_archive_binder, register_info, false);
    }

    Result AsyncGpuResourceManager::RegisterShaderArchiveSync(ShaderArchiveBinder *shader_archive_binder, const ShaderArchiveRegisterInfo *register_info) {
        RESULT_TRY(this->ScheduleRegisterShaderArchive(shader_archive_binder, register_info, true));
        shader_archive_binder->WaitForRegister();
        shader_archive_binder->Complete();
        RESULT_RETURN_SUCCESS;
    }

    void AsyncGpuResourceManager::ReleaseShaderArchiveBinder(ShaderArchiveBinder *shader_archive_binder) {
        
        /* Release buffer unit from binder */
        ShaderArchiveUnit *shader_archive_unit = shader_archive_binder->m_shader_archive_unit;
        if (shader_archive_unit == nullptr) { return; }
        shader_archive_binder->m_shader_archive_unit = nullptr;

        /* Decrement ref count */
        const u32 ref_count = shader_archive_unit->DecrementReferenceCount();
        if (ref_count != 0) { return; }

        /* Defer free */
        shader_archive_unit->RequestFinalize();

        return;
    }

    TextureUnit *AsyncGpuResourceManager::AllocateTextureUnit() {

        /* Try allocate index */
        const TextureUnitIndexType index = m_texture_unit_index_allocator.Allocate();
        if (index != TextureUnitIndexAllocator::cInvalidEntryIndex) { return std::addressof(m_texture_unit_array[index]); }

        /* Attempt to free texture units by processing free requests */
        m_memory_thread->GetQueue()->ForceCalcSyncOnThread(m_memory_thread, 0);

        /* Try allocate index */
        const TextureUnitIndexType retry_index = m_texture_unit_index_allocator.Allocate();
        if (retry_index != TextureUnitIndexAllocator::cInvalidEntryIndex) { return std::addressof(m_texture_unit_array[index]); }

        return nullptr;
    }

    void AsyncGpuResourceManager::FreeTextureUnit(TextureUnit *texture_unit) {
        texture_unit->Finalize();
        const TextureUnitIndexType index = m_texture_unit_array.IndexOf(texture_unit);
        m_texture_unit_index_allocator.Free(index);
    }

    BufferUnit *AsyncGpuResourceManager::AllocateBufferUnit() {

        /* Try allocate index */
        const BufferUnitIndexType index = m_buffer_unit_index_allocator.Allocate();
        if (index != BufferUnitIndexAllocator::cInvalidEntryIndex) { return std::addressof(m_buffer_unit_array[index]); }

        /* Attempt to free buffer units by processing free requests */
        m_memory_thread->GetQueue()->ForceCalcSyncOnThread(m_memory_thread, 0);

        /* Try allocate index */
        const BufferUnitIndexType retry_index = m_buffer_unit_index_allocator.Allocate();
        if (retry_index != BufferUnitIndexAllocator::cInvalidEntryIndex) { return std::addressof(m_buffer_unit_array[index]); }

        return nullptr;
    }

    void AsyncGpuResourceManager::FreeBufferUnit(BufferUnit *buffer_unit) {
        buffer_unit->Finalize();
        const BufferUnitIndexType index = m_buffer_unit_array.IndexOf(buffer_unit);
        m_buffer_unit_index_allocator.Free(index);
    }

    ShaderArchiveUnit *AsyncGpuResourceManager::AllocateShaderArchiveUnit() {

        /* Try allocate index */
        const ShaderArchiveUnitIndexType index = m_shader_archive_unit_index_allocator.Allocate();
        if (index != ShaderArchiveUnitIndexAllocator::cInvalidEntryIndex) { return std::addressof(m_shader_archive_unit_array[index]); }

        /* Attempt to free buffer units by processing free requests */
        m_memory_thread->GetQueue()->ForceCalcSyncOnThread(m_memory_thread, 0);

        /* Try allocate index */
        const ShaderArchiveUnitIndexType retry_index = m_shader_archive_unit_index_allocator.Allocate();
        if (retry_index != ShaderArchiveUnitIndexAllocator::cInvalidEntryIndex) { return std::addressof(m_shader_archive_unit_array[retry_index]); }

        return nullptr;
    }

    void AsyncGpuResourceManager::FreeShaderArchiveUnit(ShaderArchiveUnit *shader_archive_unit) {
        shader_archive_unit->Finalize();
        const ShaderArchiveUnitIndexType index = m_shader_archive_unit_array.IndexOf(shader_archive_unit);
        m_shader_archive_unit_index_allocator.Free(index);
    }
}
