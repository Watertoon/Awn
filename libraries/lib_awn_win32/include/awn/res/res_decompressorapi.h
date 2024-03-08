#pragma once

namespace awn::res {

	u32            AllocateDecompressor();
	void           FreeDecompressor(u32 handle);
	IDecompressor *AllocateDecompressor(u32 handle, CompressionType decompressor_type, u32 priority, sys::CoreMask core_mask);
}
