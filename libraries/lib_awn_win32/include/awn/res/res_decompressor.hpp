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

            virtual void SetPriority([[maybe_unused]] u32 priority) {/*...*/}
            virtual void SetCoreMask([[maybe_unused]] sys::CoreMask core_mask) {/*...*/}

            virtual Result LoadDecompressFile(size_t *out_size, s32 *out_alignment, const char *path, FileLoadContext *file_load_context, FileDeviceBase *file_device) { RESULT_RETURN_SUCCESS; }
    };
}
