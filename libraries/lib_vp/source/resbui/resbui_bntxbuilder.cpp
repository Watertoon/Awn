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
#include <vp.hpp>

namespace vp::resbui {

    Result BntxBuilder::AddTextureNode(BntxTextureNode *texture_node) {
        RESULT_RETURN_IF(texture_node->m_builder_node.IsLinked(), ResultAlreadyLinked);
        m_texture_list.PushBack(*texture_node);
        ++m_texture_count;
        RESULT_RETURN_SUCCESS;
    }

    void BntxBuilder::CalculateMemoryInfo(BntxBuilderMemoryInfo *out_memory_info) {

        /* Header */
        out_memory_info->location_header.offset = 0;
        out_memory_info->location_header.size = sizeof(vp::res::ResNintendoWareFileHeader) + sizeof(vp::res::ResBntxTextureContainer) + vp::res::cMaxGfxMemoryPoolSize;
        out_memory_info->location_header.alignment = alignof(size_t);
        out_memory_info->location_header.relocation_count = 1;

        /* Tally memory stats */
        u32 info_relocation_count = 0;
        NintendoWareDictionaryBuilder dic_builder;
        NintendoWareDictionaryBuilderMemoryInfo texture_dic_info = {};
        size_t texture_info_size = 0;
        size_t gpu_region_size = 0;
        for (BntxTextureNode &texture_node : m_texture_list) {

            /* Constants */
            texture_info_size += sizeof(vp::res::ResBntxTextureInfo) + vp::res::cMaxGfxTextureSize + vp::res::cMaxGfxTextureViewSize;
            const bool is_user_data = false;
            info_relocation_count += (is_user_data) ? 2 : 1;

            /* Tally texture sizes */
            size_t texture_size = texture_node.GetTextureDataSize();
            size_t gpu_size = 0;
            const u32 mip_count = texture_node.m_texture_info.mip_level_count;
            for (u32 i = 0; i < mip_count; ++i) {
                const size_t mip_size = (texture_size >> i);
                const size_t local_size = (mip_size < 1) ? 1 : 0;
                gpu_size = vp::util::AlignUp(gpu_size + local_size, texture_node.m_texture_data_alignment);
            }
            gpu_region_size += gpu_size;

            /* Add name to dic and string pool */
            RESULT_ABORT_UNLESS(dic_builder.AddNode(std::addressof(texture_node.m_name)));
            RESULT_ABORT_UNLESS(m_string_pool_builder.AddString(std::addressof(texture_node.m_name)));
        }

        /* Calculate dictionary and string pool info */
        RESULT_ABORT_UNLESS(m_string_pool_builder.AddString(std::addressof(m_file_name)));
        dic_builder.CalculateMemoryInfo(std::addressof(texture_dic_info));
        m_string_pool_builder.CalculateMemoryInfo(std::addressof(out_memory_info->string_pool_memory_info));

        /* Texture info pointer array */
        out_memory_info->location_texture_array.offset = out_memory_info->location_header.offset + out_memory_info->location_header.size;
        out_memory_info->location_texture_array.size = sizeof(size_t) * m_texture_count;
        out_memory_info->location_texture_array.alignment = alignof(size_t);
        out_memory_info->location_texture_array.relocation_count = 1;

        /* String pool */
        out_memory_info->location_string_pool.offset = out_memory_info->location_texture_array.offset + out_memory_info->location_texture_array.size;
        out_memory_info->location_string_pool.size = out_memory_info->string_pool_memory_info.total_size;
        out_memory_info->location_string_pool.alignment = out_memory_info->string_pool_memory_info.max_alignment;

        /* Texture dictionary */
        out_memory_info->location_texture_dictionary.offset = out_memory_info->location_string_pool.offset + out_memory_info->location_string_pool.size;
        out_memory_info->location_texture_dictionary.size = texture_dic_info.total_size;
        out_memory_info->location_texture_dictionary.alignment = texture_dic_info.max_alignment;
        out_memory_info->location_texture_dictionary.relocation_count = 1;

        /* Texture info array */
        out_memory_info->location_texture_info_array.offset = out_memory_info->location_texture_dictionary.offset + out_memory_info->location_texture_dictionary.size;
        out_memory_info->location_texture_info_array.size = texture_info_size;
        out_memory_info->location_texture_info_array.alignment = alignof(size_t);
        out_memory_info->location_texture_info_array.relocation_count = info_relocation_count;

        /* Gpu region header */
        out_memory_info->location_texture_data.offset = vp::util::AlignUp(out_memory_info->location_texture_info_array.offset + out_memory_info->location_texture_info_array.size + sizeof(vp::res::ResBntxTextureData), vp::util::c4KB) - sizeof(vp::res::ResBntxTextureData);
        out_memory_info->location_texture_data.size = sizeof(vp::res::ResBntxTextureData);
        out_memory_info->location_texture_data.alignment = alignof(u32);

        /* Gpu region */
        out_memory_info->location_gpu_region.offset = out_memory_info->location_texture_data.offset + out_memory_info->location_texture_data.size;
        out_memory_info->location_gpu_region.size = vp::util::AlignUp(gpu_region_size, vp::util::c4KB);
        out_memory_info->location_gpu_region.alignment = vp::util::c4KB;

        /* Relocation table */
        out_memory_info->location_relocation_table.offset = out_memory_info->location_gpu_region.offset + out_memory_info->location_gpu_region.size;
        out_memory_info->location_relocation_table.size = sizeof(vp::res::ResNintendoWareRelocationTable) + sizeof(vp::res::ResNintendoWareRelocationTable::ResSection) + sizeof(vp::res::ResNintendoWareRelocationTable::ResEntry) * (info_relocation_count + 3);
        out_memory_info->location_relocation_table.alignment = alignof(size_t);

        out_memory_info->total_size = out_memory_info->location_relocation_table.offset + out_memory_info->location_relocation_table.size;
        out_memory_info->max_alignment = vp::util::c4KB;
        out_memory_info->total_relocation_count = info_relocation_count + 3;

        return;
    }

