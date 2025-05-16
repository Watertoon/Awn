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

namespace awn::grr {

    namespace {
        constinit bool cDisplayManagerInitGuard = false;
    }

    void InitializeDisplayManager(mem::Heap *heap) {

        /* Set guard */
        VP_ASSERT(!cDisplayManagerInitGuard);
        cDisplayManagerInitGuard = true;

        /* Create Direct3d11, DirectComposition, and Composition Swapchain context */
        impl::ContextDxgi11 *context = impl::ContextDxgi11::CreateInstance(heap);
        VP_ASSERT(context != nullptr);
        context->Initialize();

        /* Create Display manager */
        impl::DisplayManager *manager = impl::DisplayManager::CreateInstance(heap);
        VP_ASSERT(manager != nullptr);
        manager->Initialize(heap);

        return;
    }

    void FinalizeDisplayManager() {

        /* Clear guard */
        VP_ASSERT(cDisplayManagerInitGuard);
        cDisplayManagerInitGuard = false;

        /* Destroy Display manager */
        impl::DisplayManager *manager = impl::DisplayManager::GetInstance();
        VP_ASSERT(manager != nullptr);
        manager->Finalize();
        impl::DisplayManager::DeleteInstance();

        /* Destroy Direct3d11, DirectComposition, and Composition Swapchain context */
        impl::ContextDxgi11 *context = impl::ContextDxgi11::GetInstance();
        VP_ASSERT(context != nullptr);
        context->Finalize();
        impl::ContextDxgi11::DeleteInstance();

        return;
    }
    
    void SubmitAndPresentAsync(gfx::CommandList *command_list_array, u32 command_list_count) {
        impl::DisplayManager *manager = impl::DisplayManager::GetInstance();
        VP_ASSERT(manager != nullptr);
        manager->SubmitAndPresentAsync(command_list_array, command_list_count);
    }

    void WaitAndCompletePresent() {
        impl::DisplayManager *manager = impl::DisplayManager::GetInstance();
        VP_ASSERT(manager != nullptr);
        manager->WaitAndCompletePresent();
    }
}
