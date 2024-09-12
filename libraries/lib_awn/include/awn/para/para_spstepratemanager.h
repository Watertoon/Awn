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

    struct SpStepRateCell_Layout {
        float SpStepRate = 1.0f;

        AWN_PARA_LAYOUT_TYPE_TRAITS(SpStepRateCell_Layout);
    };

    AWN_PARA_DECLARE_TYPED_PARAM_AUTOV(
        awn__para__SpStepRateCell, SpStepRateCell,
        CombinationDataCellBase,   SpStepRateCell_Layout,
        SpStepRate
    );

    static constexpr u32 cMaxSpStepRateTargetCount = 0x8;

    class SpStepRateManager {
        public:
            struct SpStepRateEvent {
                const char *event_name;
                u32         tick_timer;
                float       max_sp_rate = 1.0f;
                float       sp_step_rate_target_array[cMaxSpStepRateTargetCount];

                constexpr SpStepRateEvent() : event_name(""), tick_timer(), sp_step_rate_target_array{} {
                    for (u32 i = 0; i < cMaxSpStepRateTargetCount; ++i) {
                        sp_step_rate_target_array[i] = 1.0f;
                    }
                }
                constexpr ~SpStepRateEvent() {/*...*/}
            };
        public:
            using SpStepRateEventArray = vp::util::HeapArray<SpStepRateEvent>;
        public:
            static constexpr u32 cInfiniteTimer = 0xffff'ffff;
        private:
            DocumentBinder       m_document_sp_step_rate_table;
            SpStepRateEventArray m_sp_rate_event_array;
        public:
            AWN_SINGLETON_TRAITS(SpStepRateManager);
        public:
            constexpr SpStepRateManager() : m_document_sp_step_rate_table(), m_sp_rate_event_array() {/*...*/}
            constexpr ~SpStepRateManager() {/*...*/}

            bool Initialize(mem::Heap *heap, const char *sp_step_rate_table_path_no_ext, const char *sp_event_enum_class, const char *sp_target_enum_class);
            void Finalize();

            u32 GetSpStepRateTargets(float *out_sp_step_rate_target_array, bool is_advance_tick);

            template <typename T>
                requires (!vp::util::TEnumTraits<T>::cIsDummy)
            constexpr void ClearSpStepRateEvent(T event_enum) {
                const u32 event_index = vp::util::TEnumTraits<T>::Traits::GetIndexByEnumValue(event_enum);
                if (m_sp_rate_event_array.GetCount() <= event_index) { return; }
                vp::util::InterlockedExchange(std::addressof(m_sp_rate_event_array[event_index].tick_timer), 0u);
            }

            template <typename T>
                requires (!vp::util::TEnumTraits<T>::cIsDummy)
            constexpr void SetSpStepRateEvent(T event_enum) {
                const u32 event_index = vp::util::TEnumTraits<T>::Traits::GetIndexByEnumValue(event_enum);
                if (m_sp_rate_event_array.GetCount() <= event_index) { return; }
                vp::util::InterlockedExchange(std::addressof(m_sp_rate_event_array[event_index].tick_timer), cInfiniteTimer);
            }

            template <typename T>
                requires (!vp::util::TEnumTraits<T>::cIsDummy)
            constexpr void SetTimedSpStepRateEvent(T event_enum, u32 tick_count) {

                /* Increase event timer to tick count */
                const u32 event_index = vp::util::TEnumTraits<T>::Traits::GetIndexByEnumValue(event_enum);
                if (m_sp_rate_event_array.GetCount() <= event_index) { return; }
                u32 *event_timer = std::addressof(m_sp_rate_event_array[event_index].tick_timer);
                u32  last_tick   = *event_timer;
                while (last_tick < tick_count) {
                    vp::util::InterlockedCompareExchange(std::addressof(last_tick), event_timer, last_tick, tick_count);
                }

                return;
            }
    };
}
