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

namespace vp {

    typedef s64 TickSpan;

    class TimeSpan {
        public:
            static constexpr inline s64 cMaxTime = LLONG_MAX;
            static constexpr inline s64 cMinTime = LLONG_MIN;
        private:
            s64 m_time_ns;
        public:
            constexpr ALWAYS_INLINE TimeSpan() : m_time_ns(0) {/*...*/}
            constexpr ALWAYS_INLINE TimeSpan(s64 time_ns) : m_time_ns(time_ns) {/*...*/}
            constexpr ALWAYS_INLINE TimeSpan(const TimeSpan &rhs) : m_time_ns(rhs.m_time_ns) {/*...*/}
            constexpr ~TimeSpan() {/*...*/}

            constexpr TimeSpan &operator=(const TimeSpan &rhs) {
                m_time_ns = rhs.m_time_ns;
                return *this;
            }

            static TimeSpan FromTick(s64 tick) {

                /* Query frequency and limit */
                const s64 frequency            = util::GetSystemTickFrequency();
                const s64 max_tick_to_timespan = util::GetMaxTickToTimeSpan();

                /* Verify tick to timespan limit */
                if (max_tick_to_timespan < tick)  { return cMaxTime; }
                if (tick < -max_tick_to_timespan) { return cMinTime; }

                const s64 residual = ((tick % frequency) * 1'000'000'000) / frequency;
                const s64 quotient = (tick / frequency) * 1'000'000'000;
                const s64 time     = residual + quotient;

                return time;
            }
            static constexpr ALWAYS_INLINE TimeSpan FromNanoSeconds(s64 time_ns)  { return TimeSpan(time_ns); }
            static constexpr ALWAYS_INLINE TimeSpan FromMicroSeconds(s64 time_us) { return TimeSpan(time_us  * 1'000); }
            static constexpr ALWAYS_INLINE TimeSpan FromMilliSeconds(s64 time_ms) { return TimeSpan(time_ms  * 1'000'000); }
            static constexpr ALWAYS_INLINE TimeSpan FromSeconds(s64 time_sec)     { return TimeSpan(time_sec * 1'000'000'000); }

            static constexpr TimeSpan GetTimeLeftOnTarget(s64 tick) {
                if (tick == 0) { return 0; }

                const s64 tick_left = tick - util::GetSystemTick();

                if (tick_left <= 0) { return 0; }

                return FromTick(tick_left);
            }
            
            static TickSpan GetAbsoluteTimeToWakeup(TimeSpan timeout_ns) {

                /* Convert to tick */
                const s64 timeout_tick = timeout_ns.GetTick();

                /* "Infinite" time on zero */
                if (0 >= timeout_tick) { return TimeSpan::cMaxTime; }

                /* Calculate absolute timeout */
                const s64 absolute_time = timeout_tick + vp::util::GetSystemTick();

                if (absolute_time + 1 <= 1) { return TimeSpan::cMaxTime; }

                return absolute_time + 2;
            }

            s64 GetTick() const {

                if (m_time_ns == cMinTime) {
                    return cMinTime;
                }

                const s64 frequency = util::GetSystemTickFrequency();

                s64 r_coeff =  (m_time_ns % 1'000'000'000);
                if (-1 >= m_time_ns) {
                    r_coeff = -(m_time_ns % 1'000'000'000);
                }

                s64 p_coeff = (m_time_ns / 1'000'000'000);
                if (-1 >= m_time_ns) {
                    p_coeff = -(m_time_ns / 1'000'000'000);
                }

                const s64 residual = ((r_coeff * frequency) + 999'999'999) / 1'000'000'000;
                const s64 product  = p_coeff * frequency;
                
                s64 result = residual + product;
                if (-1 >= result) {
                    result = -result;
                }
                
                return result;
            }
            constexpr ALWAYS_INLINE s64 GetNanoSeconds()  const { return m_time_ns; }
            constexpr ALWAYS_INLINE s64 GetMicroSeconds() const { return (m_time_ns / 1'000); }
            constexpr ALWAYS_INLINE s64 GetMilliSeconds() const { return (m_time_ns / 1'000'000); }
            constexpr ALWAYS_INLINE s64 GetSeconds()      const { return (m_time_ns / 1'000'000'000); }
    };
}
