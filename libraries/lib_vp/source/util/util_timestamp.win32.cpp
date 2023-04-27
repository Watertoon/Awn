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

namespace vp::util {

    namespace x86 {

        NO_INLINE s64 rdtscp(u32 *frequency) {
            s64 low = 0;
            s64 high = 0;
            s32 freq = 0;
            asm volatile ("rdtscp\n" : "=a" (low), "=d" (high), "=c" (freq) : : );
            *frequency = freq;
            return (high << 32) + low;
        }

        NO_INLINE s64 rdtsc() {
            return __builtin_ia32_rdtsc();
        }
    }

    s64 sSystemFrequency   = 0;
    s64 sMaxTickToTimeSpan = TimeSpan::MaxTime;

    void InitializeTimeStamp() {
        const bool result = ::QueryPerformanceFrequency(reinterpret_cast<LARGE_INTEGER*>(std::addressof(sSystemFrequency)));
        VP_ASSERT(result == true);
        sMaxTickToTimeSpan = ((TimeSpan::MaxTime - (TimeSpan::MaxTime % sSystemFrequency)) / 1'000'000'000) * sSystemFrequency;
    }

    s64 GetSystemTick() {
        s64 time = 0;
        const bool result = ::QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(std::addressof(time)));
        VP_ASSERT(result == true);
        return time;
    }

    s32 GetSystemTickFrequency() {
        return sSystemFrequency;
    }
    
    s64 GetMaxTickToTimeSpan() {
        return sMaxTickToTimeSpan;
    }
}
