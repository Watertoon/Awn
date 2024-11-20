#pragma once

namespace awn::gfx::impl {

    struct ShaderArchiveUnitInfo {
        vp::res::ResBnsh *bnsh;
        u32               priority;
    };

    class ShaderArchiveUnit {
        public:
            friend class AsyncGpuResourceManager;
            friend class gfx::ShaderArchiveBinder;
        public:
            static constexpr u32 cSmallVariationCount = 0x10;
        public:
            enum class Status : u32 {
                Uninitialized = 0,
                Freed         = 1,
                InRegister    = 2,
                Registered    = 3,
                Error         = 4,
            };
        private:
            Status                           m_status;
            union {
                u32 m_state;
                struct {
                    u32 m_is_fail_create_api_resource : 1;
                    u32 m_is_fail_invalid_file        : 1;
                    u32 m_reserve                     : 31;
                };
            };
            u32                              m_reference_count;
            u32                              m_priority;
            vp::res::ResBnsh                *m_bnsh;
            vp::res::ResBnshShaderContainer *m_bnsh_container;
            async::AsyncTask                 m_transfer_task;
        public:
            ShaderArchiveUnit() : m_status(), m_state(), m_reference_count(), m_priority(), m_bnsh(), m_bnsh_container(), m_transfer_task() {/*...*/}
            ~ShaderArchiveUnit() {/*...*/}

            bool Initialize(const ShaderArchiveUnitInfo *shader_archive_unit_info);

            void Finalize();

            void RequestFinalize();

            bool RequestTransfer();

            constexpr void SetBinderStatusFromError(ShaderArchiveBinder *shader_archive_binder) {
                if (m_is_fail_create_api_resource) { shader_archive_binder->m_status = ShaderArchiveBinder::Status::CreateApiResourceError; }
                if (m_is_fail_invalid_file)        { shader_archive_binder->m_status = ShaderArchiveBinder::Status::InvalidFileError; }
                return;
            }
        public:
            void Transfer();

            void WaitForTransfer();
        public:
            constexpr vp::res::ResBnshShaderContainer *GetShaderContainer() { return (m_status == Status::Registered) ? m_bnsh_container : nullptr; }

            constexpr u32 IncrementReferenceCount() { return ++m_reference_count; }
            constexpr u32 DecrementReferenceCount() { return --m_reference_count; }

            constexpr bool IsInRegister() const {
                if (m_transfer_task.IsCancelled()) { return true; }
                if (m_transfer_task.IsComplete()) { return (m_status == Status::InRegister); }
                return false;
            }
            constexpr bool IsRegistered() const { return m_status == Status::Registered; }
            constexpr bool IsErrorStatus() const { return m_status == Status::Error; }
    };
}

namespace awn::gfx {
    
    constexpr bool ShaderArchiveBinder::Complete() {
        /* Succeed if we are already complete */
        if (m_complete_guard) { return true; }
        /* Fail if we did not schedule a register, or have been released/completed since */
        if ((m_state & 0x7) != 1) { return false; }
        /* Fail if the memory task is still pending */
        if (m_watcher.IsPending()) { return false; }
        /* Fail if the texture unit is still in register */
        if (m_shader_archive_unit != nullptr && m_shader_archive_unit->IsInRegister() == true) { return false; }
        /* Succeed if the memory task got cancelled */
        if (m_shader_archive_unit == nullptr && m_watcher.IsCancelled() == true) { 
            m_status = Status::Cancelled; 
            return true; 
        }
        /* Succeed if the binder has an error result */
        if (this->IsErrorStatus() == true) { return true; }
        /* Fail if the memory task did not set a shader archive unit */
        if (m_shader_archive_unit == nullptr || m_status == Status::Uninitialized) { return false; }

        /* Update completion from TextureUnit status */
        switch(m_shader_archive_unit->m_status) {
            case impl::ShaderArchiveUnit::Status::Error:
            {
                /* Set error result */
                m_shader_archive_unit->SetBinderStatusFromError(this);
                break;
            }
            case impl::ShaderArchiveUnit::Status::Registered:
                break;
            default:
                return false;
        };

        /* Complete */
        m_complete_guard = true;

        return true;
    }

    constexpr vp::res::ResBnshShaderContainer *ShaderArchiveBinder::GetShaderContainer() {
        /* Fail if we aren't complete */
        if (!m_complete_guard) { return nullptr; }
        /* Fail if we completed without a texture unit */
        if (m_shader_archive_unit == nullptr) { return nullptr; }
        /* Fail if an error occurred */
        if (this->IsErrorStatus()) { return nullptr; }
        /* Defer to texture unit */
        return m_shader_archive_unit->GetShaderContainer();
    }

    constexpr bool ShaderArchiveBinder::IsInRegister() const {
        /* Succeed if our memory task is pending */
        if (m_watcher.IsPending()) { return true; }
        /* Fail we did not schedule a register */
        if (!m_register_guard) { return false; }
        /* Fail if we do not have a texture unit after the memory task */
        if (m_shader_archive_unit == nullptr) { return false; }
        /* Defer to the texture unit */
        return m_shader_archive_unit->IsInRegister();
    }

    constexpr bool ShaderArchiveBinder::IsRegistered() const {
        /* Fail if we did not schedule a register */
        if (!m_register_guard) { return false; }
        /* Fail if the memory task is still pending */
        if (m_watcher.IsPending()) { return false; }
        /* Fail if the memory task did not set a shader archive unit */
        if (m_shader_archive_unit == nullptr) { return false; }
        /* Fail if we are not complete */
        if (!m_complete_guard) { return false; }
        /* Defer to the texture unit */
        return m_shader_archive_unit->IsRegistered();
    }

    constexpr bool ShaderArchiveBinder::IsFailed() const {
        /* Fail if we are still registering */
        if (this->IsInRegister()) { return false; }
        /* Fail if an error is set in either the binder or unit (if we have one) */
        bool is_unit_failed = false;
        if (m_register_guard && m_shader_archive_unit != nullptr) {
            is_unit_failed = m_shader_archive_unit->IsErrorStatus();
        }
        return this->IsErrorStatus() | is_unit_failed;
    }

    constexpr bool ShaderArchiveBinder::IsErrorStatus() const {
        return (static_cast<u32>(Status::CreateApiResourceError) <= static_cast<u32>(m_status));
    }
}
