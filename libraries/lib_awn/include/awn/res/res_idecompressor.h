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

    enum class CompressionType : u16 {
        None        = (0 << 0),
        Auto        = (1 << 0),
        Szs         = (1 << 1),
        Zstandard   = (1 << 2),
    };
    VP_ENUM_FLAG_TRAITS(CompressionType);
    static constexpr const char cSzsExt[] = ".szs";
    static constexpr const char cZstdExt[] = ".zs";

    using MaxExtensionString = vp::util::MaxExtensionString;

    constexpr CompressionType GetCompressionType(const MaxExtensionString *extension) {
        static constexpr const MaxExtensionString zs_ext(cZstdExt);
        if (*extension == zs_ext) {
            return CompressionType::Zstandard;
        }
        static constexpr const MaxExtensionString szs_ext(cSzsExt);
        if (*extension == szs_ext) {
            return CompressionType::Szs;
        }
        return CompressionType::None;
    }

    class IDecompressor {
        public:
            VP_RTTI_BASE(IDecompressor);
        public:
            constexpr IDecompressor() {/*...*/}
            constexpr virtual ~IDecompressor() {/*...*/}

            virtual Result LoadFileWithDecompression(size_t *out_size, s32 *out_alignment, const char *path, FileLoadContext *file_load_context, FileDeviceBase *file_device) { VP_UNUSED(out_size, out_alignment, path, file_load_context, file_device); RESULT_RETURN_SUCCESS; }
    };
    
    class IParallelDecompressor : public IDecompressor {
        public:
            VP_RTTI_DERIVED(IParallelDecompressor);
        public:
            constexpr IParallelDecompressor() : IDecompressor() {/*...*/}
            constexpr virtual ~IParallelDecompressor() {/*...*/}

            virtual void SetPriority(u32 priority)            { VP_UNUSED(priority); }
            virtual void SetCoreMask(sys::CoreMask core_mask) { VP_UNUSED(core_mask); }
            virtual u32           GetPriority() { return 0; }
            virtual sys::CoreMask GetCoreMask() { return 0; }
     };
}
