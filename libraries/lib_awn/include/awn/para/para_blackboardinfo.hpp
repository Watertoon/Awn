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

namespace awn::para {

    struct BlackboardInfo_Layout {
        PropertyArray<const char*> BlackboardParamTables = {};
        bool                       IsSharedBlackboard    = false;
        bool                       IsProductRecord       = false;
        bool                       IsDebugRecord         = false;

        AWN_PARA_LAYOUT_TYPE_TRAITS(BlackboardInfo_Layout);
    };

    AWN_PARA_DECLARE_TYPED_PARAM_AUTOV(
        awn__para__BlackboardInfo, BlackboardInfo, 
        TypedParam,                BlackboardInfo_Layout,
        BlackboardParamTables,
        IsSharedBlackboard,
        IsProductRecord,
        IsDebugRecord
    );

    struct BlackboardParam_Layout {
        const char *BBKey                       = "";
        bool        IsInstanceParam             = false;
        bool        IsSafeParamForExternalTools = false;
        bool        IsVisibleForMapUnit         = true;
        bool        IsRecord                    = false;

        AWN_PARA_LAYOUT_TYPE_TRAITS(BlackboardParam_Layout);
    };

    AWN_PARA_DECLARE_TYPED_PARAM_AUTOV(
        awn__para__BlackboardParam, BlackboardParam,
        TypedParam,                 BlackboardParam_Layout,
        BBKey,
        IsInstanceParam,
        IsSafeParamForExternalTools,
        IsVisibleForMapUnit,
        IsRecord
    );

    struct BlackboardParamBool_Layout {
        bool InitVal = 0.0f;

        AWN_PARA_LAYOUT_TYPE_TRAITS(BlackboardParamBool_Layout);
    };

    AWN_PARA_DECLARE_TYPED_PARAM_AUTOV(
        awn__para__BlackboardParamBool, BlackboardParamBool,
        BlackboardParam,                BlackboardParamBool_Layout,
        InitVal
    );

    struct BlackboardParamClr4f_Layout {
        AWN_PARA_LAYOUT_TYPE_TRAITS(BlackboardParamClr4f_Layout);
    };

    AWN_PARA_DECLARE_TYPED_PARAM_AUTO(
        awn__para__BlackboardParamClr4f, BlackboardParamClr4f,
        BlackboardParam,                 BlackboardParamClr4f_Layout
    );

    struct BlackboardParamCustomType_Layout {
        const char *AIEditorBindType = "";
        const char *ClassName = "";

        AWN_PARA_LAYOUT_TYPE_TRAITS(BlackboardParamCustomType_Layout);
    };

    AWN_PARA_DECLARE_TYPED_PARAM_AUTOV(
        awn__para__BlackboardParamCustomType, BlackboardParamCustomType,
        BlackboardParam,                      BlackboardParamCustomType_Layout,
        ClassName,
        AIEditorBindType
    );

    struct BlackboardParamFloatBase_Layout {
        float InitVal = 0.0f;

        AWN_PARA_LAYOUT_TYPE_TRAITS(BlackboardParamFloatBase_Layout);
    };

    AWN_PARA_DECLARE_TYPED_PARAM_AUTOV(
        awn__para__BlackboardParamFloatBase, BlackboardParamFloatBase,
        BlackboardParam,                     BlackboardParamFloatBase_Layout,
        InitVal
    );

    struct BlackboardParamF32_Layout {
        AWN_PARA_LAYOUT_TYPE_TRAITS(BlackboardParamF32_Layout);
    };

    AWN_PARA_DECLARE_TYPED_PARAM_AUTO(
        awn__para__BlackboardParamF32, BlackboardParamF32,
        BlackboardParamFloatBase,      BlackboardParamF32_Layout
    );

    struct BlackboardParamF64_Layout {
        AWN_PARA_LAYOUT_TYPE_TRAITS(BlackboardParamF64_Layout);
    };

    AWN_PARA_DECLARE_TYPED_PARAM_AUTO(
        awn__para__BlackboardParamF64, BlackboardParamF64,
        BlackboardParamFloatBase,      BlackboardParamF64_Layout
    );

    struct BlackboardParamIntBase_Layout {
        s32 InitVal = 0;

        AWN_PARA_LAYOUT_TYPE_TRAITS(BlackboardParamIntBase_Layout);
    };

    AWN_PARA_DECLARE_TYPED_PARAM_AUTOV(
        awn__para__BlackboardParamIntBase, BlackboardParamIntBase,
        BlackboardParam,                   BlackboardParamIntBase_Layout,
        InitVal
    );

    struct BlackboardParamS8_Layout {
        AWN_PARA_LAYOUT_TYPE_TRAITS(BlackboardParamS8_Layout);
    };

