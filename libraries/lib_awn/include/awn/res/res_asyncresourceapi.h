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
    
    struct AsyncResourceThreadInfo {
        const char *name;
        u32         core_number;
        u32         priority;
        size_t      stack_size;
    };

    using AsyncResourceThreadCreateFunction = vp::util::IFunction<async::AsyncQueueThread*(mem::Heap*, async::AsyncQueue*, AsyncResourceThreadInfo*)>;

    struct AsyncResourceManagerInfo {
        IAsyncResourceApplicationImpl      *app_impl;
        u32                                 max_resource_unit_count;
        u32                                 load_task_count;
        u32                                 max_thread_local_archive_count;
        size_t                              zsdic_heap_size;
        const char                         *resource_size_table_path;
        AsyncResourceThreadInfo             control_thread_info;
        AsyncResourceThreadInfo             memory_thread_info;
        u32                                 load_thread_count;
        AsyncResourceThreadInfo            *load_thread_info_array;
        u32                                 compose_thread_count;
        AsyncResourceThreadInfo            *compose_thread_info_array;
        AsyncResourceThreadCreateFunction  *control_thread_create_function;
        AsyncResourceThreadCreateFunction  *memory_thread_create_function;
        AsyncResourceThreadCreateFunction  *load_thread_create_function;
        AsyncResourceThreadCreateFunction  *compose_thread_create_function;

        constexpr void SetDefaults() {
            this->app_impl = nullptr;
            this->max_resource_unit_count = 100;
            this->load_task_count = 10;
            this->max_thread_local_archive_count = 128;
            this->zsdic_heap_size = 0;
            this->resource_size_table_path = nullptr;
            this->control_thread_info = {
                .name = "awn::res::ControlThread",
                .core_number = 1,
                .priority = awn::sys::cPriorityNormal + 3,
                .stack_size = 0x14000,
            };
            this->memory_thread_info = {
                .name = "awn::res::MemoryThread",
                .core_number = 1,
                .priority = awn::sys::cPriorityNormal + 4,
                .stack_size = 0x14000,
            };
            this->load_thread_count = 0;
            this->load_thread_info_array = nullptr;
            this->compose_thread_count = 0;
            this->compose_thread_info_array = nullptr;
            this->control_thread_create_function = nullptr;
            this->memory_thread_create_function = nullptr;
            this->load_thread_create_function = nullptr;
            this->compose_thread_create_function = nullptr;
        }

        constexpr void SetApplicationImpl(IAsyncResourceApplicationImpl *app_impl) {
            this->app_impl = app_impl;
        }
        constexpr void SetMaxResourceCount(u32 count) {
            this->max_resource_unit_count = count;
        }
        constexpr void SetLoadTaskCount(u32 count) {
            this->load_task_count = count;
        }
        constexpr void SetMaxThreadLocalArchiveCount(u32 count) {
            this->max_thread_local_archive_count = count;
        }

        constexpr void SetControlThreadPriority(u32 priority) { this->control_thread_info.priority = priority; }
        constexpr void SetControlThreadStackSize(size_t stack_size) { this->control_thread_info.stack_size = stack_size; }
        constexpr void SetControlThreadCoreNumber(u32 core_number) { this->control_thread_info.core_number = core_number; }
        constexpr void SetMemoryThreadPriority(u32 priority) { this->memory_thread_info.priority = priority; }
        constexpr void SetMemoryThreadStackSize(size_t stack_size) { this->memory_thread_info.stack_size = stack_size; }
        constexpr void SetMemoryThreadCoreNumber(u32 core_number) { this->memory_thread_info.core_number = core_number; }
        constexpr void SetLoadThreadInfoArray(AsyncResourceThreadInfo *info_array, u32 count) {
            this->load_thread_count = count;
            this->load_thread_info_array = info_array;
        }
        constexpr void SetComposeThreadInfoArray(AsyncResourceThreadInfo *info_array, u32 count) {
            this->compose_thread_count = count;
            this->compose_thread_info_array = info_array;
        }
        constexpr void SetControlThreadCreateFunction(AsyncResourceThreadCreateFunction *function) {
            this->control_thread_create_function = function;
        }
        constexpr void SetMemoryThreadCreateFunction(AsyncResourceThreadCreateFunction *function) {
            this->memory_thread_create_function = function;
        }
        constexpr void SetLoadThreadsCreateFunction(AsyncResourceThreadCreateFunction *function) {
            this->load_thread_create_function = function;
        }
        constexpr void SetComposeThreadCreateFunction(AsyncResourceThreadCreateFunction *function) {
            this->compose_thread_create_function = function;
        }

        constexpr void SetZsDictionaryHeapSize(size_t size) {
            this->zsdic_heap_size = size;
        }
        constexpr void SetResourceSizeTablePath(const char *path) {
            this->resource_size_table_path = path;
        }
    };
    Result InitializeAsyncResourceManager(mem::Heap *heap, AsyncResourceManagerInfo *info);
    void FinalizeAsyncResourceManager();
    void CalculateAsyncResourceManager();
    void ClearAllCachesSync();

    Result LoadAsyncResourceSync(Resource **out_resource, const char *path, const AsyncResourceLoadInfo *load_info);

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

    bool ReferenceThreadLocalArchiveToBinder(ResourceBinder *binder);
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
