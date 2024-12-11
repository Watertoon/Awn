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

    class Projection {
        protected:
            MatrixRowMajor4x4f m_projection_matrix;
            bool m_is_require_update;
        public:
            VP_RTTI_BASE(Projection);
        public:
            constexpr Projection() : m_is_require_update(true) {/*...*/}
            constexpr virtual ~Projection() {/*...*/}

            constexpr void UpdateMatrix() {
                if (m_is_require_update) {        
                    this->CalculateProjectionMatrix(std::addressof(m_projection_matrix));
                }
                m_is_require_update = false;
            }

            constexpr MatrixRowMajor4x4f *GetProjectionMatrix() {
                return std::addressof(m_projection_matrix);
            }

            constexpr const MatrixRowMajor4x4f *GetProjectionMatrix() const {
                return std::addressof(m_projection_matrix);
            }

            constexpr virtual void CalculateProjectionMatrix(MatrixRowMajor4x4f *out_proj_matrix) const = 0;
    };
    
    class FrustumProjection : public Projection {
        public:
            friend class PerspectiveProjection;
        protected:
            float m_near;
            float m_far;
            float m_top;
            float m_bottom;
            float m_left;
            float m_right;
        public:
            VP_RTTI_DERIVED(FrustumProjection, Projection);
        public:
            constexpr FrustumProjection() {/*...*/}
            constexpr FrustumProjection(float near, float far, const BoundingBox2f& bound_box) : Projection(), m_near(near), m_far(far), m_top(bound_box.max_y), m_bottom(bound_box.min_y), m_left(bound_box.max_x), m_right(bound_box.min_x) {/*...*/}
            constexpr FrustumProjection(float near, float far, float top, float bottom, float left, float right) : Projection(), m_near(near), m_far(far), m_top(top), m_bottom(bottom), m_left(left), m_right(right) {/*...*/}
            constexpr virtual ~FrustumProjection() override {/*...*/}

            constexpr virtual void CalculateProjectionMatrix(MatrixRowMajor4x4f *out_proj_matrix) const override {

                const float reciprocal_a = 1.0f / (m_right + m_left);
                const float reciprocal_b = 1.0f / (m_top - m_bottom);
                const float reciprocal_c = 1.0f / (m_far - m_near);

                const v4f row0 = { (m_near + m_near) *  reciprocal_a, 0.0f,                            (m_left + m_right) * reciprocal_a,  0.0f, };
                const v4f row1 = { 0.0f,                              (m_near + m_near) * reciprocal_b, (m_top + m_bottom) * reciprocal_b, 0.0f, };
                const v4f row2 = { 0.0f,                              0.0f,                            -((m_near + m_far) * reciprocal_c), -((m_far + m_far) * m_near * reciprocal_c), };
                const v4f row3 = { 0.0f,                              0.0f,                            -1.0f,                              0.0f };

                StoreRow(out_proj_matrix, row0, 0);
                StoreRow(out_proj_matrix, row1, 1);
                StoreRow(out_proj_matrix, row2, 2);
                StoreRow(out_proj_matrix, row3, 3);
            }
    };

    class PerspectiveProjection : public FrustumProjection {
        private:
            float m_fov_x;
        public:
            VP_RTTI_DERIVED(PerspectiveProjection, FrustumProjection);
        public:
            constexpr PerspectiveProjection() : FrustumProjection(1.0f, 10000.0f, TRadians<float, 45.0f>, vp::util::Sin(TRadians<float, 45.0f> / 2), vp::util::Cos(TRadians<float, 45.0f> / 2), vp::util::Tan(TRadians<float, 45.0f> / 2)), m_fov_x(4.0 / 3.0f) {/*...*/}
            constexpr PerspectiveProjection(float near, float far, float fovy, float aspect) : FrustumProjection(near, far, fovy, vp::util::Sin(fovy / 2), vp::util::Cos(fovy / 2), vp::util::Tan(fovy / 2)), m_fov_x(aspect) {/*...*/}
            constexpr virtual ~PerspectiveProjection() override {/*...*/}

            constexpr virtual void CalculateProjectionMatrix(MatrixRowMajor4x4f *out_proj_matrix) const override {

                const float reciprocal_c = 1.0f / (m_far - m_near);
                const v4f row0 = { 1.0f / (m_right * m_fov_x), 0.0f,           0.0f,                               0.0f, };
                const v4f row1 = { 0.0f,                       1.0f / m_right, 0.0f,                               0.0f, };
                const v4f row2 = { 0.0f,                       0.0f,           -((m_near + m_far) * reciprocal_c), -((m_far + m_far) * m_near * reciprocal_c), };
                const v4f row3 = { 0.0f,                       0.0f,           -1.0f,                              0.0f };

                StoreRow(out_proj_matrix, row0, 0);
                StoreRow(out_proj_matrix, row1, 1);
                StoreRow(out_proj_matrix, row2, 2);
                StoreRow(out_proj_matrix, row3, 3);
            }

            void SetAspect(float new_aspect) {
                m_fov_x = new_aspect;
                m_is_require_update = true;
            }

            void SetFovX(float new_fov_x) {
                m_fov_x = ::tanf(new_fov_x * 0.5f) / m_right;
                m_is_require_update = true;
            }
    };
}
