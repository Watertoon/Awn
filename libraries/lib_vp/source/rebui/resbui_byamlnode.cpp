#include <vp.hpp>

namespace vp::resbui {

    void ByamlNodeBigData::PushKeys(ByamlStringPoolBuilder *pool_builder) {
        this->ByamlNodeBase::PushKeys(pool_builder);
        m_big_data_node->PushKeys(pool_builder);
    }
    void ByamlNodeBigData::PushStrings(ByamlStringPoolBuilder *pool_builder) { m_big_data_node->PushStrings(pool_builder); }

    void ByamlNodeBigData::Serialize(void *data_offset, [[maybe_unused]] vp::res::ResByaml *head) {
        /* Write offset to big data */
        const u32 offset = m_big_data_node->GetOffset();
        *reinterpret_cast<u32*>(data_offset) = offset;
    }

    void ByamlNodeBigData::SerializeForBigData(uintptr_t *big_data_offset, uintptr_t *container_offset, vp::res::ResByaml *head) {
        if (m_big_data_node->GetOffset() != ByamlBigDataNodeBase::cInvalidOffset) { return; }
        m_big_data_node->Serialize(big_data_offset, container_offset, head);
    }

    void ByamlNodeBigData::CalculateBigDataEndOffsets(uintptr_t *big_data_offset_iter, uintptr_t *container_offset_iter, size_t head) {
        m_big_data_node->CalculateEndOffset(big_data_offset_iter, container_offset_iter, head);
    }
}
