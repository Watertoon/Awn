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

     ResourceFactoryBase::~ResourceFactoryBase() {
        impl::ResourceFactoryManager *mgr = impl::ResourceFactoryManager::GetInstance();
        if (mgr == nullptr) { return; }
        mgr->RemoveResourceFactory(this);
    }

	Result ResourceFactoryBase::LoadResourceWithDecompressor(Resource **out_resource, const char *path, ResourceLoadContext *resource_load_context, IDecompressor *decompressor)  const{

		/* Integrity checks */
		RESULT_RETURN_UNLESS(out_resource != nullptr, ResultNullOutputResource);

		/* Allocate resource */
		Resource *resource = this->AllocateResource(resource_load_context->resource_heap, this->GetResourceAlignment());
		RESULT_RETURN_UNLESS(resource != nullptr, ResultResourceAllocationFailed);
        auto fail_guard0 = SCOPE_GUARD {
			delete resource;
        };
		RESULT_RETURN_UNLESS(decompressor != nullptr, ResultInvalidDecompressor);

		/* Load decompressed */
		RESULT_TRY(decompressor->LoadFileWithDecompression(nullptr, nullptr, path, std::addressof(resource_load_context->file_load_context), resource_load_context->file_device));

		/* Setup resource */
		resource->m_file = resource_load_context->file_load_context.file;
		resource->m_file_size = resource_load_context->file_load_context.file_size;

		/* Resource pre-prepare */
		RESULT_TRY(resource->OnFileLoad(resource_load_context->resource_heap, resource_load_context->file_load_context.file, resource_load_context->file_load_context.file_size));

		/* Set output */
        fail_guard0.Cancel();
		*out_resource = resource;

		RESULT_RETURN_SUCCESS;
	}

	Result ResourceFactoryBase::LoadResource(Resource **out_resource, const char *path, ResourceLoadContext *resource_load_context) const {

		/* Integrity checks */
		RESULT_RETURN_UNLESS(out_resource != nullptr, ResultNullOutputResource);

		/* Allocate resource */
		Resource *resource = this->AllocateResource(resource_load_context->resource_heap, this->GetResourceAlignment());
		RESULT_RETURN_UNLESS(resource != nullptr, ResultResourceAllocationFailed);
        auto fail_guard0 = SCOPE_GUARD {
			delete resource;
        };

		/* Load file */
		if (resource_load_context->file_device == nullptr) {
			RESULT_TRY(res::LoadFile(path, std::addressof(resource_load_context->file_load_context)));
		} else {
			RESULT_TRY(resource_load_context->file_device->LoadFile(path, std::addressof(resource_load_context->file_load_context)));
		}

		/* Setup resource */
		resource->m_file      = resource_load_context->file_load_context.file;
		resource->m_file_size = resource_load_context->file_load_context.file_size;

		/* Resource load callback */
		RESULT_TRY(resource->OnFileLoad(resource_load_context->resource_heap, resource_load_context->file_load_context.file, resource_load_context->file_load_context.file_size));

		/* Set output */
        fail_guard0.Cancel();
		*out_resource = resource;

		RESULT_RETURN_SUCCESS;
	}
}
