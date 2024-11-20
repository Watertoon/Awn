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

namespace awn::res {

    void ResourceBinder::HandleResourceUnitError() {

        /* Convert resource unit error to status */
        if (m_resource_unit != nullptr) { m_resource_unit->SetBinderStatusFromError(this); }

        if (!this->IsReferenceBinder()) {            
            this->HandleMemoryAllocationError();
        }
        this->Release();

        return;
    }

    void ResourceBinder::InitializeResource(ResourceUserContext *resource_user_context) {

        /* TODO; provide memory allocation error handling */
        auto finalize_on_error_guard = SCOPE_GUARD {
            this->Release();
        };
        ON_SCOPE_EXIT {
            if (!this->IsReferenceBinder()) {            
                this->HandleMemoryAllocationError();
            }
        };

        /* Nothing to do if no resource unit */
        if (m_resource_unit == nullptr) { finalize_on_error_guard.Cancel(); return; }

        /* Try initialize resource */
        if (!m_resource_unit->IsResourceInitialized()) {
            const bool result = m_resource_unit->InitializeResource(resource_user_context);
            if (!result) { m_status = Status::FailedToInitializeResource; return; }
        }
        finalize_on_error_guard.Cancel();

        /* Update status for initialization */
        if (m_status == Status::InLoad) {
            m_status = Status::ResourceInitialized;
        }

        return;
    }
    
    void ResourceBinder::Release() {

        /* Nothing to do if no load */
        if (!m_load_guard) { return; }

        /* Set finalize flag and clear init and load guard */
        m_state_mask = (m_state_mask & 0xf8) | 0x4;

        /* Cancel watcher */
        if (m_watcher.HasTask()) {
            m_watcher.CancelTask();
        }

        /* Nothing else to do if no resource unit */
        if (m_resource_unit == nullptr) { m_status = Status::NoResourceUnitOnFinalize; return; }

        /* Finalize binder impl */
        impl::AsyncResourceManager *manager = impl::AsyncResourceManager::GetInstance();
        manager->FinalizeBinder(this);

        return;
    }

    Result ResourceBinder::TryLoadSync(const char *file_path, const AsyncResourceLoadInfo *bind_info) {

        /* Enforce load guard */
        RESULT_RETURN_IF(m_load_guard, ResultLoadGuardSet);
        m_state_mask = (m_state_mask & 0xf0) | 0x2;
        m_status = Status::Uninitialized;

        impl::AsyncResourceManager *manager = impl::AsyncResourceManager::GetInstance();
        return manager->TryLoadSync(file_path, this, bind_info);
    }

    Result ResourceBinder::TryLoadAsync(const char *file_path, const AsyncResourceLoadInfo *bind_info) {

        /* Enforce load guard */
        RESULT_RETURN_IF(m_load_guard, ResultLoadGuardSet);
        m_state_mask = (m_state_mask & 0xf0) | 0x2;
        m_status = Status::Uninitialized;

        impl::AsyncResourceManager *manager = impl::AsyncResourceManager::GetInstance();
        return manager->TryLoadAsync(file_path, this, bind_info);
    }

    Result ResourceBinder::ReferenceBinderAsync(ResourceBinder *binder_to_reference) {

        /* Enforce load guard */
        RESULT_RETURN_IF(m_load_guard, ResultLoadGuardSet);
        m_state_mask = (m_state_mask & 0xf0) | 0x2;
        m_status = Status::Uninitialized;

        /* Set resource unit */
        impl::ResourceUnit *resource_unit = binder_to_reference->m_resource_unit;
        RESULT_RETURN_IF(resource_unit == nullptr, ResultInvalidReferenceBinder);

        m_resource_unit = resource_unit;
        resource_unit->IncrementReference();

        /* Update status */
        m_status = Status::Referenced;

        RESULT_RETURN_SUCCESS;
    }

    Result ResourceBinder::ReferenceBinderSync(ResourceBinder *binder_to_reference) {

        /* Bind async */
        this->ReferenceBinderAsync(binder_to_reference);

        /* Complete */
        this->Complete(nullptr);

        RESULT_RETURN_SUCCESS;
    }

    Result ResourceBinder::ReferenceLocalArchiveSync() {
        return impl::AsyncResourceManager::GetInstance()->ReferenceLocalArchiveBinder(this);
    }

    void ResourceBinder::WaitForLoad() {

        /* Wait for control task */
        if (m_watcher.IsPending()) {
            m_watcher.WaitForCompletion();
        }

        /* Ensure resource unit */
        if (m_resource_unit == nullptr) { return; }

        /* Wait for load */
        m_resource_unit->WaitForLoad();

        return;
    }

