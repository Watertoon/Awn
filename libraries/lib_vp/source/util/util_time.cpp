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

    namespace {
        s64 sLastFrameTime = 0;
        s64 sDeltaTick = 0;
        double sDeltaTime = 0;
    }

    void BeginFrame() {
        const s64 last_last_frame = sLastFrameTime;
        sLastFrameTime = GetSystemTick();
        sDeltaTick = sLastFrameTime - last_last_frame;
        sDeltaTime = static_cast<double>(sDeltaTick) / static_cast<double>(GetSystemTickFrequency());
    }

    s64 GetMillisecondsFromTick(s64 tick) {
        return ((tick % GetSystemTickFrequency()) * vp::util::TPow<s64, 10, 3> / GetSystemTickFrequency()) + ((tick / GetSystemTickFrequency()) * vp::util::TPow<s64, 10, 3>);
    }
    static_assert(vp::util::TPow<s64, 10, 3> == 1000);

    double GetDeltaTime() {
        return sDeltaTime;
    }
    
    double CalcFps() {
        return static_cast<double>(GetSystemTickFrequency()) / static_cast<double>(sDeltaTick);
    }
}
