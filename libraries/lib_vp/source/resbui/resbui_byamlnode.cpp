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
