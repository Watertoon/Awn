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

    static constexpr inline u32 cDefaultFramesPerSecond = 60;

    class VariableFramerateManager;

    VariableFramerateManager *GetVariableFramerateManager();
    void SetVariableFramerateManager(VariableFramerateManager *vfr_mgr);

    u32 GetTargetFramesPerSecond();
    void SetTargetFramesPerSecond(u32 target_fps);

    struct TimeStep {
        float step_rate;
        float delta_time;

        void SetStepRate(float step_rate) {
            this->step_rate  = step_rate;
            const float fps = static_cast<float>(para::GetTargetFramesPerSecond());
            this->delta_time = step_rate / fps;
        }
    };

    TimeStep GetBaseTimeStep();
    TimeStep GetFrameTimeStep();
    TimeStep GetTotalTimeStep();

    struct StepCounter {
        float step      = 0.0f;
        float last_step = 0.0f;
        float step_rate = 1.0f;

        void Tick() {
            const float total_step = para::GetTotalTimeStep().step_rate;
            const float prev = this->step; 
            this->step = prev + total_step * step_rate;
            this->last_step = prev;
        }
        constexpr bool IsTimeout(float step_target) {
            return step_target <= this->step;
        }
    };

	class VariableFramerateManager {
        public:
            static constexpr u32 cDefaultStepRateTarget = 0;
            static constexpr u32 cCounterRingCount = 3;
            static constexpr float cMinStepRate = 0.01f;
		public:
            using SpRateIndexArray = vp::util::FixedAtomicBitSizedArray<u32, 4, sys::cMaxCoreCount>;
        public:
            struct Counter {
                using SpRateIndexRing = vp::util::FixedRingBuffer<u32, cCounterRingCount>;
                TimeStep total_time_step;
                SpRateIndexRing sp_rate_index_ring;
            };
            struct SpTarget {
                float sp_step_rate = 1.0f;
                float update_sp_step_rate = vp::util::cFloatNan;
            };
        private:
			TimeStep           m_base_time_step;
			TimeStep           m_frame_time_step;
            SpStepRateManager *m_sp_step_rate_manager;
            Counter            m_counter_array[sys::cMaxCoreCount];
            SpTarget           m_sp_target_array[cMaxSpStepRateTargetCount];
            float              m_step_scale;
            SpRateIndexArray   m_counter_sp_index_array;
        public:
            VariableFramerateManager() : m_base_time_step(), m_frame_time_step(), m_sp_step_rate_manager(), m_counter_array(), m_sp_target_array(), m_step_scale(1.0f), m_counter_sp_index_array() {
                m_base_time_step.SetStepRate(1.0f);
                m_frame_time_step.SetStepRate(1.0f);
                for (u32 i = 0; i < sys::cMaxCoreCount; ++i) {
                    m_counter_array[i].total_time_step.SetStepRate(1.0f);
                }
            }
            constexpr ~VariableFramerateManager() {/*...*/}

            void Calculate(u32 present_interval);

            float AcquireSpStepRate(u32 sp_step_rate_index);
            void ReleaseSpStepRate();

            void SetSpStepRateTarget(u32 sp_target_index, float sp_step_rate);

            constexpr TimeStep GetBaseTimeStep() const { return m_base_time_step; }
            constexpr TimeStep GetFrameTimeStep() const { return m_frame_time_step; }
            TimeStep GetTotalTimeStep() const {
                const u32 core_number = sys::GetCurrentCoreNumber();
                return m_counter_array[core_number].total_time_step;
            }
	};
}
