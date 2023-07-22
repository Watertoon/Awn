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

namespace vp::res {

    struct ResBntxTextureData : public ResNintendoWareSubHeader {
        static constexpr u32 cMagic = util::TCharCode32("BRTD");

        constexpr ALWAYS_INLINE size_t GetGpuMemoryRegionSize() {
            return this->GetBlockSize() - sizeof(ResBntxTextureData);
        }

        ALWAYS_INLINE void *GetGpuMemoryRegion() {
            return reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(this) + sizeof(ResBntxTextureData));
        }
    };
    static_assert(sizeof(ResBntxTextureData) == 0x10);

    struct ResBntxTextureInfo;

    struct ResBntxTextureContainer {
        u32                         magic;
        u32                         texture_count;
        ResBntxTextureInfo        **texture_info_array;
        ResBntxTextureData         *texture_data;
        ResNintendoWareDictionary  *texture_dictionary;
        void                       *runtime_memory_pool;
        void                       *runtime_user_memory_pool;
        s32                         base_memory_pool_offset;
        u32                         reserve0;

        static constexpr u32 cMagicNX   = util::TCharCode32("NX  ");

        ResBntxTextureInfo *TryGetTextureInfo(const char *texture_name) {
            if (texture_dictionary == nullptr) { return nullptr; }
            const u32 entry_id = texture_dictionary->TryGetEntryIndexByKey(texture_name);
            if (entry_id == ResNintendoWareDictionary::cInvalidEntryIndex) { return nullptr; }
            return texture_info_array[entry_id];
        }
    };
    static_assert(sizeof(ResBntxTextureContainer) == 0x38);

    struct ResBntxTextureInfo : public ResNintendoWareSubHeader {
        ResGfxTextureInfo           texture_info;
        u8                          texture_layout[0x4];
        u8                          reserve1[0x14];
        u32                         mipmap_total_size;
        u32                         texture_data_alignment;
        GfxChannelSource            channel_sources[0x4];
        GfxImageDimension           image_dimension;
        u8                          reserve2[0x3];
        char                       *name;
        ResBntxTextureContainer    *container;
        void                      **mip_level_array;
        ResGfxUserData             *user_data_array;
        void                       *runtime_texture;
        void                       *runtime_texture_view;
        u64                         runtime_descriptor_slot;
        ResNintendoWareDictionary  *user_data_dictionary;

        static constexpr u32 cMagic = util::TCharCode32("BRTI");

        ResGfxUserData *TryGetUserData(const char *user_data_name) {
            if (user_data_dictionary == nullptr) { return nullptr; }
            const u32 entry_id = user_data_dictionary->TryGetEntryIndexByKey(user_data_name);
            if (entry_id == ResNintendoWareDictionary::cInvalidEntryIndex) { return nullptr; }
            return std::addressof(user_data_array[entry_id]);
        }
    };
    static_assert(sizeof(ResBntxTextureInfo) == 0xA0);

    struct ResBntx : public ResNintendoWareFileHeader {
        ResBntxTextureContainer texture_container;

        static constexpr u64 cMagic = static_cast<u64>(util::TCharCode32("BNTX"));

        ResBntxTextureInfo *FindTexture(const char *name) {
            u32 entry_index = texture_container.texture_dictionary->TryGetEntryIndexByKey(name);
            if (entry_index == static_cast<u32>(-1)) { return nullptr; }
            return texture_container.texture_info_array[entry_index];
        }

        static ResBntx *ResCast(void *file) {
            ResBntx *texture_file = reinterpret_cast<ResBntx*>(file);
            if (texture_file->IsRelocated() == false) {
                texture_file->GetRelocationTable()->Relocate(); 
            }
            return texture_file;
        }

        static bool IsValid(void *file) {
            return reinterpret_cast<ResBntx*>(file)->ResNintendoWareFileHeader::IsValid(cMagic, 4, 1, 0);
        }

        static constexpr ALWAYS_INLINE u64 GetMaxFileAlignment() {
            return 0x20000;
        }
    };
    static_assert(sizeof(ResBntx) == 0x58);
}
