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
        const char                 *reserve4;
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
            if (fres == nullptr || fres->ResNintendoWareFileHeader::IsValid(cMagic, 10, 0) == false) { return nullptr; }
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

        constexpr ALWAYS_INLINE u64 GetGpuMemorySize() {
            return (memory_pool_info == nullptr) ? 0xffff'ffff'ffff'ffff : memory_pool_info->size;
        }

        constexpr ALWAYS_INLINE void *GetGpuMemoryRegion() {
            return (memory_pool_info == nullptr) ? nullptr : memory_pool_info->storage;
        }

        /*const char *FindRelocatedString(u64 key) {

            
        }

        void RelocateStrings(ResBfres *external_string_bfres) {
            
            
            for (u32 i = 0; i < model_count; ++i) {
                
                for (u32 y = 0; y < model_array[i].shader_reflection_count; ++y) {
                    
                }
            }
        }*/
        
        ResBfresModel *TryGetModel(const char *model_name) {
            if (model_dictionary == nullptr) { return nullptr; }
            const u32 entry_id = model_dictionary->FindEntryIndex(model_name);
            if (entry_id == ResNintendoWareDictionary::cInvalidEntryId) { return nullptr; }
            return std::addressof(model_array[entry_id]);
        }
        ResBfresSkeletalAnim *TryGetSkeletalAnim(const char *skeletal_anim_name) {
            if (skeletal_anim_dictionary == nullptr) { return nullptr; }
            const u32 entry_id = skeletal_anim_dictionary->FindEntryIndex(skeletal_anim_name);
            if (entry_id == ResNintendoWareDictionary::cInvalidEntryId) { return nullptr; }
            return std::addressof(skeletal_anim_array[entry_id]);
        }
        ResBfresMaterialAnim *TryGetMaterialAnim(const char *material_anim_name) {
            if (material_anim_dictionary == nullptr) { return nullptr; }
            const u32 entry_id = material_anim_dictionary->FindEntryIndex(material_anim_name);
            if (entry_id == ResNintendoWareDictionary::cInvalidEntryId) { return nullptr; }
            return std::addressof(material_anim_array[entry_id]);
        }
        ResBfresBoneVisibilityAnim *TryGetBoneVisibilityAnim(const char *bone_visibility_anim_name) {
            if (bone_visibility_anim_dictionary == nullptr) { return nullptr; }
            const u32 entry_id = bone_visibility_anim_dictionary->FindEntryIndex(bone_visibility_anim_name);
            if (entry_id == ResNintendoWareDictionary::cInvalidEntryId) { return nullptr; }
            return std::addressof(bone_visibility_anim_array[entry_id]);
        }
        ResBfresShapeAnim *TryGetShapeAnim(const char *shape_anim_name) {
            if (shape_anim_dictionary == nullptr) { return nullptr; }
            const u32 entry_id = shape_anim_dictionary->FindEntryIndex(shape_anim_name);
            if (entry_id == ResNintendoWareDictionary::cInvalidEntryId) { return nullptr; }
            return std::addressof(shape_anim_array[entry_id]);
        }
        ResBfresSceneAnim *TryGetSceneAnim(const char *scene_anim_name) {
            if (scene_anim_dictionary == nullptr) { return nullptr; }
            const u32 entry_id = scene_anim_dictionary->FindEntryIndex(scene_anim_name);
            if (entry_id == ResNintendoWareDictionary::cInvalidEntryId) { return nullptr; }
            return std::addressof(scene_anim_array[entry_id]);
        }
        ResGfxEmbedFile *TryGetEmbedFile(const char *embed_file_name) {
            if (embed_file_dictionary == nullptr) { return nullptr; }
            const u32 entry_id = embed_file_dictionary->FindEntryIndex(embed_file_name);
            if (entry_id == ResNintendoWareDictionary::cInvalidEntryId) { return nullptr; }
            return std::addressof(embed_file_array[entry_id]);
        }
    };
    static_assert(sizeof(ResBfres) == 0xf0);
}