    AWN_PARA_DECLARE_TYPED_PARAM_AUTO(
        awn__para__BlackboardParamS8, BlackboardParamS8,
        BlackboardParamIntBase,       BlackboardParamS8_Layout
    );

    struct BlackboardParamS16_Layout {
        AWN_PARA_LAYOUT_TYPE_TRAITS(BlackboardParamS16_Layout);
    };

    AWN_PARA_DECLARE_TYPED_PARAM_AUTO(
        awn__para__BlackboardParamS16, BlackboardParamS16,
        BlackboardParamIntBase,        BlackboardParamS16_Layout
    );

    struct BlackboardParamS32_Layout {
        AWN_PARA_LAYOUT_TYPE_TRAITS(BlackboardParamS32_Layout);
    };

    AWN_PARA_DECLARE_TYPED_PARAM_AUTO(
        awn__para__BlackboardParamS32, BlackboardParamS32,
        BlackboardParamIntBase,        BlackboardParamS32_Layout
    );

    struct BlackboardParamS64_Layout {
        AWN_PARA_LAYOUT_TYPE_TRAITS(BlackboardParamS64_Layout);
    };

    AWN_PARA_DECLARE_TYPED_PARAM_AUTO(
        awn__para__BlackboardParamS64, BlackboardParamS64,
        BlackboardParamIntBase,        BlackboardParamS64_Layout
    );

    struct BlackboardParamUIntBase_Layout {
        u32 InitVal = 0;

        AWN_PARA_LAYOUT_TYPE_TRAITS(BlackboardParamUIntBase_Layout);
    };

    AWN_PARA_DECLARE_TYPED_PARAM_AUTOV(
        awn__para__BlackboardParamUIntBase, BlackboardParamUIntBase,
        BlackboardParam,                    BlackboardParamUIntBase_Layout,
        InitVal
    );

    struct BlackboardParamU8_Layout {
        AWN_PARA_LAYOUT_TYPE_TRAITS(BlackboardParamU8_Layout);
    };

    AWN_PARA_DECLARE_TYPED_PARAM_AUTO(
        awn__para__BlackboardParamU8, BlackboardParamU8,
        BlackboardParamUIntBase,      BlackboardParamU8_Layout
    );

    struct BlackboardParamU16_Layout {
        AWN_PARA_LAYOUT_TYPE_TRAITS(BlackboardParamU16_Layout);
    };

    AWN_PARA_DECLARE_TYPED_PARAM_AUTO(
        awn__para__BlackboardParamU16, BlackboardParamU16,
        BlackboardParamUIntBase,       BlackboardParamU16_Layout
    );

    struct BlackboardParamU32_Layout {
        AWN_PARA_LAYOUT_TYPE_TRAITS(BlackboardParamU32_Layout);
    };

    AWN_PARA_DECLARE_TYPED_PARAM_AUTO(
        awn__para__BlackboardParamU32, BlackboardParamU32,
        BlackboardParamUIntBase,       BlackboardParamU32_Layout
    );

    struct BlackboardParamU64_Layout {
        AWN_PARA_LAYOUT_TYPE_TRAITS(BlackboardParamU64_Layout);
    };

    AWN_PARA_DECLARE_TYPED_PARAM_AUTO(
        awn__para__BlackboardParamU64, BlackboardParamU64,
        BlackboardParamUIntBase,       BlackboardParamU64_Layout
    );

    struct BlackboardParamString_Layout {
        const char *InitValConverted = "";
        const char *StringParamType = "Reference";

        AWN_PARA_LAYOUT_TYPE_TRAITS(BlackboardParamString_Layout);
    };

    AWN_PARA_DECLARE_TYPED_PARAM_AUTOV(
        awn__para__BlackboardParamString, BlackboardParamString,
        BlackboardParam,                  BlackboardParamString_Layout,
        StringParamType,
        InitValConverted
    );

    struct BlackboardParamVec3f_Layout {
        vp::util::Float3 InitVal = {};

        AWN_PARA_LAYOUT_TYPE_TRAITS(BlackboardParamVec3f_Layout);
    };

    AWN_PARA_DECLARE_TYPED_PARAM_AUTOV(
        awn__para__BlackboardParamVec3f, BlackboardParamVec3f,
        BlackboardParam,                 BlackboardParamVec3f_Layout,
        InitVal
    );

    struct BlackboardParamVec3i_Layout {
        vp::util::Int3 InitVal = {};

        AWN_PARA_LAYOUT_TYPE_TRAITS(BlackboardParamVec3i_Layout);
    };

