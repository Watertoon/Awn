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

namespace awn::para {

    AWN_PARA_DECLARE_TYPED_PARAM_IMPL(SpStepRateCell);

    bool SpStepRateManager::Initialize(mem::Heap *heap, const char *sp_step_rate_table_path_no_ext, const char *sp_event_enum_u32_class, const char *sp_target_enum_u32_class) {

        /* Get enum dictionaries */
        vp::util::EnumDictionaryManager *enum_dict_mgr = vp::util::GetEnumDictionaryManager();
        const vp::util::EnumDictionary<u32> *event_dict  = enum_dict_mgr->GetEnumDictionary<u32>(sp_event_enum_u32_class);
        if (event_dict == nullptr) { return false; }
        const vp::util::EnumDictionary<u32> *target_dict = enum_dict_mgr->GetEnumDictionary<u32>(sp_target_enum_u32_class);
        if (target_dict == nullptr) { return false; }

        /* Try load document */
        m_document_sp_step_rate_table.LoadDocument(sp_step_rate_table_path_no_ext, "awn__para__CombinationDataTableData", false);
        CombinationDataTableData *table_data = m_document_sp_step_rate_table.GetTypedParam<CombinationDataTableData>();
        const TypedParamMap<CombinationDataCellBase> *cell_list = table_data->GetCellList();
        if (table_data == nullptr) { return false; }

        /* Failure guard */
        auto release_document_guard = SCOPE_GUARD { 
            m_document_sp_step_rate_table.Release(); 
        };

        /* Ensure valid row count  */
        const u32 target_row_count = para::GetRowCount(table_data);
        if (target_row_count == 0) { return false; }

        /* Ensure valid column count */
        const u32 event_count = event_dict->GetEnumCount();
        const u32 event_col_count = para::GetColumnCount(table_data);
        if (event_col_count == 0) { return false; }

        /* Create event array */
        m_sp_rate_event_array.Initialize(heap, event_count);

        /* Failure guard */
        auto free_array_guard = SCOPE_GUARD { 
            m_sp_rate_event_array.Finalize(); 
        };

        /* Iterate targets */
        CellIterator target_iter(cell_list, CellArrangement::Row);
        while (target_iter.IsValid()) {
            ON_SCOPE_EXIT {
                ++target_iter;
            };

            /* Get target index */
            const char *target_key   = target_iter.GetRowKey();
            u32         target_index = 0;
            const bool  result0      = target_dict->GetEnumIndexByName(std::addressof(target_index), target_key);
            if (!result0 || cMaxSpStepRateTargetCount <= target_index) { continue; }

            /* Iterate events */
            CellIterator event_iter(cell_list, CellArrangement::Column);
            while (event_iter.IsValid()) {
                ON_SCOPE_EXIT {
                    ++event_iter;
                };

                /* Get event index and set event name */
                const char *event_key   = event_iter.GetColumnKey();
                u32         event_index = 0;
                const bool  result1     = event_dict->GetEnumIndexByName(std::addressof(event_index), event_key);
                if (!result1 || event_count <= event_index) { continue; }
                m_sp_rate_event_array[event_index].event_name = event_key;

                /* Set sp step rate target */
                const SpStepRateCell *sp_rate_cell = event_iter.GetTypedParam<SpStepRateCell>();
                const float sp_step_rate = vp::util::Min(1.0f, *sp_rate_cell->GetSpStepRate());
                m_sp_rate_event_array[event_index].sp_step_rate_target_array[target_index] = sp_step_rate;
            }
        }

        /* Succeed */
        release_document_guard.Cancel();
        free_array_guard.Cancel();
        return true;
    }

    void SpStepRateManager::Finalize() {
        m_document_sp_step_rate_table.Release();
        m_sp_rate_event_array.Finalize();
    }

    u32 SpStepRateManager::GetSpStepRateTargets(float *out_sp_step_rate_target_array, bool is_advance_tick) {

        /* Clear sp target rates */
        for (u32 i = 0; i < cMaxSpStepRateTargetCount; ++i) {
            out_sp_step_rate_target_array[i] = 1.0f;
        }

        /* Get target count */
        CombinationDataTableData *table_data = m_document_sp_step_rate_table.GetTypedParam<CombinationDataTableData>();
        const u32 target_count = para::GetRowCount(table_data);

        /* Loop events */
        for (SpStepRateEvent &event : m_sp_rate_event_array) {

            /* Check event active */
            u32 timeout = event.tick_timer;
            if (is_advance_tick && timeout != 0 && timeout != cInfiniteTimer) {
                timeout = vp::util::InterlockedFetchSubtract(std::addressof(event.tick_timer), 1u);
            }
            if (timeout < 1) { continue; }

            /* Set minimum step rates */
            for (u32 y = 0; y < target_count; ++y) {
                const float min_target_rate      = vp::util::Min(event.sp_step_rate_target_array[y], out_sp_step_rate_target_array[y]);
                out_sp_step_rate_target_array[y] = min_target_rate;
            }
        }

        return target_count;
    }
}
