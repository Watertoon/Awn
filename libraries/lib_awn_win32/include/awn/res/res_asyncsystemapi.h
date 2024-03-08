#pragma once

namespace awn::res {

	void SetControlThreadPriority(u32 priority);
	void SetMemoryThreadPriority(u32 priority);
	void SetLoadThreadsPriority(u32 priority);

	void SetDefaultArchive(ResourceBinder *archive_binder);

    u32  AllocateDecompressor();
    void FreeDecompressor(u32 handle);
    IDecompressor *GetDecompressor(u32 handle, CompressionType compression_type, u32 priority, sys::CoreMask core_mask);

    Result LoadAsyncResourceSync(Resource **out_resource, const char *path, AsyncResourceLoadInfo *load_info);
}