    AWN_PARA_DECLARE_TYPED_PARAM_AUTOV(
        awn__para__BlackboardParamVec3i, BlackboardParamVec3i,
        BlackboardParam,                 BlackboardParamVec3i_Layout,
        InitVal
    );

    struct BlackboardParamQuatf_Layout {
        AWN_PARA_LAYOUT_TYPE_TRAITS(BlackboardParamQuatf_Layout);
    };

    AWN_PARA_DECLARE_TYPED_PARAM_AUTO(
        awn__para__BlackboardParamQuatf, BlackboardParamQuatf,
        BlackboardParam,                 BlackboardParamQuatf_Layout
    );

    struct BlackboardParamMtx33f_Layout {
        AWN_PARA_LAYOUT_TYPE_TRAITS(BlackboardParamMtx33f_Layout);
    };

    AWN_PARA_DECLARE_TYPED_PARAM_AUTO(
        awn__para__BlackboardParamMtx33f, BlackboardParamMtx33f,
        BlackboardParam,                  BlackboardParamMtx33f_Layout
    );

    struct BlackboardParamMtx34f_Layout {
        AWN_PARA_LAYOUT_TYPE_TRAITS(BlackboardParamMtx34f_Layout);
    };

    AWN_PARA_DECLARE_TYPED_PARAM_AUTO(
        awn__para__BlackboardParamMtx34f, BlackboardParamMtx34f,
        BlackboardParam,                  BlackboardParamMtx34f_Layout
    );

    struct BlackboardParamPtr_Layout {
        const char *AIEditorBindType = "";

        AWN_PARA_LAYOUT_TYPE_TRAITS(BlackboardParamPtr_Layout);
    };

    AWN_PARA_DECLARE_TYPED_PARAM_AUTOV(
        awn__para__BlackboardParamPtr, BlackboardParamPtr,
        BlackboardParam,               BlackboardParamPtr_Layout,
        AIEditorBindType
    );

    struct BlackboardParamTable_Layout {
        TypedParamArray<BlackboardParamBool>       BlackboardParamBoolArray;
        TypedParamArray<BlackboardParamClr4f>      BlackboardParamClr4fArray;
        TypedParamArray<BlackboardParamCustomType> BlackboardParamCustomTypeArray;
        TypedParamArray<BlackboardParamF32>        BlackboardParamF32Array;
        TypedParamArray<BlackboardParamF64>        BlackboardParamF64Array;
        TypedParamArray<BlackboardParamMtx33f>     BlackboardParamMtx33fArray;
        TypedParamArray<BlackboardParamMtx34f>     BlackboardParamMtx34fArray;
        TypedParamArray<BlackboardParamPtr>        BlackboardParamPtrArray;
        TypedParamArray<BlackboardParamQuatf>      BlackboardParamQuatfArray;
        TypedParamArray<BlackboardParamS16>        BlackboardParamS16Array;
        TypedParamArray<BlackboardParamS32>        BlackboardParamS32Array;
        TypedParamArray<BlackboardParamS64>        BlackboardParamS64Array;
        TypedParamArray<BlackboardParamS8>         BlackboardParamS8Array;
        TypedParamArray<BlackboardParamString>     BlackboardParamStringArray;
        TypedParamArray<BlackboardParamU16>        BlackboardParamU16Array;
        TypedParamArray<BlackboardParamU32>        BlackboardParamU32Array;
        TypedParamArray<BlackboardParamU64>        BlackboardParamU64Array;
        TypedParamArray<BlackboardParamU8>         BlackboardParamU8Array;
        TypedParamArray<BlackboardParamVec3f>      BlackboardParamVec3fArray;
        TypedParamArray<BlackboardParamVec3i>      BlackboardParamVec3iArray;

        AWN_PARA_LAYOUT_TYPE_TRAITS(BlackboardParamTable_Layout);
    };

    AWN_PARA_DECLARE_TYPED_PARAM_AUTOV(
        awn__para__BlackboardParamTable, BlackboardParamTable, 
        TypedParam,                      BlackboardParamTable_Layout,
        BlackboardParamStringArray,
        BlackboardParamS8Array,
        BlackboardParamS16Array,
        BlackboardParamS32Array,
        BlackboardParamS64Array,
        BlackboardParamU8Array,
        BlackboardParamU16Array,
        BlackboardParamU32Array,
        BlackboardParamU64Array,
        BlackboardParamF32Array,
        BlackboardParamF64Array,
        BlackboardParamBoolArray,
        BlackboardParamVec3fArray,
        BlackboardParamVec3iArray,
        BlackboardParamClr4fArray,
        BlackboardParamMtx33fArray,
        BlackboardParamMtx34fArray,
        BlackboardParamQuatfArray,
        BlackboardParamPtrArray,
        BlackboardParamCustomTypeArray
    );
}
