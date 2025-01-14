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

namespace awn::res {

    Result LoadAsyncResourceSync(Resource **out_resource, const char *path, AsyncResourceLoadInfo *load_info);

	void SetControlThreadPriority(s32 priority);
	void SetMemoryThreadPriority(s32 priority);
	void SetLoadThreadsPriority(s32 priority);
	void SetComposeThreadsPriority(s32 priority);

    using DecompressorHandle = u32;

	DecompressorHandle AllocateDecompressorHandle();
	void FreeDecompressorHandle(DecompressorHandle handle);

	IParallelDecompressor *GetParallelDecompressor(DecompressorHandle handle, CompressionType decompressor_type, s32 priority, sys::CoreMask core_mask);

    class ResourceBinder;

	void SetDefaultArchive(ResourceBinder *archive_binder);

    void ReferenceThreadLocalArchive();
    void ReleaseThreadLocalArchive();

    class ScopedThreadLocalArchive {
        private:
            ResourceBinder m_last_binder;
        public:
            constexpr ScopedThreadLocalArchive() : m_last_binder() {/*...*/}
            ScopedThreadLocalArchive(ResourceBinder *archive_binder);

            void SetArchive(ResourceBinder *archive_binder);

            ~ScopedThreadLocalArchive();
    };

    class ScopedThreadLocalArchiveReference {
        public:
            ScopedThreadLocalArchiveReference();
            ~ScopedThreadLocalArchiveReference();
    };
}
