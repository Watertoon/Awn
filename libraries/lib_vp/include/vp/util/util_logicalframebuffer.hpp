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

    class LogicalFrameBuffer {
        protected:
            Vector2f m_virtual_size;
            Vector2f m_dimensions;
        protected:
            virtual void BindImpl() {/*...*/}
        public:
            constexpr LogicalFrameBuffer() : m_virtual_size(), m_dimensions() {/*...*/}

            void Bind() {
                this->BindImpl();
            }

            void SetVirtualCanvasSize(float x, float y) {
                Vector2f temp = {x, y};
                m_virtual_size = temp;
            }

            void SetDimensions(float x, float y) {
                Vector2f temp = {x, y};
                m_dimensions = temp;
            }

            constexpr void GetVirtualCanvasSize(float *out_x, float *out_y) const {
                *out_x = m_virtual_size[0];
                *out_y = m_virtual_size[1];
            }

            constexpr void GetDimensions(float *out_x, float *out_y) const {
                *out_x = m_dimensions[0];
                *out_y = m_dimensions[1];
            }
    };
}
