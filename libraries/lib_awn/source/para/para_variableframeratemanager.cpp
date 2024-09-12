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

    u32 sTargetFramesPerSecond = cDefaultFramesPerSecond;

    void SetTargetFramesPerSecond(u32 target_fps) {
        sTargetFramesPerSecond = target_fps; 
    }

    u32 GetTargetFramesPerSecond() {
        return sTargetFramesPerSecond ;
    }

    VariableFramerateManager *sGlobalVfrManager = nullptr;

    VariableFramerateManager *GetGlobalVariableFramerateManager() {
        return sGlobalVfrManager;
    }

    void SetGlobalVariableFramerateManager(VariableFramerateManager *vfr_mgr) {
        sGlobalVfrManager = vfr_mgr;
    }

    TimeStep GetBaseTimeStep() {
        VariableFramerateManager *vfr_mgr = GetGlobalVariableFramerateManager();
        if (vfr_mgr != nullptr) { return vfr_mgr->GetBaseTimeStep(); }
        TimeStep time_step;
        time_step.SetStepRate(1.0f);
        return time_step;
    }

    TimeStep GetFrameTimeStep() {
        VariableFramerateManager *vfr_mgr = GetGlobalVariableFramerateManager();
        if (vfr_mgr != nullptr) { return vfr_mgr->GetFrameTimeStep(); }
        TimeStep time_step;
        time_step.SetStepRate(1.0f);
        return time_step;
    }

    TimeStep GetTotalTimeStep() {
        VariableFramerateManager *vfr_mgr = GetGlobalVariableFramerateManager();
        if (vfr_mgr != nullptr) { return vfr_mgr->GetTotalTimeStep(); }
        TimeStep time_step;
        time_step.SetStepRate(1.0f);
        return time_step;
    }

    void VariableFramerateManager::Calculate(u32 present_interval) {

        /* Set step rate for current frame */
        m_frame_time_step.SetStepRate(static_cast<float>(present_interval));

        /* Get target sp step rates */
        if (m_sp_step_rate_manager != nullptr) {
            float sp_target_array[cMaxSpStepRateTargetCount];
            const u32 sp_count = m_sp_step_rate_manager->GetSpStepRateTargets(sp_target_array, true);
            for (u32 i = 0; i < sp_count; ++i) {
                this->SetSpStepRateTarget(i, sp_target_array[i]);
            }
        }

        /* Update sp step rates */
        for (u32 i = 0; i < cMaxSpStepRateTargetCount; ++i) {

            /* Skip update if no sp rate is set */
            const float update_sp_rate = m_sp_target_array[i].update_sp_step_rate;
            if (vp::util::IsNan(update_sp_rate)) { continue; }

            /* Clear sp step rate if it ends up 1 */
            const float sp_rate = m_sp_target_array[i].sp_step_rate;
            float clear_rate = update_sp_rate;
            auto clear_guard = SCOPE_GUARD {
                if (clear_rate == 1.0f) { m_sp_target_array[i].update_sp_step_rate = vp::util::cFloatNan; }
            };

            /* Lower sp rate to new rate, or lerp sp rate to the target */
            if (update_sp_rate < sp_rate) {
                m_sp_target_array[i].sp_step_rate = update_sp_rate;
            } else if (update_sp_rate > sp_rate) {
                const float scale_rate = m_frame_time_step.step_rate * sp_rate;
                const float adj_rate = vp::util::Max(cMinStepRate, scale_rate);
                const float frame_sp_rate = adj_rate + sp_rate;
                const bool  is_lerp_in_bounds = (sp_rate <= frame_sp_rate) && (frame_sp_rate < update_sp_rate);
                const float next_sp_rate = (is_lerp_in_bounds) ? frame_sp_rate : update_sp_rate;
                clear_rate = next_sp_rate;
                m_sp_target_array[i].sp_step_rate = next_sp_rate;
                if (is_lerp_in_bounds) { clear_guard.Cancel(); }
            }
        }

        /* Update base timestep and each total timestep */
        const float step_scale = m_step_scale;
        const float base_step = m_frame_time_step.step_rate * m_sp_target_array[cDefaultStepRateTarget].sp_step_rate * step_scale;
        m_base_time_step.SetStepRate(base_step);
        const u32 core_count = sys::GetProcessCoreCount();
        for (u32 i = 0; i < core_count; ++i) {
            const u32 sp_index = m_counter_sp_index_array.GetElement(i);
            const float total_step = m_frame_time_step.step_rate * m_sp_target_array[sp_index].sp_step_rate * step_scale;
            m_counter_array[i].total_time_step.SetStepRate(total_step);
        }

        return;
    }

    float VariableFramerateManager::AcquireSpStepRate(u32 sp_step_rate_index) {

        VP_ASSERT(sp_step_rate_index < cMaxSpStepRateTargetCount);
        const u32 core_number = sys::GetCurrentCoreNumber();
        Counter *counter = std::addressof(m_counter_array[core_number]);

        /* Fail if ring is full */
        if (counter->sp_rate_index_ring.IsFull()) {
            const u32 last_index = counter->sp_rate_index_ring.PeekBack();
            return m_sp_target_array[last_index].sp_step_rate / m_sp_target_array[cDefaultStepRateTarget].sp_step_rate;
        }

        /* Change total step rate */
        counter->sp_rate_index_ring.PushBack(sp_step_rate_index);
        const float sp_rate = m_sp_target_array[sp_step_rate_index].sp_step_rate;
        const float total_step = m_frame_time_step.step_rate * sp_rate * m_step_scale;
        counter->total_time_step.SetStepRate(total_step);
        m_counter_sp_index_array.SetElement(core_number, sp_step_rate_index);
        return sp_rate / m_sp_target_array[cDefaultStepRateTarget].sp_step_rate;
    }

    void VariableFramerateManager::ReleaseSpStepRate() {

        /* Drop top of stack */
        const u32 core_number = sys::GetCurrentCoreNumber();
        Counter *counter = std::addressof(m_counter_array[core_number]);
        if (!counter->sp_rate_index_ring.DropBack()) { return; }

        /* Swap to default sp rate if ring is emptied */
        if (counter->sp_rate_index_ring.IsEmpty()) {
            const u32 sp_index = cDefaultStepRateTarget;
            const float sp_rate = m_sp_target_array[sp_index].sp_step_rate;
            const float total_step = m_frame_time_step.step_rate * sp_rate * m_step_scale;
            counter->total_time_step.SetStepRate(total_step);
            m_counter_sp_index_array.SetElement(core_number, sp_index);
            return;
        }

        /* Use previous sp rate */
        const u32 sp_index = counter->sp_rate_index_ring.PeekBack();
        const float sp_rate = m_sp_target_array[sp_index].sp_step_rate;
        const float total_step = m_frame_time_step.step_rate * sp_rate * m_step_scale;
        counter->total_time_step.SetStepRate(total_step);
        m_counter_sp_index_array.SetElement(core_number, sp_index);

        return;
    }

    void VariableFramerateManager::SetSpStepRateTarget(u32 sp_target_index, float sp_step_rate) {
        VP_ASSERT(0.0f < sp_step_rate && sp_target_index < cMaxSpStepRateTargetCount);
        m_sp_target_array[sp_target_index].update_sp_step_rate = sp_step_rate;
    }
}
