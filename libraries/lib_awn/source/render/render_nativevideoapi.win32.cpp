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

namespace awn::render {

    namespace {
        constinit bool cNativeVideoInitGuard = false;
    }

    void InitializeNativeVideo(mem::Heap *heap) {
        VP_ASSERT(!cNativeVideoInitGuard);
        cNativeVideoInitGuard = true;
        impl::ContextDxgi *context = impl::ContextDxgi::CreateInstance(heap);
        VP_ASSERT(context != nullptr);
        context->Initialize();
        impl::NativeVideoManager *manager = impl::NativeVideoManager::CreateInstance(heap);
        VP_ASSERT(manager != nullptr);
        manager->Initialize();
    }

    void FinalizeNativeVideo() {
        VP_ASSERT(cNativeVideoInitGuard);
        impl::NativeVideoManager *manager = impl::NativeVideoManager::GetInstance();
        VP_ASSERT(manager != nullptr);
        manager->Finalize();
        impl::ContextDxgi *context = impl::ContextDxgi::GetInstance();
        VP_ASSERT(context != nullptr);
        context->Finalize();
    }

    void CalculateNativeVideo() {
        VP_ASSERT(cNativeVideoInitGuard);
        impl::NativeVideoManager *manager = impl::NativeVideoManager::GetInstance();
        manager->Calculate();
    }
}
