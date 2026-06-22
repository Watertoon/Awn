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

    enum class BfresBoneRotationType : u8 {
        Quaternion = 0,
        EulerXYZ = 1,
    };
    enum class BfresBoneBillboardMode : u8 {
        None             = 0,
        Child            = 1,
        WorldViewVector  = 2,
        WorldViewPoint   = 3,
        ScreenViewVector = 4,
        ScreenViewPoint  = 5,
        YAxisViewVector  = 6,
        YAxisViewPoint   = 7,
    };
    enum class BfresBoneLocalTransformMode : u8 {
        None                    = 0,
        SegmentScaleCompensate  = (1 << 0),
        ScaleUniform            = (1 << 1),
        ScaleVolumeOne          = (1 << 2),
        NoRotation              = (1 << 3),
        NoTranslation           = (1 << 4),
    };
    VP_ENUM_FLAG_TRAITS(BfresBoneLocalTransformMode);
    enum class BfresBoneHierarchyTransformMode : u8 {
        None           = 0,
        ScaleUniform   = (1 << 0),
        ScaleVolumeOne = (1 << 1),
        NoRotation     = (1 << 2),
        NoTranslation  = (1 << 3),
    };
    VP_ENUM_FLAG_TRAITS(BfresBoneHierarchyTransformMode);
    enum class BfresBoneMirrorMode : u8 {
        ReflectYZ = 0,
        TransYZInvertAxes = 1,
        ReflectAxes = 2,
        InvertAxes = 3,
        TransXYZ = 4,
    };

    struct ResBfresBone {
        const NWString8           *name;
        ResGfxUserData            *user_data_array;
        ResNintendoWareDictionary *user_data_dictionary;
        void                      *reserve0;
        u16                        bone_id;
        u16                        parent_bone_id;
        u16                        bone_id_smooth;
        u16                        bone_id_rigid;
        u16                        billboard_bone_id;
        u16                        user_data_count;
        union {
            u32 flags;
            struct {
                u32 is_visible               : 1;
                u32 reserve1                 : 7;
                u32 unknown0                 : 4;
                u32 rotation_type            : 3;
                u32 reserve2                 : 1;
                u32 billboard_mode           : 3;
                u32 reserve3                 : 1;
                u32 mirror_mode              : 3;
                u32 local_transform_mode     : 5;
                u32 hierarchy_transform_mode : 4;
            };
        };
        util::Float3               scale;
        util::Float4               rotation;
        util::Float3               translation;

        static constexpr u32 cInvalidEntryIndex = 0xffff'ffff;

        constexpr u32 GetBoneId() const {
            return this->bone_id;
        }
        constexpr u32 GetParentBoneId() const {
            return (this->parent_bone_id != 0xffff) ? this->parent_bone_id : cInvalidEntryIndex;
        }
        constexpr u32 GetBillboardBoneId() const {
            return (this->billboard_bone_id != 0xffff) ? this->billboard_bone_id : cInvalidEntryIndex;
        }
        constexpr BfresBoneRotationType GetRotationType() const {
            return static_cast<BfresBoneRotationType>(this->rotation_type);
        }
        constexpr BfresBoneBillboardMode GetBillboardMode() const {
            return static_cast<BfresBoneBillboardMode>(this->billboard_mode);
        }
        constexpr BfresBoneMirrorMode GetMirrorMode() const {
            return static_cast<BfresBoneMirrorMode>(this->mirror_mode);
        }
        constexpr BfresBoneLocalTransformMode GetLocalTransformMode() const {
            return static_cast<BfresBoneLocalTransformMode>(this->local_transform_mode);
        }
        constexpr BfresBoneHierarchyTransformMode GetHierarchyTransformMode() const {
            return static_cast<BfresBoneHierarchyTransformMode>(this->hierarchy_transform_mode);
        }

        constexpr ResGfxUserData *TryGetUserData(const char *user_data_name) {
            if (user_data_dictionary == nullptr) { return nullptr; }
            const u32 entry_id = user_data_dictionary->TryGetEntryIndexByKey(user_data_name);
            if (entry_id == ResNintendoWareDictionary::cInvalidEntryIndex) { return nullptr; }
            return std::addressof(user_data_array[entry_id]);
        }
    };
    static_assert(sizeof(ResBfresBone) == 0x58);

    enum class BfresSkeletonMirrorAxes : u8 {
        YZAxes = 0,
        XZAxes = 1,
        XYAxes = 2,
    };
    enum class BfresSkeletonScaleMode : u8 {
        None      = 0,
        Standard  = 1,
        Maya      = 2,
        //SoftImage = 3, /* Deprecated */
    };
    enum class BfresSkeletonRotationMode : u8 {
        Quaternion = 0,
        EulerXYZ = 1,
        PerBoneRotationMode = 2,
    };

    struct ResBfresSkeleton {
        u32                        magic;
        union {
            u32 flags;
            struct {
                u32 has_user_pointer : 2;
                u32 reserve0         : 4;
                u32 mirror_axes      : 2;
                u32 scale_mode       : 2;
                u32 reserve1         : 2;
                u32 rotation_mode    : 3;
                u32 reserve2         : 17;
            };
        };
        ResNintendoWareDictionary *bone_dictionary;
        ResBfresBone              *bone_array;
        u16                       *skin_bone_id_array;
        util::FloatColumnMajor4x3 *inverse_transformation_matrix_array;
        void                      *runtime_user_pointer;
        u16                       *mirrored_bone_id_array;
        u16                        bone_count;
        u16                        bone_count_smooth;
        u16                        bone_count_rigid;
        u16                        reserve3;

        static constexpr u32 cMagic = util::TCharCode32("FSKL");
        static constexpr u32 cInvalidEntryIndex = 0xffff'ffff;

        constexpr BfresSkeletonMirrorAxes GetMirrorAxes() const {
            return static_cast<BfresSkeletonMirrorAxes>(this->mirror_axes);
        }
        constexpr BfresSkeletonScaleMode GetScaleMode() const {
            return static_cast<BfresSkeletonScaleMode>(this->scale_mode);
        }
        constexpr BfresSkeletonRotationMode GetRotationMode() const {
            return static_cast<BfresSkeletonRotationMode>(this->rotation_mode);
        }

        constexpr bool IsMirroredBone(u32 bone_id) const {
            if (this->mirrored_bone_id_array == nullptr) { return false; }
            if (this->bone_count <= bone_id) { return false; }
            return this->mirrored_bone_id_array[bone_id] < 0x8000;
        }

        constexpr const u16 *GetSmoothBoneIdArray() const {
            return this->skin_bone_id_array;
        }

        constexpr const u16 *GetRigidBoneIdArray() const {
            return std::addressof(this->skin_bone_id_array[this->bone_count_smooth]);
        }

        constexpr u32 FindBoneIdAtEndOfBranch(u32 bone_id) const {
            const u32 count = static_cast<u32>(this->bone_count);
            if (count <= bone_id) { return cInvalidEntryIndex; }
            u32 parent_bone = (bone_id == 0) ? cInvalidEntryIndex : static_cast<u32>(this->bone_array[bone_id].parent_bone_id);
            u32 end_bone = (count < bone_id + 1) ? bone_id + 1 : count;
            for (; bone_id < count; ++bone_id) {
                const u32 next_parent_iter = static_cast<u32>(this->bone_array[bone_id + 1].parent_bone_id);
                if (parent_bone < next_parent_iter) { return next_parent_iter; }
            }
            return end_bone;
        }

        constexpr u32 TryGetBoneId(const char *bone_name) const {
            if (bone_dictionary == nullptr) { return cInvalidEntryIndex; }
            const u32 entry_id = bone_dictionary->TryGetEntryIndexByKey(bone_name);
            return entry_id;
        }

        constexpr ResBfresBone *GetBoneById(u32 bone_id) {
            if (this->bone_count <= bone_id) { return nullptr; }
            return std::addressof(bone_array[bone_id]);
        }
        constexpr ResBfresBone *GetBoneByName(const char *bone_name) {
            if (bone_dictionary == nullptr) { return nullptr; }
            const u32 entry_id = bone_dictionary->TryGetEntryIndexByKey(bone_name);
            if (entry_id == ResNintendoWareDictionary::cInvalidEntryIndex) { return nullptr; }
            return std::addressof(bone_array[entry_id]);
        }
    };
    static_assert(sizeof(ResBfresSkeleton) == 0x40);
}
