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

    void ByamlNodeBigData::PushKeys(ByamlStringPoolBuilder *pool_builder) {
        this->ByamlNodeBase::PushKeys(pool_builder);
        m_big_data_node->PushKeys(pool_builder);
    }
    void ByamlNodeBigData::PushStrings(ByamlStringPoolBuilder *pool_builder) { m_big_data_node->PushStrings(pool_builder); }

    void ByamlNodeBigData::Serialize(void *data_offset, vp::res::ResByaml *head) {
        const u32 offset = m_big_data_node->GetOffset();
        *reinterpret_cast<u32*>(data_offset) = (!head->IsReverseEndian()) ? offset : vp::util::SwapEndian(offset);
    }

    void ByamlNodeBigData::SerializeForBigData(uintptr_t *big_data_offset, uintptr_t *container_offset, vp::res::ResByaml *head) {
        if (m_big_data_node->m_cache.is_serialize) { return; }
        m_big_data_node->Serialize(big_data_offset, container_offset, head);
        m_big_data_node->m_cache.is_serialize = true;
    }

    void ByamlNodeBigData::CalculateBigDataOffsets(uintptr_t *big_data_offset_iter, size_t head) {
        if (m_big_data_node->m_cache.big_data_offset != ByamlBigDataNodeBase::cInvalidOffset) { return; }
        m_big_data_node->CalculateBigDataOffset(big_data_offset_iter, head);
    }
    void ByamlNodeBigData::CalculateContainerOffsets(uintptr_t *container_offset_iter, size_t head) {
        if (m_big_data_node->m_cache.container_offset != ByamlBigDataNodeBase::cInvalidOffset) { return; }
        m_big_data_node->CalculateContainerOffset(container_offset_iter, head);
    }
    void ByamlNodeBigData::ClearCache() {
        m_big_data_node->ClearCache();
    }
}
