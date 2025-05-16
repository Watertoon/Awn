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

    struct ResBfresEmbedFile {
        void *file;
        u32   file_size;
        u32   reserve0;
    };
    static_assert(sizeof(ResBfresEmbedFile) == 0x10);

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
        void                       *runtime_memory_pool;
        ResGfxMemoryPoolInfo       *memory_pool_info;
        ResBfresEmbedFile          *embed_file_array;
        ResNintendoWareDictionary  *embed_file_dictionary;
        void                       *runtime_user_pointer;
        const char                 *empty_string;
        u32                         string_pool_size;
        u16                         model_count;
        u16                         reserve4;
        u16                         reserve5;
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
                u8                  reserve6                       : 4;
            };
        };
        u8                          reserve7;

        static constexpr u64 cMagic = util::TCharCode64("FRES    ");
        static constexpr u16 cTargetVersionMajor = 10;
        static constexpr u8  cTargetVersionMinor = 2;
        static constexpr u8  cTargetVersionMicro = 0;
        static constexpr u64 cInvalidGpuMemorySize = 0xffff'ffff'ffff'ffff;

        static ResBfres *ResCast(void *file) {
            ResBfres *fres = reinterpret_cast<ResBfres*>(file);
            if (fres == nullptr || !ResBfres::IsValid(file)) { return nullptr; }
            fres->Relocate();
            return fres;
        }

        static bool IsValid(void *file) {
            ResBfres *fres = reinterpret_cast<ResBfres*>(file);
            return fres->ResNintendoWareFileHeader::IsValid(cMagic, cTargetVersionMajor, cTargetVersionMinor, cTargetVersionMicro);
        }

        void BindTextures(GfxBindTextureCallback bind_callback, void *user_data) {
            for (u32 i = 0; i < this->model_count; ++i) {
                this->model_array[i].BindTextures(bind_callback, user_data);
            }
            for (u32 i = 0; i < this->material_anim_count; ++i) {
                this->material_anim_array[i].BindTextures(bind_callback, user_data);
            }
            return;
        }

        constexpr ALWAYS_INLINE u64 GetGpuRegionSize() const {
            const u64 size = (this->memory_pool_info == nullptr) ? cInvalidGpuMemorySize : this->memory_pool_info->size;
            return size;
        }
        constexpr ALWAYS_INLINE void *GetGpuRegion() {
            void *memory = (this->memory_pool_info == nullptr) ? nullptr : this->memory_pool_info->gpu_region_base;
            return memory;
        }

        const char *TryFindRelocatedString(u64 key) {

            /* Binary search for key */
            u32  entry_index = 0;
            u32  iter = this->embed_file_dictionary->node_count;
            u64 *key_array = reinterpret_cast<u64*>(this->embed_file_array);
            if (iter == 0) { return this->empty_string; }

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
            if (key_array[entry_index] != key) { return this->empty_string; }
            return this->embed_file_dictionary->GetKeyByEntryIndex(entry_index);
        }

        void RelocateStrings(ResBfres *external_string_bfres) {

            /* Cache empty string */
            const char *ext_empty_string = external_string_bfres->empty_string;

            /* Relocate model strings */
            for (u32 i = 0; i < this->model_count; ++i) {

                /* Relocate shader reflection strings */
                ResBfresModel *model = std::addressof(this->model_array[i]);
                for (u32 j = 0; j < model->shader_reflection_count; ++j) {

                    /* Relocate option dictionary strings */
                    ResBfresShaderReflection *reflection = std::addressof(model->shader_reflection_array[j]);
                    ResNintendoWareDictionary *option_dic = reflection->static_shader_option_dictionary;
                    if (option_dic != nullptr && option_dic->root_node.key == nullptr) {

                        option_dic->root_node.key = ext_empty_string;
                        for (s32 k = 0; k < option_dic->node_count; ++k) {
                            std::addressof(option_dic->root_node)[k + 1].key = external_string_bfres->TryFindRelocatedString(reinterpret_cast<u64>(std::addressof(option_dic->root_node)[k + 1].key));
                        }
                    }

                    /* Relocate render info dictionary strings */
                    ResNintendoWareDictionary *render_info_dic = reflection->render_info_dictionary;
                    if (render_info_dic != nullptr && render_info_dic->root_node.key == nullptr) {

                        const u32 render_info_count = reflection->render_info_count;
                        render_info_dic->root_node.key = ext_empty_string;
                        for (u32 k = 0; k < render_info_count; ++k) {
                            ResBfresRenderInfo *render_info = std::addressof(reflection->render_info_array[k]);
                            const char *string = external_string_bfres->TryFindRelocatedString(reinterpret_cast<u64>(render_info->name));
                            render_info->name = string;
                            std::addressof(render_info_dic->root_node)[k + 1].key = string;
                        }
                    }

                    /* Relocate shader param dictionary strings */
                    ResNintendoWareDictionary *shader_param_dic = reflection->shader_param_dictionary;
                    if (shader_param_dic != nullptr && shader_param_dic->root_node.key == nullptr) {

                        const u32 shader_param_count = reflection->shader_param_count;
                        shader_param_dic->root_node.key = ext_empty_string;
                        for (u32 k = 0; k < shader_param_count; ++k) {
                            ResBfresShaderParam *shader_param = std::addressof(reflection->shader_param_array[k]);
                            const char *string = external_string_bfres->TryFindRelocatedString(reinterpret_cast<u64>(shader_param->name));
                            shader_param->name = string;
                            std::addressof(shader_param_dic->root_node)[k + 1].key = string;
                        }
                    }
                }
            }

            /* Relocate material anim strings */
            for (u32 i = 0; i < this->material_anim_count; ++i) {

                /* Relocate per material anim strings */
                ResBfresMaterialAnim *material_anim = std::addressof(this->material_anim_array[i]);
                const u32 per_material_anim_count = material_anim->per_material_anim_count;
                for (u32 j = 0; j < per_material_anim_count; ++j) {

                    /* Relocate shader param strings */
                    ResBfresPerMaterialAnim *per_material_anim = std::addressof(material_anim->per_material_anim_array[j]);
                    const u32 shader_param_anim_count = per_material_anim->shader_param_anim_count;
                    for (u32 k = 0; k < shader_param_anim_count; ++k) {
                        ResBfresShaderParamAnim *shader_param_anim = std::addressof(per_material_anim->shader_param_anim_array[k]);
                        const char *string = external_string_bfres->TryFindRelocatedString(reinterpret_cast<u64>(shader_param_anim->shader_param_name));
                        shader_param_anim->shader_param_name = string;
                    }
                }
            }

            /* Clear external strings flag */
            this->has_external_strings = false;

            return;
        }

        ResBfresModel *TryGetModel(const char *model_name) {
            if (this->model_dictionary == nullptr) { return nullptr; }
            const u32 entry_id = this->model_dictionary->TryGetEntryIndexByKey(model_name);
            if (entry_id == ResNintendoWareDictionary::cInvalidEntryIndex) { return nullptr; }
            return std::addressof(this->model_array[entry_id]);
        }
        ResBfresSkeletalAnim *TryGetSkeletalAnim(const char *skeletal_anim_name) {
            if (this->skeletal_anim_dictionary == nullptr) { return nullptr; }
            const u32 entry_id = this->skeletal_anim_dictionary->TryGetEntryIndexByKey(skeletal_anim_name);
            if (entry_id == ResNintendoWareDictionary::cInvalidEntryIndex) { return nullptr; }
            return std::addressof(this->skeletal_anim_array[entry_id]);
        }
        ResBfresMaterialAnim *TryGetMaterialAnim(const char *material_anim_name) {
            if (this->material_anim_dictionary == nullptr) { return nullptr; }
            const u32 entry_id = this->material_anim_dictionary->TryGetEntryIndexByKey(material_anim_name);
            if (entry_id == ResNintendoWareDictionary::cInvalidEntryIndex) { return nullptr; }
            return std::addressof(this->material_anim_array[entry_id]);
        }
        ResBfresBoneVisibilityAnim *TryGetBoneVisibilityAnim(const char *bone_visibility_anim_name) {
            if (this->bone_visibility_anim_dictionary == nullptr) { return nullptr; }
            const u32 entry_id = this->bone_visibility_anim_dictionary->TryGetEntryIndexByKey(bone_visibility_anim_name);
            if (entry_id == ResNintendoWareDictionary::cInvalidEntryIndex) { return nullptr; }
            return std::addressof(this->bone_visibility_anim_array[entry_id]);
        }
        ResBfresShapeAnim *TryGetShapeAnim(const char *shape_anim_name) {
            if (this->shape_anim_dictionary == nullptr) { return nullptr; }
            const u32 entry_id = this->shape_anim_dictionary->TryGetEntryIndexByKey(shape_anim_name);
            if (entry_id == ResNintendoWareDictionary::cInvalidEntryIndex) { return nullptr; }
            return std::addressof(this->shape_anim_array[entry_id]);
        }
        ResBfresSceneAnim *TryGetSceneAnim(const char *scene_anim_name) {
            if (this->scene_anim_dictionary == nullptr) { return nullptr; }
            const u32 entry_id = this->scene_anim_dictionary->TryGetEntryIndexByKey(scene_anim_name);
            if (entry_id == ResNintendoWareDictionary::cInvalidEntryIndex) { return nullptr; }
            return std::addressof(this->scene_anim_array[entry_id]);
        }
        ResBfresEmbedFile *TryGetEmbedFile(const char *embed_file_name) {
            if (this->embed_file_dictionary == nullptr) { return nullptr; }
            const u32 entry_id = this->embed_file_dictionary->TryGetEntryIndexByKey(embed_file_name);
            if (entry_id == ResNintendoWareDictionary::cInvalidEntryIndex) { return nullptr; }
            return std::addressof(this->embed_file_array[entry_id]);
        }
    };
    static_assert(sizeof(ResBfres) == 0xf0);
}
