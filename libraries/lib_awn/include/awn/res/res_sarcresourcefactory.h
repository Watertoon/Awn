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

    class SarcResourceFactory : public ResourceFactoryBase {
        public:
            static constexpr u32             cResourceSize             = sizeof(SarcArchiveResource);
            static constexpr u32             cResourceAlignment        = alignof(SarcArchiveResource);
            static constexpr u32             cFileAlignment            = alignof(u32);
            static constexpr CompressionType cDefaultCompressionType   = CompressionType::None;
            static constexpr float           cDefaultFallbackSizeScale = 1.0f;
            static constexpr u32             cDefaultExtraSize         = 0;
        public:
            constexpr SarcResourceFactory() : ResourceFactoryBase("sarc", cResourceSize, cResourceAlignment, cFileAlignment, cDefaultCompressionType, cDefaultFallbackSizeScale, cDefaultExtraSize, true, false) {/*...*/}
            constexpr SarcResourceFactory(const char *extension, CompressionType compression_type, u32 file_alignment) : ResourceFactoryBase(extension, cResourceSize, cResourceAlignment, file_alignment, compression_type, cDefaultFallbackSizeScale, cDefaultExtraSize, true, false) {/*...*/}
            virtual ~SarcResourceFactory() override {/*...*/}
        public:
            virtual Resource *AllocateResource(mem::Heap *heap, s32 alignment) const override {
                return new (heap, alignment) SarcArchiveResource();
            }
    };
}
