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

    struct ResBfres : public ResNintendoWareFileHeader {
        const char                 *fres_name;
        ResBfresModel              *model_array;
        ResNintendoWareDictionary  *model_dictionary;
        u64                         reserve0;
        u64                         reserve1;
        u64                         reserve2;
        u64                         reserve3;
        ResBfresSkeletalAnim       *skeletal_anim_array;
        ResNintendoWareDictionary  *skeletal_anim_dictionary;
        ResBfresMaterialAnim       *material_anim_array;
        ResNintendoWareDictionary  *material_anim_dictionary;
        ResBfresBoneVisibilityAnim *bone_visibility_anim_array;
        ResNintendoWareDictionary  *bone_visibility_anim_dictionary;
        ResBfresShapeAnim          *shape_anim_array;
        ResNintendoWareDictionary  *shape_anim_dictionary;
        ResBfresSceneAnim          *scene_anim_array;
        ResNintendoWareDictionary  *scene_anim_dictionary;
        void                       *user_memory_pool;
        ResGfxMemoryPoolInfo       *memory_pool_info;
        ResGfxEmbedFile            *embed_file_array;
        ResNintendoWareDictionary  *embed_file_dictionary;
        void                       *transform_tree;
        const char                 *empty_string;
        u32                         reserve5;
        u16                         model_count;
        u16                         reserve6;
        u16                         reserve7;
        u16                         skeletal_anim_count;
        u16                         material_anim_count;
        u16                         bone_visibility_anim_count;
        u16                         shape_anim_count;
        u16                         scene_anim_count;
        u16                         embed_file_count;
        union {
            u8                      external_options;
            struct {
                u8                  is_external_model_uninitalized : 1;
                u8                  has_external_strings           : 1;
                u8                  holds_external_strings         : 1;
                u8                  is_external_gpu_region         : 1;
                u8                  reserve9                       : 4;
            };
        };
        u8                          reserve10;

        static constexpr u64 cMagic = util::TCharCode64("FRES    ");

        static ResBfres *ResCast(void *file) {
            ResBfres *fres = reinterpret_cast<ResBfres*>(file);
            if (fres == nullptr || fres->ResNintendoWareFileHeader::IsValid(cMagic, 10, 0, 0) == false) { return nullptr; }
            fres->Relocate();
            return fres;
        }

        static bool IsValid(void *file) {
            ResBfres *fres = reinterpret_cast<ResBfres*>(file);
            return fres->ResNintendoWareFileHeader::IsValid(cMagic, 10, 0, 0);
        }
        
        void BindTextures(GfxBindTextureCallback bind_callback, ResBntx *res_bntx) {
            for (u32 i = 0; i < model_count; ++i) {
                model_array[i].BindTextures(bind_callback, res_bntx);
            }
            for (u32 i = 0; i < material_anim_count; ++i) {
                material_anim_array[i].BindTextures(bind_callback, res_bntx);
            }
        }

        constexpr ALWAYS_INLINE u64   GetGpuMemorySize()   const { return (memory_pool_info == nullptr) ? 0xffff'ffff'ffff'ffff : memory_pool_info->size; }
        constexpr ALWAYS_INLINE void *GetGpuMemoryRegion()       { return (memory_pool_info == nullptr) ? nullptr : memory_pool_info->gpu_region_base; }

        const char *TryFindRelocatedString(u64 key) {

            /* Binary search for key */
            u32  entry_index = 0;
            u32  iter        = embed_file_dictionary->node_count;
            u64 *key_array = reinterpret_cast<u64*>(embed_file_array);
            if (iter == 0) { return empty_string; }

            while (iter != 0) {
                const u32 next = iter >> 1;
                const u32 low = iter + ~next;
                iter = next;
                if (key_array[entry_index + next] < key) {
                    entry_index  = entry_index + next + 1;
                    iter = low;
                }
            }

            /* Try get key */
            if (key_array[entry_index] != key) { return empty_string; }
            return embed_file_dictionary->GetKeyByEntryIndex(entry_index);
        }

        void RelocateStrings(ResBfres *external_string_bfres) {

            /* Cache empty string */
            const char *ext_empty_string = external_string_bfres->empty_string;

            /* Relocate model strings */
            for (u32 i = 0; i < model_count; ++i) {

                /* Relocate shader reflection strings */
                for (u32 j = 0; j < model_array[i].shader_reflection_count; ++j) {

                    /* Relocate option dictionary strings */
                    ResNintendoWareDictionary *option_dic = model_array[i].shader_reflection_array[j].static_shader_option_dictionary;
                    if (option_dic != nullptr && option_dic->root_node.key == nullptr) {

                        option_dic->root_node.key = ext_empty_string;
                        for (s32 k = 0; k < option_dic->node_count; ++k) {
                            std::addressof(option_dic->root_node)[k + 1].key = external_string_bfres->TryFindRelocatedString(reinterpret_cast<u64>(std::addressof(option_dic->root_node)[k + 1].key));
                        }
                    }

                    /* Relocate render info dictionary strings */
                    ResNintendoWareDictionary *render_info_dic = model_array[i].shader_reflection_array[j].render_info_dictionary;
                    if (render_info_dic != nullptr && render_info_dic->root_node.key == nullptr) {

                        const u32 render_info_count    = model_array[i].shader_reflection_array[j].render_info_count;
                        render_info_dic->root_node.key = ext_empty_string;
                        for (u32 k = 0; k < render_info_count; ++k) {
                            const char *string = external_string_bfres->TryFindRelocatedString(reinterpret_cast<u64>(model_array[i].shader_reflection_array[j].render_info_array[k].render_info_name));

                            model_array[i].shader_reflection_array[j].render_info_array[k].render_info_name = string;
                            std::addressof(render_info_dic->root_node)[k + 1].key                           = string;
                        }
                    }

                    /* Relocate shader param dictionary strings */
                    ResNintendoWareDictionary *shader_param_dic = model_array[i].shader_reflection_array[j].shader_param_dictionary;
                    if (shader_param_dic != nullptr && shader_param_dic->root_node.key == nullptr) {

                        const u32 shader_param_count = model_array[i].shader_reflection_array[j].shader_param_count;
                        shader_param_dic->root_node.key = ext_empty_string;
                        for (u32 k = 0; k < shader_param_count; ++k) {
                            const char *string = external_string_bfres->TryFindRelocatedString(reinterpret_cast<u64>(model_array[i].shader_reflection_array[j].shader_param_array[k].shader_param_name));

                            model_array[i].shader_reflection_array[j].shader_param_array[k].shader_param_name = string;
                            std::addressof(shader_param_dic->root_node)[k + 1].key                            = string;
                        }
                    }
                }
            }

            /* Relocate material anim strings */
            for (u32 i = 0; i < material_anim_count; ++i) {

                /* Relocate per material anim strings */
                for (u32 j = 0; j < material_anim_array[i].per_material_anim_count; ++j) {

                    /* Relocate shader param strings */
                    for (u32 k = 0; k < material_anim_array[i].per_material_anim_array[j].shader_param_anim_count; ++k) {
                        material_anim_array[i].per_material_anim_array[j].shader_param_anim_array[k].shader_param_name = external_string_bfres->TryFindRelocatedString(reinterpret_cast<u64>(material_anim_array[i].per_material_anim_array[j].shader_param_anim_array[k].shader_param_name));
                    }
                }
            }

            /* Clear external strings flag */
            has_external_strings = false;

            return;
        }

        ResBfresModel *TryGetModel(const char *model_name) {
            if (model_dictionary == nullptr) { return nullptr; }
            const u32 entry_id = model_dictionary->TryGetEntryIndexByKey(model_name);
            if (entry_id == ResNintendoWareDictionary::cInvalidEntryIndex) { return nullptr; }
            return std::addressof(model_array[entry_id]);
        }
        ResBfresSkeletalAnim *TryGetSkeletalAnim(const char *skeletal_anim_name) {
            if (skeletal_anim_dictionary == nullptr) { return nullptr; }
            const u32 entry_id = skeletal_anim_dictionary->TryGetEntryIndexByKey(skeletal_anim_name);
            if (entry_id == ResNintendoWareDictionary::cInvalidEntryIndex) { return nullptr; }
            return std::addressof(skeletal_anim_array[entry_id]);
        }
        ResBfresMaterialAnim *TryGetMaterialAnim(const char *material_anim_name) {
            if (material_anim_dictionary == nullptr) { return nullptr; }
            const u32 entry_id = material_anim_dictionary->TryGetEntryIndexByKey(material_anim_name);
            if (entry_id == ResNintendoWareDictionary::cInvalidEntryIndex) { return nullptr; }
            return std::addressof(material_anim_array[entry_id]);
        }
        ResBfresBoneVisibilityAnim *TryGetBoneVisibilityAnim(const char *bone_visibility_anim_name) {
            if (bone_visibility_anim_dictionary == nullptr) { return nullptr; }
            const u32 entry_id = bone_visibility_anim_dictionary->TryGetEntryIndexByKey(bone_visibility_anim_name);
            if (entry_id == ResNintendoWareDictionary::cInvalidEntryIndex) { return nullptr; }
            return std::addressof(bone_visibility_anim_array[entry_id]);
        }
        ResBfresShapeAnim *TryGetShapeAnim(const char *shape_anim_name) {
            if (shape_anim_dictionary == nullptr) { return nullptr; }
            const u32 entry_id = shape_anim_dictionary->TryGetEntryIndexByKey(shape_anim_name);
            if (entry_id == ResNintendoWareDictionary::cInvalidEntryIndex) { return nullptr; }
            return std::addressof(shape_anim_array[entry_id]);
        }
        ResBfresSceneAnim *TryGetSceneAnim(const char *scene_anim_name) {
            if (scene_anim_dictionary == nullptr) { return nullptr; }
            const u32 entry_id = scene_anim_dictionary->TryGetEntryIndexByKey(scene_anim_name);
            if (entry_id == ResNintendoWareDictionary::cInvalidEntryIndex) { return nullptr; }
            return std::addressof(scene_anim_array[entry_id]);
        }
        ResGfxEmbedFile *TryGetEmbedFile(const char *embed_file_name) {
            if (embed_file_dictionary == nullptr) { return nullptr; }
            const u32 entry_id = embed_file_dictionary->TryGetEntryIndexByKey(embed_file_name);
            if (entry_id == ResNintendoWareDictionary::cInvalidEntryIndex) { return nullptr; }
            return std::addressof(embed_file_array[entry_id]);
        }
    };
    static_assert(sizeof(ResBfres) == 0xf0);
}
