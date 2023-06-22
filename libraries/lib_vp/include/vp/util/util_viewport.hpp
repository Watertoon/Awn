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

namespace vp::util {

    template <typename T>
    struct BoundingBox2 {
        T min_x;
        T min_y;
        T max_x;
        T max_y;
    };

    class Viewport {
        private:
            BoundingBox2<float> m_bounding_box;
        public:
            constexpr Viewport() {/*...*/}
            constexpr Viewport(float min_x, float min_y, float max_x, float max_y) : m_bounding_box{min_x, min_y, max_x, max_y } {/*...*/}
            constexpr Viewport(const BoundingBox2<float>& bound_box) : m_bounding_box(bound_box) {/*...*/}

            constexpr Viewport(const LogicalFrameBuffer *framebuffer) : m_bounding_box() {
                float width  = 0.0f;
                float height = 0.0f;
                framebuffer->GetVirtualCanvasSize(std::addressof(width), std::addressof(height));

                m_bounding_box.min_x = Min(width, 0.0f);
                m_bounding_box.min_y = Min(height, 0.0f);
                m_bounding_box.max_x = Max(width, 0.0f);
                m_bounding_box.max_y = Max(height, 0.0f);
            }

            constexpr ~Viewport() {/*...*/}
    };
}