    void BntxBuilder::Serialize(void *out_memory, size_t out_memory_size, const BntxBuilderMemoryInfo *memory_info) {

        /* Clear memory */
        ::memset(out_memory, 0, out_memory_size);

        /* Calc locations */
        uintptr_t memory_head = reinterpret_cast<uintptr_t>(out_memory);
        vp::res::ResNintendoWareFileHeader *header = reinterpret_cast<vp::res::ResNintendoWareFileHeader*>(memory_head);
        vp::res::ResBntxTextureContainer *container = reinterpret_cast<vp::res::ResBntxTextureContainer*>(reinterpret_cast<uintptr_t>(header) + sizeof(vp::res::ResNintendoWareFileHeader));
        vp::res::ResBntxTextureInfo **info_ptr_array = reinterpret_cast<vp::res::ResBntxTextureInfo**>(memory_head + memory_info->location_texture_array.offset);
        void *string_pool_head = reinterpret_cast<void*>(memory_head + memory_info->location_string_pool.offset);
        void *texture_dictionary = reinterpret_cast<void*>(memory_head + memory_info->location_texture_dictionary.offset);
        uintptr_t info_array = memory_head + memory_info->location_texture_info_array.offset;
        vp::res::ResBntxTextureData *texture_data = reinterpret_cast<vp::res::ResBntxTextureData*>(memory_head + memory_info->location_texture_data.offset);
        uintptr_t gpu_region = memory_head + memory_info->location_gpu_region.offset;
        void *relocation_table = reinterpret_cast<void*>(memory_head + memory_info->location_relocation_table.offset);

        /* Write string pool */
        if (0 < memory_info->string_pool_memory_info.string_count) {            
            m_string_pool_builder.Serialize(std::addressof(memory_info->string_pool_memory_info), string_pool_head, memory_info->location_string_pool.size);
            header->file_name_offset = static_cast<u32>(reinterpret_cast<uintptr_t>(reinterpret_cast<vp::res::ResNintendoWareStringPool*>(string_pool_head)->FindString(m_file_name.GetString())) - memory_head + 2);
        }
        vp::res::ResNintendoWareStringPool *string_pool = reinterpret_cast<vp::res::ResNintendoWareStringPool*>(string_pool_head);

        /* Write relocation table header */
        NintendoWareRelocationTableStream relocation_stream;
        relocation_stream.SetStream(header, relocation_table, memory_info->total_section_count, memory_info->total_relocation_count);
        RESULT_ABORT_UNLESS(relocation_stream.StartSection(0, memory_info->location_relocation_table.offset));

        /* Set header */
        header->magic                   = vp::res::ResBntx::cMagic;
        header->micro_version           = vp::res::ResBntx::cTargetVersionMicro;
        header->minor_version           = vp::res::ResBntx::cTargetVersionMinor;
        header->major_version           = vp::res::ResBntx::cTargetVersionMajor;
        header->endianess               = vp::res::ByteOrder_Native;
        header->packed_alignment        = vp::util::CountRightZeroBits(vp::util::c4KB);
        header->address_size            = BITSIZEOF(void*);
        header->is_relocated            = true;
        header->first_block_offset      = static_cast<u16>(memory_info->location_string_pool.offset);
        header->relocation_table_offset = static_cast<u32>(memory_info->location_relocation_table.offset);
        header->file_size               = static_cast<u32>(memory_info->total_size);

        /* Set texture data header */
        texture_data->magic = vp::res::ResBntx::cMagic;
        texture_data->next_block_offset = 0;
        texture_data->block_size = static_cast<u32>(memory_info->location_texture_data.size + memory_info->location_gpu_region.size);

        /* Set container */
        container->magic = vp::res::ResBntxTextureContainer::cMagicVpVk;
        container->texture_count = m_texture_count;
        container->texture_info_array = info_ptr_array;
        container->texture_data = texture_data;
        container->texture_dictionary = reinterpret_cast<vp::res::ResNintendoWareDictionary*>(texture_dictionary);
        container->runtime_memory_pool = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(container) + sizeof(vp::res::ResBntxTextureContainer));
        RESULT_ABORT_UNLESS(relocation_stream.AddEntry(std::addressof(container->texture_info_array), 1, 4, 0));
        RESULT_ABORT_UNLESS(relocation_stream.AddEntry(std::addressof(*container->texture_info_array), m_texture_count, 1, 0));

