#pragma once

namespace awn::para {

    struct DocumentManagerInfo {
        u32                       typed_param_parent_count;
        u32                       remove_queue_count;
        IDocumentApplicationImpl *app_impl;

        constexpr void SetDefaults() {
            this->typed_param_parent_count = 0x20;
            this->remove_queue_count = 0x80;
            this->app_impl = nullptr;
        }
        constexpr void SetApplicationImpl(IDocumentApplicationImpl *app_impl) {
            this->app_impl = app_impl;
        }
        constexpr void SetMaxTypedParamParentCount(u32 count) {
            this->typed_param_parent_count = count;
        }
        constexpr void SetMaxRemoveQueueCount(u32 count) {
            this->remove_queue_count = count;
        }
    };

    Result InitializeDocumentManager(mem::Heap *heap, const DocumentManagerInfo *info);
    void FinalizeDocumentManager();
    void CalculateDocumentManager();
}
