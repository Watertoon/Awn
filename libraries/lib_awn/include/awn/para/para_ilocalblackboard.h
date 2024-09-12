#pragma once

namespace awn::para {

    class IBlackboardAccessor;

    class ILocalBlackboard {
        public:
            constexpr ILocalBlackboard() {/*...*/}
            virtual constexpr ~ILocalBlackboard() {/*...*/}

            virtual void SetStringByKey(const char *key, const char *value) { VP_UNUSED(key, value); return; }
            virtual void SetS32ByKey(const char *key, s32 *value) { VP_UNUSED(key, value); return; }
            virtual void SetFloatByKey(const char *key, float *value) { VP_UNUSED(key, value); return; }
            virtual void SetBoolByKey(const char *key, bool *value) { VP_UNUSED(key, value); return; }
            virtual void SetVector3fByKey(const char *key, vp::util::Float3 *value) { VP_UNUSED(key, value); return; }
            virtual void SetPointerByKey(const char *key, void *value) { VP_UNUSED(key, value); return; }

            virtual void SetStringByIndex(u32 index, const char *value) { VP_UNUSED(index, value); return; }
            virtual void SetS32ByIndex(u32 index, s32 *value) { VP_UNUSED(index, value); return; }
            virtual void SetFloatByIndex(u32 index, float *value) { VP_UNUSED(index, value); return; }
            virtual void SetBoolByIndex(u32 index, bool *value) { VP_UNUSED(index, value); return; }
            virtual void SetVector3fByIndex(u32 index, vp::util::Float3 *value) { VP_UNUSED(index, value); return; }
            virtual void SetPointerByIndex(u32 index, void *value) { VP_UNUSED(index, value); return; }

            virtual bool TrySetStringByKey(const char *key, const char *value) { VP_UNUSED(key, value); return false; }
            virtual bool TrySetS32ByKey(const char *key, s32 *value) { VP_UNUSED(key, value); return false; }
            virtual bool TrySetFloatByKey(const char *key, float *value) { VP_UNUSED(key, value); return false; }
            virtual bool TrySetBoolByKey(const char *key, bool *value) { VP_UNUSED(key, value); return false; }
            virtual bool TrySetVector3fByKey(const char *key, vp::util::Float3 *value) { VP_UNUSED(key, value); return false; }
            virtual bool TrySetPointerByKey(const char *key, void *value) { VP_UNUSED(key, value); return false; }

            virtual const char       **GetStringPointerByKey(const char *key, const char *value) { VP_UNUSED(key, value); return nullptr; }
            virtual s32               *GetS32PointerByKey(const char *key, s32 *value) { VP_UNUSED(key, value); return nullptr; }
            virtual float             *GetFloatPointerByKey(const char *key, float *value) { VP_UNUSED(key, value); return nullptr; }
            virtual bool              *GetBoolPointerByKey(const char *key, bool *value) { VP_UNUSED(key, value); return nullptr; }
            virtual vp::util::Float3  *GetVector3fPointerByKey(const char *key, vp::util::Float3 *value) { VP_UNUSED(key, value); return nullptr; }
            virtual void             **GetPointerPointerByKey(const char *key, void *value) { VP_UNUSED(key, value); return nullptr; }
                                     
            virtual const char       **GetStringPointerByIndex(u32 index, const char *value) { VP_UNUSED(index, value); return nullptr; }
            virtual s32               *GetS32PointerByIndex(u32 index, s32 *value) { VP_UNUSED(index, value); return nullptr; }
            virtual float             *GetFloatPointerByIndex(u32 index, float *value) { VP_UNUSED(index, value); return nullptr; }
            virtual bool              *GetBoolPointerByIndex(u32 index, bool *value) { VP_UNUSED(index, value); return nullptr; }
            virtual vp::util::Float3  *GetVector3fPointerByIndex(u32 index, vp::util::Float3 *value) { VP_UNUSED(index, value); return nullptr; }
            virtual void             **GetPointerPointerByIndex(u32 index, void *value) { VP_UNUSED(index, value); return nullptr; }
                                     
            virtual const char       *GetStringByKey(const char *key) const { VP_UNUSED(key); return ""; }
            virtual s32               GetS32ByKey(const char *key) const { VP_UNUSED(key); return {}; }
            virtual float             GetFloatByKey(const char *key) const { VP_UNUSED(key); return {}; }
            virtual bool              GetBoolByKey(const char *key) const { VP_UNUSED(key); return {}; }
            virtual vp::util::Float3  GetVector3fByKey(const char *key) const { VP_UNUSED(key); return {}; }
            virtual void             *GetPointerByKey(const char *key) const { VP_UNUSED(key); return nullptr; }
                                     
