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

    struct FileLoadContext;
    class  FileDeviceBase;

    class IDecompressor {
        public:
            VP_RTTI_BASE(IDecompressor);
        public:
            constexpr IDecompressor() {/*...*/}
            constexpr virtual ~IDecompressor() {/*...*/}

            virtual void SetPriority(u32 priority)            { VP_UNUSED(priority); }
            virtual void SetCoreMask(sys::CoreMask core_mask) { VP_UNUSED(core_mask); }
            virtual u32           GetPriority() { return 0; }
            virtual sys::CoreMask GetCoreMask() { return 0; }

            virtual Result LoadDecompressFile(size_t *out_size, s32 *out_alignment, const char *path, FileLoadContext *file_load_context, FileDeviceBase *file_device) { VP_UNUSED(out_size, out_alignment, path, file_load_context, file_device); RESULT_RETURN_SUCCESS; }
    };
}
