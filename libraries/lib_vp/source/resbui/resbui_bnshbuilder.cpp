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

    namespace {

        constexpr inline vp::res::GfxShaderInterfaceType cInterfaceTypeArray[] = {
            vp::res::GfxShaderInterfaceType::InputAttribute,
            vp::res::GfxShaderInterfaceType::OutputAttribute,
            vp::res::GfxShaderInterfaceType::TextureSampler,
            vp::res::GfxShaderInterfaceType::UniformBuffer,
            vp::res::GfxShaderInterfaceType::StorageBuffer,
            vp::res::GfxShaderInterfaceType::Image,
            vp::res::GfxShaderInterfaceType::SeparateTexture,
            vp::res::GfxShaderInterfaceType::SeparateSampler,
        };
        constexpr inline u32 cInterfaceTypeCount = sizeof(cInterfaceTypeArray) / sizeof(vp::res::GfxShaderInterfaceType);
    }

    bool BnshBuilder::AddCodeUnit(BnshShaderNode::CodeUnit *unit) {
        if (unit->list_node.IsLinked()) { unit->list_node.Unlink(); }
        for (BnshShaderNode::CodeUnit &col : m_cache.code_list) {
            if (col.code_size != unit->code_size) { continue; }
            if (col.hash != unit->hash) { continue; }
            if (::memcmp(col.code, unit->code, unit->code_size) != 0) { continue; }
            m_cache.collision_code_list.PushBack(*unit);
            unit->collision = std::addressof(col);
            return false;
        }
        m_cache.code_list.PushBack(*unit);
        unit->collision = nullptr;
        return true;
    }

    void BnshBuilder::Serialize(void *out_bnsh, size_t output_size, const BnshBuilderMemoryInfo *memory_info) {

        /* Clear memory */
        ::memset(out_bnsh, 0, output_size);

        /* Get offsets */
        vp::res::ResBnsh                    *bnsh             = reinterpret_cast<vp::res::ResBnsh*>(reinterpret_cast<uintptr_t>(out_bnsh) + memory_info->location_header.offset);
        vp::res::ResBnshShaderContainer     *container        = reinterpret_cast<vp::res::ResBnshShaderContainer*>(reinterpret_cast<uintptr_t>(out_bnsh) + memory_info->location_container.offset);
        vp::res::ResBnshShaderVariation     *variation_array  = reinterpret_cast<vp::res::ResBnshShaderVariation*>(reinterpret_cast<uintptr_t>(out_bnsh) + memory_info->location_variation_array.offset);
        vp::res::ResBnshShaderProgram       *program_array    = reinterpret_cast<vp::res::ResBnshShaderProgram*>(reinterpret_cast<uintptr_t>(out_bnsh) + memory_info->location_program_array.offset);
        vp::res::ResBnshShaderContainerData *container_data   = reinterpret_cast<vp::res::ResBnshShaderContainerData*>(reinterpret_cast<uintptr_t>(out_bnsh) + memory_info->location_container_data.offset);
        const uintptr_t                      gpu_region       = reinterpret_cast<uintptr_t>(out_bnsh) + memory_info->location_gpu_region.offset;
        void                                *string_pool      = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(out_bnsh) + memory_info->location_string_pool.offset);
        void                                *relocation_table = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(out_bnsh) + memory_info->location_relocation_table.offset);

        /* Write string pool */
        if (0 < memory_info->string_pool_memory_info.string_count) {            
            m_string_builder.Serialize(std::addressof(memory_info->string_pool_memory_info), string_pool, memory_info->location_string_pool.size);
            bnsh->file_name_offset = static_cast<u32>(reinterpret_cast<uintptr_t>(reinterpret_cast<vp::res::ResNintendoWareStringPool*>(string_pool)->FindString(m_file_name.GetString())) - reinterpret_cast<uintptr_t>(out_bnsh) + 2);
        }

        /* Write relocation table header */
        NintendoWareRelocationTableStream relocation_stream;
        relocation_stream.SetStream(bnsh, relocation_table, memory_info->total_section_count, memory_info->total_relocation_count);
        relocation_stream.StartSection(0, memory_info->location_relocation_table.offset);

        /* Write header */
        bnsh->magic                   = res::ResBnsh::cMagic;
        bnsh->micro_version           = res::ResBnsh::cTargetVersionMicro;
        bnsh->minor_version           = res::ResBnsh::cTargetVersionMinor;
        bnsh->major_version           = res::ResBnsh::cTargetVersionMajor;
        bnsh->endianess               = res::ByteOrder_Native;
        bnsh->packed_alignment        = vp::util::CountRightZeroBits(vp::util::c4KB);
        bnsh->address_size            = BITSIZEOF(void*);
        bnsh->is_relocated            = true;
        bnsh->first_block_offset      = static_cast<u16>(memory_info->location_container.offset);
        bnsh->relocation_table_offset = static_cast<u32>(memory_info->location_relocation_table.offset);
        bnsh->file_size               = static_cast<u32>(memory_info->total_size);

        /* Write container */
        container->magic              = res::ResBnshShaderContainer::cMagic;
        const u32 next_offset         = static_cast<u32>(memory_info->location_string_pool.offset - memory_info->location_container.offset);
        container->next_block_offset  = next_offset;
        container->block_size         = next_offset;
        container->target_api_type    = m_api_type;
        container->target_api_version = m_api_version;
        container->target_code_type   = m_code_type;
        container->compiler_version   = m_compiler_version;
        container->variation_array    = variation_array;
        container->container_data     = container_data;
        relocation_stream.AddEntry(std::addressof(container->variation_array), 1, 2, 0);

        /* Write container data */
        container_data->memory_pool_info.memory_pool_flags  = static_cast<u32>(res::GfxMemoryPoolFlags::ShaderCode | res::GfxMemoryPoolFlags::GpuCached | res::GfxMemoryPoolFlags::CpuNoAccess);
        container_data->memory_pool_info.size               = static_cast<u32>(memory_info->location_gpu_region.size);
        container_data->memory_pool_info.gpu_region_base    = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(out_bnsh) + memory_info->location_gpu_region.offset);
        uintptr_t container_data_iter                       = reinterpret_cast<uintptr_t>(container_data) + sizeof(res::ResBnshShaderContainerData);
        container_data->runtime_memory_pool                 = reinterpret_cast<void*>(container_data_iter);
        container_data_iter                                += vp::res::cDefaultBnshRuntimeMemoryPoolSize;
        relocation_stream.AddEntry(std::addressof(container_data->memory_pool_info.gpu_region_base), 2, 1, 2);

        /* Write code */
        const uintptr_t code_alignment = (m_code_type == static_cast<u32>(res::GfxShaderCodeType::Intermediate)) ? 0x10 : 0x100;
        uintptr_t gpu_region_iter = gpu_region;
        for (BnshShaderNode::CodeUnit &unit : m_cache.code_list) {
            ::memcpy(reinterpret_cast<void*>(gpu_region_iter), unit.code, unit.code_size);
            unit.code_offset = gpu_region_iter;
            gpu_region_iter = vp::util::AlignUp(gpu_region_iter + unit.code_size, code_alignment);
        }

        /* Write variation */
        u32 variation_i = 0;
        uintptr_t program_iter = reinterpret_cast<uintptr_t>(program_array);
        for (BnshShaderVariationNode &variation_node : m_variation_list) {
            vp::res::ResBnshShaderVariation *variation = std::addressof(variation_array[variation_i]);
            variation->parent_container = container;

            /* Write code types */
            for (u32 i = 0; i < res::cMaxBnshShaderCodeTypeCount; ++i) {
                BnshShaderProgramNode *program_node = variation_node.m_program_array[i];
                if (program_node == nullptr) { continue; }

                /* Set program */
                vp::res::ResBnshShaderProgram *program = reinterpret_cast<vp::res::ResBnshShaderProgram*>(program_iter);
                program_iter += sizeof(vp::res::ResBnshShaderProgram);
                const u32 code_type_index = (i == 2) ? 0 : (i == 0) ? 2 : i;
                std::addressof(variation->program_source)[code_type_index] = program;
                program->option = 2;
                program->code_type = i;
                program->size_of_runtime_shader = vp::res::cDefaultBnshRuntimeShaderSize;
                program->runtime_shader = reinterpret_cast<void*>(container_data_iter);
                container_data_iter += vp::res::cDefaultBnshRuntimeShaderSize;
                program->parent_variation = variation;
                relocation_stream.AddEntry(std::addressof(program->code_info_vertex), 1, 8, 0);
                relocation_stream.AddEntry(std::addressof(program->runtime_shader), 1, 3, 0);

                /* Set code infos */
                for (u32 j = 0; j < res::cMaxBnshShaderStageCount; ++j) {
                    BnshShaderNode *shader_node = program_node->m_shader_array[j];
                    if (shader_node == nullptr) { continue; }

                    vp::res::ResBnshShaderCodeInfo *code_info = reinterpret_cast<vp::res::ResBnshShaderCodeInfo*>(program_iter);
                    program_iter += sizeof(vp::res::ResBnshShaderCodeInfo);
                    program->code_info_array[j] = code_info;
                }

                /* Check if any interface slot exists */
                bool is_slot_variation = false;
                for (u32 j = 0; j < res::cMaxBnshShaderStageCount; ++j) {
                    BnshShaderNode *shader_node = program_node->m_shader_array[j];
                    if (shader_node == nullptr) { continue; }
                    is_slot_variation = (shader_node->m_interface_array.GetUsedCount() != 0);
                    if (is_slot_variation) { break; }
                }

                /* Allocate interface info table */
                vp::res::ResBnshShaderInterfaceInfoTable *interface_info_table = nullptr;
                if (is_slot_variation) {
                    interface_info_table = reinterpret_cast<vp::res::ResBnshShaderInterfaceInfoTable*>(program_iter);
                    program_iter += sizeof(vp::res::ResBnshShaderInterfaceInfoTable);
                    program->shader_interface_info_table = interface_info_table;
                    relocation_stream.AddEntry(interface_info_table, 1, 8, 0);
                }

                /* Write control and set shader code */
                for (u32 j = 0; j < res::cMaxBnshShaderStageCount; ++j) {
                    BnshShaderNode *shader_node = program_node->m_shader_array[j];
                    if (shader_node == nullptr) { continue; }
                    vp::res::ResBnshShaderCodeInfo *code_info = std::addressof(program->code_info_vertex)[j];

                    if (shader_node->m_control != nullptr) {
                        void *control = reinterpret_cast<void*>(program_iter);                                
                        program_iter += shader_node->m_control_size;
                        code_info->control_size = shader_node->m_control_size;
                        code_info->control = control;
                        ::memcpy(control, shader_node->m_control, shader_node->m_control_size);
                        relocation_stream.AddEntry(std::addressof(code_info->control), 1, 2, 0);
                    } else {
                        relocation_stream.AddEntry(std::addressof(code_info->code), 1, 1, 0);
                    }

                    const size_t code_offset = (shader_node->m_code_unit.collision != nullptr) ? shader_node->m_code_unit.collision->code_offset : shader_node->m_code_unit.code_offset;
                    void *code = reinterpret_cast<void*>(code_offset);
                    code_info->code_size = shader_node->m_code_unit.code_size;
                    code_info->code = code;
                }

                /* Allocate interface info */
                if (!is_slot_variation) { continue; }
                for (u32 j = 0; j < res::cMaxBnshShaderStageCount; ++j) {
                    BnshShaderNode *shader_node = program_node->m_shader_array[j];
                    if (shader_node == nullptr) { continue; }

                    /* Determine whether ResBnshShaderInterfaceInfo and ResBnshExtendedInterfaceInfo exist */
                    bool is_slot_program = false;
                    bool is_slot_ext_program = false;
                    for (BnshShaderInterfaceNode *&node : shader_node->m_interface_array) {
                        if (!node->IsValid()) { continue; }
                        if (node->interface_type == vp::res::GfxShaderInterfaceType::SeparateTexture || node->interface_type == vp::res::GfxShaderInterfaceType::SeparateSampler) {
                            is_slot_ext_program = true;
                            if (is_slot_program) { break; }
                            continue;
                        }
                        if (node->interface_type != vp::res::GfxShaderInterfaceType::SeparateTexture && node->interface_type != vp::res::GfxShaderInterfaceType::SeparateSampler) {                            
                            is_slot_program = true;
                            if (is_slot_ext_program) { break; }
                            continue;
                        }
                    }

                    if (is_slot_program || is_slot_ext_program) {                        
                        vp::res::ResBnshShaderInterfaceInfo *interface_info = reinterpret_cast<vp::res::ResBnshShaderInterfaceInfo*>(program_iter);
                        program_iter += sizeof(vp::res::ResBnshShaderInterfaceInfo);
                        interface_info_table->interface_info_array[j] = interface_info;
                        relocation_stream.AddEntry(std::addressof(interface_info->input_attribute_dictionary), 1, 5, 0);
                        relocation_stream.AddEntry(std::addressof(interface_info->interface_slot_array), 1, 1, 0);
                        if (is_slot_ext_program) {
                            vp::res::ResBnshExtendedInterfaceInfo *interface_info_ext = reinterpret_cast<vp::res::ResBnshExtendedInterfaceInfo*>(program_iter);
                            program_iter += sizeof(vp::res::ResBnshExtendedInterfaceInfo);
                            interface_info->extended_interface_info = interface_info_ext;
                            relocation_stream.AddEntry(std::addressof(interface_info->image_dictionary), 1, 4, 0);
                        } else {
                            relocation_stream.AddEntry(std::addressof(interface_info->image_dictionary), 1, 1, 0);
                        }
                    }
                }

                /* Write interface infos */
                for (u32 j = 0; j < res::cMaxBnshShaderStageCount; ++j) {
                    BnshShaderNode *shader_node = program_node->m_shader_array[j];
                    vp::res::ResBnshShaderInterfaceInfo *interface_info = interface_info_table->interface_info_array[j];
                    if (shader_node == nullptr || interface_info == nullptr) { continue; }

                    /* Write interface dictionaries */
                    u32 total_slot_count = 0;
                    const auto interface_build_lambda = [&](vp::res::GfxShaderInterfaceType type) -> vp::res::ResNintendoWareDictionary* {
                        NintendoWareDictionaryBuilder dict_builder;
                        u32 slot_count = 0;
                        for (BnshShaderInterfaceNode *&interface_node : shader_node->m_interface_array) {
                            /* Ensure current interface type */
                            if (type != interface_node->interface_type) { continue; }
                            if (!interface_node->IsValid()) { continue; }
                            
                            /* Add to dictionary */
                            ++slot_count;
                            dict_builder.AddNode(std::addressof(interface_node->name));
                        }
                        if (slot_count == 0) { return nullptr; }
                        total_slot_count += slot_count;
                        NintendoWareDictionaryBuilderMemoryInfo dict_memory_info = {};
                        dict_builder.CalculateMemoryInfo(std::addressof(dict_memory_info));
                        void *dict_memory = reinterpret_cast<void*>(program_iter);
                        dict_builder.Serialize(std::addressof(dict_memory_info), std::addressof(relocation_stream), reinterpret_cast<vp::res::ResNintendoWareStringPool*>(string_pool), dict_memory, dict_memory_info.total_size);
                        program_iter = vp::util::AlignUp(program_iter, dict_memory_info.max_alignment) + dict_memory_info.total_size;
                        return reinterpret_cast<vp::res::ResNintendoWareDictionary*>(dict_memory);
                    };
                    interface_info->input_attribute_dictionary  = interface_build_lambda(vp::res::GfxShaderInterfaceType::InputAttribute);

                    interface_info->base_index_output_attribute = (interface_info->input_attribute_dictionary != nullptr) ? interface_info->input_attribute_dictionary->node_count : 0;
                    interface_info->output_attribute_dictionary = interface_build_lambda(vp::res::GfxShaderInterfaceType::OutputAttribute);

                    interface_info->base_index_texture_sampler  = (interface_info->output_attribute_dictionary != nullptr) ? interface_info->base_index_output_attribute + interface_info->output_attribute_dictionary->node_count : interface_info->base_index_output_attribute;
                    interface_info->texture_sampler_dictionary  = interface_build_lambda(vp::res::GfxShaderInterfaceType::TextureSampler);

                    interface_info->base_index_uniform_buffer   = (interface_info->texture_sampler_dictionary != nullptr) ? interface_info->base_index_texture_sampler + interface_info->texture_sampler_dictionary->node_count : interface_info->base_index_texture_sampler;
                    interface_info->uniform_buffer_dictionary   = interface_build_lambda(vp::res::GfxShaderInterfaceType::UniformBuffer);

                    interface_info->base_index_storage_buffer   = (interface_info->uniform_buffer_dictionary != nullptr) ? interface_info->base_index_uniform_buffer + interface_info->uniform_buffer_dictionary->node_count : interface_info->base_index_uniform_buffer;
                    interface_info->storage_buffer_dictionary   = interface_build_lambda(vp::res::GfxShaderInterfaceType::StorageBuffer);

                    interface_info->base_index_image            = (interface_info->storage_buffer_dictionary != nullptr) ? interface_info->base_index_storage_buffer + interface_info->storage_buffer_dictionary->node_count : interface_info->base_index_storage_buffer;
                    interface_info->image_dictionary            = interface_build_lambda(vp::res::GfxShaderInterfaceType::Image);

                    vp::res::ResBnshExtendedInterfaceInfo *interface_info_ext = interface_info->extended_interface_info;
                    if (interface_info_ext != nullptr) {
                        interface_info_ext->base_index_separate_texture = (interface_info->image_dictionary != nullptr) ? interface_info->base_index_image + interface_info->image_dictionary->node_count : interface_info->base_index_image;
                        interface_info_ext->separate_texture_dictionary = interface_build_lambda(vp::res::GfxShaderInterfaceType::SeparateTexture);

                        interface_info_ext->base_index_separate_sampler = (interface_info_ext->separate_texture_dictionary != nullptr) ? interface_info_ext->base_index_separate_texture + interface_info_ext->separate_texture_dictionary->node_count : interface_info_ext->base_index_separate_texture;
                        interface_info_ext->separate_sampler_dictionary = interface_build_lambda(vp::res::GfxShaderInterfaceType::SeparateSampler);
                    }

                    /* Write interface slots */
                    u32 *interface_slot_array = reinterpret_cast<u32*>(program_iter);
                    program_iter += vp::util::AlignUp(total_slot_count * sizeof(u32), alignof(size_t));
                    u32 slot_i = 0;
                    for (u32 k = 0; k < cInterfaceTypeCount ; ++k) {
                        vp::res::GfxShaderInterfaceType type = cInterfaceTypeArray[k];
                        for (BnshShaderInterfaceNode *&interface_node : shader_node->m_interface_array) {
                            /* Ensure current interface type */
                            if (type != interface_node->interface_type) { continue; }
                            if (!interface_node->IsValid()) { continue; }

                            interface_slot_array[slot_i] = interface_node->location;
                            ++slot_i;
                        }
                    }
                    interface_info->interface_slot_array = interface_slot_array;
                }
            }

            ++variation_i;
        }
        container->variation_count = variation_i;
        relocation_stream.AddEntry(variation_array, variation_i, 4, 4);

        /* End relocation stream and unrelocate file*/
        relocation_stream.EndStream();
        bnsh->Unrelocate();

        return;
    }

    Result BnshBuilder::CalculateMemoryInfo(BnshBuilderMemoryInfo *out_memory_info) {

        /* ResBnsh */
        out_memory_info->location_header.offset = 0;
        out_memory_info->location_header.size = sizeof(res::ResBnsh);

        /* ResBnshShaderContainer */
        u32 relocation_count = 0;
        out_memory_info->location_container.offset = out_memory_info->location_header.offset + out_memory_info->location_header.size;
        out_memory_info->location_container.size = sizeof(res::ResBnshShaderContainer);
        out_memory_info->location_container.relocation_count = 1;
        relocation_count += 1;

        /* Gather memory requirements and build string pool over every shader */
        u32 program_relocation_count = 0;
        size_t variation_i = 0;
        size_t program_i = 0;
        size_t program_iter = 0;
        for (BnshShaderVariationNode &variation_node : m_variation_list) {
            ++variation_i;
            for (u32 i = 0; i < res::cMaxBnshShaderCodeTypeCount; ++i) {
                BnshShaderProgramNode *program_node = variation_node.m_program_array[i];
                if (program_node == nullptr) { continue; }

                /* Program */
                program_i += 1;
                program_iter += sizeof(vp::res::ResBnshShaderProgram);
                program_relocation_count += 2;

                u32 total_slot_count = 0;
                for (u32 j = 0; j < res::cMaxBnshShaderStageCount; ++j) {
                    BnshShaderNode *shader_node = program_node->m_shader_array[j];
                    if (shader_node == nullptr) { continue; }

                    /* Code Info, Control, Code */
                    program_iter += sizeof(vp::res::ResBnshShaderCodeInfo);
                    program_iter += shader_node->m_control_size;
                    this->AddCodeUnit(std::addressof(shader_node->m_code_unit));
                    program_relocation_count += 1;

                    /* Interface slots */
                    const u32 slot_base = total_slot_count;
                    u32 slot_base_ext = 0;
                    for (u32 i = 0; i < cInterfaceTypeCount; ++i) {
                        NintendoWareDictionaryBuilder dict_builder;
                        vp::res::GfxShaderInterfaceType type = cInterfaceTypeArray[i];
                        /* Post Image is ext */
                        if (type == vp::res::GfxShaderInterfaceType::SeparateTexture) {
                            slot_base_ext = total_slot_count;
                        }
                        u32 type_count = 0;
                        for (BnshShaderInterfaceNode *&interface_node : shader_node->m_interface_array) {
                            /* Ensure current interface type */
                            if (type != interface_node->interface_type) { continue; }
                            if (!interface_node->IsValid()) { continue; }
                            
                            /* Add to dictionary */
                            ++type_count;
                            dict_builder.AddNode(std::addressof(interface_node->name));
                            m_string_builder.AddString(std::addressof(interface_node->name));
                        }
                        if (type_count == 0) { continue; }
                        total_slot_count += type_count;
                        NintendoWareDictionaryBuilderMemoryInfo memory_info = {};
                        dict_builder.CalculateMemoryInfo(std::addressof(memory_info));
                        program_iter = vp::util::AlignUp(program_iter, memory_info.max_alignment) + memory_info.total_size;
                        program_relocation_count += memory_info.total_relocation_count;
                    }
                    const u32 slot_count_ext = total_slot_count - slot_base_ext;
                    const u32 slot_count = (slot_base_ext - slot_base);

                    if (0 < slot_count) {
                        program_iter += sizeof(vp::res::ResBnshShaderInterfaceInfo);
                        program_relocation_count += 3;
                        if (0 < slot_count_ext) {
                            program_iter += sizeof(vp::res::ResBnshExtendedInterfaceInfo);
                        }
                        program_iter += vp::util::AlignUp((slot_count + slot_count_ext) * sizeof(u32), alignof(size_t));
                    }
                }

                /* Interface slot table */
                if (0 < total_slot_count) {
                    program_iter += sizeof(vp::res::ResBnshShaderInterfaceInfoTable);
                    program_relocation_count += 1;
                }
            }
        }
        RESULT_RETURN_UNLESS(0 < variation_i && variation_i <= 0xffff, ResultOutOfBoundsVariation);

        /* ResBnshShaderVariation array */
        out_memory_info->location_variation_array.offset = out_memory_info->location_container.offset + out_memory_info->location_container.size;
        out_memory_info->location_variation_array.size = variation_i * sizeof(vp::res::ResBnshShaderVariation);
        out_memory_info->location_variation_array.relocation_count = 1;
        relocation_count += 1;

        /* Program array */
        /*   ResBnshShaderProgram */
        /*   ResBnshCodeInfo array */
        /*   ResBnshShaderInterfaceInfoTable */
        /*   ResBnshExtendedInterfaceInfoTable */
        /*   ResDics */
        /*   binding array */
        out_memory_info->location_program_array.offset = out_memory_info->location_variation_array.offset + out_memory_info->location_variation_array.size;
        out_memory_info->location_program_array.size = program_iter;
        out_memory_info->location_program_array.relocation_count = program_relocation_count;
        relocation_count += program_relocation_count;

        /* ResBnshShaderContainerData */
        out_memory_info->location_container_data.offset = out_memory_info->location_program_array.offset + out_memory_info->location_program_array.size;
        out_memory_info->location_container_data.size = sizeof(res::ResBnshShaderContainerData) + vp::res::cDefaultBnshRuntimeMemoryPoolSize + vp::res::cDefaultBnshRuntimeShaderSize * program_i;
        out_memory_info->location_container_data.relocation_count = 1;
        relocation_count += 1;

        /* Gpu region */
        const size_t code_alignment = (m_code_type == static_cast<u32>(res::GfxShaderCodeType::Intermediate)) ? 0x10 : 0x100;
        const size_t gpu_alignment = (m_code_type == static_cast<u32>(res::GfxShaderCodeType::Intermediate)) ? 0x10 : vp::util::c4KB;
        out_memory_info->location_gpu_region.offset = vp::util::AlignUp(out_memory_info->location_container_data.offset + out_memory_info->location_container_data.size, gpu_alignment);
        size_t gpu_region_iter = 0;
        for (BnshShaderNode::CodeUnit &unit : m_cache.code_list) {
            gpu_region_iter = vp::util::AlignUp(gpu_region_iter + unit.code_size, code_alignment);
        }
        out_memory_info->location_gpu_region.alignment = gpu_alignment;
        out_memory_info->location_gpu_region.size = gpu_region_iter;

        m_string_builder.AddString(std::addressof(m_file_name));
        m_string_builder.CalculateMemoryInfo(std::addressof(out_memory_info->string_pool_memory_info));

        out_memory_info->location_string_pool.offset = vp::util::AlignUp(out_memory_info->location_gpu_region.offset + out_memory_info->location_gpu_region.size, gpu_alignment);
        out_memory_info->location_string_pool.size = (out_memory_info->string_pool_memory_info.string_count == 0) ? 0 : out_memory_info->string_pool_memory_info.total_size;

        out_memory_info->location_relocation_table.offset = out_memory_info->location_string_pool.offset + out_memory_info->location_string_pool.size;
        out_memory_info->location_relocation_table.size = sizeof(vp::res::ResNintendoWareRelocationTable) + sizeof(vp::res::ResNintendoWareRelocationTable::ResSection) + sizeof(vp::res::ResNintendoWareRelocationTable::ResEntry) * relocation_count;

        out_memory_info->total_size = out_memory_info->location_relocation_table.offset + out_memory_info->location_relocation_table.size;
        out_memory_info->max_alignment = gpu_alignment;
        out_memory_info->total_section_count = 1;
        out_memory_info->total_relocation_count = relocation_count;

        RESULT_RETURN_SUCCESS;
    }
}
