#include <awn.hpp>

namespace awn::para {

    Result InitializeDocumentManager(mem::Heap *heap, const DocumentManagerInfo *info) {
        impl::DocumentManager *mgr = impl::DocumentManager::CreateInstance(heap);
        RESULT_RETURN_IF(mgr == nullptr, ResultNoMemory);
        RESULT_TRY(mgr->Initialize(heap, info));
        RESULT_RETURN_SUCCESS;
    }
    void FinalizeDocumentManager() {
        impl::DocumentManager *mgr = impl::DocumentManager::GetInstance();
        if (mgr != nullptr) {
            mgr->Finalize();
            impl::DocumentManager::DeleteInstance();
        }
    }
    void CalculateDocumentManager() {
        impl::DocumentManager *mgr = impl::DocumentManager::GetInstance();
        if (mgr != nullptr) {
            mgr->Calculate();
        }
    }
}
