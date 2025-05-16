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

namespace vp::resbui {

    class BntxBuilder;

    class BntxTextureNode {
        public:
            friend class BntxBuilder;
        public:
            static constexpr u32 cMaxMipLevelCount = 0x20;
        private:
            vp::util::IntrusiveListNode m_builder_node;
            DictionaryString m_name;
            void *m_mip_level_array[cMaxMipLevelCount];
            s32 m_texture_data_alignment;
            vp::res::ResGfxTextureInfo m_texture_info;
            vp::res::GfxImageDimension m_image_dimension;
            vp::res::GfxTextureSwizzle m_texture_swizzle_array[4];
        public:
            constexpr BntxTextureNode() : m_builder_node(), m_name(), m_mip_level_array(), m_texture_data_alignment(0x200), m_image_dimension(vp::res::GfxImageDimension::Type2d), m_texture_swizzle_array{ vp::res::GfxTextureSwizzle::R, vp::res::GfxTextureSwizzle::B, vp::res::GfxTextureSwizzle::G, vp::res::GfxTextureSwizzle::A, }  {
                m_texture_info.SetDefaults();
            }
            constexpr ~BntxTextureNode() {/*...*/}

            constexpr void SetTextureInfo(const vp::res::ResGfxTextureInfo *texture_info) {
                m_texture_info = *texture_info;
            }
            constexpr void SetImageDimension(vp::res::GfxImageDimension image_dimension) {
                m_image_dimension = image_dimension;
            }
            constexpr void SetSwizzle(vp::res::GfxTextureSwizzle x, vp::res::GfxTextureSwizzle y, vp::res::GfxTextureSwizzle z, vp::res::GfxTextureSwizzle w) {
                m_texture_swizzle_array[0] = x;
                m_texture_swizzle_array[1] = y;
                m_texture_swizzle_array[2] = z;
                m_texture_swizzle_array[3] = w;
            }

            constexpr void SetTextureData(void **mip_level_array, u32 count) {
                for (u32 i = 0; i < count; ++i) {
                    m_mip_level_array[i] = mip_level_array[i];
                }
                m_texture_info.SetMipLevelCount(count);
            }

            constexpr void SetName(const char *name) {
                m_name.SetString(name);
            }

            constexpr size_t GetTextureDataSize() const {
                const vp::res::GfxChannelFormat channel_format = vp::res::GetGfxChannelFormat(static_cast<vp::res::GfxImageFormat>(m_texture_info.image_format));
                return vp::res::CalculateGfxImageSize(channel_format, m_texture_info.width, m_texture_info.height, m_texture_info.depth) * m_texture_info.array_layer_count;
            }
    };

    struct BntxBuilderMemoryInfo {
        size_t         total_size;
        size_t         max_alignment;
        u32            total_section_count;
        u32            total_relocation_count;
        BufferLocation location_header;
        BufferLocation location_texture_array;
        BufferLocation location_string_pool;
        BufferLocation location_texture_dictionary;
        BufferLocation location_texture_info_array;
        BufferLocation location_texture_data;
        BufferLocation location_gpu_region;
        BufferLocation location_relocation_table;
        NintendoWareStringPoolBuilderMemoryInfo string_pool_memory_info;
    };

    class BntxBuilder {
        public:
            using TextureList = vp::util::IntrusiveListTraits<BntxTextureNode, &BntxTextureNode::m_builder_node>::List;
        private:
            StringPoolString m_file_name;
            TextureList m_texture_list;
            u32 m_texture_count;
            NintendoWareStringPoolBuilder m_string_pool_builder;
        public:
            constexpr BntxBuilder() : m_file_name("textures"), m_texture_list(), m_texture_count(), m_string_pool_builder() {/*...*/}
            constexpr ~BntxBuilder() {/*...*/}

            void CalculateMemoryInfo(BntxBuilderMemoryInfo *out_memory_info);
            void Serialize(void *out_memory, size_t out_memory_size, const BntxBuilderMemoryInfo *memory_info);
        public:
            Result AddTextureNode(BntxTextureNode *texture_node);
            constexpr void SetName(const char *name) {
                m_file_name.SetString(name);
            }
    };
}
