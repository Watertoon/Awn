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
#include <awn.hpp>

namespace awn::para::impl {

    void BlackboardStateBase::InitializeBase(mem::Heap *heap, const BlackboardStateInfo *state_info) {
        const u32 entry_count = state_info->entry_count;
        m_key_array.Initialize(heap, entry_count);
        m_flags_array.Initialize(heap, entry_count);
        m_value_set_counter_array.Initialize(heap, entry_count);
        m_index_map.Initialize(heap, entry_count * 2);
    }

    void BlackboardStateBase::FinalizeBase() {
        m_key_array.Finalize();
        m_flags_array.Finalize();
        m_value_set_counter_array.Finalize();
        m_index_map.Finalize();
    }
}
