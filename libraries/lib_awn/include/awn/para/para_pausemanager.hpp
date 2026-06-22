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

    struct PauseCell_Layout {
        bool IsPause = false;

        AWN_PARA_TYPED_PARAM_LAYOUT_TRAITS(PauseCell_Layout);
    };

    AWN_PARA_DECLARE_TYPED_PARAM(
        awn__para__PauseCell,     PauseCell,
        CombinationDataCellBase,  PauseCell_Layout
    );

    class PauseManager {
        public:
            static constexpr u32 cMaxPauseTargetCount = 0x20;
            static constexpr const char *cDocumentName = "System/CombinationDataTableData/Default";
        public:
            using PauseTargetMask = vp::util::FixedConnection<u32, cMaxPauseTargetCount>;
        public:
            struct PauseKey {
                u32              reference_count;
                u32              hash;
                const char      *key;
                PauseTargetMask  pause_target_mask;

                constexpr void AcquireReference() {
                    vp::util::InterlockedIncrement(std::addressof(reference_count));
                }
                constexpr void ReleaseReference() {
                    vp::util::InterlockedDecrement(std::addressof(reference_count));
                }
            };
        public:
            using PauseKeyArray = vp::util::HeapArray<PauseKey>;
            using PauseTargetIndexMap = vp::util::KeyIndexMap<u32, u32>;
        private:
            DocumentBinder      m_document_pause_config;
            PauseKeyArray       m_pause_key_array;
            PauseTargetIndexMap m_pause_target_index_map;
            PauseTargetMask     m_pause_mask;
        public:
            constexpr PauseManager() : m_document_pause_config(), m_pause_key_array(), m_pause_target_index_map(), m_pause_mask() {/*...*/}
            ~PauseManager() {/*...*/}

            bool Initialize(mem::Heap *heap, const char *pause_config_file_name) {

                /* Try load document */
                m_document_pause_config.LoadDocument(pause_config_file_name, CombinationDataTableData::cClassName, false);
                CombinationDataTableData *data_table = m_document_pause_config.GetTypedParam<CombinationDataTableData>();
                if (data_table == nullptr) { return false; }
                auto document_fail_guard = SCOPE_GUARD {
                    m_document_pause_config.Release();
                };

                /* Ensure valid pause key count */
                const u32 key_count = para::GetColumnCount(data_table);
                if (key_count < 1) { return false; }
                const Result result0 = m_pause_key_array.Initialize(heap, key_count);
                if (RESULT_FAILED(result0)) { return false; }
                auto key_array_fail_guard = SCOPE_GUARD {
                    m_pause_key_array.Finalize();
                };

                /* Ensure valid pause target count  */
                const u32 target_count = para::GetRowCount(data_table);
                if (target_count < 1) { return false; }

                /* Build pause target map */
                const TypedParamMap<CombinationDataCellBase> *cell_list = data_table->GetComposite<"CellList">();
                u32 target_index = 0;
                CellIterator target_iter(cell_list, CellArrangement::Row);
                while (target_iter.IsValid()) {
                    ON_SCOPE_EXIT {
                        ++target_iter;
                        ++target_index;
                    };
                    if (cMaxPauseTargetCount <= target_index) { continue; }
                    const char *pause_target_name = target_iter.GetColumnKey();
                    const u32   target_hash       = vp::util::HashMurmur3(pause_target_name);
                    m_pause_target_index_map.AddEntry(target_hash, target_index);
                }

                /* Build pause keys */
                CellIterator key_iter(cell_list, CellArrangement::Column);
                u32 key_index = 0;
                while (key_iter.IsValid()) {
                    ON_SCOPE_EXIT {
                        ++key_iter;
                        ++key_index;
                    };

                    /* Set key */
                    PauseKey *pause_key = std::addressof(m_pause_key_array[key_index]);
                    const char *key_name = key_iter.GetColumnKey();
                    pause_key->key = key_name;
                    pause_key->hash = vp::util::HashMurmur3(key_name);
                    pause_key->reference_count = 0;

                    /* Build key target mask */
                    CellIterator key_target_iter(key_iter, CellArrangement::Row);
                    u32 key_target_index = 0;
                    while (key_target_iter.IsValid()) {
                        ON_SCOPE_EXIT {
                            ++key_target_iter;
                            ++key_target_index;
                        };
                        if (cMaxPauseTargetCount <= key_target_index) { continue; }
                        PauseCell *pause_cell = key_target_iter.GetTypedParam<PauseCell>();
                        const bool is_target_paused = pause_cell->GetProperty<"IsPause">();
                        if (!is_target_paused) { continue; }
                        pause_key->pause_target_mask.SetBit(target_index);
                    }
                }
                m_pause_key_array.Sort(PauseKeyArray::DefaultCompareLessMember<&PauseKey::hash>());

                document_fail_guard.Cancel();
                key_array_fail_guard.Cancel();
                return true;
            }

            void Finalize() {
                m_document_pause_config.Release();
                m_pause_target_index_map.Finalize();
            }

            constexpr void Calculate() {

                /* Clear pause mask */
                m_pause_mask.Clear();

                /* Build pause mask */
                for (const PauseKey &pause_key : m_pause_key_array) {                    
                    if (pause_key.reference_count < 1) { continue; }
                    for (const u32 index : pause_key.pause_target_mask) {
                        m_pause_mask.SetBit(index);
                    }
                }

                return;
            }

            constexpr ALWAYS_INLINE bool IsPaused(const char *pause_target_name) {
                const u32 hash  = vp::util::HashMurmur3(pause_target_name);
                return this->IsPaused(hash);
            }

            constexpr bool IsPaused(u32 pause_target_hash) {
                const u32 index = m_pause_target_index_map.TryGetIndexByKey(pause_target_hash);
                if (index == PauseTargetIndexMap::cInvalidEntryIndex) { return false; }
                return m_pause_mask.GetBit(index);
            }

            constexpr void RequestPause(u32 pause_key_hash) {
                PauseKeyArray::Iterator key_iter = m_pause_key_array.BinarySearch(pause_key_hash, PauseKeyArray::DefaultCompareLessSearchMember<&PauseKey::hash>());
                if (key_iter == m_pause_key_array.end()) { return; }
                (*key_iter).AcquireReference();
            }

            constexpr void ReleasePause(u32 pause_key_hash) {
                PauseKeyArray::Iterator key_iter = m_pause_key_array.BinarySearch(pause_key_hash, PauseKeyArray::DefaultCompareLessSearchMember<&PauseKey::hash>());
                if (key_iter == m_pause_key_array.end()) { return; }
                (*key_iter).ReleaseReference();
            }
	};
}
