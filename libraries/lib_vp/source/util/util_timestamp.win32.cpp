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

    NO_INLINE s64 rdtsc() {
        s64 low = 0;
        s64 high = 0;
        asm volatile ("rdtsc\n" : "=a" (low), "=d" (high) : : );
        vp::util::MemoryBarrierLoad();
        const s64 ret = (high << 32) | low;
        return ret;
    }

    struct CpuFeatures {
        u32 eax;
        u32 ebx;
        u32 ecx;
        u32 edx;
    };

    void GetCpuFeatures(CpuFeatures *features, u32 main_info_type, u32 sub_info_type) {
        __asm__ volatile(
            "cpuid"
            : "=a"(features->eax), "=b"(features->ebx),"=c"(features->ecx),"=d"(features->edx)
            : "a"(main_info_type), "c"(sub_info_type)
        );
        return;
    }

    s64 sSystemFrequency   = 0;
    s64 sMaxTickToTimeSpan = TimeSpan::cMaxTime;

    void InitializeTimeStamp() {
        LARGE_INTEGER freq = {};
        const bool result = ::QueryPerformanceFrequency(std::addressof(freq));
        VP_ASSERT(result);
        sSystemFrequency = static_cast<s64>(freq.QuadPart);
        sMaxTickToTimeSpan = ((TimeSpan::cMaxTime - (TimeSpan::cMaxTime % sSystemFrequency)) / 1'000'000'000) * sSystemFrequency;
    }

    s64 GetSystemTick() {
        LARGE_INTEGER count = {};
        const bool result = ::QueryPerformanceCounter(std::addressof(count));
        VP_ASSERT(result);
        return count.QuadPart;
    }

    s64 GetSystemTickFrequency() {
        return sSystemFrequency;
    }
    
    s64 GetMaxTickToTimeSpan() {
        return sMaxTickToTimeSpan;
    }
}
