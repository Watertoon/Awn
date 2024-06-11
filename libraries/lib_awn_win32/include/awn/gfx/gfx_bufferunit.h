#pragma once

namespace awn::gfx {

    struct BufferUnitInfo {
        const BufferInfo      *buffer_info;
        const void            *copy_memory;
        GpuMemoryRequirements  memory_requirements;
        MemoryPropertyFlags    memory_property_flags;
        u32                    priority;
    };

    class BufferUnit {
        public:
            friend class AsyncGpuResourceManager;
            friend class BufferBinder;
        public:
            static constexpr u32 cSmallMemoryCopySize = vp::util::c4KB;
        public:
            enum class Status : u32 {
                Uninitialized = 0,
                Freed         = 1,
                InRegister    = 2,
                Registered    = 3,
                Error         = 4,
            };
        private:
            Status                 m_status;
            union {
                u32 m_state;
                struct {
                    u32 m_is_fail_create_api_resource : 1;
                    u32 m_is_fail_out_of_memory       : 1;
                    u32 m_is_fail_memory_allocation   : 1;
                    u32 m_is_fail_memory_mapping      : 1;
                    u32 m_reserve                     : 28;
                };
            };
            u32                    m_reference_count;
            u32                    m_priority;
            MemoryPropertyFlags    m_memory_property_flags;
            const void            *m_import_memory;
            VkDeviceMemory         m_vk_device_memory;
            GpuMemoryRequirements  m_memory_requirements;
            Buffer                 m_buffer;
            async::AsyncTask       m_transfer_task;
        public:
             BufferUnit() : m_status(), m_state(), m_reference_count(), m_priority(), m_import_memory(), m_vk_device_memory(), m_memory_requirements{}, m_buffer(), m_transfer_task() {/*...*/}
            ~BufferUnit() {/*...*/}

            bool Initialize(const BufferUnitInfo *buffer_unit_info);

            void Finalize();

            void RequestFinalize();
            bool RequestTransfer();

            constexpr void SetBinderStatusFromError(BufferBinder *buffer_binder) {
                if (m_is_fail_create_api_resource) { buffer_binder->m_status = BufferBinder::Status::CreateApiResourceError; }
                if (m_is_fail_out_of_memory)       { buffer_binder->m_status = BufferBinder::Status::OutOfMemoryError; }
                if (m_is_fail_memory_allocation)   { buffer_binder->m_status = BufferBinder::Status::MemoryAllocationError; }
                if (m_is_fail_memory_mapping)      { buffer_binder->m_status = BufferBinder::Status::MemoryMappingError; }
                return;
            }
        public:
            void Transfer();

            void WaitForTransfer();
        public:
            constexpr gfx::Buffer *GetBuffer() { return (m_status == Status::Registered) ? std::addressof(m_buffer) : nullptr; }

            constexpr u32 IncrementReferenceCount() { return ++m_reference_count; }
            constexpr u32 DecrementReferenceCount() { return --m_reference_count; }

            constexpr bool IsInRegister() const {
                const async::AsyncTask::Status status = m_transfer_task.GetStatus();
                if ((async::AsyncTask::Status::Uninitialized == status) || (async::AsyncTask::Status::Complete == status)) { return (m_status == Status::InRegister); }
                if (async::AsyncTask::Status::Cancelled != status)                                                         { return true; }
                return false;
            }
            constexpr bool IsRegistered() const { return m_status == Status::Registered; }
            constexpr bool IsErrorStatus() const { return m_status == Status::Error; }
    };

    constexpr bool BufferBinder::Complete() {
        /* Succeed if we are already complete */
        if (m_complete_guard) { return true; }
        /* Fail if we did not schedule a register, or have been released/completed since */
        if ((m_state & 0x7) != 1) { return false; }
        /* Fail if the memory task is still pending */
        if (m_watcher.IsPending()) { return false; }
        /* Fail if the texture unit is still in register */
        if (m_buffer_unit != nullptr && m_buffer_unit->IsInRegister() == true) { return false; }
        /* Succeed if the memory task got cancelled */
        if (m_buffer_unit == nullptr && m_watcher.IsCancelled() == true) { 
            m_status = Status::Cancelled; 
            return true; 
        }
        /* Succeed if the binder has an error result */
        if (this->IsErrorStatus() == true) { return true; }
        /* Fail if the memory task did not set a texture unit */
        if (m_buffer_unit == nullptr || m_status == Status::Uninitialized) { return false; }

        /* Update completion from TextureUnit status */
        switch(m_buffer_unit->m_status) {
            case BufferUnit::Status::Error:
            {
                /* Set error result */
                m_buffer_unit->SetBinderStatusFromError(this);
                break;
            }
            case BufferUnit::Status::Registered:
                break;
            default:
                return false;
        };

        /* Complete */
        m_complete_guard = true;

        return true;
    }

    constexpr Buffer *BufferBinder::GetBuffer() {
        /* Fail if we aren't complete */
        if (!m_complete_guard) { return nullptr; }
        /* Fail if we completed without a texture unit */
        if (m_buffer_unit == nullptr) { return nullptr; }
        /* Fail if an error occurred */
        if (this->IsErrorStatus()) { return nullptr; }
        /* Defer to texture unit */
        return m_buffer_unit->GetBuffer();
    }

    constexpr bool BufferBinder::IsInRegister() const {
        /* Succeed if our memory task is pending */
        if (m_watcher.IsPending()) { return true; }
        /* Fail we did not schedule a register */
        if (!m_register_guard) { return false; }
        /* Fail if we do not have a texture unit after the memory task */
        if (m_buffer_unit == nullptr) { return false; }
        /* Defer to the texture unit */
        return m_buffer_unit->IsInRegister();
    }

    constexpr bool BufferBinder::IsRegistered() const {
        /* Fail if we did not schedule a register */
        if (!m_register_guard) { return false; }
        /* Fail if the memory task is still pending */
        if (m_watcher.IsPending()) { return false; }
        /* Fail if the memory task did not set a texture unit */
        if (m_buffer_unit == nullptr) { return false; }
        /* Fail if we are not complete */
        if (!m_complete_guard) { return false; }
        /* Defer to the texture unit */
        return m_buffer_unit->IsRegistered();
    }

    constexpr bool BufferBinder::IsFailed() const {
        /* Check if we are in register*/
        if (this->IsInRegister()) { return false; }
        /* If we have a texture unit, see if it's failed */
        bool is_unit_failed = false;
        if (m_register_guard && m_buffer_unit != nullptr) {
            is_unit_failed = m_buffer_unit->IsErrorStatus();
        }
        return this->IsErrorStatus() | is_unit_failed;
    }

    constexpr bool BufferBinder::IsErrorStatus() const {
        return (static_cast<u32>(Status::CreateApiResourceError) <= static_cast<u32>(m_status));
    }
}
