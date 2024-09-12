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

	AWN_PARA_DECLARE_TYPED_PARAM_IMPL(CombinationDataCellBase);
	AWN_PARA_DECLARE_TYPED_PARAM_IMPL(CombinationDataTableData);

    CellIterator::CellIterator(const TypedParamMap<CombinationDataCellBase> *map, CellArrangement arrangement) : m_iter(map->begin()), m_end_iter(map->end()), m_key(), m_is_col_iter(static_cast<bool>(arrangement)) {
        if (m_iter == m_end_iter) { return; }
        m_key = this->GetArrangementKey();
    }

    CellIterator::CellIterator(const CellIterator &iter, CellArrangement arrangement) : m_iter(iter.m_iter), m_end_iter(iter.m_end_iter), m_key(), m_is_col_iter(static_cast<bool>(arrangement)) {
        if (m_iter == m_end_iter) { return; }
        m_key = this->GetArrangementKey();
    }

    const char *CellIterator::GetArrangementKey() const {
        return (m_is_col_iter) ? this->GetRowKey() : this->GetColumnKey();
    }

    const char *CellIterator::GetRowKey() const {
        return (m_is_col_iter) ? m_key : *reinterpret_cast<CombinationDataCellBase*>((*m_iter).typed_param)->GetRowKey();
    }

    const char *CellIterator::GetColumnKey() const {
        return (!m_is_col_iter) ? m_key : *reinterpret_cast<CombinationDataCellBase*>((*m_iter).typed_param)->GetColumnKey();
    }

    void CellIterator::SetArrangement(CellArrangement arrangment) {

        /* No change */
        if (static_cast<bool>(arrangment) == m_is_col_iter) { return; }

        /* Ensure valid iter */
        if (m_iter == m_end_iter) { return; }

        /* Swap arrangment */
        m_is_col_iter = static_cast<bool>(arrangment);
        m_key         = this->GetArrangementKey();

        return;
    }

    const TypedParamMapNode *CellIterator::operator*() {
         return std::addressof(*m_iter);
    }

    CellIterator &CellIterator::operator++() {

        /* Get next node */
        ++m_iter;

        /* Ensure next row or column element */
        while (m_iter != m_end_iter) {

            /* Succeed if we are the same key */
            CombinationDataCellBase *next = reinterpret_cast<CombinationDataCellBase*>((*m_iter).typed_param);
            const char              *key = (m_is_col_iter) ? *next->GetRowKey() : *next->GetColumnKey();
            if (vp::util::TStringCompare(key, m_key, 0x400'0000)) { break; }

            ++m_iter;
        }

        return *this;
    }

    u32 GetRowCount(CombinationDataTableData *data_table) {

        RowIterator row_iter(data_table);
        u32 i = 0;
        for ([[maybe_unused]] const TypedParamMapNode *node : row_iter) {
            ++i;
        }

        return i;
    }

    u32 GetColumnCount(CombinationDataTableData *data_table) {

        ColumnIterator col_iter(data_table);
        u32 i = 0;
        for ([[maybe_unused]] const TypedParamMapNode *node : col_iter) {
            ++i;
        }

        return i;
    }
}