        /* Stream out textures */
        NintendoWareDictionaryBuilder dic_builder;
        uintptr_t texture_iter = info_array;
        uintptr_t gpu_iter = gpu_region;
        u32 tex_i = 0;
        for (BntxTextureNode &texture_node : m_texture_list) {

            /* Allocate info */
            vp::res::ResBntxTextureInfo *info = reinterpret_cast<vp::res::ResBntxTextureInfo*>(texture_iter);
            container->texture_info_array[tex_i] = info;
            ++tex_i;
            const uintptr_t r_texture_offset = texture_iter + sizeof(vp::res::ResBntxTextureInfo);
            info->runtime_texture = reinterpret_cast<void*>(r_texture_offset);
            const uintptr_t r_texture_view_offset = r_texture_offset + vp::res::cMaxGfxTextureSize;
            info->runtime_texture_view = reinterpret_cast<void*>(r_texture_view_offset);
            const uintptr_t r_mip_offset = r_texture_view_offset + vp::res::cMaxGfxTextureViewSize;
            info->mip_level_array = reinterpret_cast<void**>(r_mip_offset);
            const u32 mip_count = texture_node.m_texture_info.mip_level_count;
            const u32 block_size = (r_mip_offset + sizeof(size_t) * mip_count) - texture_iter;

            /* Set block header */
            info->magic = vp::res::ResBntxTextureInfo::cMagic;
            const u32 next_offset = (tex_i != m_texture_count) ? block_size : vp::util::AlignUp(block_size + sizeof(vp::res::ResBntxTextureData), vp::util::c4KB) - sizeof(vp::res::ResBntxTextureData);
            info->next_block_offset = next_offset;
            info->block_size = next_offset;

            texture_iter += block_size;

            /* Set info */
            info->texture_info = texture_node.m_texture_info;
            info->texture_data_alignment = texture_node.m_texture_data_alignment;
            info->channel_sources[0] = static_cast<u8>(texture_node.m_texture_swizzle_array[0]);
            info->channel_sources[1] = static_cast<u8>(texture_node.m_texture_swizzle_array[1]);
            info->channel_sources[2] = static_cast<u8>(texture_node.m_texture_swizzle_array[2]);
            info->channel_sources[3] = static_cast<u8>(texture_node.m_texture_swizzle_array[3]);
            info->image_dimension = static_cast<u8>(texture_node.m_image_dimension);
            info->name = reinterpret_cast<char*>(reinterpret_cast<uintptr_t>(string_pool->FindString(texture_node.m_name.GetString())) - memory_head);
            info->container = container;

            /* TODO; Serialize user data */
            //info->user_data_array = ;
            //info->user_data_dictionary = ;

            RESULT_ABORT_UNLESS(dic_builder.AddNode(std::addressof(texture_node.m_name)));
            RESULT_ABORT_UNLESS(relocation_stream.AddEntry(std::addressof(info->name), 1, 6, 0));
            const bool is_user_data = false;
            if (is_user_data) {                        
                RESULT_ABORT_UNLESS(relocation_stream.AddEntry(std::addressof(info->user_data_dictionary), 1, 1, 0));
            }

            /* Stream out texture mip levels */
            size_t texture_size = texture_node.GetTextureDataSize();
            size_t gpu_size = 0;
            for (u32 i = 0; i < mip_count; ++i) {
                void *memory_offset = reinterpret_cast<void*>(gpu_iter + gpu_size);
                info->mip_level_array[i] = memory_offset;
                const size_t mip_size = (texture_size >> i);
                const size_t local_size = (mip_size < 1) ? 1 : 0;
                ::memcpy(memory_offset, texture_node.m_mip_level_array[i], local_size);
                gpu_size = vp::util::AlignUp(gpu_size + local_size, texture_node.m_texture_data_alignment);
            }
            info->texture_data_size = gpu_size;
            gpu_iter += gpu_size;
        }

        /* Serialize texture dictionary */
        NintendoWareDictionaryBuilderMemoryInfo dic_memory_info = {};
        dic_builder.CalculateMemoryInfo(std::addressof(dic_memory_info));
        dic_builder.Serialize(std::addressof(dic_memory_info), std::addressof(relocation_stream), string_pool, texture_dictionary, dic_memory_info.total_size);

        /* Unrelocate */
        header->Unrelocate();

        return;
    }
}
