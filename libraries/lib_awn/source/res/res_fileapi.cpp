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

    Result LoadFile(const char *path, FileLoadContext *file_load_context) {
		return impl::FileDeviceManager::GetInstance()->LoadFile(path, file_load_context);
	}
    Result LoadResource(Resource **out_resource, const char *path, ResourceLoadContext *resource_load_context) {
		return impl::ResourceFactoryManager::GetInstance()->LoadResource(out_resource, path, resource_load_context);
	}
    Result LoadResourceWithDecompressor(Resource **out_resource, const char *path, ResourceLoadContext *resource_load_context, IDecompressor *decompressor) {
		return impl::ResourceFactoryManager::GetInstance()->LoadResourceWithDecompressor(out_resource, path, resource_load_context, decompressor);
	}

    void *LoadFileFull(mem::Heap *heap, const char *path) {

        /* Load file */
        FileLoadContext file_load_context = {
            .file_heap = heap,
        };
        const Result result0 = LoadFile(path, std::addressof(file_load_context));
        if (RESULT_FAILED(result0)) { return nullptr; }

        /* Decompress zstd */
        void *file = file_load_context.file_buffer;
        size_t decomp_size = 0;
        const Result result1 = vp::codec::GetDecompressedSizeZstd(std::addressof(decomp_size), file_load_context.file_buffer, file_load_context.file_size);
        if (RESULT_SUCCEEDED(result1)) {
            ON_SCOPE_EXIT { ::operator delete(file_load_context.file_buffer); };
            void *decomp_memory = ::operator new(decomp_size, heap, alignof(u32));
            if (decomp_memory != nullptr) { return nullptr; }
            const Result result2 = vp::codec::DecompressZstd(nullptr, decomp_memory, decomp_size, file_load_context.file_buffer, file_load_context.file_size);
            if (RESULT_FAILED(result2)) { ::operator delete(decomp_memory); return nullptr; }
            file = decomp_memory;
        }

        return file;
    }
}