    bool ResourceBinder::Complete(ResourceUserContext *resource_user_context) {

        /* Succeed if we previously completed */
        if (m_complete_guard) { return true; }
        /* Fail if we were finalized */
        if (m_is_finalize) { return false; }
        /* Fail if we are not loaded */
        if (!this->IsLoaded()) { return false; }

        /* Finalize for error on exit guard */
        Status exit_status = Status::Uninitialized;
        auto finalize_error_guard = SCOPE_GUARD {
            m_status = exit_status;
            this->Release();
        };

        /* Complete flag update guard */
        auto complete_guard = SCOPE_GUARD {
            m_complete_guard = true;
        };

        /* Succeed if we don't have a resource unit as a result of the watcher getting cancelled */
        if (m_resource_unit == nullptr && m_watcher.IsCancelled()) { exit_status = Status::NoResourceUnitOnFinalize; return true; }
        /* Succeed if the binder had an error */
        if (this->IsErrorStatus()) { exit_status = m_status; return true; }
        /* Fail if the binder is uninitialized */
        if (m_resource_unit == nullptr || m_status == Status::Uninitialized) { return false; }

        /* Switch for 5 cases of resource unit status:
         * Fail if uninitialized
         * Succeed with initialize resource
         * Succeed with an error status
         * Fail for resource initialization failure
         * Succeed for the resource having already been initialized
         */
        switch (m_resource_unit->m_status) {
            case impl::ResourceUnit::Status::Uninitialized:
            case impl::ResourceUnit::Status::InLoad:
            default:
                complete_guard.Cancel();
                finalize_error_guard.Cancel();
                return false;
            case impl::ResourceUnit::Status::Loaded:
            case impl::ResourceUnit::Status::InResourceInitialize:
            case impl::ResourceUnit::Status::ResourcePreFinalized:
            case impl::ResourceUnit::Status::ResourceInitialized:
                this->InitializeResource(resource_user_context);
                break;
            case impl::ResourceUnit::Status::Error:
                this->HandleResourceUnitError();
                break;
            case impl::ResourceUnit::Status::FailedToInitializeResource:
            case impl::ResourceUnit::Status::FailedToPostInitializeResource:
                exit_status = Status::FailedToInitializeResource;
                return false;
            case impl::ResourceUnit::Status::ResourcePostInitialized:
                if (m_status == Status::InLoad) {
                    m_status = Status::ResourceInitialized;
                }
                if (!this->IsReferenceBinder()) {            
                    this->HandleMemoryAllocationError();
                }
                break;
        }

        finalize_error_guard.Cancel();

        return true;
    }

    Resource *ResourceBinder::GetResourceDirect() const {
        /* Fail if the resource is not initialized */
        if (!this->IsResourceInitialized()) { return nullptr; }
        /* Get resource */
        Resource *resource = m_resource_unit->m_resource;
        return resource;
    }

    bool ResourceBinder::IsInLoad() const {
        /* Fail if we have not scheduled a load */
        if (!m_load_guard) { return false; }
        /* Succeed if the watcher is pending */
        if (m_watcher.IsPending()) { return true; }
        /* Fail if we do not have a resource unit, as the control task is pending or it failed */
        if (m_resource_unit == nullptr) { return false; }
        /* Defer to the resource unit */
        return m_resource_unit->IsInLoad();
    }

    bool ResourceBinder::IsLoaded() const {
        /* Fail if we are complete */
        if (m_complete_guard) { return false; }
        /* Fail if we are in load */
        return !this->IsInLoad();
    }

    bool ResourceBinder::IsResourceInitialized() const {
        /* Fail if we are still in load */
        if (this->IsInLoad()) { return false; }
        /* Fail if we are not complete */
        if (!m_complete_guard) { return false; }
        /* Fail if no resource unit */
        if (m_resource_unit == nullptr) { return false; }
        /* Defer to the resource unit */
        return m_resource_unit->IsResourceInitialized();
    }

    bool ResourceBinder::IsFailed() const {
        /* We haven't failed if we are still loading */
        if (this->IsInLoad()) { return false; }
        /* If we or our resource unit has an error status, we failed */
        const bool is_res_unit_failed = (m_load_guard && m_resource_unit != nullptr) ? m_resource_unit->IsErrorStatus() : false;
        return this->IsErrorStatus() | is_res_unit_failed;
    }
}
