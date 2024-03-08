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
#include <ares.h>

namespace awn::res {

    void ResourceBinder::HandleResourceUnitError() {

        /* Convert resource unit error to status */
        if (m_resource_unit != nullptr) { m_resource_unit->SetBinderStatusFromError(this); }

        this->HandleMemoryAllocationError();
        this->Finalize();

        return;
    }

    void ResourceBinder::InitializeResource(ResourceUserContext *resource_user_context) {

        /* TODO; provide memory allocation error handling */
        auto finalize_on_error_guard = SCOPE_GUARD {
            this->Finalize();
        };
        ON_SCOPE_EXIT {
            this->HandleMemoryAllocationError();
        };

        /* Nothing to do if no resource unit */
        if (m_resource_unit == nullptr) { finalize_on_error_guard.Cancel(); return; }

        /* Try initialize resource */
        if (m_resource_unit->IsResourceInitialized() == false) {
            const bool result = m_resource_unit->InitializeResource(resource_user_context);
            if (result == false) { m_status = Status::FailedToInitializeResource; return; }
        }
        finalize_on_error_guard.Cancel();

        /* Update status for initialization */
        if (m_status == Status::InLoad) {
            m_status = Status::ResourceInitialized;
        }

        return;
    }
    
    void ResourceBinder::Finalize() {

        /* Nothing to do if no load */
        if (m_load_guard == false) { return; }

        /* Set finalize flag and clear init and load guard */
        m_is_finalize = (m_is_finalize & 0xf8) | 0x4;

        /* Release task */
        if (m_watcher.HasTask() == true) {
            m_watcher.CancelTask();
        }

        /* Nothing else to do if no resource unit */
        if (m_resource_unit == nullptr) { m_status = Status::NoResourceUnitOnFinalize; return; }

        /* Finalize binder impl */
        AsyncResourceManager *manager = AsyncResourceManager::GetInstance();
        manager->FinalizeBinder(this);

        return;
    }

    Result ResourceBinder::TryLoadSync(const char *file_path, const AsyncResourceLoadInfo *bind_info) {

        /* Enforce load guard */
        VP_ASSERT(m_load_guard != true);
        m_state_mask = (m_state_mask & 0xf0) | 0x2;

        AsyncResourceManager *manager = AsyncResourceManager::GetInstance();
        return manager->TryLoadSync(file_path, this, bind_info);
    }

    Result ResourceBinder::TryLoadAsync(const char *file_path, const AsyncResourceLoadInfo *bind_info) {

        /* Enforce load guard */
        VP_ASSERT(m_load_guard != true);
        m_state_mask = (m_state_mask & 0xf0) | 0x2;

        AsyncResourceManager *manager = AsyncResourceManager::GetInstance();
        return manager->TryLoadAsync(file_path, this, bind_info);
    }

    Result ResourceBinder::ReferenceBinderAsync(ResourceBinder *binder_to_reference) {

        /* Enforce load guard */
        VP_ASSERT(m_load_guard != true);
        m_state_mask = (m_state_mask & 0xf0) | 0x2;

        /* Set resource unit */
        ResourceUnit *resource_unit = binder_to_reference->m_resource_unit;
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
        return AsyncResourceManager::GetInstance()->ReferenceLocalArchiveBinder(this);
    }

    void ResourceBinder::WaitForLoad() {

        /* Wait for async task */
        m_watcher.WaitForCompletion();

        /* Wait for resource unit to initialize */
        if (m_resource_unit != nullptr) { m_resource_unit->WaitForLoad(); }

        return;
    }

    bool ResourceBinder::Complete(ResourceUserContext *resource_user_context) {

        /* Check if already complete */
        if (m_complete_guard == true)      { return true; }
        /* Check if not finalized or complete, but in load */
        if ((m_state_mask & 0xe) != 2)       { return false; }
        /* Check if the load task is still pending */
        if (m_watcher.IsPending() == true) { return false; }
        /* Check if resource unit is in load */
        if (m_resource_unit != nullptr && m_resource_unit->IsInLoad() == true) { return false; }

        /* Finalize on exit guard */
        Status exit_status = Status::Uninitialized;
        auto finalize_error_guard = SCOPE_GUARD {
            m_status = exit_status;
            this->Finalize();
        };

        /* Complete flag update guard */
        auto complete_guard = SCOPE_GUARD {
            m_complete_guard = true;
        };

        /* Check if resource unit does not exists */
        if (m_resource_unit == nullptr && m_watcher.IsCancelled() == true) { exit_status = Status::NoResourceUnitOnFinalize; return true; }
        /* Check if the binder had an error */
        if (this->IsErrorStatus() == true) { exit_status = m_status; return true; }
        /* Check if uninitialized */
        if (m_resource_unit == nullptr || m_status == Status::Uninitialized) { return false; }

        /* Switch for 4 cases, uninitialized, requires initialize, handle error, complete */
        switch (m_resource_unit->m_status) {
            case ResourceUnit::Status::Uninitialized:
            case ResourceUnit::Status::InLoad:
            default:
                complete_guard.Cancel();
                finalize_error_guard.Cancel();
                return false;
            case ResourceUnit::Status::Loaded:
            case ResourceUnit::Status::InResourceInitialize:
            case ResourceUnit::Status::ResourcePreFinalized:
            case ResourceUnit::Status::ResourceInitialized:
                this->InitializeResource(resource_user_context);
                break;
            case ResourceUnit::Status::Error:
                this->HandleResourceUnitError();
                break;
            case ResourceUnit::Status::FailedToInitializeResource:
            case ResourceUnit::Status::FailedToPostInitializeResource:
                exit_status = Status::FailedToInitializeResource;
                return false;
            case ResourceUnit::Status::ResourcePostInitialized:
                if (m_status == Status::InLoad) {
                    m_status = Status::ResourceInitialized;
                }
                this->HandleMemoryAllocationError();
                break;
        }

        finalize_error_guard.Cancel();

        return true;
    }
}