            virtual const char       *GetStringByIndex(u32 index) const { VP_UNUSED(index); return ""; }
            virtual s32               GetS32ByIndex(u32 index) const { VP_UNUSED(index); return {}; }
            virtual float             GetFloatByIndex(u32 index) const { VP_UNUSED(index); return {}; }
            virtual bool              GetBoolByIndex(u32 index) const { VP_UNUSED(index); return {}; }
            virtual vp::util::Float3  GetVector3fByIndex(u32 index) const { VP_UNUSED(index); return {}; }
            virtual void             *GetPointerByIndex(u32 index) const { VP_UNUSED(index); return nullptr; }

            virtual const char *GetStringFieldNameByIndex(u32 index) const { VP_UNUSED(index); return ""; }
            virtual const char *GetS32FieldNameByIndex(u32 index) const { VP_UNUSED(index); return ""; }
            virtual const char *GetFloatFieldNameByIndex(u32 index) const { VP_UNUSED(index); return ""; }
            virtual const char *GetBoolFieldNameByIndex(u32 index) const { VP_UNUSED(index); return ""; }
            virtual const char *GetVector3fFieldNameByIndex(u32 index) const { VP_UNUSED(index); return ""; }
            virtual const char *GetPointerFieldNameByIndex(u32 index) const { VP_UNUSED(index); return ""; }

            virtual const char *GetStringFilePathByIndex(u32 index) const { VP_UNUSED(index); return ""; }
            virtual const char *GetS32FilePathByIndex(u32 index) const { VP_UNUSED(index); return ""; }
            virtual const char *GetFloatFilePathByIndex(u32 index) const { VP_UNUSED(index); return ""; }
            virtual const char *GetBoolFilePathByIndex(u32 index) const { VP_UNUSED(index); return ""; }
            virtual const char *GetVector3fFilePathByIndex(u32 index) const { VP_UNUSED(index); return ""; }
            virtual const char *GetPointerFilePathByIndex(u32 index) const { VP_UNUSED(index); return ""; }

            virtual const char *GetStringDescriptionByIndex(u32 index) const { VP_UNUSED(index); return ""; }
            virtual const char *GetS32DescriptionByIndex(u32 index) const { VP_UNUSED(index); return ""; }
            virtual const char *GetFloatDescriptionByIndex(u32 index) const { VP_UNUSED(index); return ""; }
            virtual const char *GetBoolDescriptionByIndex(u32 index) const { VP_UNUSED(index); return ""; }
            virtual const char *GetVector3fDescriptionByIndex(u32 index) const { VP_UNUSED(index); return ""; }
            virtual const char *GetPointerDescriptionByIndex(u32 index) const { VP_UNUSED(index); return ""; }

            virtual u32 GetStringCount() const { return 0; }
            virtual u32 GetS32Count() const { return 0; }
            virtual u32 GetFloatCount() const { return 0; }
            virtual u32 GetBoolCount() const { return 0; }
            virtual u32 GetVector3fCount() const { return 0; }
            virtual u32 GetPointerCount() const { return 0; }

            virtual u32 GetStringIndexByKey(const char *key) const { VP_UNUSED(key); return 0; }
            virtual u32 GetS32IndexByKey(const char *key) const { VP_UNUSED(key); return 0; }
            virtual u32 GetFloatIndexByKey(const char *key) const { VP_UNUSED(key); return 0; }
            virtual u32 GetBoolIndexByKey(const char *key) const { VP_UNUSED(key); return 0; }
            virtual u32 GetVector3fIndexByKey(const char *key) const { VP_UNUSED(key); return 0; }
            virtual u32 GetPointerIndexByKey(const char *key) const { VP_UNUSED(key); return 0; }

            virtual const char       *GetStringDefaultByKey(const char *key) const { VP_UNUSED(key); return ""; }
            virtual s32               GetS32DefaultByKey(const char *key) const { VP_UNUSED(key); return {}; }
            virtual float             GetFloatDefaultByKey(const char *key) const { VP_UNUSED(key); return {}; }
            virtual bool              GetBoolDefaultByKey(const char *key) const { VP_UNUSED(key); return {}; }
            virtual vp::util::Float3  GetVector3fDefaultByKey(const char *key) const { VP_UNUSED(key); return {}; }
            virtual void             *GetPointerDefaultByKey(const char *key) const { VP_UNUSED(key); return nullptr; }
    };
}
